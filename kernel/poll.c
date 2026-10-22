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

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>
#அந்தர்பாவ <zephyr/sys/util.h>
#அந்தர்பாவ <zephyr/sys/__assert.h>
#அந்தர்பாவ <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
நிலையான கட்டமைப்பு k_spinlock poll_lock;

எண்ணிக்கை POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

நிலையான முழு signal_poller(கட்டமைப்பு k_poll_event *event, uint32_t state);
நிலையான முழு signal_triggered_work(கட்டமைப்பு k_poll_event *event, uint32_t status);

வெறுமை k_poll_event_init(கட்டமைப்பு k_poll_event *event, uint32_t type,
		       முழு mode, வெறுமை *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_இலக்கம்_TYPES)), "invalid type\n");
	__ASSERT(obj != வெற்று, "must provide an object\n");

	event->poller = வெற்று;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
நிலையான உள்வரி bool is_condition_met(கட்டமைப்பு k_poll_event *event, uint32_t *state)
{
	தேர்வு (event->type) {
	நிலை K_POLL_TYPE_SEM_AVAILABLE:
		எனில் (k_sem_மொத்தம்_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			திரும்பு true;
		}
		நிறுத்து;
	நிலை K_POLL_TYPE_DATA_AVAILABLE:
		எனில் (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			திரும்பு true;
		}
		நிறுத்து;
	நிலை K_POLL_TYPE_SIGNAL:
		எனில் (event->சமிக்ஞை->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			திரும்பு true;
		}
		நிறுத்து;
	நிலை K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		எனில் (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			திரும்பு true;
		}
		நிறுத்து;
	நிலை K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		எனில் (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			திரும்பு true;
		}
		நிறுத்து;
	நிலை K_POLL_TYPE_IGNORE:
		நிறுத்து;
	இயல்பு:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		நிறுத்து;
	}

	திரும்பு false;
}

நிலையான கட்டமைப்பு k_thread *poller_thread(கட்டமைப்பு z_poller *p)
{
	திரும்பு p ? CONTAINER_OF(p, கட்டமைப்பு k_thread, poller) : வெற்று;
}

நிலையான உள்வரி வெறுமை add_event(sys_dlist_t *events, கட்டமைப்பு k_poll_event *event,
			     கட்டமைப்பு z_poller *poller)
{
	கட்டமைப்பு k_poll_event *pending;

	pending = (கட்டமைப்பு k_poll_event *)sys_dlist_peek_tail(events);
	எனில் ((pending == வெற்று) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->முனை_2);
		திரும்பு;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, முனை_2) {
		எனில் (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_செருகு(&pending->முனை_2, &event->முனை_2);
			திரும்பு;
		}
	}

	sys_dlist_append(events, &event->முனை_2);
}

/* must be called with interrupts locked */
நிலையான உள்வரி வெறுமை register_event(கட்டமைப்பு k_poll_event *event,
				 கட்டமைப்பு z_poller *poller)
{
	தேர்வு (event->type) {
	நிலை K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != வெற்று, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		நிறுத்து;
	நிலை K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != வெற்று, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		நிறுத்து;
	நிலை K_POLL_TYPE_SIGNAL:
		__ASSERT(event->சமிக்ஞை != வெற்று, "invalid poll signal\n");
		add_event(&event->சமிக்ஞை->poll_events, event, poller);
		நிறுத்து;
	நிலை K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != வெற்று, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		நிறுத்து;
	நிலை K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != வெற்று, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		நிறுத்து;
	நிலை K_POLL_TYPE_IGNORE:
		/* nothing to do */
		நிறுத்து;
	இயல்பு:
		__ASSERT(false, "invalid event type\n");
		நிறுத்து;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
நிலையான உள்வரி வெறுமை clear_event_registration(கட்டமைப்பு k_poll_event *event)
{
	bool remove_event = false;

	event->poller = வெற்று;

	தேர்வு (event->type) {
	நிலை K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != வெற்று, "invalid semaphore\n");
		remove_event = true;
		நிறுத்து;
	நிலை K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != வெற்று, "invalid queue\n");
		remove_event = true;
		நிறுத்து;
	நிலை K_POLL_TYPE_SIGNAL:
		__ASSERT(event->சமிக்ஞை != வெற்று, "invalid poll signal\n");
		remove_event = true;
		நிறுத்து;
	நிலை K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != வெற்று, "invalid message queue\n");
		remove_event = true;
		நிறுத்து;
	நிலை K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != வெற்று, "invalid pipe\n");
		remove_event = true;
		நிறுத்து;
	நிலை K_POLL_TYPE_IGNORE:
		/* nothing to do */
		நிறுத்து;
	இயல்பு:
		__ASSERT(false, "invalid event type\n");
		நிறுத்து;
	}
	எனில் (remove_event && sys_dnode_is_linked(&event->முனை_2)) {
		sys_dlist_remove(&event->முனை_2);
	}
}

/* must be called with interrupts locked */
நிலையான உள்வரி வெறுமை clear_event_registrations(கட்டமைப்பு k_poll_event *events,
					      முழு இலக்கம்_events,
					      k_spinlock_சாவி_t சாவி)
{
	போது (இலக்கம்_events--) {
		clear_event_registration(&events[இலக்கம்_events]);
		k_spin_unlock(&poll_lock, சாவி);
		சாவி = k_spin_lock(&poll_lock);
	}
}

நிலையான உள்வரி வெறுமை set_event_ready(கட்டமைப்பு k_poll_event *event, uint32_t state)
{
	event->poller = வெற்று;
	event->state |= state;
}

நிலையான உள்வரி முழு register_events(கட்டமைப்பு k_poll_event *events,
				  முழு இலக்கம்_events,
				  கட்டமைப்பு z_poller *poller,
				  bool just_check)
{
	முழு events_registered = 0;

	க்கு (முழு ii = 0; ii < இலக்கம்_events; ii++) {
		k_spinlock_சாவி_t சாவி;
		uint32_t state;

		சாவி = k_spin_lock(&poll_lock);
		எனில் (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} இல்லையெனில் எனில் (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} இல்லையெனில் {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, சாவி);
	}

	திரும்பு events_registered;
}

நிலையான முழு signal_poller(கட்டமைப்பு k_poll_event *event, uint32_t state)
{
	கட்டமைப்பு k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != வெற்று, "poller should have a thread\n");

	எனில் (!z_is_thread_pending(thread)) {
		திரும்பு 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_மதிப்பு_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	எனில் (!z_is_thread_ready(thread)) {
		திரும்பு 0;
	}

	z_ready_thread(thread);

	திரும்பு 0;
}

முழு z_impl_k_poll(கட்டமைப்பு k_poll_event *events, முழு இலக்கம்_events,
		  k_timeout_t timeout)
{
	முழு events_registered;
	k_spinlock_சாவி_t சாவி;
	கட்டமைப்பு z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != வெற்று, "NULL events\n");
	__ASSERT(இலக்கம்_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, இலக்கம்_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	சாவி = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	எனில் (!poller->is_polling) {
		clear_event_registrations(events, events_registered, சாவி);
		k_spin_unlock(&poll_lock, சாவி);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		திரும்பு 0;
	}

	poller->is_polling = false;

	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, சாவி);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		திரும்பு -EAGAIN;
	}

	நிலையான _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	முழு swap_rc = z_pend_curr(&poll_lock, சாவி, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	சாவி = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, சாவி);
	k_spin_unlock(&poll_lock, சாவி);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	திரும்பு swap_rc;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_poll(கட்டமைப்பு k_poll_event *events,
				முழு இலக்கம்_events, k_timeout_t timeout)
{
	முழு ret;
	k_spinlock_சாவி_t சாவி;
	கட்டமைப்பு k_poll_event *events_copy = வெற்று;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	எனில் (K_SYSCALL_VERIFY(இலக்கம்_events >= 0)) {
		ret = -EINVAL;
		செல் out;
	}
	எனில் (K_SYSCALL_VERIFY_செய்தி(!u32_mul_overflow(இலக்கம்_events,
						   அளவு(கட்டமைப்பு k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		செல் out;
	}
	events_copy = z_thread_malloc(bounds);
	எனில் (!events_copy) {
		ret = -ENOMEM;
		செல் out;
	}

	எனில் (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		செல் oops_free;
	}

	சாவி = k_spin_lock(&poll_lock);
	(வெறுமை)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, சாவி);

	/* Validate what's inside events_copy */
	க்கு (முழு i = 0; i < இலக்கம்_events; i++) {
		கட்டமைப்பு k_poll_event *e = &events_copy[i];

		எனில் (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			செல் out_free;
		}

		தேர்வு (e->type) {
		நிலை K_POLL_TYPE_IGNORE:
			நிறுத்து;
		நிலை K_POLL_TYPE_SIGNAL:
			எனில் (K_SYSCALL_OBJ(e->சமிக்ஞை, K_OBJ_POLL_SIGNAL)) {
				செல் oops_free;
			}
			நிறுத்து;
		நிலை K_POLL_TYPE_SEM_AVAILABLE:
			எனில் (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				செல் oops_free;
			}
			நிறுத்து;
		நிலை K_POLL_TYPE_DATA_AVAILABLE:
			எனில் (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				செல் oops_free;
			}
			நிறுத்து;
		நிலை K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			எனில் (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				செல் oops_free;
			}
			நிறுத்து;
		நிலை K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			எனில் (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				செல் oops_free;
			}
			நிறுத்து;
		இயல்பு:
			ret = -EINVAL;
			செல் out_free;
		}
	}

	ret = k_poll(events_copy, இலக்கம்_events, timeout);
	(வெறுமை)memcpy((வெறுமை *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	திரும்பு ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#அந்தர்பாவ <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
நிலையான முழு signal_poll_event(கட்டமைப்பு k_poll_event *event, uint32_t state)
{
	கட்டமைப்பு z_poller *poller = event->poller;
	முழு retcode = 0;

	எனில் (poller != வெற்று) {
		எனில் (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} இல்லையெனில் எனில் (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} இல்லையெனில் {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		எனில் (retcode < 0) {
			திரும்பு retcode;
		}
	}

	set_event_ready(event, state);
	திரும்பு retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	கட்டமைப்பு k_poll_event *poll_event;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&poll_lock);

	poll_event = (கட்டமைப்பு k_poll_event *)sys_dlist_get(events);
	எனில் (poll_event != வெற்று) {
		(வெறுமை) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, சாவி);

	திரும்பு (poll_event != வெற்று);
}

வெறுமை z_impl_k_poll_signal_init(கட்டமைப்பு k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_poll_signal_init(கட்டமைப்பு k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#அந்தர்பாவ <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_impl_k_poll_signal_reset(கட்டமைப்பு k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

வெறுமை z_impl_k_poll_signal_check(கட்டமைப்பு k_poll_signal *sig,
			       unsigned முழு *signaled, முழு *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
வெறுமை z_vrfy_k_poll_signal_check(கட்டமைப்பு k_poll_signal *sig,
			       unsigned முழு *signaled, முழு *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, அளவு(unsigned முழு)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, அளவு(முழு)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#அந்தர்பாவ <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_poll_signal_raise(கட்டமைப்பு k_poll_signal *sig, முழு result)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&poll_lock);
	கட்டமைப்பு k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (கட்டமைப்பு k_poll_event *)sys_dlist_get(&sig->poll_events);
	எனில் (poll_event == வெற்று) {
		k_spin_unlock(&poll_lock, சாவி);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		திரும்பு 0;
	}

	முழு rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, சாவி);
	திரும்பு rc;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_poll_signal_raise(கட்டமைப்பு k_poll_signal *sig,
					     முழு result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	திரும்பு z_impl_k_poll_signal_raise(sig, result);
}
#அந்தர்பாவ <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

நிலையான உள்வரி வெறுமை z_vrfy_k_poll_signal_reset(கட்டமைப்பு k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#அந்தர்பாவ <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

நிலையான வெறுமை triggered_work_handler(கட்டமைப்பு k_work *work)
{
	கட்டமைப்பு k_work_poll *twork =
			CONTAINER_OF(work, கட்டமைப்பு k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	எனில் (twork->poller.mode != MODE_NONE) {
		k_spinlock_சாவி_t சாவி;

		சாவி = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->இலக்கம்_events, சாவி);
		k_spin_unlock(&poll_lock, சாவி);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = வெற்று;
	twork->real_handler(work);
}

வெளி முழு z_work_submit_to_queue(கட்டமைப்பு k_work_q *queue,
			 கட்டமைப்பு k_work *work);

நிலையான வெறுமை triggered_work_expiration_handler(கட்டமைப்பு _timeout *timeout)
{
	கட்டமைப்பு k_work_poll *twork =
		CONTAINER_OF(timeout, கட்டமைப்பு k_work_poll, timeout);
	k_spinlock_சாவி_t சாவி = k_spin_lock(&poll_lock);

	எனில் (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, சாவி);
		திரும்பு;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, சாவி);
}

நிலையான முழு signal_triggered_work(கட்டமைப்பு k_poll_event *event, uint32_t status)
{
	கட்டமைப்பு z_poller *poller = event->poller;
	கட்டமைப்பு k_work_poll *twork =
		CONTAINER_OF(poller, கட்டமைப்பு k_work_poll, poller);

	எனில் (poller->is_polling && twork->workq != வெற்று) {
		கட்டமைப்பு k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(வெறுமை)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	திரும்பு 0;
}

நிலையான முழு triggered_work_cancel(கட்டமைப்பு k_work_poll *work,
				 k_spinlock_சாவி_t சாவி)
{
	/* Check if the work waits for event. */
	எனில் (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		போது (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, சாவி);
			சாவி = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->இலக்கம்_events, சாவி);
		work->workq = வெற்று;
		திரும்பு 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	திரும்பு -EINVAL;
}

வெறுமை k_work_poll_init(கட்டமைப்பு k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (கட்டமைப்பு k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

முழு k_work_poll_submit_to_queue(கட்டமைப்பு k_work_q *work_q,
				கட்டமைப்பு k_work_poll *work,
				கட்டமைப்பு k_poll_event *events,
				முழு இலக்கம்_events,
				k_timeout_t timeout)
{
	முழு events_registered;
	k_spinlock_சாவி_t சாவி;

	__ASSERT(work_q != வெற்று, "NULL work_q\n");
	__ASSERT(work != வெற்று, "NULL work\n");
	__ASSERT(events != வெற்று, "NULL events\n");
	__ASSERT(இலக்கம்_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	சாவி = k_spin_lock(&poll_lock);
	எனில் (work->workq != வெற்று) {
		எனில் (work->workq == work_q) {
			முழு retval;

			retval = triggered_work_cancel(work, சாவி);
			எனில் (retval < 0) {
				k_spin_unlock(&poll_lock, சாவி);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				திரும்பு retval;
			}
		} இல்லையெனில் {
			k_spin_unlock(&poll_lock, சாவி);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			திரும்பு -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, சாவி);

	/* Save list of events. */
	work->events = events;
	work->இலக்கம்_events = இலக்கம்_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, இலக்கம்_events,
					    &work->poller, false);

	சாவி = k_spin_lock(&poll_lock);
	எனில் (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(இலக்கம்_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, சாவி);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		திரும்பு 0;
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
	எனில் (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} இல்லையெனில் {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, சாவி);
	k_spin_unlock(&poll_lock, சாவி);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	திரும்பு 0;
}

முழு k_work_poll_submit(கட்டமைப்பு k_work_poll *work,
				     கட்டமைப்பு k_poll_event *events,
				     முழு இலக்கம்_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	முழு ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, இலக்கம்_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	திரும்பு ret;
}

முழு k_work_poll_cancel(கட்டமைப்பு k_work_poll *work)
{
	k_spinlock_சாவி_t சாவி;
	முழு retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	எனில் (work == வெற்று || work->workq == வெற்று) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		திரும்பு -EINVAL;
	}

	சாவி = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, சாவி);
	k_spin_unlock(&poll_lock, சாவி);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	திரும்பு retval;
}
