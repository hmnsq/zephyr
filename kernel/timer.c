/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>

#অন্তর্ভাৱ <zephyr/init.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <zephyr/sys/check.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <zephyr/spinlock.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <wait_q.h>

স্থির সংরচনা k_spinlock timer_lock;

#ifdef CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, রিক্ত);
#endif /* CONFIG_OBJ_CORE_TIMER */

#যদি defined(CONFIG_TIMER_OBSERVER)
স্থির অন্তঃস্থ শূন্য z_timer_observer_on_init(সংরচনা k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		যদি (obs->on_init != রিক্ত) {
			obs->on_init(timer);
		}
	}
}

স্থির অন্তঃস্থ শূন্য z_timer_observer_on_start(সংরচনা k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		যদি (obs->on_start != রিক্ত) {
			obs->on_start(timer, duration, period);
		}
	}
}

স্থির অন্তঃস্থ শূন্য z_timer_observer_on_stop(সংরচনা k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		যদি (obs->on_stop != রিক্ত) {
			obs->on_stop(timer);
		}
	}
}

স্থির অন্তঃস্থ শূন্য z_timer_observer_on_expiry(সংরচনা k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		যদি (obs->on_expiry != রিক্ত) {
			obs->on_expiry(timer);
		}
	}
}
#অন্যথা
#define z_timer_observer_on_init(timer) (শূন্য)0
#define z_timer_observer_on_start(timer, duration, period) (শূন্য)0
#define z_timer_observer_on_stop(timer) (শূন্য)0
#define z_timer_observer_on_expiry(timer) (শূন্য)0
#endif /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
শূন্য z_timer_expiration_handler(সংরচনা _timeout *t)
{
	সংরচনা k_timer *timer = CONTAINER_OF(t, সংরচনা k_timer, timeout);
	সংরচনা k_thread *thread;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	যদি (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, চাবি);
		ঘূৰাই;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	যদি (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
	    !K_TIMEOUT_EQ(timer->period, K_FOREVER)) {
		k_timeout_t next = timer->period;

#ifdef CONFIG_TIMEOUT_64BIT
		/* Exploit the fact that uptime during a kernel
		 * timeout handler reflects the time of the scheduled
		 * event and not real time to get some inexpensive
		 * protection against late interrupts.  If we're
		 * delayed for any reason, we still end up calculating
		 * the next expiration as a regular stride from where
		 * we "should" have run.  Requires absolute timeouts.
		 */
		next = K_TIMEOUT_ABS_TICKS(k_uptime_ticks() + next.ticks);
#endif /* CONFIG_TIMEOUT_64BIT */
		z_add_timeout(&timer->timeout, z_timer_expiration_handler,
			      next);
	}

	/* update timer's status */
	timer->status += 1U;

	z_timer_observer_on_expiry(timer);

	/* invoke timer expiry function */
	যদি (timer->expiry_fn != রিক্ত) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		চাবি = k_spin_lock(&timer_lock);
	}

	যদি (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, চাবি);
		ঘূৰাই;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(&timer->wait_q);

		যদি (thread != রিক্ত) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_মান_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, চাবি);

}


পূর্ণাঙ্ক z_timer_cleanup(সংরচনা k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	পূর্ণাঙ্ক ret = 0;
	k_spinlock_চাবি_t চাবি;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	চাবি = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_head_locked(&timer->wait_q) != রিক্ত)) {
		ret = -EAGAIN;
		গচ্ছ out;
	}

	CHECKIF(!locked && (z_waitq_head(&timer->wait_q) != রিক্ত)) {
		ret = -EAGAIN;
		গচ্ছ out;
	}

	/* Cancel the timeout AND wait for any in-flight expiration
	 * handler on another CPU to complete before returning. Unlike
	 * k_timer_stop(), we do not call any user stop_fn here: the
	 * caller is about to free the storage and there is no further
	 * consumer of the timer.
	 *
	 * This is the one timer path that waits on the handler (it must,
	 * before the storage is freed). The wait can only stall if the
	 * handler itself blocks on this CPU making progress -- e.g. an
	 * expiry_fn that aborts a thread running here. Freeing a timer
	 * whose handler does that is a caller bug; ordinary stop/start do
	 * not wait and so cannot stall.
	 */
	যদি (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, চাবি);
		গচ্ছ retry;
	}

out:
	k_spin_unlock(&timer_lock, চাবি);

#ifdef CONFIG_OBJ_CORE_TIMER
	যদি (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#endif /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_timer_cleanup(সংরচনা k_timer *timer)
{
	ঘূৰাই z_timer_cleanup(timer, false);
}


শূন্য k_timer_init(সংরচনা k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	যদি (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_data = রিক্ত;

	k_object_init(timer);

#ifdef CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#endif /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


শূন্য z_impl_k_timer_start(সংরচনা k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	যদি (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		ঘূৰাই;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_চাবি_t চাবি = k_spin_lock(&timer_lock);

	(শূন্য)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, চাবি);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_timer_start(সংরচনা k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_start_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_timer_stop(সংরচনা k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&timer_lock);

	যদি (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, চাবি);
		ঘূৰাই;
	}

	z_timer_observer_on_stop(timer);

	যদি (timer->stop_fn != রিক্ত) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, চাবি);

		timer->stop_fn(timer);

		চাবি = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	যদি (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, চাবি);
		ঘূৰাই;

	}

	সংরচনা k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	যদি (pending_thread != রিক্ত) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&timer_lock, চাবি);
	}
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_timer_stop(সংরচনা k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_stop_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(সংরচনা k_timer *timer)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, চাবি);

	ঘূৰাই result;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ uint32_t z_vrfy_k_timer_status_get(সংরচনা k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ঘূৰাই z_impl_k_timer_status_get(timer);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(সংরচনা k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	যদি (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t result;

		কৰা {
			unsigned পূর্ণাঙ্ক চাবি = irq_lock();

			যদি (!z_is_inactive_timeout(&timer->timeout)) {
				result = *(চঞ্চল uint32_t *)&timer->status;
				timer->status = 0U;
				যদি (result > 0) {
					irq_unlock(চাবি);
					ভঙ্গ;
				} অন্যথা {
					k_cpu_atomic_idle(চাবি);
				}
			} অন্যথা {
				result = timer->status;
				irq_unlock(চাবি);
				ভঙ্গ;
			}
		} যেতিয়ালৈ (true);

		ঘূৰাই result;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	যদি (result == 0U) {
		যদি (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(শূন্য)z_pend_curr(&timer_lock, চাবি, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			চাবি = k_spin_lock(&timer_lock);
			result = timer->status;
		} অন্যথা {
			/* timer is already stopped */
		}
	} অন্যথা {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, চাবি);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, result);

	ঘূৰাই result;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ uint32_t z_vrfy_k_timer_status_sync(সংরচনা k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ঘূৰাই z_impl_k_timer_status_sync(timer);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

স্থির অন্তঃস্থ k_ticks_t z_vrfy_k_timer_remaining_ticks(
						ধ্রুৱ সংরচনা k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ঘূৰাই z_impl_k_timer_remaining_ticks(timer);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

স্থির অন্তঃস্থ k_ticks_t z_vrfy_k_timer_expires_ticks(
						ধ্রুৱ সংরচনা k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ঘূৰাই z_impl_k_timer_expires_ticks(timer);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

স্থির অন্তঃস্থ শূন্য *z_vrfy_k_timer_user_data_get(ধ্রুৱ সংরচনা k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ঘূৰাই z_impl_k_timer_user_data_get(timer);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

স্থির অন্তঃস্থ শূন্য z_vrfy_k_timer_user_data_set(সংরচনা k_timer *timer,
						শূন্য *user_data)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_data_set(timer, user_data);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#endif /* CONFIG_USERSPACE */
