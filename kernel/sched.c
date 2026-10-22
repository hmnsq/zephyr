/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <priority_q.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <ipi.h>
#అన్తర్భావ <kernel_arch_func.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/drivers/timer/system_timer.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/logging/log.h>
#అన్తర్భావ <zephyr/sys/atomic.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/timing/timing.h>
#అన్తర్భావ <zephyr/sys/util.h>
#అన్తర్భావ <metairq.h>
#అన్తర్భావ <run_q.h>
#అన్తర్భావ <timeslicing.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* pending_current is owned by timeslicing.c; sleep.c also accesses it */
#ఒకవేళ defined(CONFIG_మార్పిడి_NONATOMIC) && defined(CONFIG_TIMESLICING)
బాహ్య నిర్మాణం k_thread *pending_current;
#అయితే_ముగింపు

నిర్మాణం k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent నిర్మాణం k_thread _thread_dummy;

స్థిర ALWAYS_INLINE శూన్య update_cache(పూర్ణాంకం preempt_ok);
స్థిర ALWAYS_INLINE శూన్య halt_thread(నిర్మాణం k_thread *thread, uint8_t new_state,
				      k_spinlock_కీలకం_t *కీలకం);
స్థిర శూన్య కూడు_గమ్యం_waitq_locked(నిర్మాణం k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
స్థిర అంతఃవరుస శూన్య clear_halting(నిర్మాణం k_thread *thread)
{
	ఒకవేళ (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_అంకె_CPUS > 1)) {
		barrier_dmem_fence_నిండిన(); /* Other cpus spin on this locklessly! */
		thread->పునాది.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

స్థిర ALWAYS_INLINE నిర్మాణం k_thread *తదుపరి_పైకి(శూన్య)
{
#నిర్వచితమైతే CONFIG_SMP
	bool ipi_idle_లక్ష్యం_rebound = false;
	నిర్మాణం k_thread *ipi_idle_లక్ష్యం = ipi_idle_reserved_take();

	ఒకవేళ (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, ఖాళీ);
	}
#అయితే_ముగింపు /* CONFIG_SMP */

	నిర్మాణం k_thread *thread = runq_ఉత్తమ();

	thread = metairq_preempt_recover(thread);

#నిర్వచితం_కాకపోతే CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	తిరిగి (thread != ఖాళీ) ? thread : _current_cpu->idle_thread;
#లేకపోతే
	/* Under SMP, the "cache" mechanism for selecting the next
	 * thread doesn't work, so we have more work to do to test
	 * _current against the best choice from the queue.  Here, the
	 * thread selected above represents "the best thread that is
	 * not current".
	 *
	 * Subtle note on "queued": in SMP mode, neither _current nor
	 * metairq_premepted live in the queue, so this isn't exactly the
	 * same thing as "ready", it means "the thread already been
	 * added back to the queue such that we don't want to re-add it".
	 */
	bool queued = z_is_thread_queued(_current);
	bool చురుకైన = z_is_thread_ready(_current);

	ఒకవేళ (thread == ఖాళీ) {
		thread = _current_cpu->idle_thread;
	}

	ఒకవేళ (చురుకైన) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		ఒకవేళ ((cmp > 0) || ((cmp == 0) && !_current_cpu->మార్పిడి_ok)) {
			thread = _current;
		}

		ఒకవేళ (!should_preempt(thread, _current_cpu->మార్పిడి_ok)) {
			thread = _current;
		}
	}

	ఒకవేళ (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		ఒకవేళ (చురుకైన && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			వరుస_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	ఒకవేళ (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		ఒకవేళ (ipi_idle_లక్ష్యం != ఖాళీ &&
		    thread != ipi_idle_లక్ష్యం &&
		    z_is_thread_queued(ipi_idle_లక్ష్యం)) {
			ipi_idle_లక్ష్యం_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_లక్ష్యం);
		} లేకపోతే {
			ipi_idle_thread_unreserve(thread);
		}
		వరుస_నుండి_తీయి_thread(thread);
	}

	_current_cpu->మార్పిడి_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	ఒకవేళ (!ipi_idle_లక్ష్యం_rebound &&
	    ipi_idle_లక్ష్యం != ఖాళీ && z_is_thread_queued(ipi_idle_లక్ష్యం)) {
		flag_ipi(ipi_ముసుగు_create(ipi_idle_లక్ష్యం));
	}
	తిరిగి thread;
#అయితే_ముగింపు /* CONFIG_SMP */
}

శూన్య move_current_గమ్యం_చివర_యొక్క_prio_q(శూన్య)
{
	runq_yield();

	update_cache(1);
}

స్థిర ALWAYS_INLINE శూన్య update_cache(పూర్ణాంకం preempt_ok)
{
#నిర్వచితం_కాకపోతే CONFIG_SMP
	నిర్మాణం k_thread *thread = తదుపరి_పైకి();

	ఒకవేళ (should_preempt(thread, preempt_ok)) {
#నిర్వచితమైతే CONFIG_TIMESLICING
		ఒకవేళ (thread != _current) {
			z_సమయం_ముక్క_reset(thread);
		}
#అయితే_ముగింపు /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} లేకపోతే {
		_kernel.ready_q.cache = _current;
	}

#లేకపోతే
	/* The way this works is that the CPU record keeps its
	 * "cooperative swapping is OK" flag until the next reschedule
	 * call or context switch.  It doesn't need to be tracked per
	 * thread because if the thread gets preempted for whatever
	 * reason the scheduler will make the same decision anyway.
	 */
	_current_cpu->మార్పిడి_ok = preempt_ok;
#అయితే_ముగింపు /* CONFIG_SMP */
}

/**
 * Returns pointer to _cpu if the thread is currently running on
 * another CPU.
 */
స్థిర నిర్మాణం _cpu *thread_చురుకైన_elsewhere(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SMP
	పూర్ణాంకం thread_cpu_id = thread->పునాది.cpu;
	నిర్మాణం _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_అంకె_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	ఒకవేళ ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		తిరిగి thread_cpu;
	}

#అయితే_ముగింపు /* CONFIG_SMP */
	ARG_వాడని(thread);
	తిరిగి ఖాళీ;
}

స్థిర అంతఃవరుస శూన్య ready_thread(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	ఒకవేళ (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		వరుస_thread(thread);
		update_cache(0);

		flag_ipi(ipi_ముసుగు_create(thread));
	}
}

శూన్య z_ready_thread(నిర్మాణం k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		ఒకవేళ (thread_చురుకైన_elsewhere(thread) == ఖాళీ) {
			ready_thread(thread);
		}
	}
}

శూన్య z_sched_ready_locked(నిర్మాణం k_thread *thread) ALIAS_యొక్క(ready_thread);

స్థిర శూన్య unready_thread(నిర్మాణం k_thread *thread)
{
	ఒకవేళ (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		వరుస_నుండి_తీయి_thread(thread);
	}
	update_cache(thread == _current);
}

/* This routine exists for benchmarking purposes. It is not used in
 * general production code.
 */
శూన్య z_unready_thread(నిర్మాణం k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		unready_thread(thread);
	}
}


శూన్య z_sched_unready_locked(నిర్మాణం k_thread *thread) ALIAS_యొక్క(unready_thread);

/* This routine only used for testing purposes */
శూన్య z_yield_testing_only(శూన్య)
{
	Z_SCHED_SPINLOCK {
		move_current_గమ్యం_చివర_యొక్క_prio_q();
	}
}

/* Spins in ISR context, waiting for a thread known to be running on
 * another CPU to catch the IPI we sent and halt.  Note that we check
 * for ourselves being asynchronously halted first to prevent simple
 * deadlocks (but not complex ones involving cycles of 3+ threads!).
 * Acts to release the provided lock before returning.
 */
స్థిర శూన్య thread_halt_spin(నిర్మాణం k_thread *thread, k_spinlock_కీలకం_t కీలకం)
{
	ఒకవేళ (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &కీలకం);
	}
	z_sched_spinlock_unlock(కీలకం);
	అయితే (z_is_thread_halting(thread)) {
		గుర్తులేని పూర్ణాంకం k = arch_irq_తాళం();

		arch_spin_relax(); /* Requires interrupts be masked */
		arch_irq_unlock(k);
	}
}

/* Shared handler for k_thread_{suspend,abort}().  Called with the
 * scheduler lock held and the key passed (which it may
 * release/reacquire!) which will be released before a possible return
 * (aborting _current will not return, obviously), which may be after
 * a context switch.
 */
శూన్య z_thread_halt(నిర్మాణం k_thread *thread, k_spinlock_కీలకం_t కీలకం,
					bool terminate)
{
	_wait_q_t *wq = &thread->join_వరుస;
#నిర్వచితమైతే CONFIG_SMP
	wq = terminate ? wq : &thread->halt_వరుస;
#అయితే_ముగింపు

	z_metairq_preempted_clear(thread);

	/* If the target is a thread running on another CPU, flag and
	 * poke (note that we might spin to wait, so a true
	 * synchronous IPI is needed here, not deferred!), it will
	 * halt itself in the IPI.  Otherwise it's unscheduled, so we
	 * can clean it up directly.
	 */

	నిర్మాణం _cpu *cpu = thread_చురుకైన_elsewhere(thread);

	ఒకవేళ (cpu != ఖాళీ) {
		thread->పునాది.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#ఒకవేళ defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#నిర్వచితమైతే CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_ముసుగు(cpu->id));
#లేకపోతే
		arch_sched_ప్రసారం_ipi();
#అయితే_ముగింపు /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#అయితే_ముగింపు /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		ఒకవేళ (arch_is_లోపల_isr()) {
			thread_halt_spin(thread, కీలకం);
		} లేకపోతే  {
			కూడు_గమ్యం_waitq_locked(_current, wq);
			z_మార్పిడి_locked(కీలకం);
		}
		/* The target's next_up self-halt path passed NULL to
		 * halt_thread() and could not retry on -EAGAIN; an
		 * in-flight handler on a third CPU may not have run yet
		 * (it is blocked on the scheduler spinlock and will only
		 * acquire it after we drop it via the swap/spin above). Wait
		 * now, outside any lock, before the caller may free the thread
		 * storage. The handler, when it runs, sees _THREAD_DEAD /
		 * _THREAD_SUSPENDED and either bails (killed check) or
		 * no-ops in ready_thread() (z_is_thread_ready() rejects
		 * suspended threads). After this loop returns, no further
		 * dereference of thread->base will occur.
		 */
		అయితే (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} లేకపోతే {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &కీలకం);
		ఒకవేళ ((thread == _current) && !arch_is_లోపల_isr()) {
			ఒకవేళ (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(కీలకం);
				k_panic();
				కీలకం = z_sched_spinlock_తాళం();
			}
			z_మార్పిడి_locked(కీలకం);
			__ASSERT(!terminate, "aborted _current back from dead");
		} లేకపోతే {
			z_sched_spinlock_unlock(కీలకం);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

స్థిర అంతఃవరుస bool resched(uint32_t కీలకం)
{
#నిర్వచితమైతే CONFIG_SMP
	_current_cpu->మార్పిడి_ok = 0;
#అయితే_ముగింపు /* CONFIG_SMP */

	తిరిగి arch_irq_unlocked(కీలకం) && !arch_is_లోపల_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
స్థిర అంతఃవరుస bool need_మార్పిడి(శూన్య)
{
	/* the SMP case will be handled in C based z_swap() */
#నిర్వచితమైతే CONFIG_SMP
	తిరిగి true;
#లేకపోతే
	నిర్మాణం k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	తిరిగి new_thread != _current;
#అయితే_ముగింపు /* CONFIG_SMP */
}

స్థిర శూన్య reschedule(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం)
{
	ఒకవేళ (resched(కీలకం.కీలకం) && need_మార్పిడి()) {
		z_మార్పిడి(తాళం, కీలకం);
	} లేకపోతే {
		signal_pending_ipi();
		k_spin_unlock(తాళం, కీలకం);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
స్థిర శూన్య reschedule_locked(k_spinlock_కీలకం_t కీలకం)
{
	తిరిగి reschedule(&_sched_spinlock, కీలకం);
}

శూన్య z_sched_తాళం_reschedule(k_spinlock_కీలకం_t కీలకం)
{
	update_cache(0);
	reschedule_locked(కీలకం);
}

శూన్య z_sched_yield(శూన్య)
{
	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	runq_yield();
	update_cache(1);
	z_మార్పిడి_locked(కీలకం);
}

/* The scheduler's spinlock must be held */
స్థిర శూన్య కూడు_గమ్యం_waitq_locked(నిర్మాణం k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->పునాది.pended_on == ఖాళీ);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_పోర్టు_TRACING_FUNC(k_thread, sched_pend, thread);

	ఒకవేళ (wait_q != ఖాళీ) {
		thread->పునాది.pended_on = wait_q;
		priq_wait_కూడు(&wait_q->waitq, thread);
	}
}

శూన్య z_sched_కూడు_గమ్యం_waitq_locked(నిర్మాణం k_thread *thread, _wait_q_t *wait_q)
	ALIAS_యొక్క(కూడు_గమ్యం_waitq_locked);

స్థిర శూన్య pend_locked(నిర్మాణం k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == ఖాళీ || sys_cache_is_mem_coherent(wait_q));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */
	కూడు_గమ్యం_waitq_locked(thread, wait_q);
	z_కూడు_thread_timeout(thread, timeout);
}

శూన్య z_pend_thread(నిర్మాణం k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || is_thread_dummy(thread));
	Z_SCHED_SPINLOCK {
		pend_locked(thread, wait_q, timeout);
	}
}

శూన్య z_unpend_thread_no_timeout(నిర్మాణం k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		ఒకవేళ (thread->పునాది.pended_on != ఖాళీ) {
			unpend_thread_no_timeout(thread);
		}
	}
}

శూన్య z_sched_wake_thread_locked(నిర్మాణం k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->పునాది.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	ఒకవేళ (!killed) {
		/* The thread is not being killed */
		ఒకవేళ (thread->పునాది.pended_on != ఖాళీ) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
శూన్య z_thread_timeout(నిర్మాణం _timeout *timeout)
{
	నిర్మాణం k_thread *thread = CONTAINER_యొక్క(timeout,
					       నిర్మాణం k_thread, పునాది.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		ఒకవేళ (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */

పూర్ణాంకం z_pend_curr(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	ఒకవేళ (arch_is_లోపల_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#ఒకవేళ defined(CONFIG_TIMESLICING) && defined(CONFIG_మార్పిడి_NONATOMIC)
	pending_current = _current;
#అయితే_ముగింపు /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((పరిమాణం(నిర్మాణం k_spinlock) == 0) || !z_is_sched_spinlock(తాళం));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(శూన్య) z_sched_spinlock_తాళం();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(తాళం);
	తిరిగి z_మార్పిడి_locked(కీలకం);
}

నిర్మాణం k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	నిర్మాణం k_thread *thread = ఖాళీ;

	Z_SCHED_SPINLOCK {
		thread = priq_wait_ఉత్తమ(&wait_q->waitq);

		ఒకవేళ (thread != ఖాళీ) {
			unpend_thread_no_timeout(thread);
		}
	}

	తిరిగి thread;
}

శూన్య z_unpend_thread(నిర్మాణం k_thread *thread)
{
	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	ఒకవేళ (thread->పునాది.pended_on != ఖాళీ) {
		unpend_thread_no_timeout(thread);
	}
	అయితే (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(కీలకం);
		కీలకం = z_sched_spinlock_తాళం();
	}
	z_sched_spinlock_unlock(కీలకం);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_సమితి(నిర్మాణం k_thread *thread, పూర్ణాంకం prio)
{
	bool need_sched = false;
	పూర్ణాంకం old_prio = thread->పునాది.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		ఒకవేళ (need_sched) {
			ఒకవేళ (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				వరుస_నుండి_తీయి_thread(thread);
				thread->పునాది.prio = prio;
				వరుస_thread(thread);

				ఒకవేళ (old_prio > prio) {
					flag_ipi(ipi_ముసుగు_create(thread));
				}
			} లేకపోతే {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->పునాది.prio = prio;

				నిర్మాణం _cpu *cpu;

				cpu = thread_చురుకైన_elsewhere(thread);
				ఒకవేళ ((cpu != ఖాళీ) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_ముసుగు(cpu->id));
				}
			}

			update_cache(1);
		} లేకపోతే ఒకవేళ (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->పునాది.prio = prio;
			priq_wait_కూడు(&wait_q->waitq, thread);
		} లేకపోతే {
			thread->పునాది.prio = prio;
		}
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_thread, sched_ప్రాధాన్యత_సమితి, thread, prio);

	తిరిగి need_sched;
}

శూన్య z_reschedule(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం) ALIAS_యొక్క(reschedule);

శూన్య z_reschedule_locked(k_spinlock_కీలకం_t కీలకం) ALIAS_యొక్క(reschedule_locked);

శూన్య z_reschedule_irqlock(uint32_t కీలకం)
{
	ఒకవేళ (resched(కీలకం) && need_మార్పిడి()) {
		z_మార్పిడి_irqlock(కీలకం);
	} లేకపోతే {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(కీలకం);
	}
}

నిర్మాణం k_thread *z_మార్పిడి_తదుపరి_thread(శూన్య)
{
	నిర్మాణం k_thread *ret = తదుపరి_పైకి();

	ఒకవేళ (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
స్థిర అంతఃవరుస శూన్య సమితి_current(నిర్మాణం k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
		z_thread_mark_switched_బహిర్గతం();
	}
	z_current_thread_సమితి(new_thread);
}

/**
 * @brief Determine next thread to execute upon completion of an interrupt
 *
 * Thread preemption is performed by context switching after the completion
 * of a non-recursed interrupt. This function determines which thread to
 * switch to if any. This function accepts as @p interrupted either:
 *
 * - The handle for the interrupted thread in which case the thread's context
 *   must already be fully saved and ready to be picked up by a different CPU.
 *
 * - NULL if more work is required to fully save the thread's state after
 *   it is known that a new thread is to be scheduled. It is up to the caller
 *   to store the handle resulting from the thread that is being switched out
 *   in that thread's "switch_handle" field after its
 *   context has fully been saved, following the same requirements as with
 *   the @ref arch_switch() function.
 *
 * If a new thread needs to be scheduled then its handle is returned.
 * Otherwise the same value provided as @p interrupted is returned back.
 * Those handles are the same opaque types used by the @ref arch_switch()
 * function.
 *
 * @warning
 * The _current value may have changed after this call and not refer
 * to the interrupted thread anymore. It might be necessary to make a local
 * copy before calling this function.
 *
 * @param interrupted Handle for the thread that was interrupted or NULL.
 * @return Handle for the next thread to execute, or @p interrupted when
 *         no new thread is to be scheduled.
 */
శూన్య *z_get_తదుపరి_switch_handle(శూన్య *interrupted)
{
	z_check_దొంతర_sentinel();

#నిర్వచితమైతే CONFIG_SMP
	శూన్య *ret = ఖాళీ;

	Z_SCHED_SPINLOCK {
		నిర్మాణం k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == ఖాళీ || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = తదుపరి_పైకి();

		z_sched_usage_switch(new_thread);

		ఒకవేళ (old_thread != new_thread) {
			uint8_t  cpu_id;

			z_sched_switch_spin(new_thread);
			arch_cohere_stacks(old_thread, interrupted, new_thread);

			_current_cpu->మార్పిడి_ok = 0;
			cpu_id = arch_curr_cpu()->id;
			new_thread->పునాది.cpu = cpu_id;
			సమితి_current(new_thread);

#నిర్వచితమైతే CONFIG_TIMESLICING
			z_సమయం_ముక్క_reset(new_thread);
#అయితే_ముగింపు /* CONFIG_TIMESLICING */

			/* Changed _current!  Update the scheduler's spinlock
			 * bookkeeping so the validation doesn't get
			 * confused when the "wrong" thread tries to
			 * release the lock.
			 */
			z_sched_spinlock_transfer_owner(new_thread);

			/* A queued (runnable) old/current thread
			 * needs to be added back to the run queue
			 * here, and atomically with its switch handle
			 * being set below.  This is safe now, as we
			 * will not return into it.
			 */
			ఒకవేళ (z_is_thread_queued(old_thread)) {
#నిర్వచితమైతే CONFIG_SCHED_IPI_CASCADE
				ఒకవేళ ((new_thread->పునాది.cpu_ముసుగు != -1) &&
				    (old_thread->పునాది.cpu_ముసుగు != బిట్టు(cpu_id))) {
					flag_ipi(ipi_ముసుగు_create(old_thread));
				}
#అయితే_ముగింపు
				runq_కూడు(old_thread);
			}
		}
		old_thread->switch_handle = interrupted;
		ret = new_thread->switch_handle;
		/* Active threads MUST have a null here */
		new_thread->switch_handle = ఖాళీ;

		/* Check for IPIs under the lock to avoid silently consuming a
		 * rescheduling IPI flagged by another CPU for ourselves.
		 */
		signal_pending_ipi();
	}
	తిరిగి ret;
#లేకపోతే
	నిర్మాణం k_thread *తదుపరి = _kernel.ready_q.cache;

	z_sched_usage_switch(తదుపరి);
	_current->switch_handle = interrupted;
	సమితి_current(తదుపరి);
	తిరిగి తదుపరి->switch_handle;
#అయితే_ముగింపు /* CONFIG_SMP */
}
#అయితే_ముగింపు /* CONFIG_USE_SWITCH */

పూర్ణాంకం z_unpend_all(_wait_q_t *wait_q)
{
	పూర్ణాంకం need_sched = 0;
	నిర్మాణం k_thread *thread;

	Z_SCHED_SPINLOCK {
		కొరకు (thread = z_waitq_తల_locked(wait_q);
		     thread != ఖాళీ;
		     thread = z_waitq_తల_locked(wait_q)) {
			unpend_thread_no_timeout(thread);
			/* Abort the timeout and ready the thread unconditionally. If
			 * the timeout handler is in flight on another CPU, the abort
			 * flags it superseded and z_thread_timeout() bails when it
			 * runs -- so it will NOT ready the thread, we must.
			 */
			(శూన్య)z_try_abort_thread_timeout(thread);
			ready_thread(thread);
			need_sched = 1;
		}
	}

	తిరిగి need_sched;
}

స్థిర అంతఃవరుస శూన్య unpend_all(_wait_q_t *wait_q)
{
	నిర్మాణం k_thread *thread;

	కొరకు (thread = z_waitq_తల_locked(wait_q);
	     thread != ఖాళీ;
	     thread = z_waitq_తల_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_విలువ_సమితి(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(శూన్య)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#నిర్వచితమైతే CONFIG_THREAD_ABORT_HOOK
బాహ్య శూన్య thread_abort_hook(నిర్మాణం k_thread *thread);
#అయితే_ముగింపు /* CONFIG_THREAD_ABORT_HOOK */

/**
 * @brief Dequeues the specified thread
 *
 * Dequeues the specified thread and move it into the specified new state.
 *
 * @param thread Identify the thread to halt
 * @param new_state New thread state (_THREAD_DEAD or _THREAD_SUSPENDED)
 * @param key Pointer to the scheduler spinlock key held by the caller
 */
స్థిర ALWAYS_INLINE శూన్య halt_thread(నిర్మాణం k_thread *thread, uint8_t new_state,
				      k_spinlock_కీలకం_t *కీలకం)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	ఒకవేళ ((thread->పునాది.thread_state & new_state) == 0U) {
		thread->పునాది.thread_state |= new_state;
		ఒకవేళ (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			వరుస_నుండి_తీయి_thread(thread);
		}

		ఒకవేళ (new_state == _THREAD_DEAD) {
			ఒకవేళ (thread->పునాది.pended_on != ఖాళీ) {
				unpend_thread_no_timeout(thread);
			}
			/* Wait for any in-flight handler to complete before
			 * we proceed: the caller may free this thread's
			 * storage (dynamic threads), and an in-flight handler
			 * would UAF when it eventually runs z_thread_timeout()
			 * and dereferences thread->base. _THREAD_DEAD is
			 * already set, so once the handler runs it bails via
			 * z_sched_wake_thread_locked()'s killed check.
			 *
			 * NULL key is the next_up() self-halt path on a
			 * running _current that has no linked timeout; there
			 * is nothing to do here. That path is always reached
			 * via z_thread_halt()'s IF branch, which spins on
			 * z_try_abort_thread_timeout(thread) outside any lock
			 * after the halt-queue wait completes, closing any
			 * remaining in-flight window before the caller of
			 * z_thread_halt() returns.
			 */
			ఒకవేళ (కీలకం != ఖాళీ) {
				అయితే (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*కీలకం);
					*కీలకం = z_sched_spinlock_తాళం();
				}
			}
			unpend_all(&thread->join_వరుస);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			ఒకవేళ (thread == _current && arch_is_లోపల_isr()) {
				dummify = true;
			}
		}
#నిర్వచితమైతే CONFIG_SMP
		unpend_all(&thread->halt_వరుస);
#అయితే_ముగింపు /* CONFIG_SMP */
		update_cache(1);

		ఒకవేళ (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			తిరిగి;
		}

		arch_coprocessors_disable(thread);

		SYS_పోర్టు_TRACING_FUNC(k_thread, sched_abort, thread);

		z_thread_monitor_exit(thread);
#నిర్వచితమైతే CONFIG_THREAD_ABORT_HOOK
		thread_abort_hook(thread);
#అయితే_ముగింపు /* CONFIG_THREAD_ABORT_HOOK */

#నిర్వచితమైతే CONFIG_OBJ_CORE_THREAD
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_THREAD
		k_obj_core_stats_deregister(K_OBJ_CORE(thread));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_THREAD */
		k_obj_core_unlink(K_OBJ_CORE(thread));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_THREAD */

#నిర్వచితమైతే CONFIG_USERSPACE
		z_mem_domain_exit_thread(thread);
		k_thread_perms_all_clear(thread);
		k_object_uninit(thread->దొంతర_obj);
		k_object_uninit(thread);
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_THREAD_ABORT_NEED_CLEANUP
		k_thread_abort_cleanup(thread);
#అయితే_ముగింపు /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

		/* Do this "set _current to dummy" step last so that
		 * subsystems above can rely on _current being
		 * unchanged.  Disabled for posix as that arch
		 * continues to use the _current pointer in its swap
		 * code.  Note that we must leave a non-null switch
		 * handle for any threads spinning in join() (this can
		 * never be used, as our thread is flagged dead, but
		 * it must not be NULL otherwise join can deadlock).
		 * Use 1 as a clearly invalid but non-NULL value.
		 */
		ఒకవేళ (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#నిర్వచితమైతే CONFIG_USE_SWITCH
			_current->switch_handle = (శూన్య *)1;
#అయితే_ముగింపు
#నిర్వచితమైతే CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			ఒకవేళ (thread->పునాది.మార్పిడి_దత్తాంశం ==
			    (శూన్య *)&z_spinlock_abort_sentinel) {
				z_spin_validate_reset(true);
			}
#అయితే_ముగింపు
			z_dummy_thread_init(&_thread_dummy);

		}

		/* Finally update the halting thread state, on which
		 * other CPUs might be spinning (see
		 * thread_halt_spin()).
		 */
		clear_halting(thread);
	}
}

శూన్య z_thread_suspend_current(నిర్మాణం k_thread *thread)
{
	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	వరుస_నుండి_తీయి_thread(thread);
	update_cache(1);
	z_మార్పిడి_locked(కీలకం);
}
