/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/sys/check.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <wait_q.h>

స్థిర నిర్మాణం k_spinlock timer_తాళం;

#నిర్వచితమైతే CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_TIMER */

#ఒకవేళ defined(CONFIG_TIMER_OBSERVER)
స్థిర అంతఃవరుస శూన్య z_timer_observer_on_init(నిర్మాణం k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ఒకవేళ (obs->on_init != ఖాళీ) {
			obs->on_init(timer);
		}
	}
}

స్థిర అంతఃవరుస శూన్య z_timer_observer_on_ప్రారంభం(నిర్మాణం k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ఒకవేళ (obs->on_ప్రారంభం != ఖాళీ) {
			obs->on_ప్రారంభం(timer, duration, period);
		}
	}
}

స్థిర అంతఃవరుస శూన్య z_timer_observer_on_stop(నిర్మాణం k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ఒకవేళ (obs->on_stop != ఖాళీ) {
			obs->on_stop(timer);
		}
	}
}

స్థిర అంతఃవరుస శూన్య z_timer_observer_on_expiry(నిర్మాణం k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		ఒకవేళ (obs->on_expiry != ఖాళీ) {
			obs->on_expiry(timer);
		}
	}
}
#లేకపోతే
#నిర్వచించు z_timer_observer_on_init(timer) (శూన్య)0
#నిర్వచించు z_timer_observer_on_ప్రారంభం(timer, duration, period) (శూన్య)0
#నిర్వచించు z_timer_observer_on_stop(timer) (శూన్య)0
#నిర్వచించు z_timer_observer_on_expiry(timer) (శూన్య)0
#అయితే_ముగింపు /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
శూన్య z_timer_expiration_handler(నిర్మాణం _timeout *t)
{
	నిర్మాణం k_timer *timer = CONTAINER_యొక్క(t, నిర్మాణం k_timer, timeout);
	నిర్మాణం k_thread *thread;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&timer_తాళం);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	ఒకవేళ (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_తాళం, కీలకం);
		తిరిగి;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	ఒకవేళ (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
	    !K_TIMEOUT_EQ(timer->period, K_FOREVER)) {
		k_timeout_t తదుపరి = timer->period;

#నిర్వచితమైతే CONFIG_TIMEOUT_64_బిట్టు
		/* Exploit the fact that uptime during a kernel
		 * timeout handler reflects the time of the scheduled
		 * event and not real time to get some inexpensive
		 * protection against late interrupts.  If we're
		 * delayed for any reason, we still end up calculating
		 * the next expiration as a regular stride from where
		 * we "should" have run.  Requires absolute timeouts.
		 */
		తదుపరి = K_TIMEOUT_ABS_TICKS(k_uptime_ticks() + తదుపరి.ticks);
#అయితే_ముగింపు /* CONFIG_TIMEOUT_64BIT */
		z_కూడు_timeout(&timer->timeout, z_timer_expiration_handler,
			      తదుపరి);
	}

	/* update timer's status */
	timer->స్థితి_2 += 1U;

	z_timer_observer_on_expiry(timer);

	/* invoke timer expiry function */
	ఒకవేళ (timer->expiry_fn != ఖాళీ) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		కీలకం = k_spin_తాళం(&timer_తాళం);
	}

	ఒకవేళ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_తాళం, కీలకం);
		తిరిగి;
	}

	తాళం_SCHED_SPINLOCK {
		thread = z_waitq_తల_locked(&timer->wait_q);

		ఒకవేళ (thread != ఖాళీ) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_విలువ_సమితి(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_తాళం, కీలకం);

}


పూర్ణాంకం z_timer_cleanup(నిర్మాణం k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_లోపల_isr(), "");

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	పూర్ణాంకం ret = 0;
	k_spinlock_కీలకం_t కీలకం;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	కీలకం = k_spin_తాళం(&timer_తాళం);

	CHECKIF(locked && (z_waitq_తల_locked(&timer->wait_q) != ఖాళీ)) {
		ret = -EAGAIN;
		వెళ్ళు బహిర్గతం;
	}

	CHECKIF(!locked && (z_waitq_తల(&timer->wait_q) != ఖాళీ)) {
		ret = -EAGAIN;
		వెళ్ళు బహిర్గతం;
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
	ఒకవేళ (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_తాళం, కీలకం);
		వెళ్ళు retry;
	}

బహిర్గతం:
	k_spin_unlock(&timer_తాళం, కీలకం);

#నిర్వచితమైతే CONFIG_OBJ_CORE_TIMER
	ఒకవేళ (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_TIMER */

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_timer_cleanup(నిర్మాణం k_timer *timer)
{
	తిరిగి z_timer_cleanup(timer, false);
}


శూన్య k_timer_init(నిర్మాణం k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->స్థితి_2 = 0U;

	ఒకవేళ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_పోర్టు_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_దత్తాంశం = ఖాళీ;

	k_object_init(timer);

#నిర్వచితమైతే CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


శూన్య z_impl_k_timer_ప్రారంభం(నిర్మాణం k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC(k_timer, ప్రారంభం, timer, duration, period);

	ఒకవేళ (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		తిరిగి;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&timer_తాళం);

	(శూన్య)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->స్థితి_2 = 0U;

	z_కూడు_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_ప్రారంభం(timer, duration, period);

	k_spin_unlock(&timer_తాళం, కీలకం);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_timer_ప్రారంభం(నిర్మాణం k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_ప్రారంభం(timer, duration, period);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_start_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_impl_k_timer_stop(నిర్మాణం k_timer *timer)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&timer_తాళం);

	ఒకవేళ (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_తాళం, కీలకం);
		తిరిగి;
	}

	z_timer_observer_on_stop(timer);

	ఒకవేళ (timer->stop_fn != ఖాళీ) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_తాళం, కీలకం);

		timer->stop_fn(timer);

		కీలకం = k_spin_తాళం(&timer_తాళం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	ఒకవేళ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_తాళం, కీలకం);
		తిరిగి;

	}

	నిర్మాణం k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	ఒకవేళ (pending_thread != ఖాళీ) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&timer_తాళం, కీలకం);
	}
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_timer_stop(నిర్మాణం k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_stop_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_స్థితి_get(నిర్మాణం k_timer *timer)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&timer_తాళం);
	uint32_t ఫలితం = timer->స్థితి_2;

	timer->స్థితి_2 = 0U;
	k_spin_unlock(&timer_తాళం, కీలకం);

	తిరిగి ఫలితం;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస uint32_t z_vrfy_k_timer_స్థితి_get(నిర్మాణం k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	తిరిగి z_impl_k_timer_స్థితి_get(timer);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_స్థితి_sync(నిర్మాణం k_timer *timer)
{
	__ASSERT(!arch_is_లోపల_isr(), "");
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_timer, స్థితి_sync, timer);

	ఒకవేళ (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t ఫలితం;

		చేయి {
			గుర్తులేని పూర్ణాంకం కీలకం = irq_తాళం();

			ఒకవేళ (!z_is_inactive_timeout(&timer->timeout)) {
				ఫలితం = *(అస్థిర uint32_t *)&timer->స్థితి_2;
				timer->స్థితి_2 = 0U;
				ఒకవేళ (ఫలితం > 0) {
					irq_unlock(కీలకం);
					ఆపు;
				} లేకపోతే {
					k_cpu_atomic_idle(కీలకం);
				}
			} లేకపోతే {
				ఫలితం = timer->స్థితి_2;
				irq_unlock(కీలకం);
				ఆపు;
			}
		} అయితే (true);

		తిరిగి ఫలితం;
	}

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&timer_తాళం);
	uint32_t ఫలితం = timer->స్థితి_2;

	ఒకవేళ (ఫలితం == 0U) {
		ఒకవేళ (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_timer, స్థితి_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(శూన్య)z_pend_curr(&timer_తాళం, కీలకం, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			కీలకం = k_spin_తాళం(&timer_తాళం);
			ఫలితం = timer->స్థితి_2;
		} లేకపోతే {
			/* timer is already stopped */
		}
	} లేకపోతే {
		/* timer has already expired at least once */
	}

	timer->స్థితి_2 = 0U;
	k_spin_unlock(&timer_తాళం, కీలకం);

	/**
	 * @note	New tracing hook
	 */
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_timer, స్థితి_sync, timer, ఫలితం);

	తిరిగి ఫలితం;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస uint32_t z_vrfy_k_timer_స్థితి_sync(నిర్మాణం k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	తిరిగి z_impl_k_timer_స్థితి_sync(timer);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

స్థిర అంతఃవరుస k_ticks_t z_vrfy_k_timer_మిగిలిన_ticks(
						స్థిరరాశి నిర్మాణం k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	తిరిగి z_impl_k_timer_మిగిలిన_ticks(timer);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

స్థిర అంతఃవరుస k_ticks_t z_vrfy_k_timer_expires_ticks(
						స్థిరరాశి నిర్మాణం k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	తిరిగి z_impl_k_timer_expires_ticks(timer);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

స్థిర అంతఃవరుస శూన్య *z_vrfy_k_timer_user_దత్తాంశం_get(స్థిరరాశి నిర్మాణం k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	తిరిగి z_impl_k_timer_user_దత్తాంశం_get(timer);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

స్థిర అంతఃవరుస శూన్య z_vrfy_k_timer_user_దత్తాంశం_సమితి(నిర్మాణం k_timer *timer,
						శూన్య *user_దత్తాంశం)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_దత్తాంశం_సమితి(timer, user_దత్తాంశం);
}
#అన్తర్భావ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#అయితే_ముగింపు /* CONFIG_USERSPACE */
