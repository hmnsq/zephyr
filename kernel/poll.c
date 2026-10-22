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

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/util.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/__assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
ಸ್ಥಿರ ರಚನೆ k_spinlock poll_ಬೀಗ;

ಎಣಿಕೆ POLL_ಬಹುಲಕ { ಬಹುಲಕ_NONE, ಬಹುಲಕ_POLL, ಬಹುಲಕ_TRIGGERED };

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_poller(ರಚನೆ k_poll_event *event, uint32_t state);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_triggered_work(ರಚನೆ k_poll_event *event, uint32_t ಸ್ಥಿತಿ_2);

ಶೂನ್ಯ k_poll_event_init(ರಚನೆ k_poll_event *event, uint32_t type,
		       ಪೂರ್ಣಾಂಕ ಬಹುಲಕ, ಶೂನ್ಯ *obj)
{
	__ASSERT(ಬಹುಲಕ == K_POLL_ಬಹುಲಕ_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (ಬಿಟ್(POLL_ಅಂಕೆ_TYPES)), "invalid type\n");
	__ASSERT(obj != ಖಾಲಿ, "must provide an object\n");

	event->poller = ಖಾಲಿ;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->ಬಹುಲಕ = ಬಹುಲಕ;
	event->ಬಳಸದ = 0U;
	event->obj = obj;

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool is_condition_met(ರಚನೆ k_poll_event *event, uint32_t *state)
{
	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ K_POLL_TYPE_SEM_ಲಭ್ಯ:
		ಆದರೆ (k_sem_ಎಣಿಕೆ_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_ಲಭ್ಯ;
			ಹಿಂತಿರುಗಿ true;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		ಆದರೆ (!k_ಸರತಿ_is_ಖಾಲಿಗಳು(event->ಸರತಿ)) {
			*state = K_POLL_STATE_FIFO_ದತ್ತಾಂಶ_ಲಭ್ಯ;
			ಹಿಂತಿರುಗಿ true;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_SIGNAL:
		ಆದರೆ (event->ಸಂಕೇತ->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			ಹಿಂತಿರುಗಿ true;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_MSGQ_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		ಆದರೆ (event->msgq->ಬಳಸಿದ_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_ದತ್ತಾಂಶ_ಲಭ್ಯ;
			ಹಿಂತಿರುಗಿ true;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_PIPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		ಆದರೆ (!ring_buf_is_ಖಾಲಿಗಳು(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_ದತ್ತಾಂಶ_ಲಭ್ಯ;
			ಹಿಂತಿರುಗಿ true;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_IGNORE:
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ ರಚನೆ k_thread *poller_thread(ರಚನೆ z_poller *p)
{
	ಹಿಂತಿರುಗಿ p ? CONTAINER_ದ(p, ರಚನೆ k_thread, poller) : ಖಾಲಿ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಕೂಡಿಸು_event(sys_dlist_t *events, ರಚನೆ k_poll_event *event,
			     ರಚನೆ z_poller *poller)
{
	ರಚನೆ k_poll_event *pending;

	pending = (ರಚನೆ k_poll_event *)sys_dlist_peek_ಬಾಲ(events);
	ಆದರೆ ((pending == ಖಾಲಿ) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->ಗಂಟು_2);
		ಹಿಂತಿರುಗಿ;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, ಗಂಟು_2) {
		ಆದರೆ (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_ಸೇರಿಸು(&pending->ಗಂಟು_2, &event->ಗಂಟು_2);
			ಹಿಂತಿರುಗಿ;
		}
	}

	sys_dlist_append(events, &event->ಗಂಟು_2);
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ರಿಜಿಸ್ಟರ್_event(ರಚನೆ k_poll_event *event,
				 ರಚನೆ z_poller *poller)
{
	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ K_POLL_TYPE_SEM_ಲಭ್ಯ:
		__ASSERT(event->sem != ಖಾಲಿ, "invalid semaphore\n");
		ಕೂಡಿಸು_event(&event->sem->poll_events, event, poller);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		__ASSERT(event->ಸರತಿ != ಖಾಲಿ, "invalid queue\n");
		ಕೂಡಿಸು_event(&event->ಸರತಿ->poll_events, event, poller);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->ಸಂಕೇತ != ಖಾಲಿ, "invalid poll signal\n");
		ಕೂಡಿಸು_event(&event->ಸಂಕೇತ->poll_events, event, poller);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_MSGQ_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		__ASSERT(event->msgq != ಖಾಲಿ, "invalid message queue\n");
		ಕೂಡಿಸು_event(&event->msgq->poll_events, event, poller);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_PIPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		__ASSERT(event->pipe != ಖಾಲಿ, "invalid pipe\n");
		ಕೂಡಿಸು_event(&event->pipe->poll_events, event, poller);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		__ASSERT(false, "invalid event type\n");
		ನಿಲ್ಲಿಸು;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ clear_event_registration(ರಚನೆ k_poll_event *event)
{
	bool remove_event = false;

	event->poller = ಖಾಲಿ;

	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ K_POLL_TYPE_SEM_ಲಭ್ಯ:
		__ASSERT(event->sem != ಖಾಲಿ, "invalid semaphore\n");
		remove_event = true;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		__ASSERT(event->ಸರತಿ != ಖಾಲಿ, "invalid queue\n");
		remove_event = true;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->ಸಂಕೇತ != ಖಾಲಿ, "invalid poll signal\n");
		remove_event = true;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_MSGQ_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		__ASSERT(event->msgq != ಖಾಲಿ, "invalid message queue\n");
		remove_event = true;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_PIPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
		__ASSERT(event->pipe != ಖಾಲಿ, "invalid pipe\n");
		remove_event = true;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		__ASSERT(false, "invalid event type\n");
		ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (remove_event && sys_dnode_is_linked(&event->ಗಂಟು_2)) {
		sys_dlist_remove(&event->ಗಂಟು_2);
	}
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ clear_event_registrations(ರಚನೆ k_poll_event *events,
					      ಪೂರ್ಣಾಂಕ ಅಂಕೆ_events,
					      k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ಆಗ (ಅಂಕೆ_events--) {
		clear_event_registration(&events[ಅಂಕೆ_events]);
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);
		ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಗಣ_event_ready(ರಚನೆ k_poll_event *event, uint32_t state)
{
	event->poller = ಖಾಲಿ;
	event->state |= state;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ರಿಜಿಸ್ಟರ್_events(ರಚನೆ k_poll_event *events,
				  ಪೂರ್ಣಾಂಕ ಅಂಕೆ_events,
				  ರಚನೆ z_poller *poller,
				  bool just_check)
{
	ಪೂರ್ಣಾಂಕ events_registered = 0;

	ಗಾಗಿ (ಪೂರ್ಣಾಂಕ ii = 0; ii < ಅಂಕೆ_events; ii++) {
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
		uint32_t state;

		ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
		ಆದರೆ (is_condition_met(&events[ii], &state)) {
			ಗಣ_event_ready(&events[ii], state);
			poller->is_polling = false;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!just_check && poller->is_polling) {
			ರಿಜಿಸ್ಟರ್_event(&events[ii], poller);
			events_registered += 1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);
	}

	ಹಿಂತಿರುಗಿ events_registered;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_poller(ರಚನೆ k_poll_event *event, uint32_t state)
{
	ರಚನೆ k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != ಖಾಲಿ, "poller should have a thread\n");

	ಆದರೆ (!z_is_thread_pending(thread)) {
		ಹಿಂತಿರುಗಿ 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_ಮೌಲ್ಯ_ಗಣ(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	ಆದರೆ (!z_is_thread_ready(thread)) {
		ಹಿಂತಿರುಗಿ 0;
	}

	z_ready_thread(thread);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_impl_k_poll(ರಚನೆ k_poll_event *events, ಪೂರ್ಣಾಂಕ ಅಂಕೆ_events,
		  k_timeout_t timeout)
{
	ಪೂರ್ಣಾಂಕ events_registered;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ರಚನೆ z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->ಬಹುಲಕ = ಬಹುಲಕ_POLL;

	__ASSERT(!arch_is_ಒಳ_isr(), "");
	__ASSERT(events != ಖಾಲಿ, "NULL events\n");
	__ASSERT(ಅಂಕೆ_events >= 0, "<0 events\n");

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = ರಿಜಿಸ್ಟರ್_events(events, ಅಂಕೆ_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	ಆದರೆ (!poller->is_polling) {
		clear_event_registrations(events, events_registered, ಕೀಲಿ);
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		ಹಿಂತಿರುಗಿ 0;
	}

	poller->is_polling = false;

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		ಹಿಂತಿರುಗಿ -EAGAIN;
	}

	ಸ್ಥಿರ _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	ಪೂರ್ಣಾಂಕ ಅದಲುಬದಲು_rc = z_pend_curr(&poll_ಬೀಗ, ಕೀಲಿ, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
	clear_event_registrations(events, events_registered, ಕೀಲಿ);
	k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_poll_api, poll, events, ಅದಲುಬದಲು_rc);

	ಹಿಂತಿರುಗಿ ಅದಲುಬದಲು_rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_poll(ರಚನೆ k_poll_event *events,
				ಪೂರ್ಣಾಂಕ ಅಂಕೆ_events, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಂಕ ret;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ರಚನೆ k_poll_event *events_copy = ಖಾಲಿ;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	ಆದರೆ (K_SYSCALL_VERIFY(ಅಂಕೆ_events >= 0)) {
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}
	ಆದರೆ (K_SYSCALL_VERIFY_ಸಂದೇಶ(!u32_mul_ಅತಿಪ್ರವಾಹ(ಅಂಕೆ_events,
						   ಗಾತ್ರ(ರಚನೆ k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		ಹೋಗು ಹೊರಹೋಗು;
	}
	events_copy = z_thread_malloc(bounds);
	ಆದರೆ (!events_copy) {
		ret = -ENOMEM;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (K_SYSCALL_ಸ್ಮರಣೆ_WRITE(events, bounds)) {
		ಹೋಗು oops_free;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
	(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(events_copy, events, bounds);
	k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

	/* Validate what's inside events_copy */
	ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < ಅಂಕೆ_events; i++) {
		ರಚನೆ k_poll_event *e = &events_copy[i];

		ಆದರೆ (K_SYSCALL_VERIFY(e->ಬಹುಲಕ == K_POLL_ಬಹುಲಕ_NOTIFY_ONLY)) {
			ret = -EINVAL;
			ಹೋಗು ಹೊರಹೋಗು_free;
		}

		ಆಯ್ಕೆ (e->type) {
		ಸ್ಥಿತಿ K_POLL_TYPE_IGNORE:
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ K_POLL_TYPE_SIGNAL:
			ಆದರೆ (K_SYSCALL_OBJ(e->ಸಂಕೇತ, K_OBJ_POLL_SIGNAL)) {
				ಹೋಗು oops_free;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ K_POLL_TYPE_SEM_ಲಭ್ಯ:
			ಆದರೆ (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				ಹೋಗು oops_free;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ K_POLL_TYPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
			ಆದರೆ (K_SYSCALL_OBJ(e->ಸರತಿ, K_OBJ_ಸರತಿ)) {
				ಹೋಗು oops_free;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ K_POLL_TYPE_MSGQ_ದತ್ತಾಂಶ_ಲಭ್ಯ:
			ಆದರೆ (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				ಹೋಗು oops_free;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ K_POLL_TYPE_PIPE_ದತ್ತಾಂಶ_ಲಭ್ಯ:
			ಆದರೆ (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				ಹೋಗು oops_free;
			}
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ret = -EINVAL;
			ಹೋಗು ಹೊರಹೋಗು_free;
		}
	}

	ret = k_poll(events_copy, ಅಂಕೆ_events, timeout);
	(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ *)events, events_copy, bounds);
ಹೊರಹೋಗು_free:
	k_free(events_copy);
ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_poll_event(ರಚನೆ k_poll_event *event, uint32_t state)
{
	ರಚನೆ z_poller *poller = event->poller;
	ಪೂರ್ಣಾಂಕ retcode = 0;

	ಆದರೆ (poller != ಖಾಲಿ) {
		ಆದರೆ (poller->ಬಹುಲಕ == ಬಹುಲಕ_POLL) {
			retcode = signal_poller(event, state);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (poller->ಬಹುಲಕ == ಬಹುಲಕ_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		ಆದರೆ (retcode < 0) {
			ಹಿಂತಿರುಗಿ retcode;
		}
	}

	ಗಣ_event_ready(event, state);
	ಹಿಂತಿರುಗಿ retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	ರಚನೆ k_poll_event *poll_event;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);

	poll_event = (ರಚನೆ k_poll_event *)sys_dlist_get(events);
	ಆದರೆ (poll_event != ಖಾಲಿ) {
		(ಶೂನ್ಯ) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ (poll_event != ಖಾಲಿ);
}

ಶೂನ್ಯ z_impl_k_poll_signal_init(ರಚನೆ k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_poll_signal_init(ರಚನೆ k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_poll_signal_reset(ರಚನೆ k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

ಶೂನ್ಯ z_impl_k_poll_signal_check(ರಚನೆ k_poll_signal *sig,
			       ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *signaled, ಪೂರ್ಣಾಂಕ *ಫಲಿತಾಂಶ)
{
	*signaled = sig->signaled;
	*ಫಲಿತಾಂಶ = sig->ಫಲಿತಾಂಶ;

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಶೂನ್ಯ z_vrfy_k_poll_signal_check(ರಚನೆ k_poll_signal *sig,
			       ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *signaled, ಪೂರ್ಣಾಂಕ *ಫಲಿತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(signaled, ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(ಫಲಿತಾಂಶ, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)));
	z_impl_k_poll_signal_check(sig, signaled, ಫಲಿತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_poll_signal_raise(ರಚನೆ k_poll_signal *sig, ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
	ರಚನೆ k_poll_event *poll_event;

	sig->ಫಲಿತಾಂಶ = ಫಲಿತಾಂಶ;
	sig->signaled = 1U;

	poll_event = (ರಚನೆ k_poll_event *)sys_dlist_get(&sig->poll_events);
	ಆದರೆ (poll_event == ಖಾಲಿ) {
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		ಹಿಂತಿರುಗಿ 0;
	}

	ಪೂರ್ಣಾಂಕ rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_ಬೀಗ, ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_poll_signal_raise(ರಚನೆ k_poll_signal *sig,
					     ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	ಹಿಂತಿರುಗಿ z_impl_k_poll_signal_raise(sig, ಫಲಿತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_poll_signal_reset(ರಚನೆ k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಶೂನ್ಯ triggered_work_handler(ರಚನೆ k_work *work)
{
	ರಚನೆ k_work_poll *twork =
			CONTAINER_ದ(work, ರಚನೆ k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	ಆದರೆ (twork->poller.ಬಹುಲಕ != ಬಹುಲಕ_NONE) {
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

		ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
		clear_event_registrations(twork->events,
					  twork->ಅಂಕೆ_events, ಕೀಲಿ);
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = ಖಾಲಿ;
	twork->real_handler(work);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ z_work_submit_ಗಮ್ಯ_ಸರತಿ(ರಚನೆ k_work_q *ಸರತಿ,
			 ರಚನೆ k_work *work);

ಸ್ಥಿರ ಶೂನ್ಯ triggered_work_expiration_handler(ರಚನೆ _timeout *timeout)
{
	ರಚನೆ k_work_poll *twork =
		CONTAINER_ದ(timeout, ರಚನೆ k_work_poll, timeout);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);

	ಆದರೆ (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	twork->poller.is_polling = false;
	twork->poll_ಫಲಿತಾಂಶ = -EAGAIN;
	z_work_submit_ಗಮ್ಯ_ಸರತಿ(twork->workq, &twork->work);

	k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_triggered_work(ರಚನೆ k_poll_event *event, uint32_t ಸ್ಥಿತಿ_2)
{
	ರಚನೆ z_poller *poller = event->poller;
	ರಚನೆ k_work_poll *twork =
		CONTAINER_ದ(poller, ರಚನೆ k_work_poll, poller);

	ಆದರೆ (poller->is_polling && twork->workq != ಖಾಲಿ) {
		ರಚನೆ k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(ಶೂನ್ಯ)z_try_abort_timeout(&twork->timeout);
		twork->poll_ಫಲಿತಾಂಶ = 0;
		z_work_submit_ಗಮ್ಯ_ಸರತಿ(work_q, &twork->work);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ triggered_work_cancel(ರಚನೆ k_work_poll *work,
				 k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	/* Check if the work waits for event. */
	ಆದರೆ (work->poller.is_polling && work->poller.ಬಹುಲಕ != ಬಹುಲಕ_NONE) {
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
		ಆಗ (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);
			ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.ಬಹುಲಕ = ಬಹುಲಕ_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->ಅಂಕೆ_events, ಕೀಲಿ);
		work->workq = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	ಹಿಂತಿರುಗಿ -EINVAL;
}

ಶೂನ್ಯ k_work_poll_init(ರಚನೆ k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (ರಚನೆ k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

ಪೂರ್ಣಾಂಕ k_work_poll_submit_ಗಮ್ಯ_ಸರತಿ(ರಚನೆ k_work_q *work_q,
				ರಚನೆ k_work_poll *work,
				ರಚನೆ k_poll_event *events,
				ಪೂರ್ಣಾಂಕ ಅಂಕೆ_events,
				k_timeout_t timeout)
{
	ಪೂರ್ಣಾಂಕ events_registered;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	__ASSERT(work_q != ಖಾಲಿ, "NULL work_q\n");
	__ASSERT(work != ಖಾಲಿ, "NULL work\n");
	__ASSERT(events != ಖಾಲಿ, "NULL events\n");
	__ASSERT(ಅಂಕೆ_events >= 0, "<0 events\n");

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_work_poll, submit_ಗಮ್ಯ_ಸರತಿ, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
	ಆದರೆ (work->workq != ಖಾಲಿ) {
		ಆದರೆ (work->workq == work_q) {
			ಪೂರ್ಣಾಂಕ retval;

			retval = triggered_work_cancel(work, ಕೀಲಿ);
			ಆದರೆ (retval < 0) {
				k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

				SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_work_poll, submit_ಗಮ್ಯ_ಸರತಿ, work_q,
					work, timeout, retval);

				ಹಿಂತಿರುಗಿ retval;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

			SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_work_poll, submit_ಗಮ್ಯ_ಸರತಿ, work_q,
				work, timeout, -EADDRINUSE);

			ಹಿಂತಿರುಗಿ -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.ಬಹುಲಕ = ಬಹುಲಕ_NONE;
	k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

	/* Save list of events. */
	work->events = events;
	work->ಅಂಕೆ_events = ಅಂಕೆ_events;

	/* Clear result */
	work->poll_ಫಲಿತಾಂಶ = -EINPROGRESS;

	/* Register events */
	events_registered = ರಿಜಿಸ್ಟರ್_events(events, ಅಂಕೆ_events,
					    &work->poller, false);

	ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
	ಆದರೆ (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(ಅಂಕೆ_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_ಕೂಡಿಸು_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.ಬಹುಲಕ = ಬಹುಲಕ_TRIGGERED;
		k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_work_poll, submit_ಗಮ್ಯ_ಸರತಿ, work_q, work, timeout, 0);

		ಹಿಂತಿರುಗಿ 0;
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
	ಆದರೆ (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_ಫಲಿತಾಂಶ = -EAGAIN;
	} ಇಲ್ಲದಿದ್ದರೆ {
		work->poll_ಫಲಿತಾಂಶ = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, ಕೀಲಿ);
	k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

	/* Submit work. */
	k_work_submit_ಗಮ್ಯ_ಸರತಿ(work_q, &work->work);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_work_poll, submit_ಗಮ್ಯ_ಸರತಿ, work_q, work, timeout, 0);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_work_poll_submit(ರಚನೆ k_work_poll *work,
				     ರಚನೆ k_poll_event *events,
				     ಪೂರ್ಣಾಂಕ ಅಂಕೆ_events,
				     k_timeout_t timeout)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	ಪೂರ್ಣಾಂಕ ret = k_work_poll_submit_ಗಮ್ಯ_ಸರತಿ(&k_sys_work_q, work,
								events, ಅಂಕೆ_events, timeout);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_poll_cancel(ರಚನೆ k_work_poll *work)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ retval;

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	ಆದರೆ (work == ಖಾಲಿ || work->workq == ಖಾಲಿ) {
		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&poll_ಬೀಗ);
	retval = triggered_work_cancel(work, ಕೀಲಿ);
	k_spin_unlock(&poll_ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	ಹಿಂತಿರುಗಿ retval;
}
