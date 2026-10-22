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
ساكن بنية k_spinlock poll_جندرا;

تعداد POLL_منوال { منوال_NONE, منوال_POLL, منوال_TRIGGERED };

ساكن صحيح signal_poller(بنية k_poll_event *event, uint32_t state);
ساكن صحيح signal_triggered_work(بنية k_poll_event *event, uint32_t حالت);

فراغ k_poll_event_init(بنية k_poll_event *event, uint32_t type,
		       صحيح منوال, فراغ *obj)
{
	__ASSERT(منوال == K_POLL_منوال_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (ثنائی_ہندسہ(POLL_عدد_TYPES)), "invalid type\n");
	__ASSERT(obj != عدم, "must provide an object\n");

	event->poller = عدم;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->منوال = منوال;
	event->بن_ورتیا = 0U;
	event->obj = obj;

	SYS_پورٹ_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
ساكن مضمن bool is_condition_met(بنية k_poll_event *event, uint32_t *state)
{
	بدل (event->type) {
	حالة K_POLL_TYPE_SEM_دستیاب:
		إذا (k_sem_گنتی_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_دستیاب;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_انکڑے_دستیاب:
		إذا (!k_قطار_is_خالی_تھاواں(event->قطار)) {
			*state = K_POLL_STATE_FIFO_انکڑے_دستیاب;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_SIGNAL:
		إذا (event->إشارة->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_MSGQ_انکڑے_دستیاب:
		إذا (event->msgq->ورتیا_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_انکڑے_دستیاب;
			أرجع true;
		}
		اكسر;
	حالة K_POLL_TYPE_PIPE_انکڑے_دستیاب:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		إذا (!ring_buf_is_خالی_تھاواں(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_انکڑے_دستیاب;
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
	أرجع p ? CONTAINER_دا(p, بنية k_thread, poller) : عدم;
}

ساكن مضمن فراغ جوڑو_event(sys_dlist_t *events, بنية k_poll_event *event,
			     بنية z_poller *poller)
{
	بنية k_poll_event *pending;

	pending = (بنية k_poll_event *)sys_dlist_peek_پوچھ(events);
	إذا ((pending == عدم) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->گرہ_2);
		أرجع;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, گرہ_2) {
		إذا (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_داخل_کرو(&pending->گرہ_2, &event->گرہ_2);
			أرجع;
		}
	}

	sys_dlist_append(events, &event->گرہ_2);
}

/* must be called with interrupts locked */
ساكن مضمن فراغ رجسٹر_event(بنية k_poll_event *event,
				 بنية z_poller *poller)
{
	بدل (event->type) {
	حالة K_POLL_TYPE_SEM_دستیاب:
		__ASSERT(event->sem != عدم, "invalid semaphore\n");
		جوڑو_event(&event->sem->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_انکڑے_دستیاب:
		__ASSERT(event->قطار != عدم, "invalid queue\n");
		جوڑو_event(&event->قطار->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_SIGNAL:
		__ASSERT(event->إشارة != عدم, "invalid poll signal\n");
		جوڑو_event(&event->إشارة->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_MSGQ_انکڑے_دستیاب:
		__ASSERT(event->msgq != عدم, "invalid message queue\n");
		جوڑو_event(&event->msgq->poll_events, event, poller);
		اكسر;
	حالة K_POLL_TYPE_PIPE_انکڑے_دستیاب:
		__ASSERT(event->pipe != عدم, "invalid pipe\n");
		جوڑو_event(&event->pipe->poll_events, event, poller);
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
	حالة K_POLL_TYPE_SEM_دستیاب:
		__ASSERT(event->sem != عدم, "invalid semaphore\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_انکڑے_دستیاب:
		__ASSERT(event->قطار != عدم, "invalid queue\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_SIGNAL:
		__ASSERT(event->إشارة != عدم, "invalid poll signal\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_MSGQ_انکڑے_دستیاب:
		__ASSERT(event->msgq != عدم, "invalid message queue\n");
		remove_event = true;
		اكسر;
	حالة K_POLL_TYPE_PIPE_انکڑے_دستیاب:
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
	إذا (remove_event && sys_dnode_is_linked(&event->گرہ_2)) {
		sys_dlist_remove(&event->گرہ_2);
	}
}

/* must be called with interrupts locked */
ساكن مضمن فراغ clear_event_registrations(بنية k_poll_event *events,
					      صحيح عدد_events,
					      k_spinlock_کلید_t کلید)
{
	طالما (عدد_events--) {
		clear_event_registration(&events[عدد_events]);
		k_spin_unlock(&poll_جندرا, کلید);
		کلید = k_spin_جندرا(&poll_جندرا);
	}
}

ساكن مضمن فراغ سیٹ_event_ready(بنية k_poll_event *event, uint32_t state)
{
	event->poller = عدم;
	event->state |= state;
}

ساكن مضمن صحيح رجسٹر_events(بنية k_poll_event *events,
				  صحيح عدد_events,
				  بنية z_poller *poller,
				  bool just_check)
{
	صحيح events_registered = 0;

	لكل (صحيح ii = 0; ii < عدد_events; ii++) {
		k_spinlock_کلید_t کلید;
		uint32_t state;

		کلید = k_spin_جندرا(&poll_جندرا);
		إذا (is_condition_met(&events[ii], &state)) {
			سیٹ_event_ready(&events[ii], state);
			poller->is_polling = false;
		} وإلا إذا (!just_check && poller->is_polling) {
			رجسٹر_event(&events[ii], poller);
			events_registered += 1;
		} وإلا {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_جندرا, کلید);
	}

	أرجع events_registered;
}

ساكن صحيح signal_poller(بنية k_poll_event *event, uint32_t state)
{
	بنية k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != عدم, "poller should have a thread\n");

	إذا (!z_is_thread_pending(thread)) {
		أرجع 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_قدر_سیٹ(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	إذا (!z_is_thread_ready(thread)) {
		أرجع 0;
	}

	z_ready_thread(thread);

	أرجع 0;
}

صحيح z_impl_k_poll(بنية k_poll_event *events, صحيح عدد_events,
		  k_timeout_t timeout)
{
	صحيح events_registered;
	k_spinlock_کلید_t کلید;
	بنية z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->منوال = منوال_POLL;

	__ASSERT(!arch_is_وچ_isr(), "");
	__ASSERT(events != عدم, "NULL events\n");
	__ASSERT(عدد_events >= 0, "<0 events\n");

	SYS_پورٹ_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = رجسٹر_events(events, عدد_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	کلید = k_spin_جندرا(&poll_جندرا);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	إذا (!poller->is_polling) {
		clear_event_registrations(events, events_registered, کلید);
		k_spin_unlock(&poll_جندرا, کلید);

		SYS_پورٹ_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		أرجع 0;
	}

	poller->is_polling = false;

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_جندرا, کلید);

		SYS_پورٹ_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		أرجع -EAGAIN;
	}

	ساكن _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	صحيح وٹاندرا_rc = z_pend_curr(&poll_جندرا, کلید, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	کلید = k_spin_جندرا(&poll_جندرا);
	clear_event_registrations(events, events_registered, کلید);
	k_spin_unlock(&poll_جندرا, کلید);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_poll_api, poll, events, وٹاندرا_rc);

	أرجع وٹاندرا_rc;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_poll(بنية k_poll_event *events,
				صحيح عدد_events, k_timeout_t timeout)
{
	صحيح ret;
	k_spinlock_کلید_t کلید;
	بنية k_poll_event *events_copy = عدم;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	إذا (K_SYSCALL_VERIFY(عدد_events >= 0)) {
		ret = -EINVAL;
		اذهب باہرلا;
	}
	إذا (K_SYSCALL_VERIFY_پیغام(!u32_mul_حد_توں_ودھ(عدد_events,
						   حجم(بنية k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		اذهب باہرلا;
	}
	events_copy = z_thread_malloc(bounds);
	إذا (!events_copy) {
		ret = -ENOMEM;
		اذهب باہرلا;
	}

	إذا (K_SYSCALL_حافظہ_WRITE(events, bounds)) {
		اذهب oops_free;
	}

	کلید = k_spin_جندرا(&poll_جندرا);
	(فراغ)حافظہ_نقل(events_copy, events, bounds);
	k_spin_unlock(&poll_جندرا, کلید);

	/* Validate what's inside events_copy */
	لكل (صحيح i = 0; i < عدد_events; i++) {
		بنية k_poll_event *e = &events_copy[i];

		إذا (K_SYSCALL_VERIFY(e->منوال == K_POLL_منوال_NOTIFY_ONLY)) {
			ret = -EINVAL;
			اذهب باہرلا_free;
		}

		بدل (e->type) {
		حالة K_POLL_TYPE_IGNORE:
			اكسر;
		حالة K_POLL_TYPE_SIGNAL:
			إذا (K_SYSCALL_OBJ(e->إشارة, K_OBJ_POLL_SIGNAL)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_SEM_دستیاب:
			إذا (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_انکڑے_دستیاب:
			إذا (K_SYSCALL_OBJ(e->قطار, K_OBJ_قطار)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_MSGQ_انکڑے_دستیاب:
			إذا (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				اذهب oops_free;
			}
			اكسر;
		حالة K_POLL_TYPE_PIPE_انکڑے_دستیاب:
			إذا (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				اذهب oops_free;
			}
			اكسر;
		افتراضي:
			ret = -EINVAL;
			اذهب باہرلا_free;
		}
	}

	ret = k_poll(events_copy, عدد_events, timeout);
	(فراغ)حافظہ_نقل((فراغ *)events, events_copy, bounds);
باہرلا_free:
	k_free(events_copy);
باہرلا:
	أرجع ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#ضمن <zephyr/syscalls/k_poll_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
ساكن صحيح signal_poll_event(بنية k_poll_event *event, uint32_t state)
{
	بنية z_poller *poller = event->poller;
	صحيح retcode = 0;

	إذا (poller != عدم) {
		إذا (poller->منوال == منوال_POLL) {
			retcode = signal_poller(event, state);
		} وإلا إذا (poller->منوال == منوال_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} وإلا {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		إذا (retcode < 0) {
			أرجع retcode;
		}
	}

	سیٹ_event_ready(event, state);
	أرجع retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	بنية k_poll_event *poll_event;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&poll_جندرا);

	poll_event = (بنية k_poll_event *)sys_dlist_get(events);
	إذا (poll_event != عدم) {
		(فراغ) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_جندرا, کلید);

	أرجع (poll_event != عدم);
}

فراغ z_impl_k_poll_signal_init(بنية k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_poll_signal_init(بنية k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#ضمن <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_impl_k_poll_signal_reset(بنية k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

فراغ z_impl_k_poll_signal_check(بنية k_poll_signal *sig,
			       بنانشانی صحيح *signaled, صحيح *نتیجہ)
{
	*signaled = sig->signaled;
	*نتیجہ = sig->نتیجہ;

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
فراغ z_vrfy_k_poll_signal_check(بنية k_poll_signal *sig,
			       بنانشانی صحيح *signaled, صحيح *نتیجہ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(signaled, حجم(بنانشانی صحيح)));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(نتیجہ, حجم(صحيح)));
	z_impl_k_poll_signal_check(sig, signaled, نتیجہ);
}
#ضمن <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_impl_k_poll_signal_raise(بنية k_poll_signal *sig, صحيح نتیجہ)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&poll_جندرا);
	بنية k_poll_event *poll_event;

	sig->نتیجہ = نتیجہ;
	sig->signaled = 1U;

	poll_event = (بنية k_poll_event *)sys_dlist_get(&sig->poll_events);
	إذا (poll_event == عدم) {
		k_spin_unlock(&poll_جندرا, کلید);

		SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		أرجع 0;
	}

	صحيح rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_پورٹ_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_جندرا, کلید);
	أرجع rc;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_poll_signal_raise(بنية k_poll_signal *sig,
					     صحيح نتیجہ)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	أرجع z_impl_k_poll_signal_raise(sig, نتیجہ);
}
#ضمن <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

ساكن مضمن فراغ z_vrfy_k_poll_signal_reset(بنية k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#ضمن <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#جے_انت /* CONFIG_USERSPACE */

ساكن فراغ triggered_work_handler(بنية k_work *work)
{
	بنية k_work_poll *twork =
			CONTAINER_دا(work, بنية k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	إذا (twork->poller.منوال != منوال_NONE) {
		k_spinlock_کلید_t کلید;

		کلید = k_spin_جندرا(&poll_جندرا);
		clear_event_registrations(twork->events,
					  twork->عدد_events, کلید);
		k_spin_unlock(&poll_جندرا, کلید);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = عدم;
	twork->real_handler(work);
}

خارجي صحيح z_work_submit_منزل_قطار(بنية k_work_q *قطار,
			 بنية k_work *work);

ساكن فراغ triggered_work_expiration_handler(بنية _timeout *timeout)
{
	بنية k_work_poll *twork =
		CONTAINER_دا(timeout, بنية k_work_poll, timeout);
	k_spinlock_کلید_t کلید = k_spin_جندرا(&poll_جندرا);

	إذا (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_جندرا, کلید);
		أرجع;
	}

	twork->poller.is_polling = false;
	twork->poll_نتیجہ = -EAGAIN;
	z_work_submit_منزل_قطار(twork->workq, &twork->work);

	k_spin_unlock(&poll_جندرا, کلید);
}

ساكن صحيح signal_triggered_work(بنية k_poll_event *event, uint32_t حالت)
{
	بنية z_poller *poller = event->poller;
	بنية k_work_poll *twork =
		CONTAINER_دا(poller, بنية k_work_poll, poller);

	إذا (poller->is_polling && twork->workq != عدم) {
		بنية k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(فراغ)z_try_abort_timeout(&twork->timeout);
		twork->poll_نتیجہ = 0;
		z_work_submit_منزل_قطار(work_q, &twork->work);
	}

	أرجع 0;
}

ساكن صحيح triggered_work_cancel(بنية k_work_poll *work,
				 k_spinlock_کلید_t کلید)
{
	/* Check if the work waits for event. */
	إذا (work->poller.is_polling && work->poller.منوال != منوال_NONE) {
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
			k_spin_unlock(&poll_جندرا, کلید);
			کلید = k_spin_جندرا(&poll_جندرا);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.منوال = منوال_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->عدد_events, کلید);
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
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (بنية k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

صحيح k_work_poll_submit_منزل_قطار(بنية k_work_q *work_q,
				بنية k_work_poll *work,
				بنية k_poll_event *events,
				صحيح عدد_events,
				k_timeout_t timeout)
{
	صحيح events_registered;
	k_spinlock_کلید_t کلید;

	__ASSERT(work_q != عدم, "NULL work_q\n");
	__ASSERT(work != عدم, "NULL work\n");
	__ASSERT(events != عدم, "NULL events\n");
	__ASSERT(عدد_events >= 0, "<0 events\n");

	SYS_پورٹ_TRACING_FUNC_ENTER(k_work_poll, submit_منزل_قطار, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	کلید = k_spin_جندرا(&poll_جندرا);
	إذا (work->workq != عدم) {
		إذا (work->workq == work_q) {
			صحيح retval;

			retval = triggered_work_cancel(work, کلید);
			إذا (retval < 0) {
				k_spin_unlock(&poll_جندرا, کلید);

				SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_منزل_قطار, work_q,
					work, timeout, retval);

				أرجع retval;
			}
		} وإلا {
			k_spin_unlock(&poll_جندرا, کلید);

			SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_منزل_قطار, work_q,
				work, timeout, -EADDRINUSE);

			أرجع -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.منوال = منوال_NONE;
	k_spin_unlock(&poll_جندرا, کلید);

	/* Save list of events. */
	work->events = events;
	work->عدد_events = عدد_events;

	/* Clear result */
	work->poll_نتیجہ = -EINPROGRESS;

	/* Register events */
	events_registered = رجسٹر_events(events, عدد_events,
					    &work->poller, false);

	کلید = k_spin_جندرا(&poll_جندرا);
	إذا (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
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
		k_spin_unlock(&poll_جندرا, کلید);

		SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_منزل_قطار, work_q, work, timeout, 0);

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
	إذا (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_نتیجہ = -EAGAIN;
	} وإلا {
		work->poll_نتیجہ = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, کلید);
	k_spin_unlock(&poll_جندرا, کلید);

	/* Submit work. */
	k_work_submit_منزل_قطار(work_q, &work->work);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit_منزل_قطار, work_q, work, timeout, 0);

	أرجع 0;
}

صحيح k_work_poll_submit(بنية k_work_poll *work,
				     بنية k_poll_event *events,
				     صحيح عدد_events,
				     k_timeout_t timeout)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	صحيح ret = k_work_poll_submit_منزل_قطار(&k_sys_work_q, work,
								events, عدد_events, timeout);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	أرجع ret;
}

صحيح k_work_poll_cancel(بنية k_work_poll *work)
{
	k_spinlock_کلید_t کلید;
	صحيح retval;

	SYS_پورٹ_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	إذا (work == عدم || work->workq == عدم) {
		SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		أرجع -EINVAL;
	}

	کلید = k_spin_جندرا(&poll_جندرا);
	retval = triggered_work_cancel(work, کلید);
	k_spin_unlock(&poll_جندرا, کلید);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	أرجع retval;
}
