/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kspinlock.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <priority_q.h>
#અન્તર્ભાવ <kswap.h>
#અન્તર્ભાવ <ipi.h>
#અન્તર્ભાવ <kernel_arch_func.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/drivers/timer/system_timer.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
#અન્તર્ભાવ <zephyr/sys/atomic.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/timing/timing.h>
#અન્તર્ભાવ <zephyr/sys/util.h>
#અન્તર્ભાવ <metairq.h>
#અન્તર્ભાવ <run_q.h>
#અન્તર્ભાવ <timeslicing.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* pending_current is owned by timeslicing.c; sleep.c also accesses it */
#જો defined(CONFIG_SWAP_NONATOMIC) && defined(CONFIG_TIMESLICING)
બાહ્ય સંરચના k_thread *pending_current;
#endif

સંરચના k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent સંરચના k_thread _thread_dummy;

સ્થિર ALWAYS_INLINE શૂન્ય update_cache(પૂર્ણાંક preempt_ok);
સ્થિર ALWAYS_INLINE શૂન્ય halt_thread(સંરચના k_thread *thread, uint8_t new_state,
				      k_spinlock_ચાવી_t *ચાવી);
સ્થિર શૂન્ય add_to_waitq_locked(સંરચના k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
સ્થિર અંતઃરેખ શૂન્ય clear_halting(સંરચના k_thread *thread)
{
	જો (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_અંક_CPUS > 1)) {
		barrier_dmem_fence_full(); /* Other cpus spin on this locklessly! */
		thread->base.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

સ્થિર ALWAYS_INLINE સંરચના k_thread *next_up(શૂન્ય)
{
#ifdef CONFIG_SMP
	bool ipi_idle_target_rebound = false;
	સંરચના k_thread *ipi_idle_target = ipi_idle_reserved_take();

	જો (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, ખાલી);
	}
#endif /* CONFIG_SMP */

	સંરચના k_thread *thread = runq_best();

	thread = metairq_preempt_recover(thread);

#ifndef CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	પાછા (thread != ખાલી) ? thread : _current_cpu->idle_thread;
#નહીંતર
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

	જો (thread == ખાલી) {
		thread = _current_cpu->idle_thread;
	}

	જો (active) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		જો ((cmp > 0) || ((cmp == 0) && !_current_cpu->swap_ok)) {
			thread = _current;
		}

		જો (!should_preempt(thread, _current_cpu->swap_ok)) {
			thread = _current;
		}
	}

	જો (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		જો (active && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			queue_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	જો (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		જો (ipi_idle_target != ખાલી &&
		    thread != ipi_idle_target &&
		    z_is_thread_queued(ipi_idle_target)) {
			ipi_idle_target_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_target);
		} નહીંતર {
			ipi_idle_thread_unreserve(thread);
		}
		dequeue_thread(thread);
	}

	_current_cpu->swap_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	જો (!ipi_idle_target_rebound &&
	    ipi_idle_target != ખાલી && z_is_thread_queued(ipi_idle_target)) {
		flag_ipi(ipi_mask_create(ipi_idle_target));
	}
	પાછા thread;
#endif /* CONFIG_SMP */
}

શૂન્ય move_current_to_end_of_prio_q(શૂન્ય)
{
	runq_yield();

	update_cache(1);
}

સ્થિર ALWAYS_INLINE શૂન્ય update_cache(પૂર્ણાંક preempt_ok)
{
#ifndef CONFIG_SMP
	સંરચના k_thread *thread = next_up();

	જો (should_preempt(thread, preempt_ok)) {
#ifdef CONFIG_TIMESLICING
		જો (thread != _current) {
			z_time_slice_reset(thread);
		}
#endif /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} નહીંતર {
		_kernel.ready_q.cache = _current;
	}

#નહીંતર
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
સ્થિર સંરચના _cpu *thread_active_elsewhere(સંરચના k_thread *thread)
{
#ifdef CONFIG_SMP
	પૂર્ણાંક thread_cpu_id = thread->base.cpu;
	સંરચના _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_અંક_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	જો ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		પાછા thread_cpu;
	}

#endif /* CONFIG_SMP */
	ARG_UNUSED(thread);
	પાછા ખાલી;
}

સ્થિર અંતઃરેખ શૂન્ય ready_thread(સંરચના k_thread *thread)
{
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#endif /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	જો (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_PORT_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		queue_thread(thread);
		update_cache(0);

		flag_ipi(ipi_mask_create(thread));
	}
}

શૂન્ય z_ready_thread(સંરચના k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		જો (thread_active_elsewhere(thread) == ખાલી) {
			ready_thread(thread);
		}
	}
}

શૂન્ય z_sched_ready_locked(સંરચના k_thread *thread) ALIAS_OF(ready_thread);

સ્થિર શૂન્ય unready_thread(સંરચના k_thread *thread)
{
	જો (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		dequeue_thread(thread);
	}
	update_cache(thread == _current);
}

/* This routine exists for benchmarking purposes. It is not used in
 * general production code.
 */
શૂન્ય z_unready_thread(સંરચના k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		unready_thread(thread);
	}
}


શૂન્ય z_sched_unready_locked(સંરચના k_thread *thread) ALIAS_OF(unready_thread);

/* This routine only used for testing purposes */
શૂન્ય z_yield_testing_only(શૂન્ય)
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
સ્થિર શૂન્ય thread_halt_spin(સંરચના k_thread *thread, k_spinlock_ચાવી_t ચાવી)
{
	જો (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &ચાવી);
	}
	z_sched_spinlock_unlock(ચાવી);
	જ્યારે (z_is_thread_halting(thread)) {
		unsigned પૂર્ણાંક k = arch_irq_lock();

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
શૂન્ય z_thread_halt(સંરચના k_thread *thread, k_spinlock_ચાવી_t ચાવી,
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

	સંરચના _cpu *cpu = thread_active_elsewhere(thread);

	જો (cpu != ખાલી) {
		thread->base.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#જો defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_MASK(cpu->id));
#નહીંતર
		arch_sched_broadcast_ipi();
#endif /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#endif /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		જો (arch_is_in_isr()) {
			thread_halt_spin(thread, ચાવી);
		} નહીંતર  {
			add_to_waitq_locked(_current, wq);
			z_swap_locked(ચાવી);
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
		જ્યારે (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} નહીંતર {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &ચાવી);
		જો ((thread == _current) && !arch_is_in_isr()) {
			જો (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(ચાવી);
				k_panic();
				ચાવી = z_sched_spinlock_lock();
			}
			z_swap_locked(ચાવી);
			__ASSERT(!terminate, "aborted _current back from dead");
		} નહીંતર {
			z_sched_spinlock_unlock(ચાવી);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

સ્થિર અંતઃરેખ bool resched(uint32_t ચાવી)
{
#ifdef CONFIG_SMP
	_current_cpu->swap_ok = 0;
#endif /* CONFIG_SMP */

	પાછા arch_irq_unlocked(ચાવી) && !arch_is_in_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
સ્થિર અંતઃરેખ bool need_swap(શૂન્ય)
{
	/* the SMP case will be handled in C based z_swap() */
#ifdef CONFIG_SMP
	પાછા true;
#નહીંતર
	સંરચના k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	પાછા new_thread != _current;
#endif /* CONFIG_SMP */
}

સ્થિર શૂન્ય reschedule(સંરચના k_spinlock *lock, k_spinlock_ચાવી_t ચાવી)
{
	જો (resched(ચાવી.ચાવી) && need_swap()) {
		z_swap(lock, ચાવી);
	} નહીંતર {
		signal_pending_ipi();
		k_spin_unlock(lock, ચાવી);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
સ્થિર શૂન્ય reschedule_locked(k_spinlock_ચાવી_t ચાવી)
{
	પાછા reschedule(&_sched_spinlock, ચાવી);
}

શૂન્ય z_sched_lock_reschedule(k_spinlock_ચાવી_t ચાવી)
{
	update_cache(0);
	reschedule_locked(ચાવી);
}

શૂન્ય z_sched_yield(શૂન્ય)
{
	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	runq_yield();
	update_cache(1);
	z_swap_locked(ચાવી);
}

/* The scheduler's spinlock must be held */
સ્થિર શૂન્ય add_to_waitq_locked(સંરચના k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->base.pended_on == ખાલી);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_PORT_TRACING_FUNC(k_thread, sched_pend, thread);

	જો (wait_q != ખાલી) {
		thread->base.pended_on = wait_q;
		_priq_wait_add(&wait_q->waitq, thread);
	}
}

શૂન્ય z_sched_add_to_waitq_locked(સંરચના k_thread *thread, _wait_q_t *wait_q)
	ALIAS_OF(add_to_waitq_locked);

સ્થિર શૂન્ય pend_locked(સંરચના k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == ખાલી || sys_cache_is_mem_coherent(wait_q));
#endif /* CONFIG_KERNEL_COHERENCE */
	add_to_waitq_locked(thread, wait_q);
	z_add_thread_timeout(thread, timeout);
}

શૂન્ય z_pend_thread(સંરચના k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || is_thread_dummy(thread));
	Z_SCHED_SPINLOCK {
		pend_locked(thread, wait_q, timeout);
	}
}

શૂન્ય z_unpend_thread_no_timeout(સંરચના k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		જો (thread->base.pended_on != ખાલી) {
			unpend_thread_no_timeout(thread);
		}
	}
}

શૂન્ય z_sched_wake_thread_locked(સંરચના k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->base.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	જો (!killed) {
		/* The thread is not being killed */
		જો (thread->base.pended_on != ખાલી) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#ifdef CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
શૂન્ય z_thread_timeout(સંરચના _timeout *timeout)
{
	સંરચના k_thread *thread = CONTAINER_OF(timeout,
					       સંરચના k_thread, base.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		જો (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

પૂર્ણાંક z_pend_curr(સંરચના k_spinlock *lock, k_spinlock_ચાવી_t ચાવી,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	જો (arch_is_in_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#જો defined(CONFIG_TIMESLICING) && defined(CONFIG_SWAP_NONATOMIC)
	pending_current = _current;
#endif /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((કદ(સંરચના k_spinlock) == 0) || !z_is_sched_spinlock(lock));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(શૂન્ય) z_sched_spinlock_lock();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(lock);
	પાછા z_swap_locked(ચાવી);
}

સંરચના k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	સંરચના k_thread *thread = ખાલી;

	Z_SCHED_SPINLOCK {
		thread = _priq_wait_best(&wait_q->waitq);

		જો (thread != ખાલી) {
			unpend_thread_no_timeout(thread);
		}
	}

	પાછા thread;
}

શૂન્ય z_unpend_thread(સંરચના k_thread *thread)
{
	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	જો (thread->base.pended_on != ખાલી) {
		unpend_thread_no_timeout(thread);
	}
	જ્યારે (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(ચાવી);
		ચાવી = z_sched_spinlock_lock();
	}
	z_sched_spinlock_unlock(ચાવી);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_set(સંરચના k_thread *thread, પૂર્ણાંક prio)
{
	bool need_sched = false;
	પૂર્ણાંક old_prio = thread->base.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		જો (need_sched) {
			જો (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				dequeue_thread(thread);
				thread->base.prio = prio;
				queue_thread(thread);

				જો (old_prio > prio) {
					flag_ipi(ipi_mask_create(thread));
				}
			} નહીંતર {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->base.prio = prio;

				સંરચના _cpu *cpu;

				cpu = thread_active_elsewhere(thread);
				જો ((cpu != ખાલી) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_MASK(cpu->id));
				}
			}

			update_cache(1);
		} નહીંતર જો (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->base.prio = prio;
			_priq_wait_add(&wait_q->waitq, thread);
		} નહીંતર {
			thread->base.prio = prio;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, sched_priority_set, thread, prio);

	પાછા need_sched;
}

શૂન્ય z_reschedule(સંરચના k_spinlock *lock, k_spinlock_ચાવી_t ચાવી) ALIAS_OF(reschedule);

શૂન્ય z_reschedule_locked(k_spinlock_ચાવી_t ચાવી) ALIAS_OF(reschedule_locked);

શૂન્ય z_reschedule_irqlock(uint32_t ચાવી)
{
	જો (resched(ચાવી) && need_swap()) {
		z_swap_irqlock(ચાવી);
	} નહીંતર {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(ચાવી);
	}
}

સંરચના k_thread *z_swap_next_thread(શૂન્ય)
{
	સંરચના k_thread *ret = next_up();

	જો (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	પાછા ret;
}

#ifdef CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
સ્થિર અંતઃરેખ શૂન્ય set_current(સંરચના k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	જો (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
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
શૂન્ય *z_get_next_switch_handle(શૂન્ય *interrupted)
{
	z_check_ઢગલો_sentinel();

#ifdef CONFIG_SMP
	શૂન્ય *ret = ખાલી;

	Z_SCHED_SPINLOCK {
		સંરચના k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == ખાલી || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = next_up();

		z_sched_usage_switch(new_thread);

		જો (old_thread != new_thread) {
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
			જો (z_is_thread_queued(old_thread)) {
#ifdef CONFIG_SCHED_IPI_CASCADE
				જો ((new_thread->base.cpu_mask != -1) &&
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
		new_thread->switch_handle = ખાલી;

		/* Check for IPIs under the lock to avoid silently consuming a
		 * rescheduling IPI flagged by another CPU for ourselves.
		 */
		signal_pending_ipi();
	}
	પાછા ret;
#નહીંતર
	સંરચના k_thread *next = _kernel.ready_q.cache;

	z_sched_usage_switch(next);
	_current->switch_handle = interrupted;
	set_current(next);
	પાછા next->switch_handle;
#endif /* CONFIG_SMP */
}
#endif /* CONFIG_USE_SWITCH */

પૂર્ણાંક z_unpend_all(_wait_q_t *wait_q)
{
	પૂર્ણાંક need_sched = 0;
	સંરચના k_thread *thread;

	Z_SCHED_SPINLOCK {
		માટે (thread = z_waitq_head_locked(wait_q);
		     thread != ખાલી;
		     thread = z_waitq_head_locked(wait_q)) {
			unpend_thread_no_timeout(thread);
			/* Abort the timeout and ready the thread unconditionally. If
			 * the timeout handler is in flight on another CPU, the abort
			 * flags it superseded and z_thread_timeout() bails when it
			 * runs -- so it will NOT ready the thread, we must.
			 */
			(શૂન્ય)z_try_abort_thread_timeout(thread);
			ready_thread(thread);
			need_sched = 1;
		}
	}

	પાછા need_sched;
}

સ્થિર અંતઃરેખ શૂન્ય unpend_all(_wait_q_t *wait_q)
{
	સંરચના k_thread *thread;

	માટે (thread = z_waitq_head_locked(wait_q);
	     thread != ખાલી;
	     thread = z_waitq_head_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_કિંમત_set(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(શૂન્ય)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#ifdef CONFIG_THREAD_ABORT_HOOK
બાહ્ય શૂન્ય thread_abort_hook(સંરચના k_thread *thread);
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
સ્થિર ALWAYS_INLINE શૂન્ય halt_thread(સંરચના k_thread *thread, uint8_t new_state,
				      k_spinlock_ચાવી_t *ચાવી)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	જો ((thread->base.thread_state & new_state) == 0U) {
		thread->base.thread_state |= new_state;
		જો (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			dequeue_thread(thread);
		}

		જો (new_state == _THREAD_DEAD) {
			જો (thread->base.pended_on != ખાલી) {
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
			જો (ચાવી != ખાલી) {
				જ્યારે (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*ચાવી);
					*ચાવી = z_sched_spinlock_lock();
				}
			}
			unpend_all(&thread->join_queue);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			જો (thread == _current && arch_is_in_isr()) {
				dummify = true;
			}
		}
#ifdef CONFIG_SMP
		unpend_all(&thread->halt_queue);
#endif /* CONFIG_SMP */
		update_cache(1);

		જો (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			પાછા;
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
		k_object_uninit(thread->ઢગલો_obj);
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
		જો (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#ifdef CONFIG_USE_SWITCH
			_current->switch_handle = (શૂન્ય *)1;
#endif
#ifdef CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			જો (thread->base.swap_data ==
			    (શૂન્ય *)&z_spinlock_abort_sentinel) {
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

શૂન્ય z_thread_suspend_current(સંરચના k_thread *thread)
{
	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	dequeue_thread(thread);
	update_cache(1);
	z_swap_locked(ચાવી);
}
