/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/sys/atomic.h>
#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <ipi.h>
#అన్తర్భావ <timeslicing.h>

స్థిర పూర్ణాంకం ముక్క_ticks = DIV_విడత_పైకి(CONFIG_TIMESLICE_కొలత * Z_HZ_ticks, Z_HZ_ms);
స్థిర పూర్ణాంకం ముక్క_max_prio = CONFIG_TIMESLICE_ప్రాధాన్యత;
స్థిర నిర్మాణం _timeout ముక్క_timeouts[CONFIG_MP_MAX_అంకె_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
స్థిర atomic_t ముక్క_expired[CONFIG_MP_MAX_అంకె_CPUS];

#నిర్వచితమైతే CONFIG_మార్పిడి_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
నిర్మాణం k_thread *pending_current;
#అయితే_ముగింపు

స్థిర అంతఃవరుస పూర్ణాంకం ముక్క_సమయం(నిర్మాణం k_thread *thread)
{
	పూర్ణాంకం ret = ముక్క_ticks;

#నిర్వచితమైతే CONFIG_TIMESLICE_చొప్పున_THREAD
	ఒకవేళ (thread->పునాది.ముక్క_ticks != 0) {
		ret = thread->పునాది.ముక్క_ticks;
	}
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు
	తిరిగి ret;
}

స్థిర పూర్ణాంకం z_సమయం_ముక్క_కొలత(నిర్మాణం k_thread *thread)
{
	ఒకవేళ (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (ముక్క_సమయం(thread) == 0)) {
		తిరిగి 0;
	}

#నిర్వచితమైతే CONFIG_TIMESLICE_చొప్పున_THREAD
	ఒకవేళ (thread->పునాది.ముక్క_ticks != 0) {
		తిరిగి thread->పునాది.ముక్క_ticks;
	}
#అయితే_ముగింపు

	ఒకవేళ (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->పునాది.prio, ముక్క_max_prio)) {
		తిరిగి ముక్క_ticks;
	}

	తిరిగి 0;
}

స్థిర శూన్య ముక్క_timeout(నిర్మాణం _timeout *timeout)
{
	పూర్ణాంకం cpu = ARRAY_సూచీ(ముక్క_timeouts, timeout);

	atomic_సమితి(&ముక్క_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	ఒకవేళ (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_ముసుగు(cpu));
	}
}

స్థిర శూన్య ముక్క_reset(పూర్ణాంకం ముక్క_కొలత)
{
	పూర్ణాంకం cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(శూన్య)z_try_abort_timeout(&ముక్క_timeouts[cpu]);
	ఒకవేళ (ముక్క_కొలత != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		పూర్ణాంకం delay = atomic_get(&ముక్క_expired[cpu]) ? ముక్క_కొలత - 1 : ముక్క_కొలత;

		z_కూడు_timeout(&ముక్క_timeouts[cpu], ముక్క_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&ముక్క_expired[cpu]);
}

శూన్య z_సమయం_ముక్క_reset(నిర్మాణం k_thread *thread)
{
	ముక్క_reset(z_సమయం_ముక్క_కొలత(thread));
}

స్థిర ALWAYS_INLINE bool thread_నిర్వచిస్తుంది_సమయం_ముక్క_కొలత(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_TIMESLICE_చొప్పున_THREAD
	తిరిగి (thread->పునాది.ముక్క_ticks != 0);
#లేకపోతే  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_వాడని(thread);
	తిరిగి false;
#అయితే_ముగింపు /* !CONFIG_TIMESLICE_PER_THREAD */
}

శూన్య k_sched_సమయం_ముక్క_సమితి(int32_t ముక్క, పూర్ణాంకం prio)
{
	Z_SCHED_SPINLOCK {

		ముక్క_ticks = k_ms_గమ్యం_ticks_ceil32(ముక్క);
		ముక్క_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		ఒకవేళ (!thread_నిర్వచిస్తుంది_సమయం_ముక్క_కొలత(_current)) {
			z_సమయం_ముక్క_reset(_current);
		}
	}
}

#నిర్వచితమైతే CONFIG_TIMESLICE_చొప్పున_THREAD
శూన్య k_thread_సమయం_ముక్క_సమితి(నిర్మాణం k_thread *thread, int32_t thread_ముక్క_ticks,
			     k_thread_timeslice_fn_t expired, శూన్య *దత్తాంశం)
{
	Z_SCHED_SPINLOCK {
		thread->పునాది.ముక్క_ticks = thread_ముక్క_ticks;
		thread->పునాది.ముక్క_expired = expired;
		thread->పునాది.ముక్క_దత్తాంశం = దత్తాంశం;
		z_సమయం_ముక్క_reset(thread);
	}
}
#అయితే_ముగింపు

/* Called out of each timer and IPI interrupt */
శూన్య z_సమయం_ముక్క(శూన్య)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	ఒకవేళ (!IS_ENABLED(CONFIG_మార్పిడి_NONATOMIC) &&
	    !atomic_get(&ముక్క_expired[_current_cpu->id])) {
		తిరిగి;
	}

	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();
	నిర్మాణం k_thread *curr = _current;

#నిర్వచితమైతే CONFIG_మార్పిడి_NONATOMIC
	ఒకవేళ (pending_current == curr) {
		z_సమయం_ముక్క_reset(curr);
		z_sched_spinlock_unlock(కీలకం);
		తిరిగి;
	}
	pending_current = ఖాళీ;
#అయితే_ముగింపు

	పూర్ణాంకం ముక్క_కొలత = 0;

	ఒకవేళ (atomic_get(&ముక్క_expired[_current_cpu->id])) {
		ముక్క_కొలత = z_సమయం_ముక్క_కొలత(curr);
	}

	ఒకవేళ (ముక్క_కొలత != 0) {
#నిర్వచితమైతే CONFIG_TIMESLICE_చొప్పున_THREAD
		k_thread_timeslice_fn_t handler = curr->పునాది.ముక్క_expired;

		ఒకవేళ (handler != ఖాళీ) {
			z_sched_spinlock_unlock(కీలకం);
			handler(curr, curr->పునాది.ముక్క_దత్తాంశం);
			కీలకం = z_sched_spinlock_తాళం();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			ముక్క_కొలత = z_సమయం_ముక్క_కొలత(curr);
		}
#అయితే_ముగింపు
		ఒకవేళ (!z_is_thread_prevented_from_running(curr)) {
			move_current_గమ్యం_చివర_యొక్క_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#నిర్వచితమైతే CONFIG_SMP
			నిర్మాణం k_thread *తదుపరి = runq_ఉత్తమ();

			ఒకవేళ (తదుపరి == ఖాళీ || z_is_idle_thread_object(తదుపరి)) {
				ముక్క_reset(ముక్క_కొలత);
			}
#లేకపోతే
			ఒకవేళ (_kernel.ready_q.cache == curr) {
				ముక్క_reset(ముక్క_కొలత);
			}
#అయితే_ముగింపు
		}
	}
	z_sched_spinlock_unlock(కీలకం);
}
