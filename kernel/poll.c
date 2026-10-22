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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/util.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/__assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
ਸਥਿਰ ਬਣਤਰ k_spinlock poll_ਤਾਲਾ;

ਗਿਣਤੀ POLL_ਬਹੁਲਕ { ਬਹੁਲਕ_NONE, ਬਹੁਲਕ_POLL, ਬਹੁਲਕ_TRIGGERED };

ਸਥਿਰ ਪੂਰਨ signal_poller(ਬਣਤਰ k_poll_event *event, uint32_t state);
ਸਥਿਰ ਪੂਰਨ signal_triggered_work(ਬਣਤਰ k_poll_event *event, uint32_t ਸਥਿਤੀ);

ਖਾਲੀ k_poll_event_init(ਬਣਤਰ k_poll_event *event, uint32_t type,
		       ਪੂਰਨ ਬਹੁਲਕ, ਖਾਲੀ *obj)
{
	__ASSERT(ਬਹੁਲਕ == K_POLL_ਬਹੁਲਕ_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (ਦੋਆਧਾਰੀ_ਅੰਕ(POLL_ਅੰਕ_TYPES)), "invalid type\n");
	__ASSERT(obj != ਨਿਰਾ, "must provide an object\n");

	event->poller = ਨਿਰਾ;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->ਬਹੁਲਕ = ਬਹੁਲਕ;
	event->ਅਣਵਰਤਿਆ = 0U;
	event->obj = obj;

	SYS_ਪੋਰਟ_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool is_condition_met(ਬਣਤਰ k_poll_event *event, uint32_t *state)
{
	ਚੋਣ (event->type) {
	ਹਾਲਤ K_POLL_TYPE_SEM_ਉਪਲਬਧ:
		ਜੇ (k_sem_ਗਿਣਤੀ_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_ਉਪਲਬਧ;
			ਵਾਪਸ true;
		}
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
		ਜੇ (!k_ਕਤਾਰ_is_ਖਾਲੀ_ਥਾਵਾਂ(event->ਕਤਾਰ)) {
			*state = K_POLL_STATE_FIFO_ਅੰਕੜੇ_ਉਪਲਬਧ;
			ਵਾਪਸ true;
		}
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_SIGNAL:
		ਜੇ (event->ਸੰਕੇਤ->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			ਵਾਪਸ true;
		}
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_MSGQ_ਅੰਕੜੇ_ਉਪਲਬਧ:
		ਜੇ (event->msgq->ਵਰਤਿਆ_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_ਅੰਕੜੇ_ਉਪਲਬਧ;
			ਵਾਪਸ true;
		}
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_PIPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		ਜੇ (!ring_buf_is_ਖਾਲੀ_ਥਾਵਾਂ(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_ਅੰਕੜੇ_ਉਪਲਬਧ;
			ਵਾਪਸ true;
		}
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_IGNORE:
		ਰੋਕੋ;
	ਮੂਲ:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		ਰੋਕੋ;
	}

	ਵਾਪਸ false;
}

ਸਥਿਰ ਬਣਤਰ k_thread *poller_thread(ਬਣਤਰ z_poller *p)
{
	ਵਾਪਸ p ? CONTAINER_ਦਾ(p, ਬਣਤਰ k_thread, poller) : ਨਿਰਾ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਜੋੜੋ_event(sys_dlist_t *events, ਬਣਤਰ k_poll_event *event,
			     ਬਣਤਰ z_poller *poller)
{
	ਬਣਤਰ k_poll_event *pending;

	pending = (ਬਣਤਰ k_poll_event *)sys_dlist_peek_ਪੂਛ(events);
	ਜੇ ((pending == ਨਿਰਾ) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->ਗੰਢ_2);
		ਵਾਪਸ;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, ਗੰਢ_2) {
		ਜੇ (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_ਪਾਓ(&pending->ਗੰਢ_2, &event->ਗੰਢ_2);
			ਵਾਪਸ;
		}
	}

	sys_dlist_append(events, &event->ਗੰਢ_2);
}

/* must be called with interrupts locked */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਰਜਿਸਟਰ_event(ਬਣਤਰ k_poll_event *event,
				 ਬਣਤਰ z_poller *poller)
{
	ਚੋਣ (event->type) {
	ਹਾਲਤ K_POLL_TYPE_SEM_ਉਪਲਬਧ:
		__ASSERT(event->sem != ਨਿਰਾ, "invalid semaphore\n");
		ਜੋੜੋ_event(&event->sem->poll_events, event, poller);
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
		__ASSERT(event->ਕਤਾਰ != ਨਿਰਾ, "invalid queue\n");
		ਜੋੜੋ_event(&event->ਕਤਾਰ->poll_events, event, poller);
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->ਸੰਕੇਤ != ਨਿਰਾ, "invalid poll signal\n");
		ਜੋੜੋ_event(&event->ਸੰਕੇਤ->poll_events, event, poller);
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_MSGQ_ਅੰਕੜੇ_ਉਪਲਬਧ:
		__ASSERT(event->msgq != ਨਿਰਾ, "invalid message queue\n");
		ਜੋੜੋ_event(&event->msgq->poll_events, event, poller);
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_PIPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
		__ASSERT(event->pipe != ਨਿਰਾ, "invalid pipe\n");
		ਜੋੜੋ_event(&event->pipe->poll_events, event, poller);
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ਰੋਕੋ;
	ਮੂਲ:
		__ASSERT(false, "invalid event type\n");
		ਰੋਕੋ;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ clear_event_registration(ਬਣਤਰ k_poll_event *event)
{
	bool remove_event = false;

	event->poller = ਨਿਰਾ;

	ਚੋਣ (event->type) {
	ਹਾਲਤ K_POLL_TYPE_SEM_ਉਪਲਬਧ:
		__ASSERT(event->sem != ਨਿਰਾ, "invalid semaphore\n");
		remove_event = true;
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
		__ASSERT(event->ਕਤਾਰ != ਨਿਰਾ, "invalid queue\n");
		remove_event = true;
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->ਸੰਕੇਤ != ਨਿਰਾ, "invalid poll signal\n");
		remove_event = true;
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_MSGQ_ਅੰਕੜੇ_ਉਪਲਬਧ:
		__ASSERT(event->msgq != ਨਿਰਾ, "invalid message queue\n");
		remove_event = true;
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_PIPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
		__ASSERT(event->pipe != ਨਿਰਾ, "invalid pipe\n");
		remove_event = true;
		ਰੋਕੋ;
	ਹਾਲਤ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ਰੋਕੋ;
	ਮੂਲ:
		__ASSERT(false, "invalid event type\n");
		ਰੋਕੋ;
	}
	ਜੇ (remove_event && sys_dnode_is_linked(&event->ਗੰਢ_2)) {
		sys_dlist_remove(&event->ਗੰਢ_2);
	}
}

/* must be called with interrupts locked */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ clear_event_registrations(ਬਣਤਰ k_poll_event *events,
					      ਪੂਰਨ ਅੰਕ_events,
					      k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	ਜਦੋਂ (ਅੰਕ_events--) {
		clear_event_registration(&events[ਅੰਕ_events]);
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);
		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਸੈੱਟ_event_ready(ਬਣਤਰ k_poll_event *event, uint32_t state)
{
	event->poller = ਨਿਰਾ;
	event->state |= state;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ਰਜਿਸਟਰ_events(ਬਣਤਰ k_poll_event *events,
				  ਪੂਰਨ ਅੰਕ_events,
				  ਬਣਤਰ z_poller *poller,
				  bool just_check)
{
	ਪੂਰਨ events_registered = 0;

	ਲਈ (ਪੂਰਨ ii = 0; ii < ਅੰਕ_events; ii++) {
		k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
		uint32_t state;

		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
		ਜੇ (is_condition_met(&events[ii], &state)) {
			ਸੈੱਟ_event_ready(&events[ii], state);
			poller->is_polling = false;
		} ਨਹੀਂਤਾਂ ਜੇ (!just_check && poller->is_polling) {
			ਰਜਿਸਟਰ_event(&events[ii], poller);
			events_registered += 1;
		} ਨਹੀਂਤਾਂ {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	ਵਾਪਸ events_registered;
}

ਸਥਿਰ ਪੂਰਨ signal_poller(ਬਣਤਰ k_poll_event *event, uint32_t state)
{
	ਬਣਤਰ k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != ਨਿਰਾ, "poller should have a thread\n");

	ਜੇ (!z_is_thread_pending(thread)) {
		ਵਾਪਸ 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	ਜੇ (!z_is_thread_ready(thread)) {
		ਵਾਪਸ 0;
	}

	z_ready_thread(thread);

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_impl_k_poll(ਬਣਤਰ k_poll_event *events, ਪੂਰਨ ਅੰਕ_events,
		  k_timeout_t timeout)
{
	ਪੂਰਨ events_registered;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਬਣਤਰ z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->ਬਹੁਲਕ = ਬਹੁਲਕ_POLL;

	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "");
	__ASSERT(events != ਨਿਰਾ, "NULL events\n");
	__ASSERT(ਅੰਕ_events >= 0, "<0 events\n");

	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = ਰਜਿਸਟਰ_events(events, ਅੰਕ_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	ਜੇ (!poller->is_polling) {
		clear_event_registrations(events, events_registered, ਕੁੰਜੀ);
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		ਵਾਪਸ 0;
	}

	poller->is_polling = false;

	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		ਵਾਪਸ -EAGAIN;
	}

	ਸਥਿਰ _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	ਪੂਰਨ ਅਦਲਾ_ਬਦਲੀ_rc = z_pend_curr(&poll_ਤਾਲਾ, ਕੁੰਜੀ, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
	clear_event_registrations(events, events_registered, ਕੁੰਜੀ);
	k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_poll_api, poll, events, ਅਦਲਾ_ਬਦਲੀ_rc);

	ਵਾਪਸ ਅਦਲਾ_ਬਦਲੀ_rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_poll(ਬਣਤਰ k_poll_event *events,
				ਪੂਰਨ ਅੰਕ_events, k_timeout_t timeout)
{
	ਪੂਰਨ ret;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਬਣਤਰ k_poll_event *events_copy = ਨਿਰਾ;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	ਜੇ (K_SYSCALL_VERIFY(ਅੰਕ_events >= 0)) {
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	ਜੇ (K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(!u32_mul_ਉਛਾਲ(ਅੰਕ_events,
						   ਆਕਾਰ(ਬਣਤਰ k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}
	events_copy = z_thread_malloc(bounds);
	ਜੇ (!events_copy) {
		ret = -ENOMEM;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(events, bounds)) {
		ਜਾਓ oops_free;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
	(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(events_copy, events, bounds);
	k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

	/* Validate what's inside events_copy */
	ਲਈ (ਪੂਰਨ i = 0; i < ਅੰਕ_events; i++) {
		ਬਣਤਰ k_poll_event *e = &events_copy[i];

		ਜੇ (K_SYSCALL_VERIFY(e->ਬਹੁਲਕ == K_POLL_ਬਹੁਲਕ_NOTIFY_ONLY)) {
			ret = -EINVAL;
			ਜਾਓ ਬਾਹਰਲਾ_free;
		}

		ਚੋਣ (e->type) {
		ਹਾਲਤ K_POLL_TYPE_IGNORE:
			ਰੋਕੋ;
		ਹਾਲਤ K_POLL_TYPE_SIGNAL:
			ਜੇ (K_SYSCALL_OBJ(e->ਸੰਕੇਤ, K_OBJ_POLL_SIGNAL)) {
				ਜਾਓ oops_free;
			}
			ਰੋਕੋ;
		ਹਾਲਤ K_POLL_TYPE_SEM_ਉਪਲਬਧ:
			ਜੇ (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				ਜਾਓ oops_free;
			}
			ਰੋਕੋ;
		ਹਾਲਤ K_POLL_TYPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
			ਜੇ (K_SYSCALL_OBJ(e->ਕਤਾਰ, K_OBJ_ਕਤਾਰ)) {
				ਜਾਓ oops_free;
			}
			ਰੋਕੋ;
		ਹਾਲਤ K_POLL_TYPE_MSGQ_ਅੰਕੜੇ_ਉਪਲਬਧ:
			ਜੇ (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				ਜਾਓ oops_free;
			}
			ਰੋਕੋ;
		ਹਾਲਤ K_POLL_TYPE_PIPE_ਅੰਕੜੇ_ਉਪਲਬਧ:
			ਜੇ (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				ਜਾਓ oops_free;
			}
			ਰੋਕੋ;
		ਮੂਲ:
			ret = -EINVAL;
			ਜਾਓ ਬਾਹਰਲਾ_free;
		}
	}

	ret = k_poll(events_copy, ਅੰਕ_events, timeout);
	(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ *)events, events_copy, bounds);
ਬਾਹਰਲਾ_free:
	k_free(events_copy);
ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_poll_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
ਸਥਿਰ ਪੂਰਨ signal_poll_event(ਬਣਤਰ k_poll_event *event, uint32_t state)
{
	ਬਣਤਰ z_poller *poller = event->poller;
	ਪੂਰਨ retcode = 0;

	ਜੇ (poller != ਨਿਰਾ) {
		ਜੇ (poller->ਬਹੁਲਕ == ਬਹੁਲਕ_POLL) {
			retcode = signal_poller(event, state);
		} ਨਹੀਂਤਾਂ ਜੇ (poller->ਬਹੁਲਕ == ਬਹੁਲਕ_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} ਨਹੀਂਤਾਂ {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		ਜੇ (retcode < 0) {
			ਵਾਪਸ retcode;
		}
	}

	ਸੈੱਟ_event_ready(event, state);
	ਵਾਪਸ retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	ਬਣਤਰ k_poll_event *poll_event;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);

	poll_event = (ਬਣਤਰ k_poll_event *)sys_dlist_get(events);
	ਜੇ (poll_event != ਨਿਰਾ) {
		(ਖਾਲੀ) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ (poll_event != ਨਿਰਾ);
}

ਖਾਲੀ z_impl_k_poll_signal_init(ਬਣਤਰ k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_ਪੋਰਟ_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_poll_signal_init(ਬਣਤਰ k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_impl_k_poll_signal_reset(ਬਣਤਰ k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_ਪੋਰਟ_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

ਖਾਲੀ z_impl_k_poll_signal_check(ਬਣਤਰ k_poll_signal *sig,
			       ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *signaled, ਪੂਰਨ *ਨਤੀਜਾ)
{
	*signaled = sig->signaled;
	*ਨਤੀਜਾ = sig->ਨਤੀਜਾ;

	SYS_ਪੋਰਟ_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਖਾਲੀ z_vrfy_k_poll_signal_check(ਬਣਤਰ k_poll_signal *sig,
			       ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *signaled, ਪੂਰਨ *ਨਤੀਜਾ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(signaled, ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(ਨਤੀਜਾ, ਆਕਾਰ(ਪੂਰਨ)));
	z_impl_k_poll_signal_check(sig, signaled, ਨਤੀਜਾ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_poll_signal_raise(ਬਣਤਰ k_poll_signal *sig, ਪੂਰਨ ਨਤੀਜਾ)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
	ਬਣਤਰ k_poll_event *poll_event;

	sig->ਨਤੀਜਾ = ਨਤੀਜਾ;
	sig->signaled = 1U;

	poll_event = (ਬਣਤਰ k_poll_event *)sys_dlist_get(&sig->poll_events);
	ਜੇ (poll_event == ਨਿਰਾ) {
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		ਵਾਪਸ 0;
	}

	ਪੂਰਨ rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_ਪੋਰਟ_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_ਤਾਲਾ, ਕੁੰਜੀ);
	ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_poll_signal_raise(ਬਣਤਰ k_poll_signal *sig,
					     ਪੂਰਨ ਨਤੀਜਾ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	ਵਾਪਸ z_impl_k_poll_signal_raise(sig, ਨਤੀਜਾ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_poll_signal_reset(ਬਣਤਰ k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਸਥਿਰ ਖਾਲੀ triggered_work_handler(ਬਣਤਰ k_work *work)
{
	ਬਣਤਰ k_work_poll *twork =
			CONTAINER_ਦਾ(work, ਬਣਤਰ k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	ਜੇ (twork->poller.ਬਹੁਲਕ != ਬਹੁਲਕ_NONE) {
		k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
		clear_event_registrations(twork->events,
					  twork->ਅੰਕ_events, ਕੁੰਜੀ);
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = ਨਿਰਾ;
	twork->real_handler(work);
}

ਬਾਹਰੀ ਪੂਰਨ z_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(ਬਣਤਰ k_work_q *ਕਤਾਰ,
			 ਬਣਤਰ k_work *work);

ਸਥਿਰ ਖਾਲੀ triggered_work_expiration_handler(ਬਣਤਰ _timeout *timeout)
{
	ਬਣਤਰ k_work_poll *twork =
		CONTAINER_ਦਾ(timeout, ਬਣਤਰ k_work_poll, timeout);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);

	ਜੇ (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	twork->poller.is_polling = false;
	twork->poll_ਨਤੀਜਾ = -EAGAIN;
	z_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(twork->workq, &twork->work);

	k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਸਥਿਰ ਪੂਰਨ signal_triggered_work(ਬਣਤਰ k_poll_event *event, uint32_t ਸਥਿਤੀ)
{
	ਬਣਤਰ z_poller *poller = event->poller;
	ਬਣਤਰ k_work_poll *twork =
		CONTAINER_ਦਾ(poller, ਬਣਤਰ k_work_poll, poller);

	ਜੇ (poller->is_polling && twork->workq != ਨਿਰਾ) {
		ਬਣਤਰ k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(ਖਾਲੀ)z_try_abort_timeout(&twork->timeout);
		twork->poll_ਨਤੀਜਾ = 0;
		z_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(work_q, &twork->work);
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ triggered_work_cancel(ਬਣਤਰ k_work_poll *work,
				 k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	/* Check if the work waits for event. */
	ਜੇ (work->poller.is_polling && work->poller.ਬਹੁਲਕ != ਬਹੁਲਕ_NONE) {
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
		ਜਦੋਂ (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);
			ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.ਬਹੁਲਕ = ਬਹੁਲਕ_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->ਅੰਕ_events, ਕੁੰਜੀ);
		work->workq = ਨਿਰਾ;
		ਵਾਪਸ 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	ਵਾਪਸ -EINVAL;
}

ਖਾਲੀ k_work_poll_init(ਬਣਤਰ k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (ਬਣਤਰ k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

ਪੂਰਨ k_work_poll_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(ਬਣਤਰ k_work_q *work_q,
				ਬਣਤਰ k_work_poll *work,
				ਬਣਤਰ k_poll_event *events,
				ਪੂਰਨ ਅੰਕ_events,
				k_timeout_t timeout)
{
	ਪੂਰਨ events_registered;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	__ASSERT(work_q != ਨਿਰਾ, "NULL work_q\n");
	__ASSERT(work != ਨਿਰਾ, "NULL work\n");
	__ASSERT(events != ਨਿਰਾ, "NULL events\n");
	__ASSERT(ਅੰਕ_events >= 0, "<0 events\n");

	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_work_poll, submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
	ਜੇ (work->workq != ਨਿਰਾ) {
		ਜੇ (work->workq == work_q) {
			ਪੂਰਨ retval;

			retval = triggered_work_cancel(work, ਕੁੰਜੀ);
			ਜੇ (retval < 0) {
				k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

				SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_work_poll, submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ, work_q,
					work, timeout, retval);

				ਵਾਪਸ retval;
			}
		} ਨਹੀਂਤਾਂ {
			k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

			SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_work_poll, submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ, work_q,
				work, timeout, -EADDRINUSE);

			ਵਾਪਸ -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.ਬਹੁਲਕ = ਬਹੁਲਕ_NONE;
	k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

	/* Save list of events. */
	work->events = events;
	work->ਅੰਕ_events = ਅੰਕ_events;

	/* Clear result */
	work->poll_ਨਤੀਜਾ = -EINPROGRESS;

	/* Register events */
	events_registered = ਰਜਿਸਟਰ_events(events, ਅੰਕ_events,
					    &work->poller, false);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
	ਜੇ (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(ਅੰਕ_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_ਜੋੜੋ_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.ਬਹੁਲਕ = ਬਹੁਲਕ_TRIGGERED;
		k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_work_poll, submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ, work_q, work, timeout, 0);

		ਵਾਪਸ 0;
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
	ਜੇ (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_ਨਤੀਜਾ = -EAGAIN;
	} ਨਹੀਂਤਾਂ {
		work->poll_ਨਤੀਜਾ = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, ਕੁੰਜੀ);
	k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

	/* Submit work. */
	k_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(work_q, &work->work);

	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_work_poll, submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ, work_q, work, timeout, 0);

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_work_poll_submit(ਬਣਤਰ k_work_poll *work,
				     ਬਣਤਰ k_poll_event *events,
				     ਪੂਰਨ ਅੰਕ_events,
				     k_timeout_t timeout)
{
	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	ਪੂਰਨ ret = k_work_poll_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(&k_sys_work_q, work,
								events, ਅੰਕ_events, timeout);

	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_poll_cancel(ਬਣਤਰ k_work_poll *work)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ retval;

	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	ਜੇ (work == ਨਿਰਾ || work->workq == ਨਿਰਾ) {
		SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		ਵਾਪਸ -EINVAL;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&poll_ਤਾਲਾ);
	retval = triggered_work_cancel(work, ਕੁੰਜੀ);
	k_spin_unlock(&poll_ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	ਵਾਪਸ retval;
}
