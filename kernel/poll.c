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

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>
#অন্তর্ভাব <zephyr/sys/util.h>
#অন্তর্ভাব <zephyr/sys/__assert.h>
#অন্তর্ভাব <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
স্থির সংরচনা k_spinlock poll_lock;

গণনা POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

স্থির পূর্ণাঙ্ক signal_poller(সংরচনা k_poll_event *event, uint32_t state);
স্থির পূর্ণাঙ্ক signal_triggered_work(সংরচনা k_poll_event *event, uint32_t status);

শূন্য k_poll_event_init(সংরচনা k_poll_event *event, uint32_t type,
		       পূর্ণাঙ্ক mode, শূন্য *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_অঙ্ক_TYPES)), "invalid type\n");
	__ASSERT(obj != রিক্ত, "must provide an object\n");

	event->poller = রিক্ত;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
স্থির অন্তঃস্থ bool is_condition_met(সংরচনা k_poll_event *event, uint32_t *state)
{
	বিকল্প (event->type) {
	পক্ষ K_POLL_TYPE_SEM_AVAILABLE:
		যদি (k_sem_গুনতি_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			প্রত্যাগম true;
		}
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_DATA_AVAILABLE:
		যদি (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			প্রত্যাগম true;
		}
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_SIGNAL:
		যদি (event->সঙ্কেত->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			প্রত্যাগম true;
		}
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		যদি (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			প্রত্যাগম true;
		}
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		যদি (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			প্রত্যাগম true;
		}
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_IGNORE:
		ভঙ্গ;
	স্বভাব:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		ভঙ্গ;
	}

	প্রত্যাগম false;
}

স্থির সংরচনা k_thread *poller_thread(সংরচনা z_poller *p)
{
	প্রত্যাগম p ? CONTAINER_OF(p, সংরচনা k_thread, poller) : রিক্ত;
}

স্থির অন্তঃস্থ শূন্য add_event(sys_dlist_t *events, সংরচনা k_poll_event *event,
			     সংরচনা z_poller *poller)
{
	সংরচনা k_poll_event *pending;

	pending = (সংরচনা k_poll_event *)sys_dlist_peek_tail(events);
	যদি ((pending == রিক্ত) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->গ্রন্থি_2);
		প্রত্যাগম;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, গ্রন্থি_2) {
		যদি (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_ঢোকাও(&pending->গ্রন্থি_2, &event->গ্রন্থি_2);
			প্রত্যাগম;
		}
	}

	sys_dlist_append(events, &event->গ্রন্থি_2);
}

/* must be called with interrupts locked */
স্থির অন্তঃস্থ শূন্য register_event(সংরচনা k_poll_event *event,
				 সংরচনা z_poller *poller)
{
	বিকল্প (event->type) {
	পক্ষ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != রিক্ত, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != রিক্ত, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->সঙ্কেত != রিক্ত, "invalid poll signal\n");
		add_event(&event->সঙ্কেত->poll_events, event, poller);
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != রিক্ত, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != রিক্ত, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ভঙ্গ;
	স্বভাব:
		__ASSERT(false, "invalid event type\n");
		ভঙ্গ;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
স্থির অন্তঃস্থ শূন্য clear_event_registration(সংরচনা k_poll_event *event)
{
	bool remove_event = false;

	event->poller = রিক্ত;

	বিকল্প (event->type) {
	পক্ষ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != রিক্ত, "invalid semaphore\n");
		remove_event = true;
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != রিক্ত, "invalid queue\n");
		remove_event = true;
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->সঙ্কেত != রিক্ত, "invalid poll signal\n");
		remove_event = true;
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != রিক্ত, "invalid message queue\n");
		remove_event = true;
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != রিক্ত, "invalid pipe\n");
		remove_event = true;
		ভঙ্গ;
	পক্ষ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ভঙ্গ;
	স্বভাব:
		__ASSERT(false, "invalid event type\n");
		ভঙ্গ;
	}
	যদি (remove_event && sys_dnode_is_linked(&event->গ্রন্থি_2)) {
		sys_dlist_remove(&event->গ্রন্থি_2);
	}
}

/* must be called with interrupts locked */
স্থির অন্তঃস্থ শূন্য clear_event_registrations(সংরচনা k_poll_event *events,
					      পূর্ণাঙ্ক অঙ্ক_events,
					      k_spinlock_চাবি_t চাবি)
{
	যাবত্ (অঙ্ক_events--) {
		clear_event_registration(&events[অঙ্ক_events]);
		k_spin_unlock(&poll_lock, চাবি);
		চাবি = k_spin_lock(&poll_lock);
	}
}

স্থির অন্তঃস্থ শূন্য set_event_ready(সংরচনা k_poll_event *event, uint32_t state)
{
	event->poller = রিক্ত;
	event->state |= state;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক register_events(সংরচনা k_poll_event *events,
				  পূর্ণাঙ্ক অঙ্ক_events,
				  সংরচনা z_poller *poller,
				  bool just_check)
{
	পূর্ণাঙ্ক events_registered = 0;

	প্রতি (পূর্ণাঙ্ক ii = 0; ii < অঙ্ক_events; ii++) {
		k_spinlock_চাবি_t চাবি;
		uint32_t state;

		চাবি = k_spin_lock(&poll_lock);
		যদি (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} অন্যথা যদি (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} অন্যথা {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, চাবি);
	}

	প্রত্যাগম events_registered;
}

স্থির পূর্ণাঙ্ক signal_poller(সংরচনা k_poll_event *event, uint32_t state)
{
	সংরচনা k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != রিক্ত, "poller should have a thread\n");

	যদি (!z_is_thread_pending(thread)) {
		প্রত্যাগম 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_মান_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	যদি (!z_is_thread_ready(thread)) {
		প্রত্যাগম 0;
	}

	z_ready_thread(thread);

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক z_impl_k_poll(সংরচনা k_poll_event *events, পূর্ণাঙ্ক অঙ্ক_events,
		  k_timeout_t timeout)
{
	পূর্ণাঙ্ক events_registered;
	k_spinlock_চাবি_t চাবি;
	সংরচনা z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != রিক্ত, "NULL events\n");
	__ASSERT(অঙ্ক_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, অঙ্ক_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	চাবি = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	যদি (!poller->is_polling) {
		clear_event_registrations(events, events_registered, চাবি);
		k_spin_unlock(&poll_lock, চাবি);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		প্রত্যাগম 0;
	}

	poller->is_polling = false;

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, চাবি);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		প্রত্যাগম -EAGAIN;
	}

	স্থির _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	পূর্ণাঙ্ক swap_rc = z_pend_curr(&poll_lock, চাবি, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	চাবি = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, চাবি);
	k_spin_unlock(&poll_lock, চাবি);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	প্রত্যাগম swap_rc;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_poll(সংরচনা k_poll_event *events,
				পূর্ণাঙ্ক অঙ্ক_events, k_timeout_t timeout)
{
	পূর্ণাঙ্ক ret;
	k_spinlock_চাবি_t চাবি;
	সংরচনা k_poll_event *events_copy = রিক্ত;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	যদি (K_SYSCALL_VERIFY(অঙ্ক_events >= 0)) {
		ret = -EINVAL;
		গচ্ছ out;
	}
	যদি (K_SYSCALL_VERIFY_বার্তা(!u32_mul_overflow(অঙ্ক_events,
						   পরিমাণ(সংরচনা k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		গচ্ছ out;
	}
	events_copy = z_thread_malloc(bounds);
	যদি (!events_copy) {
		ret = -ENOMEM;
		গচ্ছ out;
	}

	যদি (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		গচ্ছ oops_free;
	}

	চাবি = k_spin_lock(&poll_lock);
	(শূন্য)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, চাবি);

	/* Validate what's inside events_copy */
	প্রতি (পূর্ণাঙ্ক i = 0; i < অঙ্ক_events; i++) {
		সংরচনা k_poll_event *e = &events_copy[i];

		যদি (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			গচ্ছ out_free;
		}

		বিকল্প (e->type) {
		পক্ষ K_POLL_TYPE_IGNORE:
			ভঙ্গ;
		পক্ষ K_POLL_TYPE_SIGNAL:
			যদি (K_SYSCALL_OBJ(e->সঙ্কেত, K_OBJ_POLL_SIGNAL)) {
				গচ্ছ oops_free;
			}
			ভঙ্গ;
		পক্ষ K_POLL_TYPE_SEM_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				গচ্ছ oops_free;
			}
			ভঙ্গ;
		পক্ষ K_POLL_TYPE_DATA_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				গচ্ছ oops_free;
			}
			ভঙ্গ;
		পক্ষ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				গচ্ছ oops_free;
			}
			ভঙ্গ;
		পক্ষ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				গচ্ছ oops_free;
			}
			ভঙ্গ;
		স্বভাব:
			ret = -EINVAL;
			গচ্ছ out_free;
		}
	}

	ret = k_poll(events_copy, অঙ্ক_events, timeout);
	(শূন্য)memcpy((শূন্য *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	প্রত্যাগম ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
স্থির পূর্ণাঙ্ক signal_poll_event(সংরচনা k_poll_event *event, uint32_t state)
{
	সংরচনা z_poller *poller = event->poller;
	পূর্ণাঙ্ক retcode = 0;

	যদি (poller != রিক্ত) {
		যদি (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} অন্যথা যদি (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} অন্যথা {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		যদি (retcode < 0) {
			প্রত্যাগম retcode;
		}
	}

	set_event_ready(event, state);
	প্রত্যাগম retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	সংরচনা k_poll_event *poll_event;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&poll_lock);

	poll_event = (সংরচনা k_poll_event *)sys_dlist_get(events);
	যদি (poll_event != রিক্ত) {
		(শূন্য) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, চাবি);

	প্রত্যাগম (poll_event != রিক্ত);
}

শূন্য z_impl_k_poll_signal_init(সংরচনা k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_poll_signal_init(সংরচনা k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_poll_signal_reset(সংরচনা k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

শূন্য z_impl_k_poll_signal_check(সংরচনা k_poll_signal *sig,
			       unsigned পূর্ণাঙ্ক *signaled, পূর্ণাঙ্ক *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
শূন্য z_vrfy_k_poll_signal_check(সংরচনা k_poll_signal *sig,
			       unsigned পূর্ণাঙ্ক *signaled, পূর্ণাঙ্ক *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, পরিমাণ(unsigned পূর্ণাঙ্ক)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, পরিমাণ(পূর্ণাঙ্ক)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_poll_signal_raise(সংরচনা k_poll_signal *sig, পূর্ণাঙ্ক result)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&poll_lock);
	সংরচনা k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (সংরচনা k_poll_event *)sys_dlist_get(&sig->poll_events);
	যদি (poll_event == রিক্ত) {
		k_spin_unlock(&poll_lock, চাবি);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		প্রত্যাগম 0;
	}

	পূর্ণাঙ্ক rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, চাবি);
	প্রত্যাগম rc;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_poll_signal_raise(সংরচনা k_poll_signal *sig,
					     পূর্ণাঙ্ক result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	প্রত্যাগম z_impl_k_poll_signal_raise(sig, result);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

স্থির অন্তঃস্থ শূন্য z_vrfy_k_poll_signal_reset(সংরচনা k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

স্থির শূন্য triggered_work_handler(সংরচনা k_work *work)
{
	সংরচনা k_work_poll *twork =
			CONTAINER_OF(work, সংরচনা k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	যদি (twork->poller.mode != MODE_NONE) {
		k_spinlock_চাবি_t চাবি;

		চাবি = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->অঙ্ক_events, চাবি);
		k_spin_unlock(&poll_lock, চাবি);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = রিক্ত;
	twork->real_handler(work);
}

বাহ্য পূর্ণাঙ্ক z_work_submit_to_queue(সংরচনা k_work_q *queue,
			 সংরচনা k_work *work);

স্থির শূন্য triggered_work_expiration_handler(সংরচনা _timeout *timeout)
{
	সংরচনা k_work_poll *twork =
		CONTAINER_OF(timeout, সংরচনা k_work_poll, timeout);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&poll_lock);

	যদি (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, চাবি);
		প্রত্যাগম;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, চাবি);
}

স্থির পূর্ণাঙ্ক signal_triggered_work(সংরচনা k_poll_event *event, uint32_t status)
{
	সংরচনা z_poller *poller = event->poller;
	সংরচনা k_work_poll *twork =
		CONTAINER_OF(poller, সংরচনা k_work_poll, poller);

	যদি (poller->is_polling && twork->workq != রিক্ত) {
		সংরচনা k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(শূন্য)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক triggered_work_cancel(সংরচনা k_work_poll *work,
				 k_spinlock_চাবি_t চাবি)
{
	/* Check if the work waits for event. */
	যদি (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		যাবত্ (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, চাবি);
			চাবি = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->অঙ্ক_events, চাবি);
		work->workq = রিক্ত;
		প্রত্যাগম 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	প্রত্যাগম -EINVAL;
}

শূন্য k_work_poll_init(সংরচনা k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (সংরচনা k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

পূর্ণাঙ্ক k_work_poll_submit_to_queue(সংরচনা k_work_q *work_q,
				সংরচনা k_work_poll *work,
				সংরচনা k_poll_event *events,
				পূর্ণাঙ্ক অঙ্ক_events,
				k_timeout_t timeout)
{
	পূর্ণাঙ্ক events_registered;
	k_spinlock_চাবি_t চাবি;

	__ASSERT(work_q != রিক্ত, "NULL work_q\n");
	__ASSERT(work != রিক্ত, "NULL work\n");
	__ASSERT(events != রিক্ত, "NULL events\n");
	__ASSERT(অঙ্ক_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	চাবি = k_spin_lock(&poll_lock);
	যদি (work->workq != রিক্ত) {
		যদি (work->workq == work_q) {
			পূর্ণাঙ্ক retval;

			retval = triggered_work_cancel(work, চাবি);
			যদি (retval < 0) {
				k_spin_unlock(&poll_lock, চাবি);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				প্রত্যাগম retval;
			}
		} অন্যথা {
			k_spin_unlock(&poll_lock, চাবি);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			প্রত্যাগম -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, চাবি);

	/* Save list of events. */
	work->events = events;
	work->অঙ্ক_events = অঙ্ক_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, অঙ্ক_events,
					    &work->poller, false);

	চাবি = k_spin_lock(&poll_lock);
	যদি (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(অঙ্ক_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, চাবি);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		প্রত্যাগম 0;
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
	যদি (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} অন্যথা {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, চাবি);
	k_spin_unlock(&poll_lock, চাবি);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক k_work_poll_submit(সংরচনা k_work_poll *work,
				     সংরচনা k_poll_event *events,
				     পূর্ণাঙ্ক অঙ্ক_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	পূর্ণাঙ্ক ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, অঙ্ক_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	প্রত্যাগম ret;
}

পূর্ণাঙ্ক k_work_poll_cancel(সংরচনা k_work_poll *work)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	যদি (work == রিক্ত || work->workq == রিক্ত) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		প্রত্যাগম -EINVAL;
	}

	চাবি = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, চাবি);
	k_spin_unlock(&poll_lock, চাবি);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	প্রত্যাগম retval;
}
