/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <priority_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kswap.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ipi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_arch_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/drivers/timer/system_timer.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/atomic.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/timing/timing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/util.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <metairq.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <run_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeslicing.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* pending_current is owned by timeslicing.c; sleep.c also accesses it */
#ਜੇ defined(CONFIG_ਅਦਲਾ_ਬਦਲੀ_NONATOMIC) && defined(CONFIG_TIMESLICING)
ਬਾਹਰੀ ਬਣਤਰ k_thread *pending_current;
#ਜੇ_ਅੰਤ

ਬਣਤਰ k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent ਬਣਤਰ k_thread _thread_dummy;

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ update_cache(ਪੂਰਨ preempt_ok);
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ halt_thread(ਬਣਤਰ k_thread *thread, uint8_t new_state,
				      k_spinlock_ਕੁੰਜੀ_t *ਕੁੰਜੀ);
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked(ਬਣਤਰ k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ clear_halting(ਬਣਤਰ k_thread *thread)
{
	ਜੇ (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_ਅੰਕ_CPUS > 1)) {
		barrier_dmem_fence_ਭਰਿਆ(); /* Other cpus spin on this locklessly! */
		thread->ਅਧਾਰ.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *ਅਗਲਾ_ਉੱਪਰ(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	bool ipi_idle_ਨਿਸ਼ਾਨਾ_rebound = false;
	ਬਣਤਰ k_thread *ipi_idle_ਨਿਸ਼ਾਨਾ = ipi_idle_reserved_take();

	ਜੇ (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, ਨਿਰਾ);
	}
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

	ਬਣਤਰ k_thread *thread = runq_ਉੱਤਮ();

	thread = metairq_preempt_recover(thread);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	ਵਾਪਸ (thread != ਨਿਰਾ) ? thread : _current_cpu->idle_thread;
#ਨਹੀਂਤਾਂ
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
	bool ਸਰਗਰਮ = z_is_thread_ready(_current);

	ਜੇ (thread == ਨਿਰਾ) {
		thread = _current_cpu->idle_thread;
	}

	ਜੇ (ਸਰਗਰਮ) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		ਜੇ ((cmp > 0) || ((cmp == 0) && !_current_cpu->ਅਦਲਾ_ਬਦਲੀ_ok)) {
			thread = _current;
		}

		ਜੇ (!should_preempt(thread, _current_cpu->ਅਦਲਾ_ਬਦਲੀ_ok)) {
			thread = _current;
		}
	}

	ਜੇ (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		ਜੇ (ਸਰਗਰਮ && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			ਕਤਾਰ_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	ਜੇ (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		ਜੇ (ipi_idle_ਨਿਸ਼ਾਨਾ != ਨਿਰਾ &&
		    thread != ipi_idle_ਨਿਸ਼ਾਨਾ &&
		    z_is_thread_queued(ipi_idle_ਨਿਸ਼ਾਨਾ)) {
			ipi_idle_ਨਿਸ਼ਾਨਾ_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_ਨਿਸ਼ਾਨਾ);
		} ਨਹੀਂਤਾਂ {
			ipi_idle_thread_unreserve(thread);
		}
		ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ_thread(thread);
	}

	_current_cpu->ਅਦਲਾ_ਬਦਲੀ_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	ਜੇ (!ipi_idle_ਨਿਸ਼ਾਨਾ_rebound &&
	    ipi_idle_ਨਿਸ਼ਾਨਾ != ਨਿਰਾ && z_is_thread_queued(ipi_idle_ਨਿਸ਼ਾਨਾ)) {
		flag_ipi(ipi_ਨਕਾਬ_create(ipi_idle_ਨਿਸ਼ਾਨਾ));
	}
	ਵਾਪਸ thread;
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}

ਖਾਲੀ move_current_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_prio_q(ਖਾਲੀ)
{
	runq_yield();

	update_cache(1);
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ update_cache(ਪੂਰਨ preempt_ok)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਬਣਤਰ k_thread *thread = ਅਗਲਾ_ਉੱਪਰ();

	ਜੇ (should_preempt(thread, preempt_ok)) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICING
		ਜੇ (thread != _current) {
			z_ਸਮਾਂ_ਟੁਕੜਾ_reset(thread);
		}
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} ਨਹੀਂਤਾਂ {
		_kernel.ready_q.cache = _current;
	}

#ਨਹੀਂਤਾਂ
	/* The way this works is that the CPU record keeps its
	 * "cooperative swapping is OK" flag until the next reschedule
	 * call or context switch.  It doesn't need to be tracked per
	 * thread because if the thread gets preempted for whatever
	 * reason the scheduler will make the same decision anyway.
	 */
	_current_cpu->ਅਦਲਾ_ਬਦਲੀ_ok = preempt_ok;
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}

/**
 * Returns pointer to _cpu if the thread is currently running on
 * another CPU.
 */
ਸਥਿਰ ਬਣਤਰ _cpu *thread_ਸਰਗਰਮ_elsewhere(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਪੂਰਨ thread_cpu_id = thread->ਅਧਾਰ.cpu;
	ਬਣਤਰ _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_ਅੰਕ_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	ਜੇ ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		ਵਾਪਸ thread_cpu;
	}

#ਜੇ_ਅੰਤ /* CONFIG_SMP */
	ARG_ਅਣਵਰਤਿਆ(thread);
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ready_thread(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	ਜੇ (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		ਕਤਾਰ_thread(thread);
		update_cache(0);

		flag_ipi(ipi_ਨਕਾਬ_create(thread));
	}
}

ਖਾਲੀ z_ready_thread(ਬਣਤਰ k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		ਜੇ (thread_ਸਰਗਰਮ_elsewhere(thread) == ਨਿਰਾ) {
			ready_thread(thread);
		}
	}
}

ਖਾਲੀ z_sched_ready_locked(ਬਣਤਰ k_thread *thread) ALIAS_ਦਾ(ready_thread);

ਸਥਿਰ ਖਾਲੀ unready_thread(ਬਣਤਰ k_thread *thread)
{
	ਜੇ (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ_thread(thread);
	}
	update_cache(thread == _current);
}

/* This routine exists for benchmarking purposes. It is not used in
 * general production code.
 */
ਖਾਲੀ z_unready_thread(ਬਣਤਰ k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		unready_thread(thread);
	}
}


ਖਾਲੀ z_sched_unready_locked(ਬਣਤਰ k_thread *thread) ALIAS_ਦਾ(unready_thread);

/* This routine only used for testing purposes */
ਖਾਲੀ z_yield_testing_only(ਖਾਲੀ)
{
	Z_SCHED_SPINLOCK {
		move_current_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_prio_q();
	}
}

/* Spins in ISR context, waiting for a thread known to be running on
 * another CPU to catch the IPI we sent and halt.  Note that we check
 * for ourselves being asynchronously halted first to prevent simple
 * deadlocks (but not complex ones involving cycles of 3+ threads!).
 * Acts to release the provided lock before returning.
 */
ਸਥਿਰ ਖਾਲੀ thread_halt_spin(ਬਣਤਰ k_thread *thread, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	ਜੇ (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &ਕੁੰਜੀ);
	}
	z_sched_spinlock_unlock(ਕੁੰਜੀ);
	ਜਦੋਂ (z_is_thread_halting(thread)) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ k = arch_irq_ਤਾਲਾ();

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
ਖਾਲੀ z_thread_halt(ਬਣਤਰ k_thread *thread, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ,
					bool terminate)
{
	_wait_q_t *wq = &thread->join_ਕਤਾਰ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	wq = terminate ? wq : &thread->halt_ਕਤਾਰ;
#ਜੇ_ਅੰਤ

	z_metairq_preempted_clear(thread);

	/* If the target is a thread running on another CPU, flag and
	 * poke (note that we might spin to wait, so a true
	 * synchronous IPI is needed here, not deferred!), it will
	 * halt itself in the IPI.  Otherwise it's unscheduled, so we
	 * can clean it up directly.
	 */

	ਬਣਤਰ _cpu *cpu = thread_ਸਰਗਰਮ_elsewhere(thread);

	ਜੇ (cpu != ਨਿਰਾ) {
		thread->ਅਧਾਰ.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#ਜੇ defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_ਨਕਾਬ(cpu->id));
#ਨਹੀਂਤਾਂ
		arch_sched_ਪ੍ਰਸਾਰਣ_ipi();
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#ਜੇ_ਅੰਤ /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		ਜੇ (arch_is_ਵਿੱਚ_isr()) {
			thread_halt_spin(thread, ਕੁੰਜੀ);
		} ਨਹੀਂਤਾਂ  {
			ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked(_current, wq);
			z_ਅਦਲਾ_ਬਦਲੀ_locked(ਕੁੰਜੀ);
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
		ਜਦੋਂ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} ਨਹੀਂਤਾਂ {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &ਕੁੰਜੀ);
		ਜੇ ((thread == _current) && !arch_is_ਵਿੱਚ_isr()) {
			ਜੇ (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(ਕੁੰਜੀ);
				k_panic();
				ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();
			}
			z_ਅਦਲਾ_ਬਦਲੀ_locked(ਕੁੰਜੀ);
			__ASSERT(!terminate, "aborted _current back from dead");
		} ਨਹੀਂਤਾਂ {
			z_sched_spinlock_unlock(ਕੁੰਜੀ);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool resched(uint32_t ਕੁੰਜੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	_current_cpu->ਅਦਲਾ_ਬਦਲੀ_ok = 0;
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

	ਵਾਪਸ arch_irq_unlocked(ਕੁੰਜੀ) && !arch_is_ਵਿੱਚ_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool need_ਅਦਲਾ_ਬਦਲੀ(ਖਾਲੀ)
{
	/* the SMP case will be handled in C based z_swap() */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਵਾਪਸ true;
#ਨਹੀਂਤਾਂ
	ਬਣਤਰ k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	ਵਾਪਸ new_thread != _current;
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}

ਸਥਿਰ ਖਾਲੀ reschedule(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	ਜੇ (resched(ਕੁੰਜੀ.ਕੁੰਜੀ) && need_ਅਦਲਾ_ਬਦਲੀ()) {
		z_ਅਦਲਾ_ਬਦਲੀ(ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		signal_pending_ipi();
		k_spin_unlock(ਤਾਲਾ, ਕੁੰਜੀ);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
ਸਥਿਰ ਖਾਲੀ reschedule_locked(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	ਵਾਪਸ reschedule(&_sched_spinlock, ਕੁੰਜੀ);
}

ਖਾਲੀ z_sched_ਤਾਲਾ_reschedule(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	update_cache(0);
	reschedule_locked(ਕੁੰਜੀ);
}

ਖਾਲੀ z_sched_yield(ਖਾਲੀ)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	runq_yield();
	update_cache(1);
	z_ਅਦਲਾ_ਬਦਲੀ_locked(ਕੁੰਜੀ);
}

/* The scheduler's spinlock must be held */
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked(ਬਣਤਰ k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->ਅਧਾਰ.pended_on == ਨਿਰਾ);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, sched_pend, thread);

	ਜੇ (wait_q != ਨਿਰਾ) {
		thread->ਅਧਾਰ.pended_on = wait_q;
		priq_wait_ਜੋੜੋ(&wait_q->waitq, thread);
	}
}

ਖਾਲੀ z_sched_ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked(ਬਣਤਰ k_thread *thread, _wait_q_t *wait_q)
	ALIAS_ਦਾ(ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked);

ਸਥਿਰ ਖਾਲੀ pend_locked(ਬਣਤਰ k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == ਨਿਰਾ || sys_cache_is_mem_coherent(wait_q));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */
	ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked(thread, wait_q);
	z_ਜੋੜੋ_thread_timeout(thread, timeout);
}

ਖਾਲੀ z_pend_thread(ਬਣਤਰ k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || is_thread_dummy(thread));
	Z_SCHED_SPINLOCK {
		pend_locked(thread, wait_q, timeout);
	}
}

ਖਾਲੀ z_unpend_thread_no_timeout(ਬਣਤਰ k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		ਜੇ (thread->ਅਧਾਰ.pended_on != ਨਿਰਾ) {
			unpend_thread_no_timeout(thread);
		}
	}
}

ਖਾਲੀ z_sched_wake_thread_locked(ਬਣਤਰ k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->ਅਧਾਰ.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	ਜੇ (!killed) {
		/* The thread is not being killed */
		ਜੇ (thread->ਅਧਾਰ.pended_on != ਨਿਰਾ) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
ਖਾਲੀ z_thread_timeout(ਬਣਤਰ _timeout *timeout)
{
	ਬਣਤਰ k_thread *thread = CONTAINER_ਦਾ(timeout,
					       ਬਣਤਰ k_thread, ਅਧਾਰ.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		ਜੇ (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#ਜੇ_ਅੰਤ /* CONFIG_SYS_CLOCK_EXISTS */

ਪੂਰਨ z_pend_curr(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	ਜੇ (arch_is_ਵਿੱਚ_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#ਜੇ defined(CONFIG_TIMESLICING) && defined(CONFIG_ਅਦਲਾ_ਬਦਲੀ_NONATOMIC)
	pending_current = _current;
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((ਆਕਾਰ(ਬਣਤਰ k_spinlock) == 0) || !z_is_sched_spinlock(ਤਾਲਾ));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(ਖਾਲੀ) z_sched_spinlock_ਤਾਲਾ();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(ਤਾਲਾ);
	ਵਾਪਸ z_ਅਦਲਾ_ਬਦਲੀ_locked(ਕੁੰਜੀ);
}

ਬਣਤਰ k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	ਬਣਤਰ k_thread *thread = ਨਿਰਾ;

	Z_SCHED_SPINLOCK {
		thread = priq_wait_ਉੱਤਮ(&wait_q->waitq);

		ਜੇ (thread != ਨਿਰਾ) {
			unpend_thread_no_timeout(thread);
		}
	}

	ਵਾਪਸ thread;
}

ਖਾਲੀ z_unpend_thread(ਬਣਤਰ k_thread *thread)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	ਜੇ (thread->ਅਧਾਰ.pended_on != ਨਿਰਾ) {
		unpend_thread_no_timeout(thread);
	}
	ਜਦੋਂ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(ਕੁੰਜੀ);
		ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();
	}
	z_sched_spinlock_unlock(ਕੁੰਜੀ);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_ਸੈੱਟ(ਬਣਤਰ k_thread *thread, ਪੂਰਨ prio)
{
	bool need_sched = false;
	ਪੂਰਨ old_prio = thread->ਅਧਾਰ.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		ਜੇ (need_sched) {
			ਜੇ (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ_thread(thread);
				thread->ਅਧਾਰ.prio = prio;
				ਕਤਾਰ_thread(thread);

				ਜੇ (old_prio > prio) {
					flag_ipi(ipi_ਨਕਾਬ_create(thread));
				}
			} ਨਹੀਂਤਾਂ {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->ਅਧਾਰ.prio = prio;

				ਬਣਤਰ _cpu *cpu;

				cpu = thread_ਸਰਗਰਮ_elsewhere(thread);
				ਜੇ ((cpu != ਨਿਰਾ) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_ਨਕਾਬ(cpu->id));
				}
			}

			update_cache(1);
		} ਨਹੀਂਤਾਂ ਜੇ (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->ਅਧਾਰ.prio = prio;
			priq_wait_ਜੋੜੋ(&wait_q->waitq, thread);
		} ਨਹੀਂਤਾਂ {
			thread->ਅਧਾਰ.prio = prio;
		}
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_thread, sched_ਤਰਜੀਹ_ਸੈੱਟ, thread, prio);

	ਵਾਪਸ need_sched;
}

ਖਾਲੀ z_reschedule(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ) ALIAS_ਦਾ(reschedule);

ਖਾਲੀ z_reschedule_locked(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ) ALIAS_ਦਾ(reschedule_locked);

ਖਾਲੀ z_reschedule_irqlock(uint32_t ਕੁੰਜੀ)
{
	ਜੇ (resched(ਕੁੰਜੀ) && need_ਅਦਲਾ_ਬਦਲੀ()) {
		z_ਅਦਲਾ_ਬਦਲੀ_irqlock(ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(ਕੁੰਜੀ);
	}
}

ਬਣਤਰ k_thread *z_ਅਦਲਾ_ਬਦਲੀ_ਅਗਲਾ_thread(ਖਾਲੀ)
{
	ਬਣਤਰ k_thread *ret = ਅਗਲਾ_ਉੱਪਰ();

	ਜੇ (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਸੈੱਟ_current(ਬਣਤਰ k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	ਜੇ (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
		z_thread_mark_switched_ਬਾਹਰਲਾ();
	}
	z_current_thread_ਸੈੱਟ(new_thread);
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
ਖਾਲੀ *z_get_ਅਗਲਾ_switch_handle(ਖਾਲੀ *interrupted)
{
	z_check_ਢੇਰ_sentinel();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਖਾਲੀ *ret = ਨਿਰਾ;

	Z_SCHED_SPINLOCK {
		ਬਣਤਰ k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == ਨਿਰਾ || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = ਅਗਲਾ_ਉੱਪਰ();

		z_sched_usage_switch(new_thread);

		ਜੇ (old_thread != new_thread) {
			uint8_t  cpu_id;

			z_sched_switch_spin(new_thread);
			arch_cohere_stacks(old_thread, interrupted, new_thread);

			_current_cpu->ਅਦਲਾ_ਬਦਲੀ_ok = 0;
			cpu_id = arch_curr_cpu()->id;
			new_thread->ਅਧਾਰ.cpu = cpu_id;
			ਸੈੱਟ_current(new_thread);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICING
			z_ਸਮਾਂ_ਟੁਕੜਾ_reset(new_thread);
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICING */

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
			ਜੇ (z_is_thread_queued(old_thread)) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_IPI_CASCADE
				ਜੇ ((new_thread->ਅਧਾਰ.cpu_ਨਕਾਬ != -1) &&
				    (old_thread->ਅਧਾਰ.cpu_ਨਕਾਬ != ਦੋਆਧਾਰੀ_ਅੰਕ(cpu_id))) {
					flag_ipi(ipi_ਨਕਾਬ_create(old_thread));
				}
#ਜੇ_ਅੰਤ
				runq_ਜੋੜੋ(old_thread);
			}
		}
		old_thread->switch_handle = interrupted;
		ret = new_thread->switch_handle;
		/* Active threads MUST have a null here */
		new_thread->switch_handle = ਨਿਰਾ;

		/* Check for IPIs under the lock to avoid silently consuming a
		 * rescheduling IPI flagged by another CPU for ourselves.
		 */
		signal_pending_ipi();
	}
	ਵਾਪਸ ret;
#ਨਹੀਂਤਾਂ
	ਬਣਤਰ k_thread *ਅਗਲਾ = _kernel.ready_q.cache;

	z_sched_usage_switch(ਅਗਲਾ);
	_current->switch_handle = interrupted;
	ਸੈੱਟ_current(ਅਗਲਾ);
	ਵਾਪਸ ਅਗਲਾ->switch_handle;
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}
#ਜੇ_ਅੰਤ /* CONFIG_USE_SWITCH */

ਪੂਰਨ z_unpend_all(_wait_q_t *wait_q)
{
	ਪੂਰਨ need_sched = 0;
	ਬਣਤਰ k_thread *thread;

	Z_SCHED_SPINLOCK {
		ਲਈ (thread = z_waitq_ਸਿਰ_locked(wait_q);
		     thread != ਨਿਰਾ;
		     thread = z_waitq_ਸਿਰ_locked(wait_q)) {
			unpend_thread_no_timeout(thread);
			/* Abort the timeout and ready the thread unconditionally. If
			 * the timeout handler is in flight on another CPU, the abort
			 * flags it superseded and z_thread_timeout() bails when it
			 * runs -- so it will NOT ready the thread, we must.
			 */
			(ਖਾਲੀ)z_try_abort_thread_timeout(thread);
			ready_thread(thread);
			need_sched = 1;
		}
	}

	ਵਾਪਸ need_sched;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ unpend_all(_wait_q_t *wait_q)
{
	ਬਣਤਰ k_thread *thread;

	ਲਈ (thread = z_waitq_ਸਿਰ_locked(wait_q);
	     thread != ਨਿਰਾ;
	     thread = z_waitq_ਸਿਰ_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(ਖਾਲੀ)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ABORT_HOOK
ਬਾਹਰੀ ਖਾਲੀ thread_abort_hook(ਬਣਤਰ k_thread *thread);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_ABORT_HOOK */

/**
 * @brief Dequeues the specified thread
 *
 * Dequeues the specified thread and move it into the specified new state.
 *
 * @param thread Identify the thread to halt
 * @param new_state New thread state (_THREAD_DEAD or _THREAD_SUSPENDED)
 * @param key Pointer to the scheduler spinlock key held by the caller
 */
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ halt_thread(ਬਣਤਰ k_thread *thread, uint8_t new_state,
				      k_spinlock_ਕੁੰਜੀ_t *ਕੁੰਜੀ)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	ਜੇ ((thread->ਅਧਾਰ.thread_state & new_state) == 0U) {
		thread->ਅਧਾਰ.thread_state |= new_state;
		ਜੇ (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ_thread(thread);
		}

		ਜੇ (new_state == _THREAD_DEAD) {
			ਜੇ (thread->ਅਧਾਰ.pended_on != ਨਿਰਾ) {
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
			ਜੇ (ਕੁੰਜੀ != ਨਿਰਾ) {
				ਜਦੋਂ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*ਕੁੰਜੀ);
					*ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();
				}
			}
			unpend_all(&thread->join_ਕਤਾਰ);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			ਜੇ (thread == _current && arch_is_ਵਿੱਚ_isr()) {
				dummify = true;
			}
		}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
		unpend_all(&thread->halt_ਕਤਾਰ);
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
		update_cache(1);

		ਜੇ (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			ਵਾਪਸ;
		}

		arch_coprocessors_disable(thread);

		SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, sched_abort, thread);

		z_thread_monitor_exit(thread);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ABORT_HOOK
		thread_abort_hook(thread);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_ABORT_HOOK */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_THREAD
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_THREAD
		k_obj_core_stats_deregister(K_OBJ_CORE(thread));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_THREAD */
		k_obj_core_unlink(K_OBJ_CORE(thread));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_THREAD */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
		z_mem_domain_exit_thread(thread);
		k_thread_perms_all_clear(thread);
		k_object_uninit(thread->ਢੇਰ_obj);
		k_object_uninit(thread);
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ABORT_NEED_CLEANUP
		k_thread_abort_cleanup(thread);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

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
		ਜੇ (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USE_SWITCH
			_current->switch_handle = (ਖਾਲੀ *)1;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			ਜੇ (thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ ==
			    (ਖਾਲੀ *)&z_spinlock_abort_sentinel) {
				z_spin_validate_reset(true);
			}
#ਜੇ_ਅੰਤ
			z_dummy_thread_init(&_thread_dummy);

		}

		/* Finally update the halting thread state, on which
		 * other CPUs might be spinning (see
		 * thread_halt_spin()).
		 */
		clear_halting(thread);
	}
}

ਖਾਲੀ z_thread_suspend_current(ਬਣਤਰ k_thread *thread)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ_thread(thread);
	update_cache(1);
	z_ਅਦਲਾ_ਬਦਲੀ_locked(ਕੁੰਜੀ);
}
