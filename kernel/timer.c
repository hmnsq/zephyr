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

ساکن ساخت k_spinlock timer_تالا;

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_TIMER */

#اگر defined(CONFIG_TIMER_OBSERVER)
ساکن اندرونی خالی z_timer_observer_on_init(ساخت k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_init != تہی) {
			obs->on_init(timer);
		}
	}
}

ساکن اندرونی خالی z_timer_observer_on_آغاز(ساخت k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_آغاز != تہی) {
			obs->on_آغاز(timer, duration, period);
		}
	}
}

ساکن اندرونی خالی z_timer_observer_on_stop(ساخت k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_stop != تہی) {
			obs->on_stop(timer);
		}
	}
}

ساکن اندرونی خالی z_timer_observer_on_expiry(ساخت k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		اگر (obs->on_expiry != تہی) {
			obs->on_expiry(timer);
		}
	}
}
#ورنہ
#تعریف_کرو z_timer_observer_on_init(timer) (خالی)0
#تعریف_کرو z_timer_observer_on_آغاز(timer, duration, period) (خالی)0
#تعریف_کرو z_timer_observer_on_stop(timer) (خالی)0
#تعریف_کرو z_timer_observer_on_expiry(timer) (خالی)0
#اگر_ختم /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
خالی z_timer_expiration_handler(ساخت _timeout *t)
{
	ساخت k_timer *timer = CONTAINER_کا(t, ساخت k_timer, timeout);
	ساخت k_thread *thread;
	k_spinlock_کلید_t کلید = k_spin_تالا(&timer_تالا);

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
		k_spin_unlock(&timer_تالا, کلید);
		واپس;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	اگر (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
	    !K_TIMEOUT_EQ(timer->period, K_FOREVER)) {
		k_timeout_t اگلا = timer->period;

#اگر_تعریف_شدہ CONFIG_TIMEOUT_64_ثنائی_ہندسہ
		/* Exploit the fact that uptime during a kernel
		 * timeout handler reflects the time of the scheduled
		 * event and not real time to get some inexpensive
		 * protection against late interrupts.  If we're
		 * delayed for any reason, we still end up calculating
		 * the next expiration as a regular stride from where
		 * we "should" have run.  Requires absolute timeouts.
		 */
		اگلا = K_TIMEOUT_ABS_TICKS(k_uptime_ticks() + اگلا.ticks);
#اگر_ختم /* CONFIG_TIMEOUT_64BIT */
		z_جوڑو_timeout(&timer->timeout, z_timer_expiration_handler,
			      اگلا);
	}

	/* update timer's status */
	timer->حالت_2 += 1U;

	z_timer_observer_on_expiry(timer);

	/* invoke timer expiry function */
	اگر (timer->expiry_fn != تہی) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_تالا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		کلید = k_spin_تالا(&timer_تالا);
	}

	اگر (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_تالا, کلید);
		واپس;
	}

	تالا_SCHED_SPINLOCK {
		thread = z_waitq_سر_locked(&timer->wait_q);

		اگر (thread != تہی) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_قدر_سیٹ(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_تالا, کلید);

}


صحیح z_timer_cleanup(ساخت k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_اندر_isr(), "");

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	صحیح ret = 0;
	k_spinlock_کلید_t کلید;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	کلید = k_spin_تالا(&timer_تالا);

	CHECKIF(locked && (z_waitq_سر_locked(&timer->wait_q) != تہی)) {
		ret = -EAGAIN;
		جاؤ برآمد;
	}

	CHECKIF(!locked && (z_waitq_سر(&timer->wait_q) != تہی)) {
		ret = -EAGAIN;
		جاؤ برآمد;
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
		k_spin_unlock(&timer_تالا, کلید);
		جاؤ retry;
	}

برآمد:
	k_spin_unlock(&timer_تالا, کلید);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_TIMER
	اگر (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#اگر_ختم /* CONFIG_OBJ_CORE_TIMER */

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	واپس ret;
}

صحیح k_timer_cleanup(ساخت k_timer *timer)
{
	واپس z_timer_cleanup(timer, false);
}


خالی k_timer_init(ساخت k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->حالت_2 = 0U;

	اگر (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_پورٹ_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_ڈیٹا = تہی;

	k_object_init(timer);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#اگر_ختم /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


خالی z_impl_k_timer_آغاز(ساخت k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_پورٹ_TRACING_OBJ_FUNC(k_timer, آغاز, timer, duration, period);

	اگر (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		واپس;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_کلید_t کلید = k_spin_تالا(&timer_تالا);

	(خالی)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->حالت_2 = 0U;

	z_جوڑو_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_آغاز(timer, duration, period);

	k_spin_unlock(&timer_تالا, کلید);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_timer_آغاز(ساخت k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_آغاز(timer, duration, period);
}
#شامل <zephyr/syscalls/k_timer_start_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_impl_k_timer_stop(ساخت k_timer *timer)
{
	SYS_پورٹ_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_کلید_t کلید = k_spin_تالا(&timer_تالا);

	اگر (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_تالا, کلید);
		واپس;
	}

	z_timer_observer_on_stop(timer);

	اگر (timer->stop_fn != تہی) {
		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_تالا, کلید);

		timer->stop_fn(timer);

		کلید = k_spin_تالا(&timer_تالا);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	اگر (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_تالا, کلید);
		واپس;

	}

	ساخت k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	اگر (pending_thread != تہی) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_تالا, کلید);
	} ورنہ {
		k_spin_unlock(&timer_تالا, کلید);
	}
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_timer_stop(ساخت k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#شامل <zephyr/syscalls/k_timer_stop_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_حالت_get(ساخت k_timer *timer)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&timer_تالا);
	uint32_t نتیجہ = timer->حالت_2;

	timer->حالت_2 = 0U;
	k_spin_unlock(&timer_تالا, کلید);

	واپس نتیجہ;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی uint32_t z_vrfy_k_timer_حالت_get(ساخت k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	واپس z_impl_k_timer_حالت_get(timer);
}
#شامل <zephyr/syscalls/k_timer_status_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_حالت_sync(ساخت k_timer *timer)
{
	__ASSERT(!arch_is_اندر_isr(), "");
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_timer, حالت_sync, timer);

	اگر (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t نتیجہ;

		کرو {
			غیرعلامتی صحیح کلید = irq_تالا();

			اگر (!z_is_inactive_timeout(&timer->timeout)) {
				نتیجہ = *(متلون uint32_t *)&timer->حالت_2;
				timer->حالت_2 = 0U;
				اگر (نتیجہ > 0) {
					irq_unlock(کلید);
					توڑو;
				} ورنہ {
					k_cpu_atomic_idle(کلید);
				}
			} ورنہ {
				نتیجہ = timer->حالت_2;
				irq_unlock(کلید);
				توڑو;
			}
		} جب (true);

		واپس نتیجہ;
	}

	k_spinlock_کلید_t کلید = k_spin_تالا(&timer_تالا);
	uint32_t نتیجہ = timer->حالت_2;

	اگر (نتیجہ == 0U) {
		اگر (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_timer, حالت_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(خالی)z_pend_curr(&timer_تالا, کلید, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			کلید = k_spin_تالا(&timer_تالا);
			نتیجہ = timer->حالت_2;
		} ورنہ {
			/* timer is already stopped */
		}
	} ورنہ {
		/* timer has already expired at least once */
	}

	timer->حالت_2 = 0U;
	k_spin_unlock(&timer_تالا, کلید);

	/**
	 * @note	New tracing hook
	 */
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_timer, حالت_sync, timer, نتیجہ);

	واپس نتیجہ;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی uint32_t z_vrfy_k_timer_حالت_sync(ساخت k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	واپس z_impl_k_timer_حالت_sync(timer);
}
#شامل <zephyr/syscalls/k_timer_status_sync_mrsh.c>

ساکن اندرونی k_ticks_t z_vrfy_k_timer_بقایا_ticks(
						ثابت ساخت k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	واپس z_impl_k_timer_بقایا_ticks(timer);
}
#شامل <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

ساکن اندرونی k_ticks_t z_vrfy_k_timer_expires_ticks(
						ثابت ساخت k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	واپس z_impl_k_timer_expires_ticks(timer);
}
#شامل <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

ساکن اندرونی خالی *z_vrfy_k_timer_user_ڈیٹا_get(ثابت ساخت k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	واپس z_impl_k_timer_user_ڈیٹا_get(timer);
}
#شامل <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

ساکن اندرونی خالی z_vrfy_k_timer_user_ڈیٹا_سیٹ(ساخت k_timer *timer,
						خالی *user_ڈیٹا)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_ڈیٹا_سیٹ(timer, user_ڈیٹا);
}
#شامل <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#اگر_ختم /* CONFIG_USERSPACE */
