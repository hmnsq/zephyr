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

#ضمن <zephyr/kernel.h>
#ضمن <kernel_internal.h>
#ضمن <wait_q.h>
#ضمن <ksched.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <zephyr/sys/util.h>
#ضمن <zephyr/sys/__assert.h>
#ضمن <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
ساكن بنية k_spinlock poll_lock;

تعداد POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

ساكن صحيح signal_poller(بنية k_poll_event *event, uint32_t state);
ساكن صحيح signal_triggered_work(بنية k_poll_event *event, uint32_t status);

فراغ k_poll_event_init(بنية k_poll_event *event, uint32_t type,
		       صحيح mode, فراغ *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(_POLL_NUM_TYPES)), "invalid type\n");
	__ASSERT(obj != عدم, "must provide an object\n");

	event->poller = عدم;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
ساكن مضمن bool is_condition_met(بنية k_poll_event *event, uint32_t *state)
{
	بدل (event->type) {
	حالة K_POLL_TYPE_SEM_AVAILABLE:
		𐡄𐡍 (k_sem_count_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_DATA_AVAILABLE:
		𐡄𐡍 (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_SIGNAL:
		𐡄𐡍 (event->إشارة->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		𐡄𐡍 (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		𐡄𐡍 (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_IGNORE:
		اكسر;
	افتراضي:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		اكسر;
	}

	أرجع false;
}

ساكن بنية k_thread *poller_thread(بنية z_poller *p)
{
	أرجع p ? CONTAINER_OF(p, بنية k_thread, poller) : عدم;
}

ساكن مضمن فراغ add_event(sys_dlist_t *events, بنية k_poll_event *event,
			     بنية z_poller *poller)
{
	بنية k_poll_event *pending;

	pending = (بنية k_poll_event *)sys_dlist_peek_tail(events);
	𐡄𐡍 ((pending == عدم) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->_node);
		أرجع;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, _node) {
		𐡄𐡍 (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_insert(&pending->_node, &event->_node);
			أرجع;
		}
	}

	sys_dlist_append(events, &event->_node);
}

/* must be called with interrupts locked */
ساكن مضمن فراغ register_event(بنية k_poll_event *event,
				 بنية z_poller *poller)
{
	بدل (event->type) {
	حالة K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != عدم, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != عدم, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_SIGNAL:
		__ASSERT(event->إشارة != عدم, "invalid poll signal\n");
		add_event(&event->إشارة->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != عدم, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != عدم, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_IGNORE:
		/* nothing to do */
		اكسر;
	افتراضي:
		__ASSERT(false, "invalid event type\n");
		اكسر;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
ساكن مضمن فراغ clear_event_registration(بنية k_poll_event *event)
{
	bool remove_event = false;

	event->poller = عدم;

	بدل (event->type) {
	حالة K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != عدم, "invalid semaphore\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != عدم, "invalid queue\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_SIGNAL:
		__ASSERT(event->إشارة != عدم, "invalid poll signal\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != عدم, "invalid message queue\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != عدم, "invalid pipe\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_IGNORE:
		/* nothing to do */
		اكسر;
	افتراضي:
		__ASSERT(false, "invalid event type\n");
		اكسر;
	}
	𐡄𐡍 (remove_event && sys_dnode_is_linked(&event->_node)) {
		sys_dlist_remove(&event->_node);
	}
}

/* must be called with interrupts locked */
ساكن مضمن فراغ clear_event_registrations(بنية k_poll_event *events,
					      صحيح num_events,
					      k_spinlock_key_t key)
{
	طالما (num_events--) {
		clear_event_registration(&events[num_events]);
		k_spin_unlock(&poll_lock, key);
		key = k_spin_lock(&poll_lock);
	}
}

ساكن مضمن فراغ set_event_ready(بنية k_poll_event *event, uint32_t state)
{
	event->poller = عدم;
	event->state |= state;
}

ساكن مضمن صحيح register_events(بنية k_poll_event *events,
				  صحيح num_events,
				  بنية z_poller *poller,
				  bool just_check)
{
	صحيح events_registered = 0;

	لكل (صحيح ii = 0; ii < num_events; ii++) {
		k_spinlock_key_t key;
		uint32_t state;

		key = k_spin_lock(&poll_lock);
		𐡄𐡍 (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} وإلا 𐡄𐡍 (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} وإلا {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, key);
	}

	أرجع events_registered;
}

ساكن صحيح signal_poller(بنية k_poll_event *event, uint32_t state)
{
	بنية k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != عدم, "poller should have a thread\n");

	𐡄𐡍 (!z_is_thread_pending(thread)) {
		أرجع 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_value_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	𐡄𐡍 (!z_is_thread_ready(thread)) {
		أرجع 0;
	}

	z_ready_thread(thread);

	أرجع 0;
}

صحيح z_impl_k_poll(بنية k_poll_event *events, صحيح num_events,
		  k_timeout_t timeout)
{
	صحيح events_registered;
	k_spinlock_key_t key;
	بنية z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != عدم, "NULL events\n");
	__ASSERT(num_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, num_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	key = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	𐡄𐡍 (!poller->is_polling) {
		clear_event_registrations(events, events_registered, key);
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		أرجع 0;
	}

	poller->is_polling = false;

	𐡄𐡍 (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		أرجع -EAGAIN;
	}

	ساكن _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	صحيح swap_rc = z_pend_curr(&poll_lock, key, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	key = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, key);
	k_spin_unlock(&poll_lock, key);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	أرجع swap_rc;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_poll(بنية k_poll_event *events,
				صحيح num_events, k_timeout_t timeout)
{
	صحيح ret;
	k_spinlock_key_t key;
	بنية k_poll_event *events_copy = عدم;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	𐡄𐡍 (K_SYSCALL_VERIFY(num_events >= 0)) {
		ret = -EINVAL;
		اذهب out;
	}
	𐡄𐡍 (K_SYSCALL_VERIFY_MSG(!u32_mul_overflow(num_events,
						   حجم(بنية k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		اذهب out;
	}
	events_copy = z_thread_malloc(bounds);
	𐡄𐡍 (!events_copy) {
		ret = -ENOMEM;
		اذهب out;
	}

	𐡄𐡍 (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		اذهب oops_free;
	}

	key = k_spin_lock(&poll_lock);
	(فراغ)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, key);

	/* Validate what's inside events_copy */
	لكل (صحيح i = 0; i < num_events; i++) {
		بنية k_poll_event *e = &events_copy[i];

		𐡄𐡍 (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			اذهب out_free;
		}

		بدل (e->type) {
		حالة K_POLL_TYPE_IGNORE:
			اكسر;
		حالة K_POLL_TYPE_SIGNAL:
			𐡄𐡍 (K_SYSCALL_OBJ(e->إشارة, K_OBJ_POLL_SIGNAL)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_SEM_AVAILABLE:
			𐡄𐡍 (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_DATA_AVAILABLE:
			𐡄𐡍 (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			𐡄𐡍 (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			𐡄𐡍 (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				اذهب oops_free;
			}
			اكسر;
		افتراضي:
			ret = -EINVAL;
			اذهب out_free;
		}
	}

	ret = k_poll(events_copy, num_events, timeout);
	(فراغ)memcpy((فراغ *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	أرجع ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#ضمن <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
ساكن صحيح signal_poll_event(بنية k_poll_event *event, uint32_t state)
{
	بنية z_poller *poller = event->poller;
	صحيح retcode = 0;

	𐡄𐡍 (poller != عدم) {
		𐡄𐡍 (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} وإلا 𐡄𐡍 (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} وإلا {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		𐡄𐡍 (retcode < 0) {
			أرجع retcode;
		}
	}

	set_event_ready(event, state);
	أرجع retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	بنية k_poll_event *poll_event;
	k_spinlock_key_t key = k_spin_lock(&poll_lock);

	poll_event = (بنية k_poll_event *)sys_dlist_get(events);
	𐡄𐡍 (poll_event != عدم) {
		(فراغ) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, key);

	أرجع (poll_event != عدم);
}

فراغ z_impl_k_poll_signal_init(بنية k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_poll_signal_init(بنية k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#ضمن <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

فراغ z_impl_k_poll_signal_reset(بنية k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

فراغ z_impl_k_poll_signal_check(بنية k_poll_signal *sig,
			       unsigned صحيح *signaled, صحيح *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
فراغ z_vrfy_k_poll_signal_check(بنية k_poll_signal *sig,
			       unsigned صحيح *signaled, صحيح *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, حجم(unsigned صحيح)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, حجم(صحيح)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#ضمن <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_poll_signal_raise(بنية k_poll_signal *sig, صحيح result)
{
	k_spinlock_key_t key = k_spin_lock(&poll_lock);
	بنية k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (بنية k_poll_event *)sys_dlist_get(&sig->poll_events);
	𐡄𐡍 (poll_event == عدم) {
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		أرجع 0;
	}

	صحيح rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, key);
	أرجع rc;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_poll_signal_raise(بنية k_poll_signal *sig,
					     صحيح result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	أرجع z_impl_k_poll_signal_raise(sig, result);
}
#ضمن <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

ساكن مضمن فراغ z_vrfy_k_poll_signal_reset(بنية k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#ضمن <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

ساكن فراغ triggered_work_handler(بنية k_work *work)
{
	بنية k_work_poll *twork =
			CONTAINER_OF(work, بنية k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	𐡄𐡍 (twork->poller.mode != MODE_NONE) {
		k_spinlock_key_t key;

		key = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->num_events, key);
		k_spin_unlock(&poll_lock, key);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = عدم;
	twork->real_handler(work);
}

خارجي صحيح z_work_submit_to_queue(بنية k_work_q *queue,
			 بنية k_work *work);

ساكن فراغ triggered_work_expiration_handler(بنية _timeout *timeout)
{
	بنية k_work_poll *twork =
		CONTAINER_OF(timeout, بنية k_work_poll, timeout);
	k_spinlock_key_t key = k_spin_lock(&poll_lock);

	𐡄𐡍 (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, key);
		أرجع;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, key);
}

ساكن صحيح signal_triggered_work(بنية k_poll_event *event, uint32_t status)
{
	بنية z_poller *poller = event->poller;
	بنية k_work_poll *twork =
		CONTAINER_OF(poller, بنية k_work_poll, poller);

	𐡄𐡍 (poller->is_polling && twork->workq != عدم) {
		بنية k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(فراغ)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	أرجع 0;
}

ساكن صحيح triggered_work_cancel(بنية k_work_poll *work,
				 k_spinlock_key_t key)
{
	/* Check if the work waits for event. */
	𐡄𐡍 (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		طالما (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, key);
			key = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->num_events, key);
		work->workq = عدم;
		أرجع 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	أرجع -EINVAL;
}

فراغ k_work_poll_init(بنية k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (بنية k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

صحيح k_work_poll_submit_to_queue(بنية k_work_q *work_q,
				بنية k_work_poll *work,
				بنية k_poll_event *events,
				صحيح num_events,
				k_timeout_t timeout)
{
	صحيح events_registered;
	k_spinlock_key_t key;

	__ASSERT(work_q != عدم, "NULL work_q\n");
	__ASSERT(work != عدم, "NULL work\n");
	__ASSERT(events != عدم, "NULL events\n");
	__ASSERT(num_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	key = k_spin_lock(&poll_lock);
	𐡄𐡍 (work->workq != عدم) {
		𐡄𐡍 (work->workq == work_q) {
			صحيح retval;

			retval = triggered_work_cancel(work, key);
			𐡄𐡍 (retval < 0) {
				k_spin_unlock(&poll_lock, key);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				أرجع retval;
			}
		} وإلا {
			k_spin_unlock(&poll_lock, key);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			أرجع -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, key);

	/* Save list of events. */
	work->events = events;
	work->num_events = num_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, num_events,
					    &work->poller, false);

	key = k_spin_lock(&poll_lock);
	𐡄𐡍 (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(num_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		أرجع 0;
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
	𐡄𐡍 (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} وإلا {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, key);
	k_spin_unlock(&poll_lock, key);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	أرجع 0;
}

صحيح k_work_poll_submit(بنية k_work_poll *work,
				     بنية k_poll_event *events,
				     صحيح num_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	صحيح ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, num_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	أرجع ret;
}

صحيح k_work_poll_cancel(بنية k_work_poll *work)
{
	k_spinlock_key_t key;
	صحيح retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	𐡄𐡍 (work == عدم || work->workq == عدم) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		أرجع -EINVAL;
	}

	key = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, key);
	k_spin_unlock(&poll_lock, key);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	أرجع retval;
}
