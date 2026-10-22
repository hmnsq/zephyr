/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <priority_q.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>
#ಅನ್ತರ್ಭಾವ <kernel_arch_func.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/drivers/timer/system_timer.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/timing/timing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/util.h>
#ಅನ್ತರ್ಭಾವ <metairq.h>
#ಅನ್ತರ್ಭಾವ <run_q.h>
#ಅನ್ತರ್ಭಾವ <timeslicing.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* pending_current is owned by timeslicing.c; sleep.c also accesses it */
#ಆದರೆ defined(CONFIG_ಅದಲುಬದಲು_NONATOMIC) && defined(CONFIG_TIMESLICING)
ಬಾಹ್ಯ ರಚನೆ k_thread *pending_current;
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent ರಚನೆ k_thread _thread_dummy;

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ update_cache(ಪೂರ್ಣಾಂಕ preempt_ok);
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ halt_thread(ರಚನೆ k_thread *thread, uint8_t new_state,
				      k_spinlock_ಕೀಲಿ_t *ಕೀಲಿ);
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_ಗಮ್ಯ_waitq_locked(ರಚನೆ k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ clear_halting(ರಚನೆ k_thread *thread)
{
	ಆದರೆ (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_ಅಂಕೆ_CPUS > 1)) {
		barrier_dmem_fence_ತುಂಬಿದ(); /* Other cpus spin on this locklessly! */
		thread->ತಳ.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *ಮುಂದಿನ_ಮೇಲೆ(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	bool ipi_idle_ಗುರಿ_rebound = false;
	ರಚನೆ k_thread *ipi_idle_ಗುರಿ = ipi_idle_reserved_take();

	ಆದರೆ (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, ಖಾಲಿ);
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

	ರಚನೆ k_thread *thread = runq_ಉತ್ತಮ();

	thread = metairq_preempt_recover(thread);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	ಹಿಂತಿರುಗಿ (thread != ಖಾಲಿ) ? thread : _current_cpu->idle_thread;
#ಇಲ್ಲದಿದ್ದರೆ
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
	bool ಸಕ್ರಿಯ = z_is_thread_ready(_current);

	ಆದರೆ (thread == ಖಾಲಿ) {
		thread = _current_cpu->idle_thread;
	}

	ಆದರೆ (ಸಕ್ರಿಯ) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		ಆದರೆ ((cmp > 0) || ((cmp == 0) && !_current_cpu->ಅದಲುಬದಲು_ok)) {
			thread = _current;
		}

		ಆದರೆ (!should_preempt(thread, _current_cpu->ಅದಲುಬದಲು_ok)) {
			thread = _current;
		}
	}

	ಆದರೆ (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		ಆದರೆ (ಸಕ್ರಿಯ && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			ಸರತಿ_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	ಆದರೆ (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		ಆದರೆ (ipi_idle_ಗುರಿ != ಖಾಲಿ &&
		    thread != ipi_idle_ಗುರಿ &&
		    z_is_thread_queued(ipi_idle_ಗುರಿ)) {
			ipi_idle_ಗುರಿ_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_ಗುರಿ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ipi_idle_thread_unreserve(thread);
		}
		ಸರತಿಯಿಂದ_ತೆಗೆ_thread(thread);
	}

	_current_cpu->ಅದಲುಬದಲು_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	ಆದರೆ (!ipi_idle_ಗುರಿ_rebound &&
	    ipi_idle_ಗುರಿ != ಖಾಲಿ && z_is_thread_queued(ipi_idle_ಗುರಿ)) {
		flag_ipi(ipi_ಮುಖವಾಡ_create(ipi_idle_ಗುರಿ));
	}
	ಹಿಂತಿರುಗಿ thread;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}

ಶೂನ್ಯ move_current_ಗಮ್ಯ_ಕೊನೆ_ದ_prio_q(ಶೂನ್ಯ)
{
	runq_yield();

	update_cache(1);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ update_cache(ಪೂರ್ಣಾಂಕ preempt_ok)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	ರಚನೆ k_thread *thread = ಮುಂದಿನ_ಮೇಲೆ();

	ಆದರೆ (should_preempt(thread, preempt_ok)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICING
		ಆದರೆ (thread != _current) {
			z_ಸಮಯ_ತುಂಡು_reset(thread);
		}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} ಇಲ್ಲದಿದ್ದರೆ {
		_kernel.ready_q.cache = _current;
	}

#ಇಲ್ಲದಿದ್ದರೆ
	/* The way this works is that the CPU record keeps its
	 * "cooperative swapping is OK" flag until the next reschedule
	 * call or context switch.  It doesn't need to be tracked per
	 * thread because if the thread gets preempted for whatever
	 * reason the scheduler will make the same decision anyway.
	 */
	_current_cpu->ಅದಲುಬದಲು_ok = preempt_ok;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}

/**
 * Returns pointer to _cpu if the thread is currently running on
 * another CPU.
 */
ಸ್ಥಿರ ರಚನೆ _cpu *thread_ಸಕ್ರಿಯ_elsewhere(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	ಪೂರ್ಣಾಂಕ thread_cpu_id = thread->ತಳ.cpu;
	ರಚನೆ _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_ಅಂಕೆ_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	ಆದರೆ ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		ಹಿಂತಿರುಗಿ thread_cpu;
	}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
	ARG_ಬಳಸದ(thread);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ready_thread(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	ಆದರೆ (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		ಸರತಿ_thread(thread);
		update_cache(0);

		flag_ipi(ipi_ಮುಖವಾಡ_create(thread));
	}
}

ಶೂನ್ಯ z_ready_thread(ರಚನೆ k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		ಆದರೆ (thread_ಸಕ್ರಿಯ_elsewhere(thread) == ಖಾಲಿ) {
			ready_thread(thread);
		}
	}
}

ಶೂನ್ಯ z_sched_ready_locked(ರಚನೆ k_thread *thread) ALIAS_ದ(ready_thread);

ಸ್ಥಿರ ಶೂನ್ಯ unready_thread(ರಚನೆ k_thread *thread)
{
	ಆದರೆ (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		ಸರತಿಯಿಂದ_ತೆಗೆ_thread(thread);
	}
	update_cache(thread == _current);
}

/* This routine exists for benchmarking purposes. It is not used in
 * general production code.
 */
ಶೂನ್ಯ z_unready_thread(ರಚನೆ k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		unready_thread(thread);
	}
}


ಶೂನ್ಯ z_sched_unready_locked(ರಚನೆ k_thread *thread) ALIAS_ದ(unready_thread);

/* This routine only used for testing purposes */
ಶೂನ್ಯ z_yield_testing_only(ಶೂನ್ಯ)
{
	Z_SCHED_SPINLOCK {
		move_current_ಗಮ್ಯ_ಕೊನೆ_ದ_prio_q();
	}
}

/* Spins in ISR context, waiting for a thread known to be running on
 * another CPU to catch the IPI we sent and halt.  Note that we check
 * for ourselves being asynchronously halted first to prevent simple
 * deadlocks (but not complex ones involving cycles of 3+ threads!).
 * Acts to release the provided lock before returning.
 */
ಸ್ಥಿರ ಶೂನ್ಯ thread_halt_spin(ರಚನೆ k_thread *thread, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ಆದರೆ (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &ಕೀಲಿ);
	}
	z_sched_spinlock_unlock(ಕೀಲಿ);
	ಆಗ (z_is_thread_halting(thread)) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ k = arch_irq_ಬೀಗ();

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
ಶೂನ್ಯ z_thread_halt(ರಚನೆ k_thread *thread, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ,
					bool terminate)
{
	_wait_q_t *wq = &thread->join_ಸರತಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	wq = terminate ? wq : &thread->halt_ಸರತಿ;
#ಆದರೆ_ಅಂತ್ಯ

	z_metairq_preempted_clear(thread);

	/* If the target is a thread running on another CPU, flag and
	 * poke (note that we might spin to wait, so a true
	 * synchronous IPI is needed here, not deferred!), it will
	 * halt itself in the IPI.  Otherwise it's unscheduled, so we
	 * can clean it up directly.
	 */

	ರಚನೆ _cpu *cpu = thread_ಸಕ್ರಿಯ_elsewhere(thread);

	ಆದರೆ (cpu != ಖಾಲಿ) {
		thread->ತಳ.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#ಆದರೆ defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_ಮುಖವಾಡ(cpu->id));
#ಇಲ್ಲದಿದ್ದರೆ
		arch_sched_ಪ್ರಸಾರ_ipi();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		ಆದರೆ (arch_is_ಒಳ_isr()) {
			thread_halt_spin(thread, ಕೀಲಿ);
		} ಇಲ್ಲದಿದ್ದರೆ  {
			ಕೂಡಿಸು_ಗಮ್ಯ_waitq_locked(_current, wq);
			z_ಅದಲುಬದಲು_locked(ಕೀಲಿ);
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
		ಆಗ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &ಕೀಲಿ);
		ಆದರೆ ((thread == _current) && !arch_is_ಒಳ_isr()) {
			ಆದರೆ (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(ಕೀಲಿ);
				k_panic();
				ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();
			}
			z_ಅದಲುಬದಲು_locked(ಕೀಲಿ);
			__ASSERT(!terminate, "aborted _current back from dead");
		} ಇಲ್ಲದಿದ್ದರೆ {
			z_sched_spinlock_unlock(ಕೀಲಿ);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool resched(uint32_t ಕೀಲಿ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	_current_cpu->ಅದಲುಬದಲು_ok = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

	ಹಿಂತಿರುಗಿ arch_irq_unlocked(ಕೀಲಿ) && !arch_is_ಒಳ_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool need_ಅದಲುಬದಲು(ಶೂನ್ಯ)
{
	/* the SMP case will be handled in C based z_swap() */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	ಹಿಂತಿರುಗಿ true;
#ಇಲ್ಲದಿದ್ದರೆ
	ರಚನೆ k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	ಹಿಂತಿರುಗಿ new_thread != _current;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}

ಸ್ಥಿರ ಶೂನ್ಯ reschedule(ರಚನೆ k_spinlock *ಬೀಗ, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ಆದರೆ (resched(ಕೀಲಿ.ಕೀಲಿ) && need_ಅದಲುಬದಲು()) {
		z_ಅದಲುಬದಲು(ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		signal_pending_ipi();
		k_spin_unlock(ಬೀಗ, ಕೀಲಿ);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
ಸ್ಥಿರ ಶೂನ್ಯ reschedule_locked(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ಹಿಂತಿರುಗಿ reschedule(&_sched_spinlock, ಕೀಲಿ);
}

ಶೂನ್ಯ z_sched_ಬೀಗ_reschedule(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	update_cache(0);
	reschedule_locked(ಕೀಲಿ);
}

ಶೂನ್ಯ z_sched_yield(ಶೂನ್ಯ)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	runq_yield();
	update_cache(1);
	z_ಅದಲುಬದಲು_locked(ಕೀಲಿ);
}

/* The scheduler's spinlock must be held */
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_ಗಮ್ಯ_waitq_locked(ರಚನೆ k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->ತಳ.pended_on == ಖಾಲಿ);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, sched_pend, thread);

	ಆದರೆ (wait_q != ಖಾಲಿ) {
		thread->ತಳ.pended_on = wait_q;
		priq_wait_ಕೂಡಿಸು(&wait_q->waitq, thread);
	}
}

ಶೂನ್ಯ z_sched_ಕೂಡಿಸು_ಗಮ್ಯ_waitq_locked(ರಚನೆ k_thread *thread, _wait_q_t *wait_q)
	ALIAS_ದ(ಕೂಡಿಸು_ಗಮ್ಯ_waitq_locked);

ಸ್ಥಿರ ಶೂನ್ಯ pend_locked(ರಚನೆ k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == ಖಾಲಿ || sys_cache_is_mem_coherent(wait_q));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */
	ಕೂಡಿಸು_ಗಮ್ಯ_waitq_locked(thread, wait_q);
	z_ಕೂಡಿಸು_thread_timeout(thread, timeout);
}

ಶೂನ್ಯ z_pend_thread(ರಚನೆ k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || is_thread_dummy(thread));
	Z_SCHED_SPINLOCK {
		pend_locked(thread, wait_q, timeout);
	}
}

ಶೂನ್ಯ z_unpend_thread_no_timeout(ರಚನೆ k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		ಆದರೆ (thread->ತಳ.pended_on != ಖಾಲಿ) {
			unpend_thread_no_timeout(thread);
		}
	}
}

ಶೂನ್ಯ z_sched_wake_thread_locked(ರಚನೆ k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->ತಳ.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	ಆದರೆ (!killed) {
		/* The thread is not being killed */
		ಆದರೆ (thread->ತಳ.pended_on != ಖಾಲಿ) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
ಶೂನ್ಯ z_thread_timeout(ರಚನೆ _timeout *timeout)
{
	ರಚನೆ k_thread *thread = CONTAINER_ದ(timeout,
					       ರಚನೆ k_thread, ತಳ.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		ಆದರೆ (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */

ಪೂರ್ಣಾಂಕ z_pend_curr(ರಚನೆ k_spinlock *ಬೀಗ, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	ಆದರೆ (arch_is_ಒಳ_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#ಆದರೆ defined(CONFIG_TIMESLICING) && defined(CONFIG_ಅದಲುಬದಲು_NONATOMIC)
	pending_current = _current;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((ಗಾತ್ರ(ರಚನೆ k_spinlock) == 0) || !z_is_sched_spinlock(ಬೀಗ));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(ಶೂನ್ಯ) z_sched_spinlock_ಬೀಗ();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(ಬೀಗ);
	ಹಿಂತಿರುಗಿ z_ಅದಲುಬದಲು_locked(ಕೀಲಿ);
}

ರಚನೆ k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	ರಚನೆ k_thread *thread = ಖಾಲಿ;

	Z_SCHED_SPINLOCK {
		thread = priq_wait_ಉತ್ತಮ(&wait_q->waitq);

		ಆದರೆ (thread != ಖಾಲಿ) {
			unpend_thread_no_timeout(thread);
		}
	}

	ಹಿಂತಿರುಗಿ thread;
}

ಶೂನ್ಯ z_unpend_thread(ರಚನೆ k_thread *thread)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	ಆದರೆ (thread->ತಳ.pended_on != ಖಾಲಿ) {
		unpend_thread_no_timeout(thread);
	}
	ಆಗ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();
	}
	z_sched_spinlock_unlock(ಕೀಲಿ);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_ಗಣ(ರಚನೆ k_thread *thread, ಪೂರ್ಣಾಂಕ prio)
{
	bool need_sched = false;
	ಪೂರ್ಣಾಂಕ old_prio = thread->ತಳ.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		ಆದರೆ (need_sched) {
			ಆದರೆ (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				ಸರತಿಯಿಂದ_ತೆಗೆ_thread(thread);
				thread->ತಳ.prio = prio;
				ಸರತಿ_thread(thread);

				ಆದರೆ (old_prio > prio) {
					flag_ipi(ipi_ಮುಖವಾಡ_create(thread));
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->ತಳ.prio = prio;

				ರಚನೆ _cpu *cpu;

				cpu = thread_ಸಕ್ರಿಯ_elsewhere(thread);
				ಆದರೆ ((cpu != ಖಾಲಿ) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_ಮುಖವಾಡ(cpu->id));
				}
			}

			update_cache(1);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->ತಳ.prio = prio;
			priq_wait_ಕೂಡಿಸು(&wait_q->waitq, thread);
		} ಇಲ್ಲದಿದ್ದರೆ {
			thread->ತಳ.prio = prio;
		}
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_thread, sched_ಆದ್ಯತೆ_ಗಣ, thread, prio);

	ಹಿಂತಿರುಗಿ need_sched;
}

ಶೂನ್ಯ z_reschedule(ರಚನೆ k_spinlock *ಬೀಗ, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ) ALIAS_ದ(reschedule);

ಶೂನ್ಯ z_reschedule_locked(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ) ALIAS_ದ(reschedule_locked);

ಶೂನ್ಯ z_reschedule_irqlock(uint32_t ಕೀಲಿ)
{
	ಆದರೆ (resched(ಕೀಲಿ) && need_ಅದಲುಬದಲು()) {
		z_ಅದಲುಬದಲು_irqlock(ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(ಕೀಲಿ);
	}
}

ರಚನೆ k_thread *z_ಅದಲುಬದಲು_ಮುಂದಿನ_thread(ಶೂನ್ಯ)
{
	ರಚನೆ k_thread *ret = ಮುಂದಿನ_ಮೇಲೆ();

	ಆದರೆ (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಗಣ_current(ರಚನೆ k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
		z_thread_mark_switched_ಹೊರಹೋಗು();
	}
	z_current_thread_ಗಣ(new_thread);
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
ಶೂನ್ಯ *z_get_ಮುಂದಿನ_switch_handle(ಶೂನ್ಯ *interrupted)
{
	z_check_ರಾಶಿ_sentinel();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	ಶೂನ್ಯ *ret = ಖಾಲಿ;

	Z_SCHED_SPINLOCK {
		ರಚನೆ k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == ಖಾಲಿ || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = ಮುಂದಿನ_ಮೇಲೆ();

		z_sched_usage_switch(new_thread);

		ಆದರೆ (old_thread != new_thread) {
			uint8_t  cpu_id;

			z_sched_switch_spin(new_thread);
			arch_cohere_stacks(old_thread, interrupted, new_thread);

			_current_cpu->ಅದಲುಬದಲು_ok = 0;
			cpu_id = arch_curr_cpu()->id;
			new_thread->ತಳ.cpu = cpu_id;
			ಗಣ_current(new_thread);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICING
			z_ಸಮಯ_ತುಂಡು_reset(new_thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICING */

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
			ಆದರೆ (z_is_thread_queued(old_thread)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_IPI_CASCADE
				ಆದರೆ ((new_thread->ತಳ.cpu_ಮುಖವಾಡ != -1) &&
				    (old_thread->ತಳ.cpu_ಮುಖವಾಡ != ಬಿಟ್(cpu_id))) {
					flag_ipi(ipi_ಮುಖವಾಡ_create(old_thread));
				}
#ಆದರೆ_ಅಂತ್ಯ
				runq_ಕೂಡಿಸು(old_thread);
			}
		}
		old_thread->switch_handle = interrupted;
		ret = new_thread->switch_handle;
		/* Active threads MUST have a null here */
		new_thread->switch_handle = ಖಾಲಿ;

		/* Check for IPIs under the lock to avoid silently consuming a
		 * rescheduling IPI flagged by another CPU for ourselves.
		 */
		signal_pending_ipi();
	}
	ಹಿಂತಿರುಗಿ ret;
#ಇಲ್ಲದಿದ್ದರೆ
	ರಚನೆ k_thread *ಮುಂದಿನ = _kernel.ready_q.cache;

	z_sched_usage_switch(ಮುಂದಿನ);
	_current->switch_handle = interrupted;
	ಗಣ_current(ಮುಂದಿನ);
	ಹಿಂತಿರುಗಿ ಮುಂದಿನ->switch_handle;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USE_SWITCH */

ಪೂರ್ಣಾಂಕ z_unpend_all(_wait_q_t *wait_q)
{
	ಪೂರ್ಣಾಂಕ need_sched = 0;
	ರಚನೆ k_thread *thread;

	Z_SCHED_SPINLOCK {
		ಗಾಗಿ (thread = z_waitq_ತಲೆ_locked(wait_q);
		     thread != ಖಾಲಿ;
		     thread = z_waitq_ತಲೆ_locked(wait_q)) {
			unpend_thread_no_timeout(thread);
			/* Abort the timeout and ready the thread unconditionally. If
			 * the timeout handler is in flight on another CPU, the abort
			 * flags it superseded and z_thread_timeout() bails when it
			 * runs -- so it will NOT ready the thread, we must.
			 */
			(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);
			ready_thread(thread);
			need_sched = 1;
		}
	}

	ಹಿಂತಿರುಗಿ need_sched;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ unpend_all(_wait_q_t *wait_q)
{
	ರಚನೆ k_thread *thread;

	ಗಾಗಿ (thread = z_waitq_ತಲೆ_locked(wait_q);
	     thread != ಖಾಲಿ;
	     thread = z_waitq_ತಲೆ_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_ಮೌಲ್ಯ_ಗಣ(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ABORT_HOOK
ಬಾಹ್ಯ ಶೂನ್ಯ thread_abort_hook(ರಚನೆ k_thread *thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_ABORT_HOOK */

/**
 * @brief Dequeues the specified thread
 *
 * Dequeues the specified thread and move it into the specified new state.
 *
 * @param thread Identify the thread to halt
 * @param new_state New thread state (_THREAD_DEAD or _THREAD_SUSPENDED)
 * @param key Pointer to the scheduler spinlock key held by the caller
 */
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ halt_thread(ರಚನೆ k_thread *thread, uint8_t new_state,
				      k_spinlock_ಕೀಲಿ_t *ಕೀಲಿ)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	ಆದರೆ ((thread->ತಳ.thread_state & new_state) == 0U) {
		thread->ತಳ.thread_state |= new_state;
		ಆದರೆ (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			ಸರತಿಯಿಂದ_ತೆಗೆ_thread(thread);
		}

		ಆದರೆ (new_state == _THREAD_DEAD) {
			ಆದರೆ (thread->ತಳ.pended_on != ಖಾಲಿ) {
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
			ಆದರೆ (ಕೀಲಿ != ಖಾಲಿ) {
				ಆಗ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*ಕೀಲಿ);
					*ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();
				}
			}
			unpend_all(&thread->join_ಸರತಿ);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			ಆದರೆ (thread == _current && arch_is_ಒಳ_isr()) {
				dummify = true;
			}
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
		unpend_all(&thread->halt_ಸರತಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
		update_cache(1);

		ಆದರೆ (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			ಹಿಂತಿರುಗಿ;
		}

		arch_coprocessors_disable(thread);

		SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, sched_abort, thread);

		z_thread_monitor_exit(thread);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ABORT_HOOK
		thread_abort_hook(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_ABORT_HOOK */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_THREAD
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_THREAD
		k_obj_core_stats_deregister(K_OBJ_CORE(thread));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_THREAD */
		k_obj_core_unlink(K_OBJ_CORE(thread));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_THREAD */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
		z_mem_domain_exit_thread(thread);
		k_thread_perms_all_clear(thread);
		k_object_uninit(thread->ರಾಶಿ_obj);
		k_object_uninit(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ABORT_NEED_CLEANUP
		k_thread_abort_cleanup(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

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
		ಆದರೆ (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USE_SWITCH
			_current->switch_handle = (ಶೂನ್ಯ *)1;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			ಆದರೆ (thread->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ ==
			    (ಶೂನ್ಯ *)&z_spinlock_abort_sentinel) {
				z_spin_validate_reset(true);
			}
#ಆದರೆ_ಅಂತ್ಯ
			z_dummy_thread_init(&_thread_dummy);

		}

		/* Finally update the halting thread state, on which
		 * other CPUs might be spinning (see
		 * thread_halt_spin()).
		 */
		clear_halting(thread);
	}
}

ಶೂನ್ಯ z_thread_suspend_current(ರಚನೆ k_thread *thread)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	ಸರತಿಯಿಂದ_ತೆಗೆ_thread(thread);
	update_cache(1);
	z_ಅದಲುಬದಲು_locked(ಕೀಲಿ);
}
