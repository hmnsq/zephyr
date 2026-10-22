/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>

#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/sys/check.h>
#شامل <stdbool.h>
#شامل <zephyr/spinlock.h>
#شامل <ksched.h>
#شامل <wait_q.h>

ایستا ساختار k_spinlock timer_lock;

#ifdef CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_TIMER */

#اگر defined(CONFIG_TIMER_OBSERVER)
ایستا درونخطی تهی z_timer_observer_on_init(ساختار k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_init != پوچ) {
			obs->on_init(timer);
		}
	}
}

ایستا درونخطی تهی z_timer_observer_on_start(ساختار k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_start != پوچ) {
			obs->on_start(timer, duration, period);
		}
	}
}

ایستا درونخطی تهی z_timer_observer_on_stop(ساختار k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_stop != پوچ) {
			obs->on_stop(timer);
		}
	}
}

ایستا درونخطی تهی z_timer_observer_on_expiry(ساختار k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_expiry != پوچ) {
			obs->on_expiry(timer);
		}
	}
}
#وگرنه
#define z_timer_observer_on_init(timer) (تهی)0
#define z_timer_observer_on_start(timer, duration, period) (تهی)0
#define z_timer_observer_on_stop(timer) (تهی)0
#define z_timer_observer_on_expiry(timer) (تهی)0
#endif /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
تهی z_timer_expiration_handler(ساختار _timeout *t)
{
	ساختار k_timer *timer = CONTAINER_OF(t, ساختار k_timer, timeout);
	ساختار k_thread *thread;
	k_spinlock_کلید_t کلید = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	اگر (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, کلید);
		بازگشت;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	اگر (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
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
	اگر (timer->expiry_fn != پوچ) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		کلید = k_spin_lock(&timer_lock);
	}

	اگر (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, کلید);
		بازگشت;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(&timer->wait_q);

		اگر (thread != پوچ) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_مقدار_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, کلید);

}


صحیح z_timer_cleanup(ساختار k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	صحیح ret = 0;
	k_spinlock_کلید_t کلید;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	کلید = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_head_locked(&timer->wait_q) != پوچ)) {
		ret = -EAGAIN;
		برو out;
	}

	CHECKIF(!locked && (z_waitq_head(&timer->wait_q) != پوچ)) {
		ret = -EAGAIN;
		برو out;
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
	اگر (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, کلید);
		برو retry;
	}

out:
	k_spin_unlock(&timer_lock, کلید);

#ifdef CONFIG_OBJ_CORE_TIMER
	اگر (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#endif /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	بازگشت ret;
}

صحیح k_timer_cleanup(ساختار k_timer *timer)
{
	بازگشت z_timer_cleanup(timer, false);
}


تهی k_timer_init(ساختار k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	اگر (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_data = پوچ;

	k_object_init(timer);

#ifdef CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#endif /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


تهی z_impl_k_timer_start(ساختار k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	اگر (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		بازگشت;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_کلید_t کلید = k_spin_lock(&timer_lock);

	(تهی)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, کلید);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_timer_start(ساختار k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#شامل <zephyr/syscalls/k_timer_start_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_impl_k_timer_stop(ساختار k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_کلید_t کلید = k_spin_lock(&timer_lock);

	اگر (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, کلید);
		بازگشت;
	}

	z_timer_observer_on_stop(timer);

	اگر (timer->stop_fn != پوچ) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, کلید);

		timer->stop_fn(timer);

		کلید = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	اگر (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, کلید);
		بازگشت;

	}

	ساختار k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	اگر (pending_thread != پوچ) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, کلید);
	} وگرنه {
		k_spin_unlock(&timer_lock, کلید);
	}
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_timer_stop(ساختار k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#شامل <zephyr/syscalls/k_timer_stop_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(ساختار k_timer *timer)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, کلید);

	بازگشت result;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی uint32_t z_vrfy_k_timer_status_get(ساختار k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	بازگشت z_impl_k_timer_status_get(timer);
}
#شامل <zephyr/syscalls/k_timer_status_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(ساختار k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	اگر (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t result;

		انجامبده {
			unsigned صحیح کلید = irq_lock();

			اگر (!z_is_inactive_timeout(&timer->timeout)) {
				result = *(فرار uint32_t *)&timer->status;
				timer->status = 0U;
				اگر (result > 0) {
					irq_unlock(کلید);
					بشکن;
				} وگرنه {
					k_cpu_atomic_idle(کلید);
				}
			} وگرنه {
				result = timer->status;
				irq_unlock(کلید);
				بشکن;
			}
		} وقتی (true);

		بازگشت result;
	}

	k_spinlock_کلید_t کلید = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	اگر (result == 0U) {
		اگر (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(تهی)z_pend_curr(&timer_lock, کلید, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			کلید = k_spin_lock(&timer_lock);
			result = timer->status;
		} وگرنه {
			/* timer is already stopped */
		}
	} وگرنه {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, کلید);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, result);

	بازگشت result;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی uint32_t z_vrfy_k_timer_status_sync(ساختار k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	بازگشت z_impl_k_timer_status_sync(timer);
}
#شامل <zephyr/syscalls/k_timer_status_sync_mrsh.c>

ایستا درونخطی k_ticks_t z_vrfy_k_timer_remaining_ticks(
						ثابت ساختار k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	بازگشت z_impl_k_timer_remaining_ticks(timer);
}
#شامل <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

ایستا درونخطی k_ticks_t z_vrfy_k_timer_expires_ticks(
						ثابت ساختار k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	بازگشت z_impl_k_timer_expires_ticks(timer);
}
#شامل <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

ایستا درونخطی تهی *z_vrfy_k_timer_user_data_get(ثابت ساختار k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	بازگشت z_impl_k_timer_user_data_get(timer);
}
#شامل <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

ایستا درونخطی تهی z_vrfy_k_timer_user_data_set(ساختار k_timer *timer,
						تهی *user_data)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_data_set(timer, user_data);
}
#شامل <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#endif /* CONFIG_USERSPACE */
