/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/sys/check.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <wait_q.h>

स्थिर संरचना k_spinlock timer_lock;

#ifdef CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_TIMER */

#सचे defined(CONFIG_TIMER_OBSERVER)
स्थिर अन्तःस्थ शून्य z_timer_observer_on_init(संरचना k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		सचे (obs->on_init != रिक्त) {
			obs->on_init(timer);
		}
	}
}

स्थिर अन्तःस्थ शून्य z_timer_observer_on_start(संरचना k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		सचे (obs->on_start != रिक्त) {
			obs->on_start(timer, duration, period);
		}
	}
}

स्थिर अन्तःस्थ शून्य z_timer_observer_on_stop(संरचना k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		सचे (obs->on_stop != रिक्त) {
			obs->on_stop(timer);
		}
	}
}

स्थिर अन्तःस्थ शून्य z_timer_observer_on_expiry(संरचना k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		सचे (obs->on_expiry != रिक्त) {
			obs->on_expiry(timer);
		}
	}
}
#अञ्ञथा
#define z_timer_observer_on_init(timer) (शून्य)0
#define z_timer_observer_on_start(timer, duration, period) (शून्य)0
#define z_timer_observer_on_stop(timer) (शून्य)0
#define z_timer_observer_on_expiry(timer) (शून्य)0
#endif /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
शून्य z_timer_expiration_handler(संरचना _timeout *t)
{
	संरचना k_timer *timer = CONTAINER_OF(t, संरचना k_timer, timeout);
	संरचना k_thread *thread;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	सचे (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, कुञ्चिका);
		पटिनिवत्त;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	सचे (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
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
	सचे (timer->expiry_fn != रिक्त) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, कुञ्चिका);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		कुञ्चिका = k_spin_lock(&timer_lock);
	}

	सचे (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, कुञ्चिका);
		पटिनिवत्त;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(&timer->wait_q);

		सचे (thread != रिक्त) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_मानम्_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, कुञ्चिका);

}


पूर्णाङ्क z_timer_cleanup(संरचना k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	पूर्णाङ्क ret = 0;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	कुञ्चिका = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_head_locked(&timer->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
	}

	CHECKIF(!locked && (z_waitq_head(&timer->wait_q) != रिक्त)) {
		ret = -EAGAIN;
		गच्छ out;
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
	सचे (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, कुञ्चिका);
		गच्छ retry;
	}

out:
	k_spin_unlock(&timer_lock, कुञ्चिका);

#ifdef CONFIG_OBJ_CORE_TIMER
	सचे (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#endif /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	पटिनिवत्त ret;
}

पूर्णाङ्क k_timer_cleanup(संरचना k_timer *timer)
{
	पटिनिवत्त z_timer_cleanup(timer, false);
}


शून्य k_timer_init(संरचना k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	सचे (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_data = रिक्त;

	k_object_init(timer);

#ifdef CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#endif /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


शून्य z_impl_k_timer_start(संरचना k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	सचे (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		पटिनिवत्त;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&timer_lock);

	(शून्य)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, कुञ्चिका);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_timer_start(संरचना k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_start_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_timer_stop(संरचना k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&timer_lock);

	सचे (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, कुञ्चिका);
		पटिनिवत्त;
	}

	z_timer_observer_on_stop(timer);

	सचे (timer->stop_fn != रिक्त) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, कुञ्चिका);

		timer->stop_fn(timer);

		कुञ्चिका = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	सचे (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, कुञ्चिका);
		पटिनिवत्त;

	}

	संरचना k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	सचे (pending_thread != रिक्त) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, कुञ्चिका);
	} अञ्ञथा {
		k_spin_unlock(&timer_lock, कुञ्चिका);
	}
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_timer_stop(संरचना k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_stop_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(संरचना k_timer *timer)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, कुञ्चिका);

	पटिनिवत्त result;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ uint32_t z_vrfy_k_timer_status_get(संरचना k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	पटिनिवत्त z_impl_k_timer_status_get(timer);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_status_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(संरचना k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	सचे (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t result;

		करोहि {
			unsigned पूर्णाङ्क कुञ्चिका = irq_lock();

			सचे (!z_is_inactive_timeout(&timer->timeout)) {
				result = *(चञ्चल uint32_t *)&timer->status;
				timer->status = 0U;
				सचे (result > 0) {
					irq_unlock(कुञ्चिका);
					भङ्ग;
				} अञ्ञथा {
					k_cpu_atomic_idle(कुञ्चिका);
				}
			} अञ्ञथा {
				result = timer->status;
				irq_unlock(कुञ्चिका);
				भङ्ग;
			}
		} याव (true);

		पटिनिवत्त result;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	सचे (result == 0U) {
		सचे (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(शून्य)z_pend_curr(&timer_lock, कुञ्चिका, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			कुञ्चिका = k_spin_lock(&timer_lock);
			result = timer->status;
		} अञ्ञथा {
			/* timer is already stopped */
		}
	} अञ्ञथा {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, कुञ्चिका);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, result);

	पटिनिवत्त result;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ uint32_t z_vrfy_k_timer_status_sync(संरचना k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	पटिनिवत्त z_impl_k_timer_status_sync(timer);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_status_sync_mrsh.c>

स्थिर अन्तःस्थ k_ticks_t z_vrfy_k_timer_remaining_ticks(
						ध्रुव संरचना k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	पटिनिवत्त z_impl_k_timer_remaining_ticks(timer);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

स्थिर अन्तःस्थ k_ticks_t z_vrfy_k_timer_expires_ticks(
						ध्रुव संरचना k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	पटिनिवत्त z_impl_k_timer_expires_ticks(timer);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_timer_user_data_get(ध्रुव संरचना k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	पटिनिवत्त z_impl_k_timer_user_data_get(timer);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_timer_user_data_set(संरचना k_timer *timer,
						शून्य *user_data)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_data_set(timer, user_data);
}
#अन्तर्भाव <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#endif /* CONFIG_USERSPACE */
