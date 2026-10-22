/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>

ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock timer_lock;

#ifdef CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, ರಿಕ್ತ);
#endif /* CONFIG_OBJ_CORE_TIMER */

#ಯದಿ defined(CONFIG_TIMER_OBSERVER)
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_timer_observer_on_init(ಸಂರಚನಾ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಯದಿ (obs->on_init != ರಿಕ್ತ) {
			obs->on_init(timer);
		}
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_timer_observer_on_start(ಸಂರಚನಾ k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಯದಿ (obs->on_start != ರಿಕ್ತ) {
			obs->on_start(timer, duration, period);
		}
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_timer_observer_on_stop(ಸಂರಚನಾ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಯದಿ (obs->on_stop != ರಿಕ್ತ) {
			obs->on_stop(timer);
		}
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_timer_observer_on_expiry(ಸಂರಚನಾ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಯದಿ (obs->on_expiry != ರಿಕ್ತ) {
			obs->on_expiry(timer);
		}
	}
}
#ಅನ್ಯಥಾ
#define z_timer_observer_on_init(timer) (ಶೂನ್ಯ)0
#define z_timer_observer_on_start(timer, duration, period) (ಶೂನ್ಯ)0
#define z_timer_observer_on_stop(timer) (ಶೂನ್ಯ)0
#define z_timer_observer_on_expiry(timer) (ಶೂನ್ಯ)0
#endif /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
ಶೂನ್ಯ z_timer_expiration_handler(ಸಂರಚನಾ _timeout *t)
{
	ಸಂರಚನಾ k_timer *timer = CONTAINER_OF(t, ಸಂರಚನಾ k_timer, timeout);
	ಸಂರಚನಾ k_thread *thread;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	ಯದಿ (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	ಯದಿ (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
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
	ಯದಿ (timer->expiry_fn != ರಿಕ್ತ) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, ಕೀಲಿ);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		ಕೀಲಿ = k_spin_lock(&timer_lock);
	}

	ಯದಿ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(&timer->wait_q);

		ಯದಿ (thread != ರಿಕ್ತ) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_ಮೌಲ್ಯ_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, ಕೀಲಿ);

}


ಪೂರ್ಣಾಙ್ಕ z_timer_cleanup(ಸಂರಚನಾ k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	ಕೀಲಿ = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_head_locked(&timer->wait_q) != ರಿಕ್ತ)) {
		ret = -EAGAIN;
		ಗಚ್ಛ out;
	}

	CHECKIF(!locked && (z_waitq_head(&timer->wait_q) != ರಿಕ್ತ)) {
		ret = -EAGAIN;
		ಗಚ್ಛ out;
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
	ಯದಿ (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, ಕೀಲಿ);
		ಗಚ್ಛ retry;
	}

out:
	k_spin_unlock(&timer_lock, ಕೀಲಿ);

#ifdef CONFIG_OBJ_CORE_TIMER
	ಯದಿ (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#endif /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_timer_cleanup(ಸಂರಚನಾ k_timer *timer)
{
	ಪ್ರತ್ಯಾಗಮ z_timer_cleanup(timer, false);
}


ಶೂನ್ಯ k_timer_init(ಸಂರಚನಾ k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	ಯದಿ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_data = ರಿಕ್ತ;

	k_object_init(timer);

#ifdef CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#endif /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


ಶೂನ್ಯ z_impl_k_timer_start(ಸಂರಚನಾ k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	ಯದಿ (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&timer_lock);

	(ಶೂನ್ಯ)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, ಕೀಲಿ);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_timer_start(ಸಂರಚನಾ k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_start_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_timer_stop(ಸಂರಚನಾ k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&timer_lock);

	ಯದಿ (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	z_timer_observer_on_stop(timer);

	ಯದಿ (timer->stop_fn != ರಿಕ್ತ) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, ಕೀಲಿ);

		timer->stop_fn(timer);

		ಕೀಲಿ = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	ಯದಿ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;

	}

	ಸಂರಚನಾ k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	ಯದಿ (pending_thread != ರಿಕ್ತ) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&timer_lock, ಕೀಲಿ);
	}
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_timer_stop(ಸಂರಚನಾ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_stop_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(ಸಂರಚನಾ k_timer *timer)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ result;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint32_t z_vrfy_k_timer_status_get(ಸಂರಚನಾ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_timer_status_get(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(ಸಂರಚನಾ k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	ಯದಿ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t result;

		ಕುರು {
			unsigned ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ = irq_lock();

			ಯದಿ (!z_is_inactive_timeout(&timer->timeout)) {
				result = *(ಚಞ್ಚಲ uint32_t *)&timer->status;
				timer->status = 0U;
				ಯದಿ (result > 0) {
					irq_unlock(ಕೀಲಿ);
					ಭಙ್ಗ;
				} ಅನ್ಯಥಾ {
					k_cpu_atomic_idle(ಕೀಲಿ);
				}
			} ಅನ್ಯಥಾ {
				result = timer->status;
				irq_unlock(ಕೀಲಿ);
				ಭಙ್ಗ;
			}
		} ಯಾವತ್ (true);

		ಪ್ರತ್ಯಾಗಮ result;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	ಯದಿ (result == 0U) {
		ಯದಿ (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(ಶೂನ್ಯ)z_pend_curr(&timer_lock, ಕೀಲಿ, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			ಕೀಲಿ = k_spin_lock(&timer_lock);
			result = timer->status;
		} ಅನ್ಯಥಾ {
			/* timer is already stopped */
		}
	} ಅನ್ಯಥಾ {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, ಕೀಲಿ);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, result);

	ಪ್ರತ್ಯಾಗಮ result;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ uint32_t z_vrfy_k_timer_status_sync(ಸಂರಚನಾ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_timer_status_sync(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_vrfy_k_timer_remaining_ticks(
						ಧ್ರುವ ಸಂರಚನಾ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_timer_remaining_ticks(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_vrfy_k_timer_expires_ticks(
						ಧ್ರುವ ಸಂರಚನಾ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_timer_expires_ticks(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *z_vrfy_k_timer_user_data_get(ಧ್ರುವ ಸಂರಚನಾ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_timer_user_data_get(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_timer_user_data_set(ಸಂರಚನಾ k_timer *timer,
						ಶೂನ್ಯ *user_data)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_data_set(timer, user_data);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#endif /* CONFIG_USERSPACE */
