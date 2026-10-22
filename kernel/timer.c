/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <wait_q.h>

സ്ഥിതം ഘടന k_spinlock timer_lock;

#ifdef CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_TIMER */

#എങ്കിൽ defined(CONFIG_TIMER_OBSERVER)
സ്ഥിതം വരിയിൽ ശൂന്യം z_timer_observer_on_init(ഘടന k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		എങ്കിൽ (obs->on_init != ശൂന്യ) {
			obs->on_init(timer);
		}
	}
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_timer_observer_on_start(ഘടന k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		എങ്കിൽ (obs->on_start != ശൂന്യ) {
			obs->on_start(timer, duration, period);
		}
	}
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_timer_observer_on_stop(ഘടന k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		എങ്കിൽ (obs->on_stop != ശൂന്യ) {
			obs->on_stop(timer);
		}
	}
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_timer_observer_on_expiry(ഘടന k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		എങ്കിൽ (obs->on_expiry != ശൂന്യ) {
			obs->on_expiry(timer);
		}
	}
}
#അല്ലെങ്കിൽ
#define z_timer_observer_on_init(timer) (ശൂന്യം)0
#define z_timer_observer_on_start(timer, duration, period) (ശൂന്യം)0
#define z_timer_observer_on_stop(timer) (ശൂന്യം)0
#define z_timer_observer_on_expiry(timer) (ശൂന്യം)0
#endif /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
ശൂന്യം z_timer_expiration_handler(ഘടന _timeout *t)
{
	ഘടന k_timer *timer = CONTAINER_OF(t, ഘടന k_timer, timeout);
	ഘടന k_thread *thread;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	എങ്കിൽ (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, താക്കോൽ);
		തിരികെ;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	എങ്കിൽ (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
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
	എങ്കിൽ (timer->expiry_fn != ശൂന്യ) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, താക്കോൽ);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		താക്കോൽ = k_spin_lock(&timer_lock);
	}

	എങ്കിൽ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, താക്കോൽ);
		തിരികെ;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(&timer->wait_q);

		എങ്കിൽ (thread != ശൂന്യ) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_മൂല്യം_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, താക്കോൽ);

}


പൂർണ്ണസംഖ്യ z_timer_cleanup(ഘടന k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	പൂർണ്ണസംഖ്യ ret = 0;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	താക്കോൽ = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_head_locked(&timer->wait_q) != ശൂന്യ)) {
		ret = -EAGAIN;
		പോകുക out;
	}

	CHECKIF(!locked && (z_waitq_head(&timer->wait_q) != ശൂന്യ)) {
		ret = -EAGAIN;
		പോകുക out;
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
	എങ്കിൽ (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, താക്കോൽ);
		പോകുക retry;
	}

out:
	k_spin_unlock(&timer_lock, താക്കോൽ);

#ifdef CONFIG_OBJ_CORE_TIMER
	എങ്കിൽ (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#endif /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_timer_cleanup(ഘടന k_timer *timer)
{
	തിരികെ z_timer_cleanup(timer, false);
}


ശൂന്യം k_timer_init(ഘടന k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	എങ്കിൽ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_data = ശൂന്യ;

	k_object_init(timer);

#ifdef CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#endif /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


ശൂന്യം z_impl_k_timer_start(ഘടന k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	എങ്കിൽ (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		തിരികെ;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&timer_lock);

	(ശൂന്യം)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, താക്കോൽ);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_timer_start(ഘടന k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_start_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_impl_k_timer_stop(ഘടന k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&timer_lock);

	എങ്കിൽ (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, താക്കോൽ);
		തിരികെ;
	}

	z_timer_observer_on_stop(timer);

	എങ്കിൽ (timer->stop_fn != ശൂന്യ) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, താക്കോൽ);

		timer->stop_fn(timer);

		താക്കോൽ = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	എങ്കിൽ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, താക്കോൽ);
		തിരികെ;

	}

	ഘടന k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	എങ്കിൽ (pending_thread != ശൂന്യ) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&timer_lock, താക്കോൽ);
	}
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_timer_stop(ഘടന k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_stop_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(ഘടന k_timer *timer)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, താക്കോൽ);

	തിരികെ result;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ uint32_t z_vrfy_k_timer_status_get(ഘടന k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	തിരികെ z_impl_k_timer_status_get(timer);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(ഘടന k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	എങ്കിൽ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t result;

		ചെയ്യുക {
			unsigned പൂർണ്ണസംഖ്യ താക്കോൽ = irq_lock();

			എങ്കിൽ (!z_is_inactive_timeout(&timer->timeout)) {
				result = *(അസ്ഥിര uint32_t *)&timer->status;
				timer->status = 0U;
				എങ്കിൽ (result > 0) {
					irq_unlock(താക്കോൽ);
					നിർത്തുക;
				} അല്ലെങ്കിൽ {
					k_cpu_atomic_idle(താക്കോൽ);
				}
			} അല്ലെങ്കിൽ {
				result = timer->status;
				irq_unlock(താക്കോൽ);
				നിർത്തുക;
			}
		} ആകുമ്പോൾ (true);

		തിരികെ result;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	എങ്കിൽ (result == 0U) {
		എങ്കിൽ (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(ശൂന്യം)z_pend_curr(&timer_lock, താക്കോൽ, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			താക്കോൽ = k_spin_lock(&timer_lock);
			result = timer->status;
		} അല്ലെങ്കിൽ {
			/* timer is already stopped */
		}
	} അല്ലെങ്കിൽ {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, താക്കോൽ);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, result);

	തിരികെ result;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ uint32_t z_vrfy_k_timer_status_sync(ഘടന k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	തിരികെ z_impl_k_timer_status_sync(timer);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

സ്ഥിതം വരിയിൽ k_ticks_t z_vrfy_k_timer_remaining_ticks(
						സ്ഥിരം ഘടന k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	തിരികെ z_impl_k_timer_remaining_ticks(timer);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

സ്ഥിതം വരിയിൽ k_ticks_t z_vrfy_k_timer_expires_ticks(
						സ്ഥിരം ഘടന k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	തിരികെ z_impl_k_timer_expires_ticks(timer);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം *z_vrfy_k_timer_user_data_get(സ്ഥിരം ഘടന k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	തിരികെ z_impl_k_timer_user_data_get(timer);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_timer_user_data_set(ഘടന k_timer *timer,
						ശൂന്യം *user_data)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_data_set(timer, user_data);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#endif /* CONFIG_USERSPACE */
