/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>

ਸਥਿਰ ਬਣਤਰ k_spinlock timer_ਤਾਲਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_TIMER */

#ਜੇ defined(CONFIG_TIMER_OBSERVER)
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_timer_observer_on_init(ਬਣਤਰ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ਜੇ (obs->on_init != ਨਿਰਾ) {
			obs->on_init(timer);
		}
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_timer_observer_on_ਸ਼ੁਰੂ(ਬਣਤਰ k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ਜੇ (obs->on_ਸ਼ੁਰੂ != ਨਿਰਾ) {
			obs->on_ਸ਼ੁਰੂ(timer, duration, period);
		}
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_timer_observer_on_stop(ਬਣਤਰ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ਜੇ (obs->on_stop != ਨਿਰਾ) {
			obs->on_stop(timer);
		}
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_timer_observer_on_expiry(ਬਣਤਰ k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ਜੇ (obs->on_expiry != ਨਿਰਾ) {
			obs->on_expiry(timer);
		}
	}
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_timer_observer_on_init(timer) (ਖਾਲੀ)0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_timer_observer_on_ਸ਼ੁਰੂ(timer, duration, period) (ਖਾਲੀ)0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_timer_observer_on_stop(timer) (ਖਾਲੀ)0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_timer_observer_on_expiry(timer) (ਖਾਲੀ)0
#ਜੇ_ਅੰਤ /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
ਖਾਲੀ z_timer_expiration_handler(ਬਣਤਰ _timeout *t)
{
	ਬਣਤਰ k_timer *timer = CONTAINER_ਦਾ(t, ਬਣਤਰ k_timer, timeout);
	ਬਣਤਰ k_thread *thread;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	ਜੇ (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	ਜੇ (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
	    !K_TIMEOUT_EQ(timer->period, K_FOREVER)) {
		k_timeout_t ਅਗਲਾ = timer->period;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMEOUT_64_ਦੋਆਧਾਰੀ_ਅੰਕ
		/* Exploit the fact that uptime during a kernel
		 * timeout handler reflects the time of the scheduled
		 * event and not real time to get some inexpensive
		 * protection against late interrupts.  If we're
		 * delayed for any reason, we still end up calculating
		 * the next expiration as a regular stride from where
		 * we "should" have run.  Requires absolute timeouts.
		 */
		ਅਗਲਾ = K_TIMEOUT_ABS_TICKS(k_uptime_ticks() + ਅਗਲਾ.ticks);
#ਜੇ_ਅੰਤ /* CONFIG_TIMEOUT_64BIT */
		z_ਜੋੜੋ_timeout(&timer->timeout, z_timer_expiration_handler,
			      ਅਗਲਾ);
	}

	/* update timer's status */
	timer->ਸਥਿਤੀ += 1U;

	z_timer_observer_on_expiry(timer);

	/* invoke timer expiry function */
	ਜੇ (timer->expiry_fn != ਨਿਰਾ) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);
	}

	ਜੇ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	ਤਾਲਾ_SCHED_SPINLOCK {
		thread = z_waitq_ਸਿਰ_locked(&timer->wait_q);

		ਜੇ (thread != ਨਿਰਾ) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);

}


ਪੂਰਨ z_timer_cleanup(ਬਣਤਰ k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "");

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	ਪੂਰਨ ret = 0;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);

	CHECKIF(locked && (z_waitq_ਸਿਰ_locked(&timer->wait_q) != ਨਿਰਾ)) {
		ret = -EAGAIN;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	CHECKIF(!locked && (z_waitq_ਸਿਰ(&timer->wait_q) != ਨਿਰਾ)) {
		ret = -EAGAIN;
		ਜਾਓ ਬਾਹਰਲਾ;
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
	ਜੇ (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
		ਜਾਓ retry;
	}

ਬਾਹਰਲਾ:
	k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_TIMER
	ਜੇ (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_TIMER */

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_timer_cleanup(ਬਣਤਰ k_timer *timer)
{
	ਵਾਪਸ z_timer_cleanup(timer, false);
}


ਖਾਲੀ k_timer_init(ਬਣਤਰ k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->ਸਥਿਤੀ = 0U;

	ਜੇ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_ਅੰਕੜੇ = ਨਿਰਾ;

	k_object_init(timer);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


ਖਾਲੀ z_impl_k_timer_ਸ਼ੁਰੂ(ਬਣਤਰ k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_timer, ਸ਼ੁਰੂ, timer, duration, period);

	ਜੇ (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		ਵਾਪਸ;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);

	(ਖਾਲੀ)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->ਸਥਿਤੀ = 0U;

	z_ਜੋੜੋ_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_ਸ਼ੁਰੂ(timer, duration, period);

	k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_timer_ਸ਼ੁਰੂ(ਬਣਤਰ k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_ਸ਼ੁਰੂ(timer, duration, period);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_start_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_impl_k_timer_stop(ਬਣਤਰ k_timer *timer)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);

	ਜੇ (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	z_timer_observer_on_stop(timer);

	ਜੇ (timer->stop_fn != ਨਿਰਾ) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);

		timer->stop_fn(timer);

		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	ਜੇ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;

	}

	ਬਣਤਰ k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	ਜੇ (pending_thread != ਨਿਰਾ) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_timer_stop(ਬਣਤਰ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_stop_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_ਸਥਿਤੀ_get(ਬਣਤਰ k_timer *timer)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);
	uint32_t ਨਤੀਜਾ = timer->ਸਥਿਤੀ;

	timer->ਸਥਿਤੀ = 0U;
	k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ਨਤੀਜਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t z_vrfy_k_timer_ਸਥਿਤੀ_get(ਬਣਤਰ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ਵਾਪਸ z_impl_k_timer_ਸਥਿਤੀ_get(timer);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_ਸਥਿਤੀ_sync(ਬਣਤਰ k_timer *timer)
{
	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "");
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_timer, ਸਥਿਤੀ_sync, timer);

	ਜੇ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t ਨਤੀਜਾ;

		ਕਰੋ {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ = irq_ਤਾਲਾ();

			ਜੇ (!z_is_inactive_timeout(&timer->timeout)) {
				ਨਤੀਜਾ = *(ਅਸਥਿਰ uint32_t *)&timer->ਸਥਿਤੀ;
				timer->ਸਥਿਤੀ = 0U;
				ਜੇ (ਨਤੀਜਾ > 0) {
					irq_unlock(ਕੁੰਜੀ);
					ਰੋਕੋ;
				} ਨਹੀਂਤਾਂ {
					k_cpu_atomic_idle(ਕੁੰਜੀ);
				}
			} ਨਹੀਂਤਾਂ {
				ਨਤੀਜਾ = timer->ਸਥਿਤੀ;
				irq_unlock(ਕੁੰਜੀ);
				ਰੋਕੋ;
			}
		} ਜਦੋਂ (true);

		ਵਾਪਸ ਨਤੀਜਾ;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);
	uint32_t ਨਤੀਜਾ = timer->ਸਥਿਤੀ;

	ਜੇ (ਨਤੀਜਾ == 0U) {
		ਜੇ (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_timer, ਸਥਿਤੀ_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(ਖਾਲੀ)z_pend_curr(&timer_ਤਾਲਾ, ਕੁੰਜੀ, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timer_ਤਾਲਾ);
			ਨਤੀਜਾ = timer->ਸਥਿਤੀ;
		} ਨਹੀਂਤਾਂ {
			/* timer is already stopped */
		}
	} ਨਹੀਂਤਾਂ {
		/* timer has already expired at least once */
	}

	timer->ਸਥਿਤੀ = 0U;
	k_spin_unlock(&timer_ਤਾਲਾ, ਕੁੰਜੀ);

	/**
	 * @note	New tracing hook
	 */
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_timer, ਸਥਿਤੀ_sync, timer, ਨਤੀਜਾ);

	ਵਾਪਸ ਨਤੀਜਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t z_vrfy_k_timer_ਸਥਿਤੀ_sync(ਬਣਤਰ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ਵਾਪਸ z_impl_k_timer_ਸਥਿਤੀ_sync(timer);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_vrfy_k_timer_ਬਚਿਆ_ticks(
						ਅਚਲ ਬਣਤਰ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ਵਾਪਸ z_impl_k_timer_ਬਚਿਆ_ticks(timer);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_vrfy_k_timer_expires_ticks(
						ਅਚਲ ਬਣਤਰ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ਵਾਪਸ z_impl_k_timer_expires_ticks(timer);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *z_vrfy_k_timer_user_ਅੰਕੜੇ_get(ਅਚਲ ਬਣਤਰ k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	ਵਾਪਸ z_impl_k_timer_user_ਅੰਕੜੇ_get(timer);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_timer_user_ਅੰਕੜੇ_ਸੈੱਟ(ਬਣਤਰ k_timer *timer,
						ਖਾਲੀ *user_ਅੰਕੜੇ)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_ਅੰਕੜੇ_ਸੈੱਟ(timer, user_ਅੰਕੜੇ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
