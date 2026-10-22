/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/sys/check.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <wait_q.h>

நிலையான கட்டமைப்பு k_spinlock timer_lock;

#ifdef CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_TIMER */

#எனில் defined(CONFIG_TIMER_OBSERVER)
நிலையான உள்வரி வெறுமை z_timer_observer_on_init(கட்டமைப்பு k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		எனில் (obs->on_init != வெற்று) {
			obs->on_init(timer);
		}
	}
}

நிலையான உள்வரி வெறுமை z_timer_observer_on_start(கட்டமைப்பு k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		எனில் (obs->on_start != வெற்று) {
			obs->on_start(timer, duration, period);
		}
	}
}

நிலையான உள்வரி வெறுமை z_timer_observer_on_stop(கட்டமைப்பு k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		எனில் (obs->on_stop != வெற்று) {
			obs->on_stop(timer);
		}
	}
}

நிலையான உள்வரி வெறுமை z_timer_observer_on_expiry(கட்டமைப்பு k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		எனில் (obs->on_expiry != வெற்று) {
			obs->on_expiry(timer);
		}
	}
}
#இல்லையெனில்
#define z_timer_observer_on_init(timer) (வெறுமை)0
#define z_timer_observer_on_start(timer, duration, period) (வெறுமை)0
#define z_timer_observer_on_stop(timer) (வெறுமை)0
#define z_timer_observer_on_expiry(timer) (வெறுமை)0
#endif /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
வெறுமை z_timer_expiration_handler(கட்டமைப்பு _timeout *t)
{
	கட்டமைப்பு k_timer *timer = CONTAINER_OF(t, கட்டமைப்பு k_timer, timeout);
	கட்டமைப்பு k_thread *thread;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	எனில் (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, சாவி);
		திரும்பு;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	எனில் (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
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
	எனில் (timer->expiry_fn != வெற்று) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, சாவி);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		சாவி = k_spin_lock(&timer_lock);
	}

	எனில் (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, சாவி);
		திரும்பு;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(&timer->wait_q);

		எனில் (thread != வெற்று) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_மதிப்பு_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, சாவி);

}


முழு z_timer_cleanup(கட்டமைப்பு k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	முழு ret = 0;
	k_spinlock_சாவி_t சாவி;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	சாவி = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_head_locked(&timer->wait_q) != வெற்று)) {
		ret = -EAGAIN;
		செல் out;
	}

	CHECKIF(!locked && (z_waitq_head(&timer->wait_q) != வெற்று)) {
		ret = -EAGAIN;
		செல் out;
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
	எனில் (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, சாவி);
		செல் retry;
	}

out:
	k_spin_unlock(&timer_lock, சாவி);

#ifdef CONFIG_OBJ_CORE_TIMER
	எனில் (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#endif /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	திரும்பு ret;
}

முழு k_timer_cleanup(கட்டமைப்பு k_timer *timer)
{
	திரும்பு z_timer_cleanup(timer, false);
}


வெறுமை k_timer_init(கட்டமைப்பு k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	எனில் (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_data = வெற்று;

	k_object_init(timer);

#ifdef CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#endif /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


வெறுமை z_impl_k_timer_start(கட்டமைப்பு k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	எனில் (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		திரும்பு;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_சாவி_t சாவி = k_spin_lock(&timer_lock);

	(வெறுமை)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, சாவி);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_timer_start(கட்டமைப்பு k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_start_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_impl_k_timer_stop(கட்டமைப்பு k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_சாவி_t சாவி = k_spin_lock(&timer_lock);

	எனில் (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, சாவி);
		திரும்பு;
	}

	z_timer_observer_on_stop(timer);

	எனில் (timer->stop_fn != வெற்று) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, சாவி);

		timer->stop_fn(timer);

		சாவி = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	எனில் (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, சாவி);
		திரும்பு;

	}

	கட்டமைப்பு k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	எனில் (pending_thread != வெற்று) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&timer_lock, சாவி);
	}
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_timer_stop(கட்டமைப்பு k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_stop_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(கட்டமைப்பு k_timer *timer)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, சாவி);

	திரும்பு result;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி uint32_t z_vrfy_k_timer_status_get(கட்டமைப்பு k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	திரும்பு z_impl_k_timer_status_get(timer);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(கட்டமைப்பு k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	எனில் (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t result;

		செய் {
			unsigned முழு சாவி = irq_lock();

			எனில் (!z_is_inactive_timeout(&timer->timeout)) {
				result = *(நிலையற்ற uint32_t *)&timer->status;
				timer->status = 0U;
				எனில் (result > 0) {
					irq_unlock(சாவி);
					நிறுத்து;
				} இல்லையெனில் {
					k_cpu_atomic_idle(சாவி);
				}
			} இல்லையெனில் {
				result = timer->status;
				irq_unlock(சாவி);
				நிறுத்து;
			}
		} போது (true);

		திரும்பு result;
	}

	k_spinlock_சாவி_t சாவி = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	எனில் (result == 0U) {
		எனில் (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(வெறுமை)z_pend_curr(&timer_lock, சாவி, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			சாவி = k_spin_lock(&timer_lock);
			result = timer->status;
		} இல்லையெனில் {
			/* timer is already stopped */
		}
	} இல்லையெனில் {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, சாவி);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, result);

	திரும்பு result;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி uint32_t z_vrfy_k_timer_status_sync(கட்டமைப்பு k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	திரும்பு z_impl_k_timer_status_sync(timer);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

நிலையான உள்வரி k_ticks_t z_vrfy_k_timer_remaining_ticks(
						மாறிலி கட்டமைப்பு k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	திரும்பு z_impl_k_timer_remaining_ticks(timer);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

நிலையான உள்வரி k_ticks_t z_vrfy_k_timer_expires_ticks(
						மாறிலி கட்டமைப்பு k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	திரும்பு z_impl_k_timer_expires_ticks(timer);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

நிலையான உள்வரி வெறுமை *z_vrfy_k_timer_user_data_get(மாறிலி கட்டமைப்பு k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	திரும்பு z_impl_k_timer_user_data_get(timer);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

நிலையான உள்வரி வெறுமை z_vrfy_k_timer_user_data_set(கட்டமைப்பு k_timer *timer,
						வெறுமை *user_data)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_data_set(timer, user_data);
}
#அந்தர்பாவ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#endif /* CONFIG_USERSPACE */
