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

ಸ್ಥಿರ ರಚನೆ k_spinlock timer_ಬೀಗ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_TIMER */

#ಆದರೆ defined(CONFIG_TIMER_OBSERVER)
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_timer_observer_on_init(ರಚನೆ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಆದರೆ (obs->on_init != ಖಾಲಿ) {
			obs->on_init(timer);
		}
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_timer_observer_on_ಆರಂಭ(ರಚನೆ k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಆದರೆ (obs->on_ಆರಂಭ != ಖಾಲಿ) {
			obs->on_ಆರಂಭ(timer, duration, period);
		}
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_timer_observer_on_stop(ರಚನೆ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಆದರೆ (obs->on_stop != ಖಾಲಿ) {
			obs->on_stop(timer);
		}
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_timer_observer_on_expiry(ರಚನೆ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ಆದರೆ (obs->on_expiry != ಖಾಲಿ) {
			obs->on_expiry(timer);
		}
	}
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು z_timer_observer_on_init(timer) (ಶೂನ್ಯ)0
#ವ್ಯಾಖ್ಯಾನಿಸು z_timer_observer_on_ಆರಂಭ(timer, duration, period) (ಶೂನ್ಯ)0
#ವ್ಯಾಖ್ಯಾನಿಸು z_timer_observer_on_stop(timer) (ಶೂನ್ಯ)0
#ವ್ಯಾಖ್ಯಾನಿಸು z_timer_observer_on_expiry(timer) (ಶೂನ್ಯ)0
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
ಶೂನ್ಯ z_timer_expiration_handler(ರಚನೆ _timeout *t)
{
	ರಚನೆ k_timer *timer = CONTAINER_ದ(t, ರಚನೆ k_timer, timeout);
	ರಚನೆ k_thread *thread;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	ಆದರೆ (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	ಆದರೆ (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
	    !K_TIMEOUT_EQ(timer->period, K_FOREVER)) {
		k_timeout_t ಮುಂದಿನ = timer->period;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMEOUT_64_ಬಿಟ್
		/* Exploit the fact that uptime during a kernel
		 * timeout handler reflects the time of the scheduled
		 * event and not real time to get some inexpensive
		 * protection against late interrupts.  If we're
		 * delayed for any reason, we still end up calculating
		 * the next expiration as a regular stride from where
		 * we "should" have run.  Requires absolute timeouts.
		 */
		ಮುಂದಿನ = K_TIMEOUT_ABS_TICKS(k_uptime_ticks() + ಮುಂದಿನ.ticks);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMEOUT_64BIT */
		z_ಕೂಡಿಸು_timeout(&timer->timeout, z_timer_expiration_handler,
			      ಮುಂದಿನ);
	}

	/* update timer's status */
	timer->ಸ್ಥಿತಿ_2 += 1U;

	z_timer_observer_on_expiry(timer);

	/* invoke timer expiry function */
	ಆದರೆ (timer->expiry_fn != ಖಾಲಿ) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);
	}

	ಆದರೆ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	ಬೀಗ_SCHED_SPINLOCK {
		thread = z_waitq_ತಲೆ_locked(&timer->wait_q);

		ಆದರೆ (thread != ಖಾಲಿ) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_ಮೌಲ್ಯ_ಗಣ(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);

}


ಪೂರ್ಣಾಂಕ z_timer_cleanup(ರಚನೆ k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_ಒಳ_isr(), "");

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	ಪೂರ್ಣಾಂಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);

	CHECKIF(locked && (z_waitq_ತಲೆ_locked(&timer->wait_q) != ಖಾಲಿ)) {
		ret = -EAGAIN;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	CHECKIF(!locked && (z_waitq_ತಲೆ(&timer->wait_q) != ಖಾಲಿ)) {
		ret = -EAGAIN;
		ಹೋಗು ಹೊರಹೋಗು;
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
	ಆದರೆ (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);
		ಹೋಗು retry;
	}

ಹೊರಹೋಗು:
	k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_TIMER
	ಆದರೆ (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_TIMER */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_timer_cleanup(ರಚನೆ k_timer *timer)
{
	ಹಿಂತಿರುಗಿ z_timer_cleanup(timer, false);
}


ಶೂನ್ಯ k_timer_init(ರಚನೆ k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->ಸ್ಥಿತಿ_2 = 0U;

	ಆದರೆ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_ದತ್ತಾಂಶ = ಖಾಲಿ;

	k_object_init(timer);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


ಶೂನ್ಯ z_impl_k_timer_ಆರಂಭ(ರಚನೆ k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_timer, ಆರಂಭ, timer, duration, period);

	ಆದರೆ (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		ಹಿಂತಿರುಗಿ;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);

	(ಶೂನ್ಯ)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->ಸ್ಥಿತಿ_2 = 0U;

	z_ಕೂಡಿಸು_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_ಆರಂಭ(timer, duration, period);

	k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_timer_ಆರಂಭ(ರಚನೆ k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_ಆರಂಭ(timer, duration, period);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_start_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_timer_stop(ರಚನೆ k_timer *timer)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);

	ಆದರೆ (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	z_timer_observer_on_stop(timer);

	ಆದರೆ (timer->stop_fn != ಖಾಲಿ) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);

		timer->stop_fn(timer);

		ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	ಆದರೆ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;

	}

	ರಚನೆ k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	ಆದರೆ (pending_thread != ಖಾಲಿ) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_timer_stop(ರಚನೆ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_stop_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_ಸ್ಥಿತಿ_get(ರಚನೆ k_timer *timer)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);
	uint32_t ಫಲಿತಾಂಶ = timer->ಸ್ಥಿತಿ_2;

	timer->ಸ್ಥಿತಿ_2 = 0U;
	k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t z_vrfy_k_timer_ಸ್ಥಿತಿ_get(ರಚನೆ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಹಿಂತಿರುಗಿ z_impl_k_timer_ಸ್ಥಿತಿ_get(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_ಸ್ಥಿತಿ_sync(ರಚನೆ k_timer *timer)
{
	__ASSERT(!arch_is_ಒಳ_isr(), "");
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_timer, ಸ್ಥಿತಿ_sync, timer);

	ಆದರೆ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t ಫಲಿತಾಂಶ;

		ಮಾಡು {
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ = irq_ಬೀಗ();

			ಆದರೆ (!z_is_inactive_timeout(&timer->timeout)) {
				ಫಲಿತಾಂಶ = *(ಅಸ್ಥಿರ uint32_t *)&timer->ಸ್ಥಿತಿ_2;
				timer->ಸ್ಥಿತಿ_2 = 0U;
				ಆದರೆ (ಫಲಿತಾಂಶ > 0) {
					irq_unlock(ಕೀಲಿ);
					ನಿಲ್ಲಿಸು;
				} ಇಲ್ಲದಿದ್ದರೆ {
					k_cpu_atomic_idle(ಕೀಲಿ);
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಫಲಿತಾಂಶ = timer->ಸ್ಥಿತಿ_2;
				irq_unlock(ಕೀಲಿ);
				ನಿಲ್ಲಿಸು;
			}
		} ಆಗ (true);

		ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);
	uint32_t ಫಲಿತಾಂಶ = timer->ಸ್ಥಿತಿ_2;

	ಆದರೆ (ಫಲಿತಾಂಶ == 0U) {
		ಆದರೆ (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_timer, ಸ್ಥಿತಿ_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(ಶೂನ್ಯ)z_pend_curr(&timer_ಬೀಗ, ಕೀಲಿ, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			ಕೀಲಿ = k_spin_ಬೀಗ(&timer_ಬೀಗ);
			ಫಲಿತಾಂಶ = timer->ಸ್ಥಿತಿ_2;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* timer is already stopped */
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* timer has already expired at least once */
	}

	timer->ಸ್ಥಿತಿ_2 = 0U;
	k_spin_unlock(&timer_ಬೀಗ, ಕೀಲಿ);

	/**
	 * @note	New tracing hook
	 */
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_timer, ಸ್ಥಿತಿ_sync, timer, ಫಲಿತಾಂಶ);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t z_vrfy_k_timer_ಸ್ಥಿತಿ_sync(ರಚನೆ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಹಿಂತಿರುಗಿ z_impl_k_timer_ಸ್ಥಿತಿ_sync(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_vrfy_k_timer_ಉಳಿದ_ticks(
						ಸ್ಥಿರಾಂಕ ರಚನೆ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಹಿಂತಿರುಗಿ z_impl_k_timer_ಉಳಿದ_ticks(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_vrfy_k_timer_expires_ticks(
						ಸ್ಥಿರಾಂಕ ರಚನೆ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಹಿಂತಿರುಗಿ z_impl_k_timer_expires_ticks(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *z_vrfy_k_timer_user_ದತ್ತಾಂಶ_get(ಸ್ಥಿರಾಂಕ ರಚನೆ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ಹಿಂತಿರುಗಿ z_impl_k_timer_user_ದತ್ತಾಂಶ_get(timer);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_timer_user_ದತ್ತಾಂಶ_ಗಣ(ರಚನೆ k_timer *timer,
						ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_ದತ್ತಾಂಶ_ಗಣ(timer, user_ದತ್ತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
