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

#شامل <zephyr/kernel.h>
#شامل <kernel_internal.h>
#شامل <wait_q.h>
#شامل <ksched.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/sys/dlist.h>
#شامل <zephyr/sys/util.h>
#شامل <zephyr/sys/__assert.h>
#شامل <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
ایستا ساختار k_spinlock poll_lock;

شمارش POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

ایستا صحیح signal_poller(ساختار k_poll_event *event, uint32_t state);
ایستا صحیح signal_triggered_work(ساختار k_poll_event *event, uint32_t status);

تهی k_poll_event_init(ساختار k_poll_event *event, uint32_t type,
		       صحیح mode, تهی *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_رقم_TYPES)), "invalid type\n");
	__ASSERT(obj != پوچ, "must provide an object\n");

	event->poller = پوچ;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
ایستا درونخطی bool is_condition_met(ساختار k_poll_event *event, uint32_t *state)
{
	انتخاب (event->type) {
	حالت K_POLL_TYPE_SEM_AVAILABLE:
		اگر (k_sem_شمار_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			بازگشت true;
		}
		بشکن;
	حالت K_POLL_TYPE_DATA_AVAILABLE:
		اگر (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			بازگشت true;
		}
		بشکن;
	حالت K_POLL_TYPE_SIGNAL:
		اگر (event->سیگنال->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			بازگشت true;
		}
		بشکن;
	حالت K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		اگر (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			بازگشت true;
		}
		بشکن;
	حالت K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		اگر (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			بازگشت true;
		}
		بشکن;
	حالت K_POLL_TYPE_IGNORE:
		بشکن;
	پیشفرض:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		بشکن;
	}

	بازگشت false;
}

ایستا ساختار k_thread *poller_thread(ساختار z_poller *p)
{
	بازگشت p ? CONTAINER_OF(p, ساختار k_thread, poller) : پوچ;
}

ایستا درونخطی تهی add_event(sys_dlist_t *events, ساختار k_poll_event *event,
			     ساختار z_poller *poller)
{
	ساختار k_poll_event *pending;

	pending = (ساختار k_poll_event *)sys_dlist_peek_tail(events);
	اگر ((pending == پوچ) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->گره_2);
		بازگشت;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, گره_2) {
		اگر (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_درج(&pending->گره_2, &event->گره_2);
			بازگشت;
		}
	}

	sys_dlist_append(events, &event->گره_2);
}

/* must be called with interrupts locked */
ایستا درونخطی تهی register_event(ساختار k_poll_event *event,
				 ساختار z_poller *poller)
{
	انتخاب (event->type) {
	حالت K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != پوچ, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		بشکن;
	حالت K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != پوچ, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		بشکن;
	حالت K_POLL_TYPE_SIGNAL:
		__ASSERT(event->سیگنال != پوچ, "invalid poll signal\n");
		add_event(&event->سیگنال->poll_events, event, poller);
		بشکن;
	حالت K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != پوچ, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		بشکن;
	حالت K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != پوچ, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		بشکن;
	حالت K_POLL_TYPE_IGNORE:
		/* nothing to do */
		بشکن;
	پیشفرض:
		__ASSERT(false, "invalid event type\n");
		بشکن;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
ایستا درونخطی تهی clear_event_registration(ساختار k_poll_event *event)
{
	bool remove_event = false;

	event->poller = پوچ;

	انتخاب (event->type) {
	حالت K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != پوچ, "invalid semaphore\n");
		remove_event = true;
		بشکن;
	حالت K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != پوچ, "invalid queue\n");
		remove_event = true;
		بشکن;
	حالت K_POLL_TYPE_SIGNAL:
		__ASSERT(event->سیگنال != پوچ, "invalid poll signal\n");
		remove_event = true;
		بشکن;
	حالت K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != پوچ, "invalid message queue\n");
		remove_event = true;
		بشکن;
	حالت K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != پوچ, "invalid pipe\n");
		remove_event = true;
		بشکن;
	حالت K_POLL_TYPE_IGNORE:
		/* nothing to do */
		بشکن;
	پیشفرض:
		__ASSERT(false, "invalid event type\n");
		بشکن;
	}
	اگر (remove_event && sys_dnode_is_linked(&event->گره_2)) {
		sys_dlist_remove(&event->گره_2);
	}
}

/* must be called with interrupts locked */
ایستا درونخطی تهی clear_event_registrations(ساختار k_poll_event *events,
					      صحیح رقم_events,
					      k_spinlock_کلید_t کلید)
{
	وقتی (رقم_events--) {
		clear_event_registration(&events[رقم_events]);
		k_spin_unlock(&poll_lock, کلید);
		کلید = k_spin_lock(&poll_lock);
	}
}

ایستا درونخطی تهی set_event_ready(ساختار k_poll_event *event, uint32_t state)
{
	event->poller = پوچ;
	event->state |= state;
}

ایستا درونخطی صحیح register_events(ساختار k_poll_event *events,
				  صحیح رقم_events,
				  ساختار z_poller *poller,
				  bool just_check)
{
	صحیح events_registered = 0;

	برای (صحیح ii = 0; ii < رقم_events; ii++) {
		k_spinlock_کلید_t کلید;
		uint32_t state;

		کلید = k_spin_lock(&poll_lock);
		اگر (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} وگرنه اگر (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} وگرنه {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, کلید);
	}

	بازگشت events_registered;
}

ایستا صحیح signal_poller(ساختار k_poll_event *event, uint32_t state)
{
	ساختار k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != پوچ, "poller should have a thread\n");

	اگر (!z_is_thread_pending(thread)) {
		بازگشت 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_مقدار_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	اگر (!z_is_thread_ready(thread)) {
		بازگشت 0;
	}

	z_ready_thread(thread);

	بازگشت 0;
}

صحیح z_impl_k_poll(ساختار k_poll_event *events, صحیح رقم_events,
		  k_timeout_t timeout)
{
	صحیح events_registered;
	k_spinlock_کلید_t کلید;
	ساختار z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != پوچ, "NULL events\n");
	__ASSERT(رقم_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, رقم_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	کلید = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	اگر (!poller->is_polling) {
		clear_event_registrations(events, events_registered, کلید);
		k_spin_unlock(&poll_lock, کلید);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		بازگشت 0;
	}

	poller->is_polling = false;

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, کلید);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		بازگشت -EAGAIN;
	}

	ایستا _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	صحیح swap_rc = z_pend_curr(&poll_lock, کلید, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	کلید = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, کلید);
	k_spin_unlock(&poll_lock, کلید);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	بازگشت swap_rc;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_poll(ساختار k_poll_event *events,
				صحیح رقم_events, k_timeout_t timeout)
{
	صحیح ret;
	k_spinlock_کلید_t کلید;
	ساختار k_poll_event *events_copy = پوچ;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	اگر (K_SYSCALL_VERIFY(رقم_events >= 0)) {
		ret = -EINVAL;
		برو out;
	}
	اگر (K_SYSCALL_VERIFY_پیام(!u32_mul_overflow(رقم_events,
						   اندازه(ساختار k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		برو out;
	}
	events_copy = z_thread_malloc(bounds);
	اگر (!events_copy) {
		ret = -ENOMEM;
		برو out;
	}

	اگر (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		برو oops_free;
	}

	کلید = k_spin_lock(&poll_lock);
	(تهی)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, کلید);

	/* Validate what's inside events_copy */
	برای (صحیح i = 0; i < رقم_events; i++) {
		ساختار k_poll_event *e = &events_copy[i];

		اگر (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			برو out_free;
		}

		انتخاب (e->type) {
		حالت K_POLL_TYPE_IGNORE:
			بشکن;
		حالت K_POLL_TYPE_SIGNAL:
			اگر (K_SYSCALL_OBJ(e->سیگنال, K_OBJ_POLL_SIGNAL)) {
				برو oops_free;
			}
			بشکن;
		حالت K_POLL_TYPE_SEM_AVAILABLE:
			اگر (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				برو oops_free;
			}
			بشکن;
		حالت K_POLL_TYPE_DATA_AVAILABLE:
			اگر (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				برو oops_free;
			}
			بشکن;
		حالت K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			اگر (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				برو oops_free;
			}
			بشکن;
		حالت K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			اگر (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				برو oops_free;
			}
			بشکن;
		پیشفرض:
			ret = -EINVAL;
			برو out_free;
		}
	}

	ret = k_poll(events_copy, رقم_events, timeout);
	(تهی)memcpy((تهی *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	بازگشت ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#شامل <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
ایستا صحیح signal_poll_event(ساختار k_poll_event *event, uint32_t state)
{
	ساختار z_poller *poller = event->poller;
	صحیح retcode = 0;

	اگر (poller != پوچ) {
		اگر (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} وگرنه اگر (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} وگرنه {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		اگر (retcode < 0) {
			بازگشت retcode;
		}
	}

	set_event_ready(event, state);
	بازگشت retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	ساختار k_poll_event *poll_event;
	k_spinlock_کلید_t کلید = k_spin_lock(&poll_lock);

	poll_event = (ساختار k_poll_event *)sys_dlist_get(events);
	اگر (poll_event != پوچ) {
		(تهی) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, کلید);

	بازگشت (poll_event != پوچ);
}

تهی z_impl_k_poll_signal_init(ساختار k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_poll_signal_init(ساختار k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#شامل <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_impl_k_poll_signal_reset(ساختار k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

تهی z_impl_k_poll_signal_check(ساختار k_poll_signal *sig,
			       unsigned صحیح *signaled, صحیح *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
تهی z_vrfy_k_poll_signal_check(ساختار k_poll_signal *sig,
			       unsigned صحیح *signaled, صحیح *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, اندازه(unsigned صحیح)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, اندازه(صحیح)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#شامل <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_poll_signal_raise(ساختار k_poll_signal *sig, صحیح result)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&poll_lock);
	ساختار k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (ساختار k_poll_event *)sys_dlist_get(&sig->poll_events);
	اگر (poll_event == پوچ) {
		k_spin_unlock(&poll_lock, کلید);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		بازگشت 0;
	}

	صحیح rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, کلید);
	بازگشت rc;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_poll_signal_raise(ساختار k_poll_signal *sig,
					     صحیح result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	بازگشت z_impl_k_poll_signal_raise(sig, result);
}
#شامل <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

ایستا درونخطی تهی z_vrfy_k_poll_signal_reset(ساختار k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#شامل <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

ایستا تهی triggered_work_handler(ساختار k_work *work)
{
	ساختار k_work_poll *twork =
			CONTAINER_OF(work, ساختار k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	اگر (twork->poller.mode != MODE_NONE) {
		k_spinlock_کلید_t کلید;

		کلید = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->رقم_events, کلید);
		k_spin_unlock(&poll_lock, کلید);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = پوچ;
	twork->real_handler(work);
}

بیرونی صحیح z_work_submit_to_queue(ساختار k_work_q *queue,
			 ساختار k_work *work);

ایستا تهی triggered_work_expiration_handler(ساختار _timeout *timeout)
{
	ساختار k_work_poll *twork =
		CONTAINER_OF(timeout, ساختار k_work_poll, timeout);
	k_spinlock_کلید_t کلید = k_spin_lock(&poll_lock);

	اگر (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, کلید);
		بازگشت;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, کلید);
}

ایستا صحیح signal_triggered_work(ساختار k_poll_event *event, uint32_t status)
{
	ساختار z_poller *poller = event->poller;
	ساختار k_work_poll *twork =
		CONTAINER_OF(poller, ساختار k_work_poll, poller);

	اگر (poller->is_polling && twork->workq != پوچ) {
		ساختار k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(تهی)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	بازگشت 0;
}

ایستا صحیح triggered_work_cancel(ساختار k_work_poll *work,
				 k_spinlock_کلید_t کلید)
{
	/* Check if the work waits for event. */
	اگر (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		وقتی (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, کلید);
			کلید = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->رقم_events, کلید);
		work->workq = پوچ;
		بازگشت 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	بازگشت -EINVAL;
}

تهی k_work_poll_init(ساختار k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (ساختار k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

صحیح k_work_poll_submit_to_queue(ساختار k_work_q *work_q,
				ساختار k_work_poll *work,
				ساختار k_poll_event *events,
				صحیح رقم_events,
				k_timeout_t timeout)
{
	صحیح events_registered;
	k_spinlock_کلید_t کلید;

	__ASSERT(work_q != پوچ, "NULL work_q\n");
	__ASSERT(work != پوچ, "NULL work\n");
	__ASSERT(events != پوچ, "NULL events\n");
	__ASSERT(رقم_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	کلید = k_spin_lock(&poll_lock);
	اگر (work->workq != پوچ) {
		اگر (work->workq == work_q) {
			صحیح retval;

			retval = triggered_work_cancel(work, کلید);
			اگر (retval < 0) {
				k_spin_unlock(&poll_lock, کلید);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				بازگشت retval;
			}
		} وگرنه {
			k_spin_unlock(&poll_lock, کلید);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			بازگشت -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, کلید);

	/* Save list of events. */
	work->events = events;
	work->رقم_events = رقم_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, رقم_events,
					    &work->poller, false);

	کلید = k_spin_lock(&poll_lock);
	اگر (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(رقم_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, کلید);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		بازگشت 0;
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
	اگر (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} وگرنه {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, کلید);
	k_spin_unlock(&poll_lock, کلید);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	بازگشت 0;
}

صحیح k_work_poll_submit(ساختار k_work_poll *work,
				     ساختار k_poll_event *events,
				     صحیح رقم_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	صحیح ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, رقم_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	بازگشت ret;
}

صحیح k_work_poll_cancel(ساختار k_work_poll *work)
{
	k_spinlock_کلید_t کلید;
	صحیح retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	اگر (work == پوچ || work->workq == پوچ) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		بازگشت -EINVAL;
	}

	کلید = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, کلید);
	k_spin_unlock(&poll_lock, کلید);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	بازگشت retval;
}
