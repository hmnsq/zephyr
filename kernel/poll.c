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
স্থির কাঠামো k_spinlock poll_lock;

গণনা POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

স্থির পূর্ণসংখ্যা signal_poller(কাঠামো k_poll_event *event, uint32_t state);
স্থির পূর্ণসংখ্যা signal_triggered_work(কাঠামো k_poll_event *event, uint32_t status);

শূন্য k_poll_event_init(কাঠামো k_poll_event *event, uint32_t type,
		       পূর্ণসংখ্যা mode, শূন্য *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_অঙ্ক_TYPES)), "invalid type\n");
	__ASSERT(obj != ফাঁকা, "must provide an object\n");

	event->poller = ফাঁকা;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
স্থির অন্তঃরেখ bool is_condition_met(কাঠামো k_poll_event *event, uint32_t *state)
{
	নির্বাচন (event->type) {
	ক্ষেত্র K_POLL_TYPE_SEM_AVAILABLE:
		যদি (k_sem_গুনতি_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			ফেরত true;
		}
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_তথ্য_AVAILABLE:
		যদি (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_তথ্য_AVAILABLE;
			ফেরত true;
		}
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_SIGNAL:
		যদি (event->সংকেত->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			ফেরত true;
		}
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_MSGQ_তথ্য_AVAILABLE:
		যদি (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_তথ্য_AVAILABLE;
			ফেরত true;
		}
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_PIPE_তথ্য_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		যদি (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_তথ্য_AVAILABLE;
			ফেরত true;
		}
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_IGNORE:
		বিরতি;
	পূর্বনির্ধারিত:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		বিরতি;
	}

	ফেরত false;
}

স্থির কাঠামো k_thread *poller_thread(কাঠামো z_poller *p)
{
	ফেরত p ? CONTAINER_OF(p, কাঠামো k_thread, poller) : ফাঁকা;
}

স্থির অন্তঃরেখ শূন্য add_event(sys_dlist_t *events, কাঠামো k_poll_event *event,
			     কাঠামো z_poller *poller)
{
	কাঠামো k_poll_event *pending;

	pending = (কাঠামো k_poll_event *)sys_dlist_peek_লেজ(events);
	যদি ((pending == ফাঁকা) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->গ্রন্থি_2);
		ফেরত;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, গ্রন্থি_2) {
		যদি (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_ঢোকাও(&pending->গ্রন্থি_2, &event->গ্রন্থি_2);
			ফেরত;
		}
	}

	sys_dlist_append(events, &event->গ্রন্থি_2);
}

/* must be called with interrupts locked */
স্থির অন্তঃরেখ শূন্য register_event(কাঠামো k_poll_event *event,
				 কাঠামো z_poller *poller)
{
	নির্বাচন (event->type) {
	ক্ষেত্র K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ফাঁকা, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_তথ্য_AVAILABLE:
		__ASSERT(event->queue != ফাঁকা, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_SIGNAL:
		__ASSERT(event->সংকেত != ফাঁকা, "invalid poll signal\n");
		add_event(&event->সংকেত->poll_events, event, poller);
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_MSGQ_তথ্য_AVAILABLE:
		__ASSERT(event->msgq != ফাঁকা, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_PIPE_তথ্য_AVAILABLE:
		__ASSERT(event->pipe != ফাঁকা, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_IGNORE:
		/* nothing to do */
		বিরতি;
	পূর্বনির্ধারিত:
		__ASSERT(false, "invalid event type\n");
		বিরতি;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
স্থির অন্তঃরেখ শূন্য clear_event_registration(কাঠামো k_poll_event *event)
{
	bool remove_event = false;

	event->poller = ফাঁকা;

	নির্বাচন (event->type) {
	ক্ষেত্র K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ফাঁকা, "invalid semaphore\n");
		remove_event = true;
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_তথ্য_AVAILABLE:
		__ASSERT(event->queue != ফাঁকা, "invalid queue\n");
		remove_event = true;
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_SIGNAL:
		__ASSERT(event->সংকেত != ফাঁকা, "invalid poll signal\n");
		remove_event = true;
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_MSGQ_তথ্য_AVAILABLE:
		__ASSERT(event->msgq != ফাঁকা, "invalid message queue\n");
		remove_event = true;
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_PIPE_তথ্য_AVAILABLE:
		__ASSERT(event->pipe != ফাঁকা, "invalid pipe\n");
		remove_event = true;
		বিরতি;
	ক্ষেত্র K_POLL_TYPE_IGNORE:
		/* nothing to do */
		বিরতি;
	পূর্বনির্ধারিত:
		__ASSERT(false, "invalid event type\n");
		বিরতি;
	}
	যদি (remove_event && sys_dnode_is_linked(&event->গ্রন্থি_2)) {
		sys_dlist_remove(&event->গ্রন্থি_2);
	}
}

/* must be called with interrupts locked */
স্থির অন্তঃরেখ শূন্য clear_event_registrations(কাঠামো k_poll_event *events,
					      পূর্ণসংখ্যা অঙ্ক_events,
					      k_spinlock_চাবি_t চাবি)
{
	যখন (অঙ্ক_events--) {
		clear_event_registration(&events[অঙ্ক_events]);
		k_spin_unlock(&poll_lock, চাবি);
		চাবি = k_spin_lock(&poll_lock);
	}
}

স্থির অন্তঃরেখ শূন্য set_event_ready(কাঠামো k_poll_event *event, uint32_t state)
{
	event->poller = ফাঁকা;
	event->state |= state;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা register_events(কাঠামো k_poll_event *events,
				  পূর্ণসংখ্যা অঙ্ক_events,
				  কাঠামো z_poller *poller,
				  bool just_check)
{
	পূর্ণসংখ্যা events_registered = 0;

	জন্য (পূর্ণসংখ্যা ii = 0; ii < অঙ্ক_events; ii++) {
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

	ফেরত events_registered;
}

স্থির পূর্ণসংখ্যা signal_poller(কাঠামো k_poll_event *event, uint32_t state)
{
	কাঠামো k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != ফাঁকা, "poller should have a thread\n");

	যদি (!z_is_thread_pending(thread)) {
		ফেরত 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_মান_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	যদি (!z_is_thread_ready(thread)) {
		ফেরত 0;
	}

	z_ready_thread(thread);

	ফেরত 0;
}

পূর্ণসংখ্যা z_impl_k_poll(কাঠামো k_poll_event *events, পূর্ণসংখ্যা অঙ্ক_events,
		  k_timeout_t timeout)
{
	পূর্ণসংখ্যা events_registered;
	k_spinlock_চাবি_t চাবি;
	কাঠামো z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != ফাঁকা, "NULL events\n");
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

		ফেরত 0;
	}

	poller->is_polling = false;

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, চাবি);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		ফেরত -EAGAIN;
	}

	স্থির _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	পূর্ণসংখ্যা swap_rc = z_pend_curr(&poll_lock, চাবি, &wait_q, timeout);

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

	ফেরত swap_rc;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_poll(কাঠামো k_poll_event *events,
				পূর্ণসংখ্যা অঙ্ক_events, k_timeout_t timeout)
{
	পূর্ণসংখ্যা ret;
	k_spinlock_চাবি_t চাবি;
	কাঠামো k_poll_event *events_copy = ফাঁকা;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	যদি (K_SYSCALL_VERIFY(অঙ্ক_events >= 0)) {
		ret = -EINVAL;
		যাও out;
	}
	যদি (K_SYSCALL_VERIFY_বার্তা(!u32_mul_overflow(অঙ্ক_events,
						   আকার(কাঠামো k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		যাও out;
	}
	events_copy = z_thread_malloc(bounds);
	যদি (!events_copy) {
		ret = -ENOMEM;
		যাও out;
	}

	যদি (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		যাও oops_free;
	}

	চাবি = k_spin_lock(&poll_lock);
	(শূন্য)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, চাবি);

	/* Validate what's inside events_copy */
	জন্য (পূর্ণসংখ্যা i = 0; i < অঙ্ক_events; i++) {
		কাঠামো k_poll_event *e = &events_copy[i];

		যদি (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			যাও out_free;
		}

		নির্বাচন (e->type) {
		ক্ষেত্র K_POLL_TYPE_IGNORE:
			বিরতি;
		ক্ষেত্র K_POLL_TYPE_SIGNAL:
			যদি (K_SYSCALL_OBJ(e->সংকেত, K_OBJ_POLL_SIGNAL)) {
				যাও oops_free;
			}
			বিরতি;
		ক্ষেত্র K_POLL_TYPE_SEM_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				যাও oops_free;
			}
			বিরতি;
		ক্ষেত্র K_POLL_TYPE_তথ্য_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				যাও oops_free;
			}
			বিরতি;
		ক্ষেত্র K_POLL_TYPE_MSGQ_তথ্য_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				যাও oops_free;
			}
			বিরতি;
		ক্ষেত্র K_POLL_TYPE_PIPE_তথ্য_AVAILABLE:
			যদি (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				যাও oops_free;
			}
			বিরতি;
		পূর্বনির্ধারিত:
			ret = -EINVAL;
			যাও out_free;
		}
	}

	ret = k_poll(events_copy, অঙ্ক_events, timeout);
	(শূন্য)memcpy((শূন্য *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	ফেরত ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
স্থির পূর্ণসংখ্যা signal_poll_event(কাঠামো k_poll_event *event, uint32_t state)
{
	কাঠামো z_poller *poller = event->poller;
	পূর্ণসংখ্যা retcode = 0;

	যদি (poller != ফাঁকা) {
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
			ফেরত retcode;
		}
	}

	set_event_ready(event, state);
	ফেরত retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	কাঠামো k_poll_event *poll_event;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&poll_lock);

	poll_event = (কাঠামো k_poll_event *)sys_dlist_get(events);
	যদি (poll_event != ফাঁকা) {
		(শূন্য) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, চাবি);

	ফেরত (poll_event != ফাঁকা);
}

শূন্য z_impl_k_poll_signal_init(কাঠামো k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ শূন্য z_vrfy_k_poll_signal_init(কাঠামো k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_poll_signal_reset(কাঠামো k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

শূন্য z_impl_k_poll_signal_check(কাঠামো k_poll_signal *sig,
			       unsigned পূর্ণসংখ্যা *signaled, পূর্ণসংখ্যা *ফলাফল)
{
	*signaled = sig->signaled;
	*ফলাফল = sig->ফলাফল;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
শূন্য z_vrfy_k_poll_signal_check(কাঠামো k_poll_signal *sig,
			       unsigned পূর্ণসংখ্যা *signaled, পূর্ণসংখ্যা *ফলাফল)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, আকার(unsigned পূর্ণসংখ্যা)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(ফলাফল, আকার(পূর্ণসংখ্যা)));
	z_impl_k_poll_signal_check(sig, signaled, ফলাফল);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_poll_signal_raise(কাঠামো k_poll_signal *sig, পূর্ণসংখ্যা ফলাফল)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&poll_lock);
	কাঠামো k_poll_event *poll_event;

	sig->ফলাফল = ফলাফল;
	sig->signaled = 1U;

	poll_event = (কাঠামো k_poll_event *)sys_dlist_get(&sig->poll_events);
	যদি (poll_event == ফাঁকা) {
		k_spin_unlock(&poll_lock, চাবি);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		ফেরত 0;
	}

	পূর্ণসংখ্যা rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, চাবি);
	ফেরত rc;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_poll_signal_raise(কাঠামো k_poll_signal *sig,
					     পূর্ণসংখ্যা ফলাফল)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	ফেরত z_impl_k_poll_signal_raise(sig, ফলাফল);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

স্থির অন্তঃরেখ শূন্য z_vrfy_k_poll_signal_reset(কাঠামো k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#অন্তর্ভাব <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

স্থির শূন্য triggered_work_handler(কাঠামো k_work *work)
{
	কাঠামো k_work_poll *twork =
			CONTAINER_OF(work, কাঠামো k_work_poll, work);

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
	twork->workq = ফাঁকা;
	twork->real_handler(work);
}

বাহ্যিক পূর্ণসংখ্যা z_work_submit_to_queue(কাঠামো k_work_q *queue,
			 কাঠামো k_work *work);

স্থির শূন্য triggered_work_expiration_handler(কাঠামো _timeout *timeout)
{
	কাঠামো k_work_poll *twork =
		CONTAINER_OF(timeout, কাঠামো k_work_poll, timeout);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&poll_lock);

	যদি (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, চাবি);
		ফেরত;
	}

	twork->poller.is_polling = false;
	twork->poll_ফলাফল = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, চাবি);
}

স্থির পূর্ণসংখ্যা signal_triggered_work(কাঠামো k_poll_event *event, uint32_t status)
{
	কাঠামো z_poller *poller = event->poller;
	কাঠামো k_work_poll *twork =
		CONTAINER_OF(poller, কাঠামো k_work_poll, poller);

	যদি (poller->is_polling && twork->workq != ফাঁকা) {
		কাঠামো k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(শূন্য)z_try_abort_timeout(&twork->timeout);
		twork->poll_ফলাফল = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা triggered_work_cancel(কাঠামো k_work_poll *work,
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
		যখন (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
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
		work->workq = ফাঁকা;
		ফেরত 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	ফেরত -EINVAL;
}

শূন্য k_work_poll_init(কাঠামো k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (কাঠামো k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

পূর্ণসংখ্যা k_work_poll_submit_to_queue(কাঠামো k_work_q *work_q,
				কাঠামো k_work_poll *work,
				কাঠামো k_poll_event *events,
				পূর্ণসংখ্যা অঙ্ক_events,
				k_timeout_t timeout)
{
	পূর্ণসংখ্যা events_registered;
	k_spinlock_চাবি_t চাবি;

	__ASSERT(work_q != ফাঁকা, "NULL work_q\n");
	__ASSERT(work != ফাঁকা, "NULL work\n");
	__ASSERT(events != ফাঁকা, "NULL events\n");
	__ASSERT(অঙ্ক_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	চাবি = k_spin_lock(&poll_lock);
	যদি (work->workq != ফাঁকা) {
		যদি (work->workq == work_q) {
			পূর্ণসংখ্যা retval;

			retval = triggered_work_cancel(work, চাবি);
			যদি (retval < 0) {
				k_spin_unlock(&poll_lock, চাবি);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				ফেরত retval;
			}
		} অন্যথা {
			k_spin_unlock(&poll_lock, চাবি);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			ফেরত -EADDRINUSE;
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
	work->poll_ফলাফল = -EINPROGRESS;

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

		ফেরত 0;
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
		work->poll_ফলাফল = -EAGAIN;
	} অন্যথা {
		work->poll_ফলাফল = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, চাবি);
	k_spin_unlock(&poll_lock, চাবি);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	ফেরত 0;
}

পূর্ণসংখ্যা k_work_poll_submit(কাঠামো k_work_poll *work,
				     কাঠামো k_poll_event *events,
				     পূর্ণসংখ্যা অঙ্ক_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	পূর্ণসংখ্যা ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, অঙ্ক_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	ফেরত ret;
}

পূর্ণসংখ্যা k_work_poll_cancel(কাঠামো k_work_poll *work)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	যদি (work == ফাঁকা || work->workq == ফাঁকা) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		ফেরত -EINVAL;
	}

	চাবি = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, চাবি);
	k_spin_unlock(&poll_lock, চাবি);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	ফেরত retval;
}
