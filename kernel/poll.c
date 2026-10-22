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
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock poll_lock;

ಗಣನಾ POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_poller(ಸಂರಚನಾ k_poll_event *event, uint32_t state);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_triggered_work(ಸಂರಚನಾ k_poll_event *event, uint32_t status);

ಶೂನ್ಯ k_poll_event_init(ಸಂರಚನಾ k_poll_event *event, uint32_t type,
		       ಪೂರ್ಣಾಙ್ಕ mode, ಶೂನ್ಯ *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_ಅಂಕೆ_TYPES)), "invalid type\n");
	__ASSERT(obj != ರಿಕ್ತ, "must provide an object\n");

	event->poller = ರಿಕ್ತ;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool is_condition_met(ಸಂರಚನಾ k_poll_event *event, uint32_t *state)
{
	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ K_POLL_TYPE_SEM_AVAILABLE:
		ಯದಿ (k_sem_ಎಣಿಕೆ_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			ಪ್ರತ್ಯಾಗಮ true;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_DATA_AVAILABLE:
		ಯದಿ (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			ಪ್ರತ್ಯಾಗಮ true;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_SIGNAL:
		ಯದಿ (event->ಸಙ್ಕೇತ->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			ಪ್ರತ್ಯಾಗಮ true;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		ಯದಿ (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			ಪ್ರತ್ಯಾಗಮ true;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		ಯದಿ (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			ಪ್ರತ್ಯಾಗಮ true;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_IGNORE:
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ ಸಂರಚನಾ k_thread *poller_thread(ಸಂರಚನಾ z_poller *p)
{
	ಪ್ರತ್ಯಾಗಮ p ? CONTAINER_OF(p, ಸಂರಚನಾ k_thread, poller) : ರಿಕ್ತ;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ add_event(sys_dlist_t *events, ಸಂರಚನಾ k_poll_event *event,
			     ಸಂರಚನಾ z_poller *poller)
{
	ಸಂರಚನಾ k_poll_event *pending;

	pending = (ಸಂರಚನಾ k_poll_event *)sys_dlist_peek_tail(events);
	ಯದಿ ((pending == ರಿಕ್ತ) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->ಗಂಟು_2);
		ಪ್ರತ್ಯಾಗಮ;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, ಗಂಟು_2) {
		ಯದಿ (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_ಸೇರಿಸು(&pending->ಗಂಟು_2, &event->ಗಂಟು_2);
			ಪ್ರತ್ಯಾಗಮ;
		}
	}

	sys_dlist_append(events, &event->ಗಂಟು_2);
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ register_event(ಸಂರಚನಾ k_poll_event *event,
				 ಸಂರಚನಾ z_poller *poller)
{
	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ರಿಕ್ತ, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != ರಿಕ್ತ, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->ಸಙ್ಕೇತ != ರಿಕ್ತ, "invalid poll signal\n");
		add_event(&event->ಸಙ್ಕೇತ->poll_events, event, poller);
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != ರಿಕ್ತ, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != ರಿಕ್ತ, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		__ASSERT(false, "invalid event type\n");
		ಭಙ್ಗ;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ clear_event_registration(ಸಂರಚನಾ k_poll_event *event)
{
	bool remove_event = false;

	event->poller = ರಿಕ್ತ;

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ರಿಕ್ತ, "invalid semaphore\n");
		remove_event = true;
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != ರಿಕ್ತ, "invalid queue\n");
		remove_event = true;
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->ಸಙ್ಕೇತ != ರಿಕ್ತ, "invalid poll signal\n");
		remove_event = true;
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != ರಿಕ್ತ, "invalid message queue\n");
		remove_event = true;
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != ರಿಕ್ತ, "invalid pipe\n");
		remove_event = true;
		ಭಙ್ಗ;
	ಪಕ್ಷ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		__ASSERT(false, "invalid event type\n");
		ಭಙ್ಗ;
	}
	ಯದಿ (remove_event && sys_dnode_is_linked(&event->ಗಂಟು_2)) {
		sys_dlist_remove(&event->ಗಂಟು_2);
	}
}

/* must be called with interrupts locked */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ clear_event_registrations(ಸಂರಚನಾ k_poll_event *events,
					      ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_events,
					      k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ಯಾವತ್ (ಅಂಕೆ_events--) {
		clear_event_registration(&events[ಅಂಕೆ_events]);
		k_spin_unlock(&poll_lock, ಕೀಲಿ);
		ಕೀಲಿ = k_spin_lock(&poll_lock);
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ set_event_ready(ಸಂರಚನಾ k_poll_event *event, uint32_t state)
{
	event->poller = ರಿಕ್ತ;
	event->state |= state;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ register_events(ಸಂರಚನಾ k_poll_event *events,
				  ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_events,
				  ಸಂರಚನಾ z_poller *poller,
				  bool just_check)
{
	ಪೂರ್ಣಾಙ್ಕ events_registered = 0;

	ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ ii = 0; ii < ಅಂಕೆ_events; ii++) {
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
		uint32_t state;

		ಕೀಲಿ = k_spin_lock(&poll_lock);
		ಯದಿ (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} ಅನ್ಯಥಾ ಯದಿ (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} ಅನ್ಯಥಾ {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, ಕೀಲಿ);
	}

	ಪ್ರತ್ಯಾಗಮ events_registered;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_poller(ಸಂರಚನಾ k_poll_event *event, uint32_t state)
{
	ಸಂರಚನಾ k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != ರಿಕ್ತ, "poller should have a thread\n");

	ಯದಿ (!z_is_thread_pending(thread)) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_ಮೌಲ್ಯ_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	ಯದಿ (!z_is_thread_ready(thread)) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	z_ready_thread(thread);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_poll(ಸಂರಚನಾ k_poll_event *events, ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_events,
		  k_timeout_t timeout)
{
	ಪೂರ್ಣಾಙ್ಕ events_registered;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಸಂರಚನಾ z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != ರಿಕ್ತ, "NULL events\n");
	__ASSERT(ಅಂಕೆ_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, ಅಂಕೆ_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	ಕೀಲಿ = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	ಯದಿ (!poller->is_polling) {
		clear_event_registrations(events, events_registered, ಕೀಲಿ);
		k_spin_unlock(&poll_lock, ಕೀಲಿ);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		ಪ್ರತ್ಯಾಗಮ 0;
	}

	poller->is_polling = false;

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, ಕೀಲಿ);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		ಪ್ರತ್ಯಾಗಮ -EAGAIN;
	}

	ಸ್ಥಿರ _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	ಪೂರ್ಣಾಙ್ಕ swap_rc = z_pend_curr(&poll_lock, ಕೀಲಿ, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	ಕೀಲಿ = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, ಕೀಲಿ);
	k_spin_unlock(&poll_lock, ಕೀಲಿ);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	ಪ್ರತ್ಯಾಗಮ swap_rc;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_poll(ಸಂರಚನಾ k_poll_event *events,
				ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_events, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಸಂರಚನಾ k_poll_event *events_copy = ರಿಕ್ತ;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	ಯದಿ (K_SYSCALL_VERIFY(ಅಂಕೆ_events >= 0)) {
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}
	ಯದಿ (K_SYSCALL_VERIFY_ಸಂದೇಶ(!u32_mul_overflow(ಅಂಕೆ_events,
						   ಪರಿಮಾಣ(ಸಂರಚನಾ k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		ಗಚ್ಛ out;
	}
	events_copy = z_thread_malloc(bounds);
	ಯದಿ (!events_copy) {
		ret = -ENOMEM;
		ಗಚ್ಛ out;
	}

	ಯದಿ (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		ಗಚ್ಛ oops_free;
	}

	ಕೀಲಿ = k_spin_lock(&poll_lock);
	(ಶೂನ್ಯ)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, ಕೀಲಿ);

	/* Validate what's inside events_copy */
	ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < ಅಂಕೆ_events; i++) {
		ಸಂರಚನಾ k_poll_event *e = &events_copy[i];

		ಯದಿ (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			ಗಚ್ಛ out_free;
		}

		ವಿಕಲ್ಪ (e->type) {
		ಪಕ್ಷ K_POLL_TYPE_IGNORE:
			ಭಙ್ಗ;
		ಪಕ್ಷ K_POLL_TYPE_SIGNAL:
			ಯದಿ (K_SYSCALL_OBJ(e->ಸಙ್ಕೇತ, K_OBJ_POLL_SIGNAL)) {
				ಗಚ್ಛ oops_free;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ K_POLL_TYPE_SEM_AVAILABLE:
			ಯದಿ (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				ಗಚ್ಛ oops_free;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ K_POLL_TYPE_DATA_AVAILABLE:
			ಯದಿ (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				ಗಚ್ಛ oops_free;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			ಯದಿ (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				ಗಚ್ಛ oops_free;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			ಯದಿ (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				ಗಚ್ಛ oops_free;
			}
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ret = -EINVAL;
			ಗಚ್ಛ out_free;
		}
	}

	ret = k_poll(events_copy, ಅಂಕೆ_events, timeout);
	(ಶೂನ್ಯ)memcpy((ಶೂನ್ಯ *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	ಪ್ರತ್ಯಾಗಮ ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_poll_event(ಸಂರಚನಾ k_poll_event *event, uint32_t state)
{
	ಸಂರಚನಾ z_poller *poller = event->poller;
	ಪೂರ್ಣಾಙ್ಕ retcode = 0;

	ಯದಿ (poller != ರಿಕ್ತ) {
		ಯದಿ (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} ಅನ್ಯಥಾ ಯದಿ (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} ಅನ್ಯಥಾ {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		ಯದಿ (retcode < 0) {
			ಪ್ರತ್ಯಾಗಮ retcode;
		}
	}

	set_event_ready(event, state);
	ಪ್ರತ್ಯಾಗಮ retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	ಸಂರಚನಾ k_poll_event *poll_event;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&poll_lock);

	poll_event = (ಸಂರಚನಾ k_poll_event *)sys_dlist_get(events);
	ಯದಿ (poll_event != ರಿಕ್ತ) {
		(ಶೂನ್ಯ) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ (poll_event != ರಿಕ್ತ);
}

ಶೂನ್ಯ z_impl_k_poll_signal_init(ಸಂರಚನಾ k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_poll_signal_init(ಸಂರಚನಾ k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_poll_signal_reset(ಸಂರಚನಾ k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

ಶೂನ್ಯ z_impl_k_poll_signal_check(ಸಂರಚನಾ k_poll_signal *sig,
			       unsigned ಪೂರ್ಣಾಙ್ಕ *signaled, ಪೂರ್ಣಾಙ್ಕ *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
ಶೂನ್ಯ z_vrfy_k_poll_signal_check(ಸಂರಚನಾ k_poll_signal *sig,
			       unsigned ಪೂರ್ಣಾಙ್ಕ *signaled, ಪೂರ್ಣಾಙ್ಕ *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, ಪರಿಮಾಣ(unsigned ಪೂರ್ಣಾಙ್ಕ)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_poll_signal_raise(ಸಂರಚನಾ k_poll_signal *sig, ಪೂರ್ಣಾಙ್ಕ result)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&poll_lock);
	ಸಂರಚನಾ k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (ಸಂರಚನಾ k_poll_event *)sys_dlist_get(&sig->poll_events);
	ಯದಿ (poll_event == ರಿಕ್ತ) {
		k_spin_unlock(&poll_lock, ಕೀಲಿ);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪೂರ್ಣಾಙ್ಕ rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, ಕೀಲಿ);
	ಪ್ರತ್ಯಾಗಮ rc;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_poll_signal_raise(ಸಂರಚನಾ k_poll_signal *sig,
					     ಪೂರ್ಣಾಙ್ಕ result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_poll_signal_raise(sig, result);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_poll_signal_reset(ಸಂರಚನಾ k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಶೂನ್ಯ triggered_work_handler(ಸಂರಚನಾ k_work *work)
{
	ಸಂರಚನಾ k_work_poll *twork =
			CONTAINER_OF(work, ಸಂರಚನಾ k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	ಯದಿ (twork->poller.mode != MODE_NONE) {
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

		ಕೀಲಿ = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->ಅಂಕೆ_events, ಕೀಲಿ);
		k_spin_unlock(&poll_lock, ಕೀಲಿ);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = ರಿಕ್ತ;
	twork->real_handler(work);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ z_work_submit_to_queue(ಸಂರಚನಾ k_work_q *queue,
			 ಸಂರಚನಾ k_work *work);

ಸ್ಥಿರ ಶೂನ್ಯ triggered_work_expiration_handler(ಸಂರಚನಾ _timeout *timeout)
{
	ಸಂರಚನಾ k_work_poll *twork =
		CONTAINER_OF(timeout, ಸಂರಚನಾ k_work_poll, timeout);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&poll_lock);

	ಯದಿ (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, ಕೀಲಿ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_triggered_work(ಸಂರಚನಾ k_poll_event *event, uint32_t status)
{
	ಸಂರಚನಾ z_poller *poller = event->poller;
	ಸಂರಚನಾ k_work_poll *twork =
		CONTAINER_OF(poller, ಸಂರಚನಾ k_work_poll, poller);

	ಯದಿ (poller->is_polling && twork->workq != ರಿಕ್ತ) {
		ಸಂರಚನಾ k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(ಶೂನ್ಯ)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ triggered_work_cancel(ಸಂರಚನಾ k_work_poll *work,
				 k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	/* Check if the work waits for event. */
	ಯದಿ (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		ಯಾವತ್ (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, ಕೀಲಿ);
			ಕೀಲಿ = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->ಅಂಕೆ_events, ಕೀಲಿ);
		work->workq = ರಿಕ್ತ;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}

ಶೂನ್ಯ k_work_poll_init(ಸಂರಚನಾ k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (ಸಂರಚನಾ k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

ಪೂರ್ಣಾಙ್ಕ k_work_poll_submit_to_queue(ಸಂರಚನಾ k_work_q *work_q,
				ಸಂರಚನಾ k_work_poll *work,
				ಸಂರಚನಾ k_poll_event *events,
				ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_events,
				k_timeout_t timeout)
{
	ಪೂರ್ಣಾಙ್ಕ events_registered;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	__ASSERT(work_q != ರಿಕ್ತ, "NULL work_q\n");
	__ASSERT(work != ರಿಕ್ತ, "NULL work\n");
	__ASSERT(events != ರಿಕ್ತ, "NULL events\n");
	__ASSERT(ಅಂಕೆ_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	ಕೀಲಿ = k_spin_lock(&poll_lock);
	ಯದಿ (work->workq != ರಿಕ್ತ) {
		ಯದಿ (work->workq == work_q) {
			ಪೂರ್ಣಾಙ್ಕ retval;

			retval = triggered_work_cancel(work, ಕೀಲಿ);
			ಯದಿ (retval < 0) {
				k_spin_unlock(&poll_lock, ಕೀಲಿ);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				ಪ್ರತ್ಯಾಗಮ retval;
			}
		} ಅನ್ಯಥಾ {
			k_spin_unlock(&poll_lock, ಕೀಲಿ);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			ಪ್ರತ್ಯಾಗಮ -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, ಕೀಲಿ);

	/* Save list of events. */
	work->events = events;
	work->ಅಂಕೆ_events = ಅಂಕೆ_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, ಅಂಕೆ_events,
					    &work->poller, false);

	ಕೀಲಿ = k_spin_lock(&poll_lock);
	ಯದಿ (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(ಅಂಕೆ_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, ಕೀಲಿ);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		ಪ್ರತ್ಯಾಗಮ 0;
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
	ಯದಿ (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} ಅನ್ಯಥಾ {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, ಕೀಲಿ);
	k_spin_unlock(&poll_lock, ಕೀಲಿ);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ k_work_poll_submit(ಸಂರಚನಾ k_work_poll *work,
				     ಸಂರಚನಾ k_poll_event *events,
				     ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	ಪೂರ್ಣಾಙ್ಕ ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, ಅಂಕೆ_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_work_poll_cancel(ಸಂರಚನಾ k_work_poll *work)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	ಯದಿ (work == ರಿಕ್ತ || work->workq == ರಿಕ್ತ) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಕೀಲಿ = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, ಕೀಲಿ);
	k_spin_unlock(&poll_lock, ಕೀಲಿ);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	ಪ್ರತ್ಯಾಗಮ retval;
}
