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
ساکن ساخت k_spinlock poll_تالا;

شمار POLL_منوال { منوال_NONE, منوال_POLL, منوال_TRIGGERED };

ساکن صحیح signal_poller(ساخت k_poll_event *event, uint32_t state);
ساکن صحیح signal_triggered_work(ساخت k_poll_event *event, uint32_t حالت_2);

خالی k_poll_event_init(ساخت k_poll_event *event, uint32_t type,
		       صحیح منوال, خالی *obj)
{
	__ASSERT(منوال == K_POLL_منوال_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (ثنائی_ہندسہ(POLL_عدد_TYPES)), "invalid type\n");
	__ASSERT(obj != تہی, "must provide an object\n");

	event->poller = تہی;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->منوال = منوال;
	event->غیر_مستعمل = 0U;
	event->obj = obj;

	SYS_پورٹ_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
ساکن اندرونی bool is_condition_met(ساخت k_poll_event *event, uint32_t *state)
{
	انتخاب (event->type) {
	حالت K_POLL_TYPE_SEM_دستیاب:
		اگر (k_sem_گنتی_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_دستیاب;
			واپس true;
		}
		توڑو;
	حالت K_POLL_TYPE_ڈیٹا_دستیاب:
		اگر (!k_قطار_is_خالی_خانے(event->قطار)) {
			*state = K_POLL_STATE_FIFO_ڈیٹا_دستیاب;
			واپس true;
		}
		توڑو;
	حالت K_POLL_TYPE_SIGNAL:
		اگر (event->اشارہ->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			واپس true;
		}
		توڑو;
	حالت K_POLL_TYPE_MSGQ_ڈیٹا_دستیاب:
		اگر (event->msgq->مستعمل_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_ڈیٹا_دستیاب;
			واپس true;
		}
		توڑو;
	حالت K_POLL_TYPE_PIPE_ڈیٹا_دستیاب:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		اگر (!ring_buf_is_خالی_خانے(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_ڈیٹا_دستیاب;
			واپس true;
		}
		توڑو;
	حالت K_POLL_TYPE_IGNORE:
		توڑو;
	طےشدہ:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		توڑو;
	}

	واپس false;
}

ساکن ساخت k_thread *poller_thread(ساخت z_poller *p)
{
	واپس p ? CONTAINER_کا(p, ساخت k_thread, poller) : تہی;
}

ساکن اندرونی خالی جوڑو_event(sys_dlist_t *events, ساخت k_poll_event *event,
			     ساخت z_poller *poller)
{
	ساخت k_poll_event *pending;

	pending = (ساخت k_poll_event *)sys_dlist_peek_دم(events);
	اگر ((pending == تہی) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->گرہ_2);
		واپس;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, گرہ_2) {
		اگر (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_داخل_کرو(&pending->گرہ_2, &event->گرہ_2);
			واپس;
		}
	}

	sys_dlist_append(events, &event->گرہ_2);
}

/* must be called with interrupts locked */
ساکن اندرونی خالی رجسٹر_event(ساخت k_poll_event *event,
				 ساخت z_poller *poller)
{
	انتخاب (event->type) {
	حالت K_POLL_TYPE_SEM_دستیاب:
		__ASSERT(event->sem != تہی, "invalid semaphore\n");
		جوڑو_event(&event->sem->poll_events, event, poller);
		توڑو;
	حالت K_POLL_TYPE_ڈیٹا_دستیاب:
		__ASSERT(event->قطار != تہی, "invalid queue\n");
		جوڑو_event(&event->قطار->poll_events, event, poller);
		توڑو;
	حالت K_POLL_TYPE_SIGNAL:
		__ASSERT(event->اشارہ != تہی, "invalid poll signal\n");
		جوڑو_event(&event->اشارہ->poll_events, event, poller);
		توڑو;
	حالت K_POLL_TYPE_MSGQ_ڈیٹا_دستیاب:
		__ASSERT(event->msgq != تہی, "invalid message queue\n");
		جوڑو_event(&event->msgq->poll_events, event, poller);
		توڑو;
	حالت K_POLL_TYPE_PIPE_ڈیٹا_دستیاب:
		__ASSERT(event->pipe != تہی, "invalid pipe\n");
		جوڑو_event(&event->pipe->poll_events, event, poller);
		توڑو;
	حالت K_POLL_TYPE_IGNORE:
		/* nothing to do */
		توڑو;
	طےشدہ:
		__ASSERT(false, "invalid event type\n");
		توڑو;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
ساکن اندرونی خالی clear_event_registration(ساخت k_poll_event *event)
{
	bool remove_event = false;

	event->poller = تہی;

	انتخاب (event->type) {
	حالت K_POLL_TYPE_SEM_دستیاب:
		__ASSERT(event->sem != تہی, "invalid semaphore\n");
		remove_event = true;
		توڑو;
	حالت K_POLL_TYPE_ڈیٹا_دستیاب:
		__ASSERT(event->قطار != تہی, "invalid queue\n");
		remove_event = true;
		توڑو;
	حالت K_POLL_TYPE_SIGNAL:
		__ASSERT(event->اشارہ != تہی, "invalid poll signal\n");
		remove_event = true;
		توڑو;
	حالت K_POLL_TYPE_MSGQ_ڈیٹا_دستیاب:
		__ASSERT(event->msgq != تہی, "invalid message queue\n");
		remove_event = true;
		توڑو;
	حالت K_POLL_TYPE_PIPE_ڈیٹا_دستیاب:
		__ASSERT(event->pipe != تہی, "invalid pipe\n");
		remove_event = true;
		توڑو;
	حالت K_POLL_TYPE_IGNORE:
		/* nothing to do */
		توڑو;
	طےشدہ:
		__ASSERT(false, "invalid event type\n");
		توڑو;
	}
	اگر (remove_event && sys_dnode_is_linked(&event->گرہ_2)) {
		sys_dlist_remove(&event->گرہ_2);
	}
}

/* must be called with interrupts locked */
ساکن اندرونی خالی clear_event_registrations(ساخت k_poll_event *events,
					      صحیح عدد_events,
					      k_spinlock_کلید_t کلید)
{
	جب (عدد_events--) {
		clear_event_registration(&events[عدد_events]);
		k_spin_unlock(&poll_تالا, کلید);
		کلید = k_spin_تالا(&poll_تالا);
	}
}

ساکن اندرونی خالی سیٹ_event_ready(ساخت k_poll_event *event, uint32_t state)
{
	event->poller = تہی;
	event->state |= state;
}

ساکن اندرونی صحیح رجسٹر_events(ساخت k_poll_event *events,
				  صحیح عدد_events,
				  ساخت z_poller *poller,
				  bool just_check)
{
	صحیح events_registered = 0;

	برائے (صحیح ii = 0; ii < عدد_events; ii++) {
		k_spinlock_کلید_t کلید;
		uint32_t state;

		کلید = k_spin_تالا(&poll_تالا);
		اگر (is_condition_met(&events[ii], &state)) {
			سیٹ_event_ready(&events[ii], state);
			poller->is_polling = false;
		} ورنہ اگر (!just_check && poller->is_polling) {
			رجسٹر_event(&events[ii], poller);
			events_registered += 1;
		} ورنہ {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_تالا, کلید);
	}

	واپس events_registered;
}

ساکن صحیح signal_poller(ساخت k_poll_event *event, uint32_t state)
{
	ساخت k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != تہی, "poller should have a thread\n");

	اگر (!z_is_thread_pending(thread)) {
		واپس 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_قدر_سیٹ(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	اگر (!z_is_thread_ready(thread)) {
		واپس 0;
	}

	z_ready_thread(thread);

	واپس 0;
}

صحیح z_impl_k_poll(ساخت k_poll_event *events, صحیح عدد_events,
		  k_timeout_t timeout)
{
	صحیح events_registered;
	k_spinlock_کلید_t کلید;
	ساخت z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->منوال = منوال_POLL;

	__ASSERT(!arch_is_اندر_isr(), "");
	__ASSERT(events != تہی, "NULL events\n");
	__ASSERT(عدد_events >= 0, "<0 events\n");

	SYS_پورٹ_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = رجسٹر_events(events, عدد_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	کلید = k_spin_تالا(&poll_تالا);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	اگر (!poller->is_polling) {
		clear_event_registrations(events, events_registered, کلید);
		k_spin_unlock(&poll_تالا, کلید);

		SYS_پورٹ_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		واپس 0;
	}

	poller->is_polling = false;

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_تالا, کلید);

		SYS_پورٹ_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		واپس -EAGAIN;
	}

	ساکن _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	صحیح تبادلہ_rc = z_pend_curr(&poll_تالا, کلید, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	کلید = k_spin_تالا(&poll_تالا);
	clear_event_registrations(events, events_registered, کلید);
	k_spin_unlock(&poll_تالا, کلید);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_poll_api, poll, events, تبادلہ_rc);

	واپس تبادلہ_rc;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_poll(ساخت k_poll_event *events,
				صحیح عدد_events, k_timeout_t timeout)
{
	صحیح ret;
	k_spinlock_کلید_t کلید;
	ساخت k_poll_event *events_copy = تہی;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	اگر (K_SYSCALL_VERIFY(عدد_events >= 0)) {
		ret = -EINVAL;
		جاؤ برآمد;
	}
	اگر (K_SYSCALL_VERIFY_پیغام(!u32_mul_حد_سے_تجاوز(عدد_events,
						   حجم(ساخت k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		جاؤ برآمد;
	}
	events_copy = z_thread_malloc(bounds);
	اگر (!events_copy) {
		ret = -ENOMEM;
		جاؤ برآمد;
	}

	اگر (K_SYSCALL_حافظہ_WRITE(events, bounds)) {
		جاؤ oops_free;
	}

	کلید = k_spin_تالا(&poll_تالا);
	(خالی)حافظہ_نقل(events_copy, events, bounds);
	k_spin_unlock(&poll_تالا, کلید);

	/* Validate what's inside events_copy */
	برائے (صحیح i = 0; i < عدد_events; i++) {
		ساخت k_poll_event *e = &events_copy[i];

		اگر (K_SYSCALL_VERIFY(e->منوال == K_POLL_منوال_NOTIFY_ONLY)) {
			ret = -EINVAL;
			جاؤ برآمد_free;
		}

		انتخاب (e->type) {
		حالت K_POLL_TYPE_IGNORE:
			توڑو;
		حالت K_POLL_TYPE_SIGNAL:
			اگر (K_SYSCALL_OBJ(e->اشارہ, K_OBJ_POLL_SIGNAL)) {
				جاؤ oops_free;
			}
			توڑو;
		حالت K_POLL_TYPE_SEM_دستیاب:
			اگر (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				جاؤ oops_free;
			}
			توڑو;
		حالت K_POLL_TYPE_ڈیٹا_دستیاب:
			اگر (K_SYSCALL_OBJ(e->قطار, K_OBJ_قطار)) {
				جاؤ oops_free;
			}
			توڑو;
		حالت K_POLL_TYPE_MSGQ_ڈیٹا_دستیاب:
			اگر (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				جاؤ oops_free;
			}
			توڑو;
		حالت K_POLL_TYPE_PIPE_ڈیٹا_دستیاب:
			اگر (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				جاؤ oops_free;
			}
			توڑو;
		طےشدہ:
			ret = -EINVAL;
			جاؤ برآمد_free;
		}
	}

	ret = k_poll(events_copy, عدد_events, timeout);
	(خالی)حافظہ_نقل((خالی *)events, events_copy, bounds);
برآمد_free:
	k_free(events_copy);
برآمد:
	واپس ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#شامل <zephyr/syscalls/k_poll_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
ساکن صحیح signal_poll_event(ساخت k_poll_event *event, uint32_t state)
{
	ساخت z_poller *poller = event->poller;
	صحیح retcode = 0;

	اگر (poller != تہی) {
		اگر (poller->منوال == منوال_POLL) {
			retcode = signal_poller(event, state);
		} ورنہ اگر (poller->منوال == منوال_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} ورنہ {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		اگر (retcode < 0) {
			واپس retcode;
		}
	}

	سیٹ_event_ready(event, state);
	واپس retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	ساخت k_poll_event *poll_event;
	k_spinlock_کلید_t کلید = k_spin_تالا(&poll_تالا);

	poll_event = (ساخت k_poll_event *)sys_dlist_get(events);
	اگر (poll_event != تہی) {
		(خالی) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_تالا, کلید);

	واپس (poll_event != تہی);
}

خالی z_impl_k_poll_signal_init(ساخت k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_poll_signal_init(ساخت k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#شامل <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_impl_k_poll_signal_reset(ساخت k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

خالی z_impl_k_poll_signal_check(ساخت k_poll_signal *sig,
			       غیرعلامتی صحیح *signaled, صحیح *نتیجہ)
{
	*signaled = sig->signaled;
	*نتیجہ = sig->نتیجہ;

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
خالی z_vrfy_k_poll_signal_check(ساخت k_poll_signal *sig,
			       غیرعلامتی صحیح *signaled, صحیح *نتیجہ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(signaled, حجم(غیرعلامتی صحیح)));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(نتیجہ, حجم(صحیح)));
	z_impl_k_poll_signal_check(sig, signaled, نتیجہ);
}
#شامل <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_poll_signal_raise(ساخت k_poll_signal *sig, صحیح نتیجہ)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&poll_تالا);
	ساخت k_poll_event *poll_event;

	sig->نتیجہ = نتیجہ;
	sig->signaled = 1U;

	poll_event = (ساخت k_poll_event *)sys_dlist_get(&sig->poll_events);
	اگر (poll_event == تہی) {
		k_spin_unlock(&poll_تالا, کلید);

		SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		واپس 0;
	}

	صحیح rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_تالا, کلید);
	واپس rc;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_poll_signal_raise(ساخت k_poll_signal *sig,
					     صحیح نتیجہ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	واپس z_impl_k_poll_signal_raise(sig, نتیجہ);
}
#شامل <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

ساکن اندرونی خالی z_vrfy_k_poll_signal_reset(ساخت k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#شامل <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#اگر_ختم /* CONFIG_USERSPACE */

ساکن خالی triggered_work_handler(ساخت k_work *work)
{
	ساخت k_work_poll *twork =
			CONTAINER_کا(work, ساخت k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	اگر (twork->poller.منوال != منوال_NONE) {
		k_spinlock_کلید_t کلید;

		کلید = k_spin_تالا(&poll_تالا);
		clear_event_registrations(twork->events,
					  twork->عدد_events, کلید);
		k_spin_unlock(&poll_تالا, کلید);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = تہی;
	twork->real_handler(work);
}

خارجی صحیح z_work_submit_مقصد_قطار(ساخت k_work_q *قطار,
			 ساخت k_work *work);

ساکن خالی triggered_work_expiration_handler(ساخت _timeout *timeout)
{
	ساخت k_work_poll *twork =
		CONTAINER_کا(timeout, ساخت k_work_poll, timeout);
	k_spinlock_کلید_t کلید = k_spin_تالا(&poll_تالا);

	اگر (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_تالا, کلید);
		واپس;
	}

	twork->poller.is_polling = false;
	twork->poll_نتیجہ = -EAGAIN;
	z_work_submit_مقصد_قطار(twork->workq, &twork->work);

	k_spin_unlock(&poll_تالا, کلید);
}

ساکن صحیح signal_triggered_work(ساخت k_poll_event *event, uint32_t حالت_2)
{
	ساخت z_poller *poller = event->poller;
	ساخت k_work_poll *twork =
		CONTAINER_کا(poller, ساخت k_work_poll, poller);

	اگر (poller->is_polling && twork->workq != تہی) {
		ساخت k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(خالی)z_try_abort_timeout(&twork->timeout);
		twork->poll_نتیجہ = 0;
		z_work_submit_مقصد_قطار(work_q, &twork->work);
	}

	واپس 0;
}

ساکن صحیح triggered_work_cancel(ساخت k_work_poll *work,
				 k_spinlock_کلید_t کلید)
{
	/* Check if the work waits for event. */
	اگر (work->poller.is_polling && work->poller.منوال != منوال_NONE) {
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
		جب (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_تالا, کلید);
			کلید = k_spin_تالا(&poll_تالا);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.منوال = منوال_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->عدد_events, کلید);
		work->workq = تہی;
		واپس 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	واپس -EINVAL;
}

خالی k_work_poll_init(ساخت k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (ساخت k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

صحیح k_work_poll_submit_مقصد_قطار(ساخت k_work_q *work_q,
				ساخت k_work_poll *work,
				ساخت k_poll_event *events,
				صحیح عدد_events,
				k_timeout_t timeout)
{
	صحیح events_registered;
	k_spinlock_کلید_t کلید;

	__ASSERT(work_q != تہی, "NULL work_q\n");
	__ASSERT(work != تہی, "NULL work\n");
	__ASSERT(events != تہی, "NULL events\n");
	__ASSERT(عدد_events >= 0, "<0 events\n");

	SYS_پورٹ_TRACING_FUNC_ENTER(k_work_poll, submit_مقصد_قطار, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	کلید = k_spin_تالا(&poll_تالا);
	اگر (work->workq != تہی) {
		اگر (work->workq == work_q) {
			صحیح retval;

			retval = triggered_work_cancel(work, کلید);
			اگر (retval < 0) {
				k_spin_unlock(&poll_تالا, کلید);

				SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_مقصد_قطار, work_q,
					work, timeout, retval);

				واپس retval;
			}
		} ورنہ {
			k_spin_unlock(&poll_تالا, کلید);

			SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_مقصد_قطار, work_q,
				work, timeout, -EADDRINUSE);

			واپس -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.منوال = منوال_NONE;
	k_spin_unlock(&poll_تالا, کلید);

	/* Save list of events. */
	work->events = events;
	work->عدد_events = عدد_events;

	/* Clear result */
	work->poll_نتیجہ = -EINPROGRESS;

	/* Register events */
	events_registered = رجسٹر_events(events, عدد_events,
					    &work->poller, false);

	کلید = k_spin_تالا(&poll_تالا);
	اگر (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(عدد_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_جوڑو_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.منوال = منوال_TRIGGERED;
		k_spin_unlock(&poll_تالا, کلید);

		SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_مقصد_قطار, work_q, work, timeout, 0);

		واپس 0;
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
		work->poll_نتیجہ = -EAGAIN;
	} ورنہ {
		work->poll_نتیجہ = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, کلید);
	k_spin_unlock(&poll_تالا, کلید);

	/* Submit work. */
	k_work_submit_مقصد_قطار(work_q, &work->work);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_مقصد_قطار, work_q, work, timeout, 0);

	واپس 0;
}

صحیح k_work_poll_submit(ساخت k_work_poll *work,
				     ساخت k_poll_event *events,
				     صحیح عدد_events,
				     k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	صحیح ret = k_work_poll_submit_مقصد_قطار(&k_sys_work_q, work,
								events, عدد_events, timeout);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	واپس ret;
}

صحیح k_work_poll_cancel(ساخت k_work_poll *work)
{
	k_spinlock_کلید_t کلید;
	صحیح retval;

	SYS_پورٹ_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	اگر (work == تہی || work->workq == تہی) {
		SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		واپس -EINVAL;
	}

	کلید = k_spin_تالا(&poll_تالا);
	retval = triggered_work_cancel(work, کلید);
	k_spin_unlock(&poll_تالا, کلید);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	واپس retval;
}
