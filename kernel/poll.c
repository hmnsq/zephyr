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

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>
#അന്തര്ഭാവ <zephyr/sys/util.h>
#അന്തര്ഭാവ <zephyr/sys/__assert.h>
#അന്തര്ഭാവ <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
സ്ഥിതം ഘടന k_spinlock poll_lock;

എണ്ണൽ POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

സ്ഥിതം പൂർണ്ണസംഖ്യ signal_poller(ഘടന k_poll_event *event, uint32_t state);
സ്ഥിതം പൂർണ്ണസംഖ്യ signal_triggered_work(ഘടന k_poll_event *event, uint32_t status);

ശൂന്യം k_poll_event_init(ഘടന k_poll_event *event, uint32_t type,
		       പൂർണ്ണസംഖ്യ mode, ശൂന്യം *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_അക്കം_TYPES)), "invalid type\n");
	__ASSERT(obj != ശൂന്യ, "must provide an object\n");

	event->poller = ശൂന്യ;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
സ്ഥിതം വരിയിൽ bool is_condition_met(ഘടന k_poll_event *event, uint32_t *state)
{
	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ K_POLL_TYPE_SEM_AVAILABLE:
		എങ്കിൽ (k_sem_എണ്ണം_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			തിരികെ true;
		}
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_DATA_AVAILABLE:
		എങ്കിൽ (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			തിരികെ true;
		}
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_SIGNAL:
		എങ്കിൽ (event->സൂചന->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			തിരികെ true;
		}
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		എങ്കിൽ (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			തിരികെ true;
		}
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		എങ്കിൽ (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			തിരികെ true;
		}
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_IGNORE:
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		നിർത്തുക;
	}

	തിരികെ false;
}

സ്ഥിതം ഘടന k_thread *poller_thread(ഘടന z_poller *p)
{
	തിരികെ p ? CONTAINER_OF(p, ഘടന k_thread, poller) : ശൂന്യ;
}

സ്ഥിതം വരിയിൽ ശൂന്യം add_event(sys_dlist_t *events, ഘടന k_poll_event *event,
			     ഘടന z_poller *poller)
{
	ഘടന k_poll_event *pending;

	pending = (ഘടന k_poll_event *)sys_dlist_peek_tail(events);
	എങ്കിൽ ((pending == ശൂന്യ) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->കണ്ണി_2);
		തിരികെ;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, കണ്ണി_2) {
		എങ്കിൽ (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_തിരുകുക(&pending->കണ്ണി_2, &event->കണ്ണി_2);
			തിരികെ;
		}
	}

	sys_dlist_append(events, &event->കണ്ണി_2);
}

/* must be called with interrupts locked */
സ്ഥിതം വരിയിൽ ശൂന്യം register_event(ഘടന k_poll_event *event,
				 ഘടന z_poller *poller)
{
	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ശൂന്യ, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != ശൂന്യ, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->സൂചന != ശൂന്യ, "invalid poll signal\n");
		add_event(&event->സൂചന->poll_events, event, poller);
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != ശൂന്യ, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != ശൂന്യ, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		__ASSERT(false, "invalid event type\n");
		നിർത്തുക;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
സ്ഥിതം വരിയിൽ ശൂന്യം clear_event_registration(ഘടന k_poll_event *event)
{
	bool remove_event = false;

	event->poller = ശൂന്യ;

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ശൂന്യ, "invalid semaphore\n");
		remove_event = true;
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != ശൂന്യ, "invalid queue\n");
		remove_event = true;
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->സൂചന != ശൂന്യ, "invalid poll signal\n");
		remove_event = true;
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != ശൂന്യ, "invalid message queue\n");
		remove_event = true;
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != ശൂന്യ, "invalid pipe\n");
		remove_event = true;
		നിർത്തുക;
	അവസ്ഥ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		__ASSERT(false, "invalid event type\n");
		നിർത്തുക;
	}
	എങ്കിൽ (remove_event && sys_dnode_is_linked(&event->കണ്ണി_2)) {
		sys_dlist_remove(&event->കണ്ണി_2);
	}
}

/* must be called with interrupts locked */
സ്ഥിതം വരിയിൽ ശൂന്യം clear_event_registrations(ഘടന k_poll_event *events,
					      പൂർണ്ണസംഖ്യ അക്കം_events,
					      k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	ആകുമ്പോൾ (അക്കം_events--) {
		clear_event_registration(&events[അക്കം_events]);
		k_spin_unlock(&poll_lock, താക്കോൽ);
		താക്കോൽ = k_spin_lock(&poll_lock);
	}
}

സ്ഥിതം വരിയിൽ ശൂന്യം set_event_ready(ഘടന k_poll_event *event, uint32_t state)
{
	event->poller = ശൂന്യ;
	event->state |= state;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ register_events(ഘടന k_poll_event *events,
				  പൂർണ്ണസംഖ്യ അക്കം_events,
				  ഘടന z_poller *poller,
				  bool just_check)
{
	പൂർണ്ണസംഖ്യ events_registered = 0;

	വേണ്ടി (പൂർണ്ണസംഖ്യ ii = 0; ii < അക്കം_events; ii++) {
		k_spinlock_താക്കോൽ_t താക്കോൽ;
		uint32_t state;

		താക്കോൽ = k_spin_lock(&poll_lock);
		എങ്കിൽ (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} അല്ലെങ്കിൽ എങ്കിൽ (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} അല്ലെങ്കിൽ {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, താക്കോൽ);
	}

	തിരികെ events_registered;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ signal_poller(ഘടന k_poll_event *event, uint32_t state)
{
	ഘടന k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != ശൂന്യ, "poller should have a thread\n");

	എങ്കിൽ (!z_is_thread_pending(thread)) {
		തിരികെ 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_മൂല്യം_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	എങ്കിൽ (!z_is_thread_ready(thread)) {
		തിരികെ 0;
	}

	z_ready_thread(thread);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_impl_k_poll(ഘടന k_poll_event *events, പൂർണ്ണസംഖ്യ അക്കം_events,
		  k_timeout_t timeout)
{
	പൂർണ്ണസംഖ്യ events_registered;
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	ഘടന z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != ശൂന്യ, "NULL events\n");
	__ASSERT(അക്കം_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, അക്കം_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	താക്കോൽ = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	എങ്കിൽ (!poller->is_polling) {
		clear_event_registrations(events, events_registered, താക്കോൽ);
		k_spin_unlock(&poll_lock, താക്കോൽ);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		തിരികെ 0;
	}

	poller->is_polling = false;

	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, താക്കോൽ);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		തിരികെ -EAGAIN;
	}

	സ്ഥിതം _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	പൂർണ്ണസംഖ്യ swap_rc = z_pend_curr(&poll_lock, താക്കോൽ, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	താക്കോൽ = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, താക്കോൽ);
	k_spin_unlock(&poll_lock, താക്കോൽ);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	തിരികെ swap_rc;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_poll(ഘടന k_poll_event *events,
				പൂർണ്ണസംഖ്യ അക്കം_events, k_timeout_t timeout)
{
	പൂർണ്ണസംഖ്യ ret;
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	ഘടന k_poll_event *events_copy = ശൂന്യ;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	എങ്കിൽ (K_SYSCALL_VERIFY(അക്കം_events >= 0)) {
		ret = -EINVAL;
		പോകുക out;
	}
	എങ്കിൽ (K_SYSCALL_VERIFY_സന്ദേശം(!u32_mul_overflow(അക്കം_events,
						   വലുപ്പം(ഘടന k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		പോകുക out;
	}
	events_copy = z_thread_malloc(bounds);
	എങ്കിൽ (!events_copy) {
		ret = -ENOMEM;
		പോകുക out;
	}

	എങ്കിൽ (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		പോകുക oops_free;
	}

	താക്കോൽ = k_spin_lock(&poll_lock);
	(ശൂന്യം)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, താക്കോൽ);

	/* Validate what's inside events_copy */
	വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < അക്കം_events; i++) {
		ഘടന k_poll_event *e = &events_copy[i];

		എങ്കിൽ (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			പോകുക out_free;
		}

		തിരഞ്ഞെടുപ്പ് (e->type) {
		അവസ്ഥ K_POLL_TYPE_IGNORE:
			നിർത്തുക;
		അവസ്ഥ K_POLL_TYPE_SIGNAL:
			എങ്കിൽ (K_SYSCALL_OBJ(e->സൂചന, K_OBJ_POLL_SIGNAL)) {
				പോകുക oops_free;
			}
			നിർത്തുക;
		അവസ്ഥ K_POLL_TYPE_SEM_AVAILABLE:
			എങ്കിൽ (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				പോകുക oops_free;
			}
			നിർത്തുക;
		അവസ്ഥ K_POLL_TYPE_DATA_AVAILABLE:
			എങ്കിൽ (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				പോകുക oops_free;
			}
			നിർത്തുക;
		അവസ്ഥ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			എങ്കിൽ (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				പോകുക oops_free;
			}
			നിർത്തുക;
		അവസ്ഥ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			എങ്കിൽ (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				പോകുക oops_free;
			}
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			ret = -EINVAL;
			പോകുക out_free;
		}
	}

	ret = k_poll(events_copy, അക്കം_events, timeout);
	(ശൂന്യം)memcpy((ശൂന്യം *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	തിരികെ ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
സ്ഥിതം പൂർണ്ണസംഖ്യ signal_poll_event(ഘടന k_poll_event *event, uint32_t state)
{
	ഘടന z_poller *poller = event->poller;
	പൂർണ്ണസംഖ്യ retcode = 0;

	എങ്കിൽ (poller != ശൂന്യ) {
		എങ്കിൽ (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} അല്ലെങ്കിൽ എങ്കിൽ (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} അല്ലെങ്കിൽ {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		എങ്കിൽ (retcode < 0) {
			തിരികെ retcode;
		}
	}

	set_event_ready(event, state);
	തിരികെ retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	ഘടന k_poll_event *poll_event;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&poll_lock);

	poll_event = (ഘടന k_poll_event *)sys_dlist_get(events);
	എങ്കിൽ (poll_event != ശൂന്യ) {
		(ശൂന്യം) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, താക്കോൽ);

	തിരികെ (poll_event != ശൂന്യ);
}

ശൂന്യം z_impl_k_poll_signal_init(ഘടന k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_poll_signal_init(ഘടന k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_impl_k_poll_signal_reset(ഘടന k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

ശൂന്യം z_impl_k_poll_signal_check(ഘടന k_poll_signal *sig,
			       unsigned പൂർണ്ണസംഖ്യ *signaled, പൂർണ്ണസംഖ്യ *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
ശൂന്യം z_vrfy_k_poll_signal_check(ഘടന k_poll_signal *sig,
			       unsigned പൂർണ്ണസംഖ്യ *signaled, പൂർണ്ണസംഖ്യ *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, വലുപ്പം(unsigned പൂർണ്ണസംഖ്യ)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, വലുപ്പം(പൂർണ്ണസംഖ്യ)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_poll_signal_raise(ഘടന k_poll_signal *sig, പൂർണ്ണസംഖ്യ result)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&poll_lock);
	ഘടന k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (ഘടന k_poll_event *)sys_dlist_get(&sig->poll_events);
	എങ്കിൽ (poll_event == ശൂന്യ) {
		k_spin_unlock(&poll_lock, താക്കോൽ);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		തിരികെ 0;
	}

	പൂർണ്ണസംഖ്യ rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, താക്കോൽ);
	തിരികെ rc;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_poll_signal_raise(ഘടന k_poll_signal *sig,
					     പൂർണ്ണസംഖ്യ result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	തിരികെ z_impl_k_poll_signal_raise(sig, result);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_poll_signal_reset(ഘടന k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

സ്ഥിതം ശൂന്യം triggered_work_handler(ഘടന k_work *work)
{
	ഘടന k_work_poll *twork =
			CONTAINER_OF(work, ഘടന k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	എങ്കിൽ (twork->poller.mode != MODE_NONE) {
		k_spinlock_താക്കോൽ_t താക്കോൽ;

		താക്കോൽ = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->അക്കം_events, താക്കോൽ);
		k_spin_unlock(&poll_lock, താക്കോൽ);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = ശൂന്യ;
	twork->real_handler(work);
}

ബാഹ്യ പൂർണ്ണസംഖ്യ z_work_submit_to_queue(ഘടന k_work_q *queue,
			 ഘടന k_work *work);

സ്ഥിതം ശൂന്യം triggered_work_expiration_handler(ഘടന _timeout *timeout)
{
	ഘടന k_work_poll *twork =
		CONTAINER_OF(timeout, ഘടന k_work_poll, timeout);
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&poll_lock);

	എങ്കിൽ (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, താക്കോൽ);
		തിരികെ;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, താക്കോൽ);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ signal_triggered_work(ഘടന k_poll_event *event, uint32_t status)
{
	ഘടന z_poller *poller = event->poller;
	ഘടന k_work_poll *twork =
		CONTAINER_OF(poller, ഘടന k_work_poll, poller);

	എങ്കിൽ (poller->is_polling && twork->workq != ശൂന്യ) {
		ഘടന k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(ശൂന്യം)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ triggered_work_cancel(ഘടന k_work_poll *work,
				 k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	/* Check if the work waits for event. */
	എങ്കിൽ (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		ആകുമ്പോൾ (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, താക്കോൽ);
			താക്കോൽ = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->അക്കം_events, താക്കോൽ);
		work->workq = ശൂന്യ;
		തിരികെ 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	തിരികെ -EINVAL;
}

ശൂന്യം k_work_poll_init(ഘടന k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (ഘടന k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

പൂർണ്ണസംഖ്യ k_work_poll_submit_to_queue(ഘടന k_work_q *work_q,
				ഘടന k_work_poll *work,
				ഘടന k_poll_event *events,
				പൂർണ്ണസംഖ്യ അക്കം_events,
				k_timeout_t timeout)
{
	പൂർണ്ണസംഖ്യ events_registered;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	__ASSERT(work_q != ശൂന്യ, "NULL work_q\n");
	__ASSERT(work != ശൂന്യ, "NULL work\n");
	__ASSERT(events != ശൂന്യ, "NULL events\n");
	__ASSERT(അക്കം_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	താക്കോൽ = k_spin_lock(&poll_lock);
	എങ്കിൽ (work->workq != ശൂന്യ) {
		എങ്കിൽ (work->workq == work_q) {
			പൂർണ്ണസംഖ്യ retval;

			retval = triggered_work_cancel(work, താക്കോൽ);
			എങ്കിൽ (retval < 0) {
				k_spin_unlock(&poll_lock, താക്കോൽ);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				തിരികെ retval;
			}
		} അല്ലെങ്കിൽ {
			k_spin_unlock(&poll_lock, താക്കോൽ);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			തിരികെ -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, താക്കോൽ);

	/* Save list of events. */
	work->events = events;
	work->അക്കം_events = അക്കം_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, അക്കം_events,
					    &work->poller, false);

	താക്കോൽ = k_spin_lock(&poll_lock);
	എങ്കിൽ (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(അക്കം_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, താക്കോൽ);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		തിരികെ 0;
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
	എങ്കിൽ (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} അല്ലെങ്കിൽ {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, താക്കോൽ);
	k_spin_unlock(&poll_lock, താക്കോൽ);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ k_work_poll_submit(ഘടന k_work_poll *work,
				     ഘടന k_poll_event *events,
				     പൂർണ്ണസംഖ്യ അക്കം_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	പൂർണ്ണസംഖ്യ ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, അക്കം_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_work_poll_cancel(ഘടന k_work_poll *work)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	എങ്കിൽ (work == ശൂന്യ || work->workq == ശൂന്യ) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		തിരികെ -EINVAL;
	}

	താക്കോൽ = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, താക്കോൽ);
	k_spin_unlock(&poll_lock, താക്കോൽ);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	തിരികെ retval;
}
