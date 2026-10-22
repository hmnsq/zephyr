/*
 * Copyright (c) 2017 Wind River Systems, Inc.
 * Copyright (c) 2023 Arm Limited (or its affiliates). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief Kernel asynchronous event polling interface.
 *
 * This polling mechanism allows waiting on multiple events concurrently,
 * either events triggered directly, or from kernel objects or other kernel
 * constructs.
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/sys/dlist.h>
#अन्तर्भाव <zephyr/sys/util.h>
#अन्तर्भाव <zephyr/sys/__assert.h>
#अन्तर्भाव <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
स्थिर संरचना k_spinlock poll_lock;

गणना POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

स्थिर पूर्णाङ्क signal_poller(संरचना k_poll_event *event, uint32_t state);
स्थिर पूर्णाङ्क signal_triggered_work(संरचना k_poll_event *event, uint32_t status);

शून्य k_poll_event_init(संरचना k_poll_event *event, uint32_t type,
		       पूर्णाङ्क mode, शून्य *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_अंक_TYPES)), "invalid type\n");
	__ASSERT(obj != रिक्त, "must provide an object\n");

	event->poller = रिक्त;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
स्थिर अन्तःस्थ bool is_condition_met(संरचना k_poll_event *event, uint32_t *state)
{
	विकल्प (event->type) {
	पक्ष K_POLL_TYPE_SEM_AVAILABLE:
		यदि (k_sem_गिनती_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			प्रत्यागम true;
		}
		भङ्ग;
	पक्ष K_POLL_TYPE_DATA_AVAILABLE:
		यदि (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			प्रत्यागम true;
		}
		भङ्ग;
	पक्ष K_POLL_TYPE_SIGNAL:
		यदि (event->सङ्केत->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			प्रत्यागम true;
		}
		भङ्ग;
	पक्ष K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		यदि (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			प्रत्यागम true;
		}
		भङ्ग;
	पक्ष K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		यदि (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			प्रत्यागम true;
		}
		भङ्ग;
	पक्ष K_POLL_TYPE_IGNORE:
		भङ्ग;
	स्वभाव:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		भङ्ग;
	}

	प्रत्यागम false;
}

स्थिर संरचना k_thread *poller_thread(संरचना z_poller *p)
{
	प्रत्यागम p ? CONTAINER_OF(p, संरचना k_thread, poller) : रिक्त;
}

स्थिर अन्तःस्थ शून्य add_event(sys_dlist_t *events, संरचना k_poll_event *event,
			     संरचना z_poller *poller)
{
	संरचना k_poll_event *pending;

	pending = (संरचना k_poll_event *)sys_dlist_peek_tail(events);
	यदि ((pending == रिक्त) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->गाँठ_2);
		प्रत्यागम;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, गाँठ_2) {
		यदि (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_जोड़ो(&pending->गाँठ_2, &event->गाँठ_2);
			प्रत्यागम;
		}
	}

	sys_dlist_append(events, &event->गाँठ_2);
}

/* must be called with interrupts locked */
स्थिर अन्तःस्थ शून्य register_event(संरचना k_poll_event *event,
				 संरचना z_poller *poller)
{
	विकल्प (event->type) {
	पक्ष K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != रिक्त, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		भङ्ग;
	पक्ष K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != रिक्त, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		भङ्ग;
	पक्ष K_POLL_TYPE_SIGNAL:
		__ASSERT(event->सङ्केत != रिक्त, "invalid poll signal\n");
		add_event(&event->सङ्केत->poll_events, event, poller);
		भङ्ग;
	पक्ष K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != रिक्त, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		भङ्ग;
	पक्ष K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != रिक्त, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		भङ्ग;
	पक्ष K_POLL_TYPE_IGNORE:
		/* nothing to do */
		भङ्ग;
	स्वभाव:
		__ASSERT(false, "invalid event type\n");
		भङ्ग;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
स्थिर अन्तःस्थ शून्य clear_event_registration(संरचना k_poll_event *event)
{
	bool remove_event = false;

	event->poller = रिक्त;

	विकल्प (event->type) {
	पक्ष K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != रिक्त, "invalid semaphore\n");
		remove_event = true;
		भङ्ग;
	पक्ष K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != रिक्त, "invalid queue\n");
		remove_event = true;
		भङ्ग;
	पक्ष K_POLL_TYPE_SIGNAL:
		__ASSERT(event->सङ्केत != रिक्त, "invalid poll signal\n");
		remove_event = true;
		भङ्ग;
	पक्ष K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != रिक्त, "invalid message queue\n");
		remove_event = true;
		भङ्ग;
	पक्ष K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != रिक्त, "invalid pipe\n");
		remove_event = true;
		भङ्ग;
	पक्ष K_POLL_TYPE_IGNORE:
		/* nothing to do */
		भङ्ग;
	स्वभाव:
		__ASSERT(false, "invalid event type\n");
		भङ्ग;
	}
	यदि (remove_event && sys_dnode_is_linked(&event->गाँठ_2)) {
		sys_dlist_remove(&event->गाँठ_2);
	}
}

/* must be called with interrupts locked */
स्थिर अन्तःस्थ शून्य clear_event_registrations(संरचना k_poll_event *events,
					      पूर्णाङ्क अंक_events,
					      k_spinlock_कुंजी_t कुंजी)
{
	यावत् (अंक_events--) {
		clear_event_registration(&events[अंक_events]);
		k_spin_unlock(&poll_lock, कुंजी);
		कुंजी = k_spin_lock(&poll_lock);
	}
}

स्थिर अन्तःस्थ शून्य set_event_ready(संरचना k_poll_event *event, uint32_t state)
{
	event->poller = रिक्त;
	event->state |= state;
}

स्थिर अन्तःस्थ पूर्णाङ्क register_events(संरचना k_poll_event *events,
				  पूर्णाङ्क अंक_events,
				  संरचना z_poller *poller,
				  bool just_check)
{
	पूर्णाङ्क events_registered = 0;

	प्रति (पूर्णाङ्क ii = 0; ii < अंक_events; ii++) {
		k_spinlock_कुंजी_t कुंजी;
		uint32_t state;

		कुंजी = k_spin_lock(&poll_lock);
		यदि (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} अन्यथा यदि (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} अन्यथा {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, कुंजी);
	}

	प्रत्यागम events_registered;
}

स्थिर पूर्णाङ्क signal_poller(संरचना k_poll_event *event, uint32_t state)
{
	संरचना k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != रिक्त, "poller should have a thread\n");

	यदि (!z_is_thread_pending(thread)) {
		प्रत्यागम 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_मूल्य_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	यदि (!z_is_thread_ready(thread)) {
		प्रत्यागम 0;
	}

	z_ready_thread(thread);

	प्रत्यागम 0;
}

पूर्णाङ्क z_impl_k_poll(संरचना k_poll_event *events, पूर्णाङ्क अंक_events,
		  k_timeout_t timeout)
{
	पूर्णाङ्क events_registered;
	k_spinlock_कुंजी_t कुंजी;
	संरचना z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != रिक्त, "NULL events\n");
	__ASSERT(अंक_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, अंक_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	कुंजी = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	यदि (!poller->is_polling) {
		clear_event_registrations(events, events_registered, कुंजी);
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		प्रत्यागम 0;
	}

	poller->is_polling = false;

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		प्रत्यागम -EAGAIN;
	}

	स्थिर _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	पूर्णाङ्क swap_rc = z_pend_curr(&poll_lock, कुंजी, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	कुंजी = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, कुंजी);
	k_spin_unlock(&poll_lock, कुंजी);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	प्रत्यागम swap_rc;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_poll(संरचना k_poll_event *events,
				पूर्णाङ्क अंक_events, k_timeout_t timeout)
{
	पूर्णाङ्क ret;
	k_spinlock_कुंजी_t कुंजी;
	संरचना k_poll_event *events_copy = रिक्त;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	यदि (K_SYSCALL_VERIFY(अंक_events >= 0)) {
		ret = -EINVAL;
		गच्छ out;
	}
	यदि (K_SYSCALL_VERIFY_संदेश(!u32_mul_overflow(अंक_events,
						   परिमाण(संरचना k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		गच्छ out;
	}
	events_copy = z_thread_malloc(bounds);
	यदि (!events_copy) {
		ret = -ENOMEM;
		गच्छ out;
	}

	यदि (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		गच्छ oops_free;
	}

	कुंजी = k_spin_lock(&poll_lock);
	(शून्य)स_नकल(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, कुंजी);

	/* Validate what's inside events_copy */
	प्रति (पूर्णाङ्क i = 0; i < अंक_events; i++) {
		संरचना k_poll_event *e = &events_copy[i];

		यदि (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			गच्छ out_free;
		}

		विकल्प (e->type) {
		पक्ष K_POLL_TYPE_IGNORE:
			भङ्ग;
		पक्ष K_POLL_TYPE_SIGNAL:
			यदि (K_SYSCALL_OBJ(e->सङ्केत, K_OBJ_POLL_SIGNAL)) {
				गच्छ oops_free;
			}
			भङ्ग;
		पक्ष K_POLL_TYPE_SEM_AVAILABLE:
			यदि (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				गच्छ oops_free;
			}
			भङ्ग;
		पक्ष K_POLL_TYPE_DATA_AVAILABLE:
			यदि (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				गच्छ oops_free;
			}
			भङ्ग;
		पक्ष K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			यदि (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				गच्छ oops_free;
			}
			भङ्ग;
		पक्ष K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			यदि (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				गच्छ oops_free;
			}
			भङ्ग;
		स्वभाव:
			ret = -EINVAL;
			गच्छ out_free;
		}
	}

	ret = k_poll(events_copy, अंक_events, timeout);
	(शून्य)स_नकल((शून्य *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	प्रत्यागम ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#अन्तर्भाव <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
स्थिर पूर्णाङ्क signal_poll_event(संरचना k_poll_event *event, uint32_t state)
{
	संरचना z_poller *poller = event->poller;
	पूर्णाङ्क retcode = 0;

	यदि (poller != रिक्त) {
		यदि (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} अन्यथा यदि (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} अन्यथा {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		यदि (retcode < 0) {
			प्रत्यागम retcode;
		}
	}

	set_event_ready(event, state);
	प्रत्यागम retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	संरचना k_poll_event *poll_event;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&poll_lock);

	poll_event = (संरचना k_poll_event *)sys_dlist_get(events);
	यदि (poll_event != रिक्त) {
		(शून्य) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, कुंजी);

	प्रत्यागम (poll_event != रिक्त);
}

शून्य z_impl_k_poll_signal_init(संरचना k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_poll_signal_init(संरचना k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#अन्तर्भाव <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_poll_signal_reset(संरचना k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

शून्य z_impl_k_poll_signal_check(संरचना k_poll_signal *sig,
			       अचिन्हित पूर्णाङ्क *signaled, पूर्णाङ्क *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
शून्य z_vrfy_k_poll_signal_check(संरचना k_poll_signal *sig,
			       अचिन्हित पूर्णाङ्क *signaled, पूर्णाङ्क *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, परिमाण(अचिन्हित पूर्णाङ्क)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, परिमाण(पूर्णाङ्क)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#अन्तर्भाव <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_poll_signal_raise(संरचना k_poll_signal *sig, पूर्णाङ्क result)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&poll_lock);
	संरचना k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (संरचना k_poll_event *)sys_dlist_get(&sig->poll_events);
	यदि (poll_event == रिक्त) {
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		प्रत्यागम 0;
	}

	पूर्णाङ्क rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, कुंजी);
	प्रत्यागम rc;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_poll_signal_raise(संरचना k_poll_signal *sig,
					     पूर्णाङ्क result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	प्रत्यागम z_impl_k_poll_signal_raise(sig, result);
}
#अन्तर्भाव <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_poll_signal_reset(संरचना k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#अन्तर्भाव <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

स्थिर शून्य triggered_work_handler(संरचना k_work *work)
{
	संरचना k_work_poll *twork =
			CONTAINER_OF(work, संरचना k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	यदि (twork->poller.mode != MODE_NONE) {
		k_spinlock_कुंजी_t कुंजी;

		कुंजी = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->अंक_events, कुंजी);
		k_spin_unlock(&poll_lock, कुंजी);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = रिक्त;
	twork->real_handler(work);
}

बाह्य पूर्णाङ्क z_work_submit_to_queue(संरचना k_work_q *queue,
			 संरचना k_work *work);

स्थिर शून्य triggered_work_expiration_handler(संरचना _timeout *timeout)
{
	संरचना k_work_poll *twork =
		CONTAINER_OF(timeout, संरचना k_work_poll, timeout);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&poll_lock);

	यदि (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, कुंजी);
		प्रत्यागम;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, कुंजी);
}

स्थिर पूर्णाङ्क signal_triggered_work(संरचना k_poll_event *event, uint32_t status)
{
	संरचना z_poller *poller = event->poller;
	संरचना k_work_poll *twork =
		CONTAINER_OF(poller, संरचना k_work_poll, poller);

	यदि (poller->is_polling && twork->workq != रिक्त) {
		संरचना k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(शून्य)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क triggered_work_cancel(संरचना k_work_poll *work,
				 k_spinlock_कुंजी_t कुंजी)
{
	/* Check if the work waits for event. */
	यदि (work->poller.is_polling && work->poller.mode != MODE_NONE) {
		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		work->poller.is_polling = false;
		/* Then wait for any in-flight handler to actually complete
		 * before we proceed. This is needed both so the caller may
		 * safely free `work` (the handler still has a pending
		 * dereference of work->poller.is_polling) and so that
		 * k_work_poll_submit_to_queue() may safely re-arm the same
		 * `work` for a new poll without the old handler eventually
		 * acting on the new setup.
		 */
		यावत् (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, कुंजी);
			कुंजी = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->अंक_events, कुंजी);
		work->workq = रिक्त;
		प्रत्यागम 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	प्रत्यागम -EINVAL;
}

शून्य k_work_poll_init(संरचना k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (संरचना k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

पूर्णाङ्क k_work_poll_submit_to_queue(संरचना k_work_q *work_q,
				संरचना k_work_poll *work,
				संरचना k_poll_event *events,
				पूर्णाङ्क अंक_events,
				k_timeout_t timeout)
{
	पूर्णाङ्क events_registered;
	k_spinlock_कुंजी_t कुंजी;

	__ASSERT(work_q != रिक्त, "NULL work_q\n");
	__ASSERT(work != रिक्त, "NULL work\n");
	__ASSERT(events != रिक्त, "NULL events\n");
	__ASSERT(अंक_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	कुंजी = k_spin_lock(&poll_lock);
	यदि (work->workq != रिक्त) {
		यदि (work->workq == work_q) {
			पूर्णाङ्क retval;

			retval = triggered_work_cancel(work, कुंजी);
			यदि (retval < 0) {
				k_spin_unlock(&poll_lock, कुंजी);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				प्रत्यागम retval;
			}
		} अन्यथा {
			k_spin_unlock(&poll_lock, कुंजी);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			प्रत्यागम -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, कुंजी);

	/* Save list of events. */
	work->events = events;
	work->अंक_events = अंक_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, अंक_events,
					    &work->poller, false);

	कुंजी = k_spin_lock(&poll_lock);
	यदि (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(अंक_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		प्रत्यागम 0;
	}

	/*
	 * The K_NO_WAIT timeout was specified or at least one event
	 * was ready at registration time or changed state since
	 * registration. Hopefully, the poller mode was not set, so
	 * work was not submitted to workqueue.
	 */

	/*
	 * If poller is still polling, no watched event occurred. This means
	 * we reached here due to K_NO_WAIT timeout "expiration".
	 */
	यदि (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} अन्यथा {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, कुंजी);
	k_spin_unlock(&poll_lock, कुंजी);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	प्रत्यागम 0;
}

पूर्णाङ्क k_work_poll_submit(संरचना k_work_poll *work,
				     संरचना k_poll_event *events,
				     पूर्णाङ्क अंक_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	पूर्णाङ्क ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, अंक_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	प्रत्यागम ret;
}

पूर्णाङ्क k_work_poll_cancel(संरचना k_work_poll *work)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णाङ्क retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	यदि (work == रिक्त || work->workq == रिक्त) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		प्रत्यागम -EINVAL;
	}

	कुंजी = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, कुंजी);
	k_spin_unlock(&poll_lock, कुंजी);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	प्रत्यागम retval;
}
