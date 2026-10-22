/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kspinlock.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <kthread.h>
#അന്തര്ഭാവ <priority_q.h>
#അന്തര്ഭാവ <kswap.h>
#അന്തര്ഭാവ <ipi.h>
#അന്തര്ഭാവ <kernel_arch_func.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/drivers/timer/system_timer.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>
#അന്തര്ഭാവ <zephyr/sys/atomic.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/timing/timing.h>
#അന്തര്ഭാവ <zephyr/sys/util.h>
#അന്തര്ഭാവ <metairq.h>
#അന്തര്ഭാവ <run_q.h>
#അന്തര്ഭാവ <timeslicing.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* pending_current is owned by timeslicing.c; sleep.c also accesses it */
#എങ്കിൽ defined(CONFIG_SWAP_NONATOMIC) && defined(CONFIG_TIMESLICING)
ബാഹ്യ ഘടന k_thread *pending_current;
#endif

ഘടന k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent ഘടന k_thread _thread_dummy;

സ്ഥിതം ALWAYS_INLINE ശൂന്യം update_cache(പൂർണ്ണസംഖ്യ preempt_ok);
സ്ഥിതം ALWAYS_INLINE ശൂന്യം halt_thread(ഘടന k_thread *thread, uint8_t new_state,
				      k_spinlock_താക്കോൽ_t *താക്കോൽ);
സ്ഥിതം ശൂന്യം add_to_waitq_locked(ഘടന k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
സ്ഥിതം വരിയിൽ ശൂന്യം clear_halting(ഘടന k_thread *thread)
{
	എങ്കിൽ (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_അക്കം_CPUS > 1)) {
		barrier_dmem_fence_full(); /* Other cpus spin on this locklessly! */
		thread->base.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *next_up(ശൂന്യം)
{
#ifdef CONFIG_SMP
	bool ipi_idle_target_rebound = false;
	ഘടന k_thread *ipi_idle_target = ipi_idle_reserved_take();

	എങ്കിൽ (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, ശൂന്യ);
	}
#endif /* CONFIG_SMP */

	ഘടന k_thread *thread = runq_best();

	thread = metairq_preempt_recover(thread);

#ifndef CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	തിരികെ (thread != ശൂന്യ) ? thread : _current_cpu->idle_thread;
#അല്ലെങ്കിൽ
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
	bool active = z_is_thread_ready(_current);

	എങ്കിൽ (thread == ശൂന്യ) {
		thread = _current_cpu->idle_thread;
	}

	എങ്കിൽ (active) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		എങ്കിൽ ((cmp > 0) || ((cmp == 0) && !_current_cpu->swap_ok)) {
			thread = _current;
		}

		എങ്കിൽ (!should_preempt(thread, _current_cpu->swap_ok)) {
			thread = _current;
		}
	}

	എങ്കിൽ (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		എങ്കിൽ (active && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			queue_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	എങ്കിൽ (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		എങ്കിൽ (ipi_idle_target != ശൂന്യ &&
		    thread != ipi_idle_target &&
		    z_is_thread_queued(ipi_idle_target)) {
			ipi_idle_target_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_target);
		} അല്ലെങ്കിൽ {
			ipi_idle_thread_unreserve(thread);
		}
		dequeue_thread(thread);
	}

	_current_cpu->swap_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	എങ്കിൽ (!ipi_idle_target_rebound &&
	    ipi_idle_target != ശൂന്യ && z_is_thread_queued(ipi_idle_target)) {
		flag_ipi(ipi_mask_create(ipi_idle_target));
	}
	തിരികെ thread;
#endif /* CONFIG_SMP */
}

ശൂന്യം move_current_to_end_of_prio_q(ശൂന്യം)
{
	runq_yield();

	update_cache(1);
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം update_cache(പൂർണ്ണസംഖ്യ preempt_ok)
{
#ifndef CONFIG_SMP
	ഘടന k_thread *thread = next_up();

	എങ്കിൽ (should_preempt(thread, preempt_ok)) {
#ifdef CONFIG_TIMESLICING
		എങ്കിൽ (thread != _current) {
			z_time_slice_reset(thread);
		}
#endif /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} അല്ലെങ്കിൽ {
		_kernel.ready_q.cache = _current;
	}

#അല്ലെങ്കിൽ
	/* The way this works is that the CPU record keeps its
	 * "cooperative swapping is OK" flag until the next reschedule
	 * call or context switch.  It doesn't need to be tracked per
	 * thread because if the thread gets preempted for whatever
	 * reason the scheduler will make the same decision anyway.
	 */
	_current_cpu->swap_ok = preempt_ok;
#endif /* CONFIG_SMP */
}

/**
 * Returns pointer to _cpu if the thread is currently running on
 * another CPU.
 */
സ്ഥിതം ഘടന _cpu *thread_active_elsewhere(ഘടന k_thread *thread)
{
#ifdef CONFIG_SMP
	പൂർണ്ണസംഖ്യ thread_cpu_id = thread->base.cpu;
	ഘടന _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_അക്കം_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	എങ്കിൽ ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		തിരികെ thread_cpu;
	}

#endif /* CONFIG_SMP */
	ARG_UNUSED(thread);
	തിരികെ ശൂന്യ;
}

സ്ഥിതം വരിയിൽ ശൂന്യം ready_thread(ഘടന k_thread *thread)
{
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#endif /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	എങ്കിൽ (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_PORT_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		queue_thread(thread);
		update_cache(0);

		flag_ipi(ipi_mask_create(thread));
	}
}

ശൂന്യം z_ready_thread(ഘടന k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		എങ്കിൽ (thread_active_elsewhere(thread) == ശൂന്യ) {
			ready_thread(thread);
		}
	}
}

ശൂന്യം z_sched_ready_locked(ഘടന k_thread *thread) ALIAS_OF(ready_thread);

സ്ഥിതം ശൂന്യം unready_thread(ഘടന k_thread *thread)
{
	എങ്കിൽ (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		dequeue_thread(thread);
	}
	update_cache(thread == _current);
}

/* This routine exists for benchmarking purposes. It is not used in
 * general production code.
 */
ശൂന്യം z_unready_thread(ഘടന k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		unready_thread(thread);
	}
}


ശൂന്യം z_sched_unready_locked(ഘടന k_thread *thread) ALIAS_OF(unready_thread);

/* This routine only used for testing purposes */
ശൂന്യം z_yield_testing_only(ശൂന്യം)
{
	Z_SCHED_SPINLOCK {
		move_current_to_end_of_prio_q();
	}
}

/* Spins in ISR context, waiting for a thread known to be running on
 * another CPU to catch the IPI we sent and halt.  Note that we check
 * for ourselves being asynchronously halted first to prevent simple
 * deadlocks (but not complex ones involving cycles of 3+ threads!).
 * Acts to release the provided lock before returning.
 */
സ്ഥിതം ശൂന്യം thread_halt_spin(ഘടന k_thread *thread, k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	എങ്കിൽ (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &താക്കോൽ);
	}
	z_sched_spinlock_unlock(താക്കോൽ);
	ആകുമ്പോൾ (z_is_thread_halting(thread)) {
		unsigned പൂർണ്ണസംഖ്യ k = arch_irq_lock();

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
ശൂന്യം z_thread_halt(ഘടന k_thread *thread, k_spinlock_താക്കോൽ_t താക്കോൽ,
					bool terminate)
{
	_wait_q_t *wq = &thread->join_queue;
#ifdef CONFIG_SMP
	wq = terminate ? wq : &thread->halt_queue;
#endif

	z_metairq_preempted_clear(thread);

	/* If the target is a thread running on another CPU, flag and
	 * poke (note that we might spin to wait, so a true
	 * synchronous IPI is needed here, not deferred!), it will
	 * halt itself in the IPI.  Otherwise it's unscheduled, so we
	 * can clean it up directly.
	 */

	ഘടന _cpu *cpu = thread_active_elsewhere(thread);

	എങ്കിൽ (cpu != ശൂന്യ) {
		thread->base.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#എങ്കിൽ defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_MASK(cpu->id));
#അല്ലെങ്കിൽ
		arch_sched_broadcast_ipi();
#endif /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#endif /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		എങ്കിൽ (arch_is_in_isr()) {
			thread_halt_spin(thread, താക്കോൽ);
		} അല്ലെങ്കിൽ  {
			add_to_waitq_locked(_current, wq);
			z_swap_locked(താക്കോൽ);
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
		ആകുമ്പോൾ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} അല്ലെങ്കിൽ {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &താക്കോൽ);
		എങ്കിൽ ((thread == _current) && !arch_is_in_isr()) {
			എങ്കിൽ (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(താക്കോൽ);
				k_panic();
				താക്കോൽ = z_sched_spinlock_lock();
			}
			z_swap_locked(താക്കോൽ);
			__ASSERT(!terminate, "aborted _current back from dead");
		} അല്ലെങ്കിൽ {
			z_sched_spinlock_unlock(താക്കോൽ);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

സ്ഥിതം വരിയിൽ bool resched(uint32_t താക്കോൽ)
{
#ifdef CONFIG_SMP
	_current_cpu->swap_ok = 0;
#endif /* CONFIG_SMP */

	തിരികെ arch_irq_unlocked(താക്കോൽ) && !arch_is_in_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
സ്ഥിതം വരിയിൽ bool need_swap(ശൂന്യം)
{
	/* the SMP case will be handled in C based z_swap() */
#ifdef CONFIG_SMP
	തിരികെ true;
#അല്ലെങ്കിൽ
	ഘടന k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	തിരികെ new_thread != _current;
#endif /* CONFIG_SMP */
}

സ്ഥിതം ശൂന്യം reschedule(ഘടന k_spinlock *lock, k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	എങ്കിൽ (resched(താക്കോൽ.താക്കോൽ) && need_swap()) {
		z_swap(lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		signal_pending_ipi();
		k_spin_unlock(lock, താക്കോൽ);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
സ്ഥിതം ശൂന്യം reschedule_locked(k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	തിരികെ reschedule(&_sched_spinlock, താക്കോൽ);
}

ശൂന്യം z_sched_lock_reschedule(k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	update_cache(0);
	reschedule_locked(താക്കോൽ);
}

ശൂന്യം z_sched_yield(ശൂന്യം)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	runq_yield();
	update_cache(1);
	z_swap_locked(താക്കോൽ);
}

/* The scheduler's spinlock must be held */
സ്ഥിതം ശൂന്യം add_to_waitq_locked(ഘടന k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->base.pended_on == ശൂന്യ);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_PORT_TRACING_FUNC(k_thread, sched_pend, thread);

	എങ്കിൽ (wait_q != ശൂന്യ) {
		thread->base.pended_on = wait_q;
		_priq_wait_add(&wait_q->waitq, thread);
	}
}

ശൂന്യം z_sched_add_to_waitq_locked(ഘടന k_thread *thread, _wait_q_t *wait_q)
	ALIAS_OF(add_to_waitq_locked);

സ്ഥിതം ശൂന്യം pend_locked(ഘടന k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == ശൂന്യ || sys_cache_is_mem_coherent(wait_q));
#endif /* CONFIG_KERNEL_COHERENCE */
	add_to_waitq_locked(thread, wait_q);
	z_add_thread_timeout(thread, timeout);
}

ശൂന്യം z_pend_thread(ഘടന k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || is_thread_dummy(thread));
	Z_SCHED_SPINLOCK {
		pend_locked(thread, wait_q, timeout);
	}
}

ശൂന്യം z_unpend_thread_no_timeout(ഘടന k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		എങ്കിൽ (thread->base.pended_on != ശൂന്യ) {
			unpend_thread_no_timeout(thread);
		}
	}
}

ശൂന്യം z_sched_wake_thread_locked(ഘടന k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->base.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	എങ്കിൽ (!killed) {
		/* The thread is not being killed */
		എങ്കിൽ (thread->base.pended_on != ശൂന്യ) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#ifdef CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
ശൂന്യം z_thread_timeout(ഘടന _timeout *timeout)
{
	ഘടന k_thread *thread = CONTAINER_OF(timeout,
					       ഘടന k_thread, base.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		എങ്കിൽ (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

പൂർണ്ണസംഖ്യ z_pend_curr(ഘടന k_spinlock *lock, k_spinlock_താക്കോൽ_t താക്കോൽ,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	എങ്കിൽ (arch_is_in_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#എങ്കിൽ defined(CONFIG_TIMESLICING) && defined(CONFIG_SWAP_NONATOMIC)
	pending_current = _current;
#endif /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((വലുപ്പം(ഘടന k_spinlock) == 0) || !z_is_sched_spinlock(lock));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(ശൂന്യം) z_sched_spinlock_lock();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(lock);
	തിരികെ z_swap_locked(താക്കോൽ);
}

ഘടന k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	ഘടന k_thread *thread = ശൂന്യ;

	Z_SCHED_SPINLOCK {
		thread = _priq_wait_best(&wait_q->waitq);

		എങ്കിൽ (thread != ശൂന്യ) {
			unpend_thread_no_timeout(thread);
		}
	}

	തിരികെ thread;
}

ശൂന്യം z_unpend_thread(ഘടന k_thread *thread)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	എങ്കിൽ (thread->base.pended_on != ശൂന്യ) {
		unpend_thread_no_timeout(thread);
	}
	ആകുമ്പോൾ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(താക്കോൽ);
		താക്കോൽ = z_sched_spinlock_lock();
	}
	z_sched_spinlock_unlock(താക്കോൽ);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_set(ഘടന k_thread *thread, പൂർണ്ണസംഖ്യ prio)
{
	bool need_sched = false;
	പൂർണ്ണസംഖ്യ old_prio = thread->base.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		എങ്കിൽ (need_sched) {
			എങ്കിൽ (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				dequeue_thread(thread);
				thread->base.prio = prio;
				queue_thread(thread);

				എങ്കിൽ (old_prio > prio) {
					flag_ipi(ipi_mask_create(thread));
				}
			} അല്ലെങ്കിൽ {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->base.prio = prio;

				ഘടന _cpu *cpu;

				cpu = thread_active_elsewhere(thread);
				എങ്കിൽ ((cpu != ശൂന്യ) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_MASK(cpu->id));
				}
			}

			update_cache(1);
		} അല്ലെങ്കിൽ എങ്കിൽ (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->base.prio = prio;
			_priq_wait_add(&wait_q->waitq, thread);
		} അല്ലെങ്കിൽ {
			thread->base.prio = prio;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, sched_priority_set, thread, prio);

	തിരികെ need_sched;
}

ശൂന്യം z_reschedule(ഘടന k_spinlock *lock, k_spinlock_താക്കോൽ_t താക്കോൽ) ALIAS_OF(reschedule);

ശൂന്യം z_reschedule_locked(k_spinlock_താക്കോൽ_t താക്കോൽ) ALIAS_OF(reschedule_locked);

ശൂന്യം z_reschedule_irqlock(uint32_t താക്കോൽ)
{
	എങ്കിൽ (resched(താക്കോൽ) && need_swap()) {
		z_swap_irqlock(താക്കോൽ);
	} അല്ലെങ്കിൽ {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(താക്കോൽ);
	}
}

ഘടന k_thread *z_swap_next_thread(ശൂന്യം)
{
	ഘടന k_thread *ret = next_up();

	എങ്കിൽ (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	തിരികെ ret;
}

#ifdef CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം set_current(ഘടന k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
		z_thread_mark_switched_out();
	}
	z_current_thread_set(new_thread);
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
ശൂന്യം *z_get_next_switch_handle(ശൂന്യം *interrupted)
{
	z_check_അട്ടി_sentinel();

#ifdef CONFIG_SMP
	ശൂന്യം *ret = ശൂന്യ;

	Z_SCHED_SPINLOCK {
		ഘടന k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == ശൂന്യ || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = next_up();

		z_sched_usage_switch(new_thread);

		എങ്കിൽ (old_thread != new_thread) {
			uint8_t  cpu_id;

			z_sched_switch_spin(new_thread);
			arch_cohere_stacks(old_thread, interrupted, new_thread);

			_current_cpu->swap_ok = 0;
			cpu_id = arch_curr_cpu()->id;
			new_thread->base.cpu = cpu_id;
			set_current(new_thread);

#ifdef CONFIG_TIMESLICING
			z_time_slice_reset(new_thread);
#endif /* CONFIG_TIMESLICING */

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
			എങ്കിൽ (z_is_thread_queued(old_thread)) {
#ifdef CONFIG_SCHED_IPI_CASCADE
				എങ്കിൽ ((new_thread->base.cpu_mask != -1) &&
				    (old_thread->base.cpu_mask != BIT(cpu_id))) {
					flag_ipi(ipi_mask_create(old_thread));
				}
#endif
				runq_add(old_thread);
			}
		}
		old_thread->switch_handle = interrupted;
		ret = new_thread->switch_handle;
		/* Active threads MUST have a null here */
		new_thread->switch_handle = ശൂന്യ;

		/* Check for IPIs under the lock to avoid silently consuming a
		 * rescheduling IPI flagged by another CPU for ourselves.
		 */
		signal_pending_ipi();
	}
	തിരികെ ret;
#അല്ലെങ്കിൽ
	ഘടന k_thread *next = _kernel.ready_q.cache;

	z_sched_usage_switch(next);
	_current->switch_handle = interrupted;
	set_current(next);
	തിരികെ next->switch_handle;
#endif /* CONFIG_SMP */
}
#endif /* CONFIG_USE_SWITCH */

പൂർണ്ണസംഖ്യ z_unpend_all(_wait_q_t *wait_q)
{
	പൂർണ്ണസംഖ്യ need_sched = 0;
	ഘടന k_thread *thread;

	Z_SCHED_SPINLOCK {
		വേണ്ടി (thread = z_waitq_head_locked(wait_q);
		     thread != ശൂന്യ;
		     thread = z_waitq_head_locked(wait_q)) {
			unpend_thread_no_timeout(thread);
			/* Abort the timeout and ready the thread unconditionally. If
			 * the timeout handler is in flight on another CPU, the abort
			 * flags it superseded and z_thread_timeout() bails when it
			 * runs -- so it will NOT ready the thread, we must.
			 */
			(ശൂന്യം)z_try_abort_thread_timeout(thread);
			ready_thread(thread);
			need_sched = 1;
		}
	}

	തിരികെ need_sched;
}

സ്ഥിതം വരിയിൽ ശൂന്യം unpend_all(_wait_q_t *wait_q)
{
	ഘടന k_thread *thread;

	വേണ്ടി (thread = z_waitq_head_locked(wait_q);
	     thread != ശൂന്യ;
	     thread = z_waitq_head_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_മൂല്യം_set(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(ശൂന്യം)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#ifdef CONFIG_THREAD_ABORT_HOOK
ബാഹ്യ ശൂന്യം thread_abort_hook(ഘടന k_thread *thread);
#endif /* CONFIG_THREAD_ABORT_HOOK */

/**
 * @brief Dequeues the specified thread
 *
 * Dequeues the specified thread and move it into the specified new state.
 *
 * @param thread Identify the thread to halt
 * @param new_state New thread state (_THREAD_DEAD or _THREAD_SUSPENDED)
 * @param key Pointer to the scheduler spinlock key held by the caller
 */
സ്ഥിതം ALWAYS_INLINE ശൂന്യം halt_thread(ഘടന k_thread *thread, uint8_t new_state,
				      k_spinlock_താക്കോൽ_t *താക്കോൽ)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	എങ്കിൽ ((thread->base.thread_state & new_state) == 0U) {
		thread->base.thread_state |= new_state;
		എങ്കിൽ (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			dequeue_thread(thread);
		}

		എങ്കിൽ (new_state == _THREAD_DEAD) {
			എങ്കിൽ (thread->base.pended_on != ശൂന്യ) {
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
			എങ്കിൽ (താക്കോൽ != ശൂന്യ) {
				ആകുമ്പോൾ (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*താക്കോൽ);
					*താക്കോൽ = z_sched_spinlock_lock();
				}
			}
			unpend_all(&thread->join_queue);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			എങ്കിൽ (thread == _current && arch_is_in_isr()) {
				dummify = true;
			}
		}
#ifdef CONFIG_SMP
		unpend_all(&thread->halt_queue);
#endif /* CONFIG_SMP */
		update_cache(1);

		എങ്കിൽ (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			തിരികെ;
		}

		arch_coprocessors_disable(thread);

		SYS_PORT_TRACING_FUNC(k_thread, sched_abort, thread);

		z_thread_monitor_exit(thread);
#ifdef CONFIG_THREAD_ABORT_HOOK
		thread_abort_hook(thread);
#endif /* CONFIG_THREAD_ABORT_HOOK */

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
		k_obj_core_stats_deregister(K_OBJ_CORE(thread));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
		k_obj_core_unlink(K_OBJ_CORE(thread));
#endif /* CONFIG_OBJ_CORE_THREAD */

#ifdef CONFIG_USERSPACE
		z_mem_domain_exit_thread(thread);
		k_thread_perms_all_clear(thread);
		k_object_uninit(thread->അട്ടി_obj);
		k_object_uninit(thread);
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
		k_thread_abort_cleanup(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

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
		എങ്കിൽ (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#ifdef CONFIG_USE_SWITCH
			_current->switch_handle = (ശൂന്യം *)1;
#endif
#ifdef CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			എങ്കിൽ (thread->base.swap_data ==
			    (ശൂന്യം *)&z_spinlock_abort_sentinel) {
				z_spin_validate_reset(true);
			}
#endif
			z_dummy_thread_init(&_thread_dummy);

		}

		/* Finally update the halting thread state, on which
		 * other CPUs might be spinning (see
		 * thread_halt_spin()).
		 */
		clear_halting(thread);
	}
}

ശൂന്യം z_thread_suspend_current(ഘടന k_thread *thread)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	dequeue_thread(thread);
	update_cache(1);
	z_swap_locked(താക്കോൽ);
}
