/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ضمن <zephyr/kernel.h>
#ضمن <kspinlock.h>
#ضمن <ksched.h>
#ضمن <zephyr/spinlock.h>
#ضمن <wait_q.h>
#ضمن <kthread.h>
#ضمن <priority_q.h>
#ضمن <kswap.h>
#ضمن <ipi.h>
#ضمن <kernel_arch_func.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/drivers/timer/system_timer.h>
#ضمن <stdbool.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/logging/log.h>
#ضمن <zephyr/sys/atomic.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/timing/timing.h>
#ضمن <zephyr/sys/util.h>
#ضمن <metairq.h>
#ضمن <run_q.h>
#ضمن <timeslicing.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* pending_current is owned by timeslicing.c; sleep.c also accesses it */
#אם defined(CONFIG_SWAP_NONATOMIC) && defined(CONFIG_TIMESLICING)
خارجي بنية k_thread *pending_current;
#endif

بنية k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent بنية k_thread _thread_dummy;

ساكن ALWAYS_INLINE فراغ update_cache(صحيح preempt_ok);
ساكن ALWAYS_INLINE فراغ halt_thread(بنية k_thread *thread, uint8_t new_state,
				      k_spinlock_key_t *key);
ساكن فراغ add_to_waitq_locked(بنية k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
ساكن مضمن فراغ clear_halting(بنية k_thread *thread)
{
	אם (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_NUM_CPUS > 1)) {
		barrier_dmem_fence_full(); /* Other cpus spin on this locklessly! */
		thread->base.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

ساكن ALWAYS_INLINE بنية k_thread *next_up(فراغ)
{
#ifdef CONFIG_SMP
	bool ipi_idle_target_rebound = false;
	بنية k_thread *ipi_idle_target = ipi_idle_reserved_take();

	אם (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, عدم);
	}
#endif /* CONFIG_SMP */

	بنية k_thread *thread = runq_best();

	thread = metairq_preempt_recover(thread);

#ifndef CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	החזר (thread != عدم) ? thread : _current_cpu->idle_thread;
#אחרת
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

	אם (thread == عدم) {
		thread = _current_cpu->idle_thread;
	}

	אם (active) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		אם ((cmp > 0) || ((cmp == 0) && !_current_cpu->swap_ok)) {
			thread = _current;
		}

		אם (!should_preempt(thread, _current_cpu->swap_ok)) {
			thread = _current;
		}
	}

	אם (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		אם (active && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			queue_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	אם (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		אם (ipi_idle_target != عدم &&
		    thread != ipi_idle_target &&
		    z_is_thread_queued(ipi_idle_target)) {
			ipi_idle_target_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_target);
		} אחרת {
			ipi_idle_thread_unreserve(thread);
		}
		dequeue_thread(thread);
	}

	_current_cpu->swap_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	אם (!ipi_idle_target_rebound &&
	    ipi_idle_target != عدم && z_is_thread_queued(ipi_idle_target)) {
		flag_ipi(ipi_mask_create(ipi_idle_target));
	}
	החזר thread;
#endif /* CONFIG_SMP */
}

فراغ move_current_to_end_of_prio_q(فراغ)
{
	runq_yield();

	update_cache(1);
}

ساكن ALWAYS_INLINE فراغ update_cache(صحيح preempt_ok)
{
#ifndef CONFIG_SMP
	بنية k_thread *thread = next_up();

	אם (should_preempt(thread, preempt_ok)) {
#ifdef CONFIG_TIMESLICING
		אם (thread != _current) {
			z_time_slice_reset(thread);
		}
#endif /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} אחרת {
		_kernel.ready_q.cache = _current;
	}

#אחרת
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
ساكن بنية _cpu *thread_active_elsewhere(بنية k_thread *thread)
{
#ifdef CONFIG_SMP
	صحيح thread_cpu_id = thread->base.cpu;
	بنية _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_num_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	אם ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		החזר thread_cpu;
	}

#endif /* CONFIG_SMP */
	ARG_UNUSED(thread);
	החזר عدم;
}

ساكن مضمن فراغ ready_thread(بنية k_thread *thread)
{
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#endif /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	אם (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_PORT_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		queue_thread(thread);
		update_cache(0);

		flag_ipi(ipi_mask_create(thread));
	}
}

فراغ z_ready_thread(بنية k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		אם (thread_active_elsewhere(thread) == عدم) {
			ready_thread(thread);
		}
	}
}

فراغ z_sched_ready_locked(بنية k_thread *thread) ALIAS_OF(ready_thread);

ساكن فراغ unready_thread(بنية k_thread *thread)
{
	אם (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		dequeue_thread(thread);
	}
	update_cache(thread == _current);
}

/* This routine exists for benchmarking purposes. It is not used in
 * general production code.
 */
فراغ z_unready_thread(بنية k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		unready_thread(thread);
	}
}


فراغ z_sched_unready_locked(بنية k_thread *thread) ALIAS_OF(unready_thread);

/* This routine only used for testing purposes */
فراغ z_yield_testing_only(فراغ)
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
ساكن فراغ thread_halt_spin(بنية k_thread *thread, k_spinlock_key_t key)
{
	אם (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &key);
	}
	z_sched_spinlock_unlock(key);
	כלעוד (z_is_thread_halting(thread)) {
		unsigned صحيح k = arch_irq_lock();

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
فراغ z_thread_halt(بنية k_thread *thread, k_spinlock_key_t key,
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

	بنية _cpu *cpu = thread_active_elsewhere(thread);

	אם (cpu != عدم) {
		thread->base.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#אם defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_MASK(cpu->id));
#אחרת
		arch_sched_broadcast_ipi();
#endif /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#endif /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		אם (arch_is_in_isr()) {
			thread_halt_spin(thread, key);
		} אחרת  {
			add_to_waitq_locked(_current, wq);
			z_swap_locked(key);
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
		כלעוד (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} אחרת {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &key);
		אם ((thread == _current) && !arch_is_in_isr()) {
			אם (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(key);
				k_panic();
				key = z_sched_spinlock_lock();
			}
			z_swap_locked(key);
			__ASSERT(!terminate, "aborted _current back from dead");
		} אחרת {
			z_sched_spinlock_unlock(key);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

ساكن مضمن bool resched(uint32_t key)
{
#ifdef CONFIG_SMP
	_current_cpu->swap_ok = 0;
#endif /* CONFIG_SMP */

	החזר arch_irq_unlocked(key) && !arch_is_in_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
ساكن مضمن bool need_swap(فراغ)
{
	/* the SMP case will be handled in C based z_swap() */
#ifdef CONFIG_SMP
	החזר true;
#אחרת
	بنية k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	החזר new_thread != _current;
#endif /* CONFIG_SMP */
}

ساكن فراغ reschedule(بنية k_spinlock *lock, k_spinlock_key_t key)
{
	אם (resched(key.key) && need_swap()) {
		z_swap(lock, key);
	} אחרת {
		signal_pending_ipi();
		k_spin_unlock(lock, key);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
ساكن فراغ reschedule_locked(k_spinlock_key_t key)
{
	החזר reschedule(&_sched_spinlock, key);
}

فراغ z_sched_lock_reschedule(k_spinlock_key_t key)
{
	update_cache(0);
	reschedule_locked(key);
}

فراغ z_sched_yield(فراغ)
{
	k_spinlock_key_t key = z_sched_spinlock_lock();

	runq_yield();
	update_cache(1);
	z_swap_locked(key);
}

/* The scheduler's spinlock must be held */
ساكن فراغ add_to_waitq_locked(بنية k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->base.pended_on == عدم);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_PORT_TRACING_FUNC(k_thread, sched_pend, thread);

	אם (wait_q != عدم) {
		thread->base.pended_on = wait_q;
		_priq_wait_add(&wait_q->waitq, thread);
	}
}

فراغ z_sched_add_to_waitq_locked(بنية k_thread *thread, _wait_q_t *wait_q)
	ALIAS_OF(add_to_waitq_locked);

ساكن فراغ pend_locked(بنية k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == عدم || sys_cache_is_mem_coherent(wait_q));
#endif /* CONFIG_KERNEL_COHERENCE */
	add_to_waitq_locked(thread, wait_q);
	z_add_thread_timeout(thread, timeout);
}

فراغ z_pend_thread(بنية k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || is_thread_dummy(thread));
	Z_SCHED_SPINLOCK {
		pend_locked(thread, wait_q, timeout);
	}
}

فراغ z_unpend_thread_no_timeout(بنية k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		אם (thread->base.pended_on != عدم) {
			unpend_thread_no_timeout(thread);
		}
	}
}

فراغ z_sched_wake_thread_locked(بنية k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->base.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	אם (!killed) {
		/* The thread is not being killed */
		אם (thread->base.pended_on != عدم) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#ifdef CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
فراغ z_thread_timeout(بنية _timeout *timeout)
{
	بنية k_thread *thread = CONTAINER_OF(timeout,
					       بنية k_thread, base.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		אם (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#endif /* CONFIG_SYS_CLOCK_EXISTS */

صحيح z_pend_curr(بنية k_spinlock *lock, k_spinlock_key_t key,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	אם (arch_is_in_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#אם defined(CONFIG_TIMESLICING) && defined(CONFIG_SWAP_NONATOMIC)
	pending_current = _current;
#endif /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((حجم(بنية k_spinlock) == 0) || !z_is_sched_spinlock(lock));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(فراغ) z_sched_spinlock_lock();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(lock);
	החזר z_swap_locked(key);
}

بنية k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	بنية k_thread *thread = عدم;

	Z_SCHED_SPINLOCK {
		thread = _priq_wait_best(&wait_q->waitq);

		אם (thread != عدم) {
			unpend_thread_no_timeout(thread);
		}
	}

	החזר thread;
}

فراغ z_unpend_thread(بنية k_thread *thread)
{
	k_spinlock_key_t key = z_sched_spinlock_lock();

	אם (thread->base.pended_on != عدم) {
		unpend_thread_no_timeout(thread);
	}
	כלעוד (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(key);
		key = z_sched_spinlock_lock();
	}
	z_sched_spinlock_unlock(key);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_set(بنية k_thread *thread, صحيح prio)
{
	bool need_sched = false;
	صحيح old_prio = thread->base.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		אם (need_sched) {
			אם (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				dequeue_thread(thread);
				thread->base.prio = prio;
				queue_thread(thread);

				אם (old_prio > prio) {
					flag_ipi(ipi_mask_create(thread));
				}
			} אחרת {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->base.prio = prio;

				بنية _cpu *cpu;

				cpu = thread_active_elsewhere(thread);
				אם ((cpu != عدم) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_MASK(cpu->id));
				}
			}

			update_cache(1);
		} אחרת אם (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->base.prio = prio;
			_priq_wait_add(&wait_q->waitq, thread);
		} אחרת {
			thread->base.prio = prio;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, sched_priority_set, thread, prio);

	החזר need_sched;
}

فراغ z_reschedule(بنية k_spinlock *lock, k_spinlock_key_t key) ALIAS_OF(reschedule);

فراغ z_reschedule_locked(k_spinlock_key_t key) ALIAS_OF(reschedule_locked);

فراغ z_reschedule_irqlock(uint32_t key)
{
	אם (resched(key) && need_swap()) {
		z_swap_irqlock(key);
	} אחרת {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(key);
	}
}

بنية k_thread *z_swap_next_thread(فراغ)
{
	بنية k_thread *ret = next_up();

	אם (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	החזר ret;
}

#ifdef CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
ساكن مضمن فراغ set_current(بنية k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	אם (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
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
فراغ *z_get_next_switch_handle(فراغ *interrupted)
{
	z_check_stack_sentinel();

#ifdef CONFIG_SMP
	فراغ *ret = عدم;

	Z_SCHED_SPINLOCK {
		بنية k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == عدم || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = next_up();

		z_sched_usage_switch(new_thread);

		אם (old_thread != new_thread) {
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
			אם (z_is_thread_queued(old_thread)) {
#ifdef CONFIG_SCHED_IPI_CASCADE
				אם ((new_thread->base.cpu_mask != -1) &&
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
		new_thread->switch_handle = عدم;

		/* Check for IPIs under the lock to avoid silently consuming a
		 * rescheduling IPI flagged by another CPU for ourselves.
		 */
		signal_pending_ipi();
	}
	החזר ret;
#אחרת
	بنية k_thread *next = _kernel.ready_q.cache;

	z_sched_usage_switch(next);
	_current->switch_handle = interrupted;
	set_current(next);
	החזר next->switch_handle;
#endif /* CONFIG_SMP */
}
#endif /* CONFIG_USE_SWITCH */

صحيح z_unpend_all(_wait_q_t *wait_q)
{
	صحيح need_sched = 0;
	بنية k_thread *thread;

	Z_SCHED_SPINLOCK {
		עבור (thread = z_waitq_head_locked(wait_q);
		     thread != عدم;
		     thread = z_waitq_head_locked(wait_q)) {
			unpend_thread_no_timeout(thread);
			/* Abort the timeout and ready the thread unconditionally. If
			 * the timeout handler is in flight on another CPU, the abort
			 * flags it superseded and z_thread_timeout() bails when it
			 * runs -- so it will NOT ready the thread, we must.
			 */
			(فراغ)z_try_abort_thread_timeout(thread);
			ready_thread(thread);
			need_sched = 1;
		}
	}

	החזר need_sched;
}

ساكن مضمن فراغ unpend_all(_wait_q_t *wait_q)
{
	بنية k_thread *thread;

	עבור (thread = z_waitq_head_locked(wait_q);
	     thread != عدم;
	     thread = z_waitq_head_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_value_set(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(فراغ)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#ifdef CONFIG_THREAD_ABORT_HOOK
خارجي فراغ thread_abort_hook(بنية k_thread *thread);
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
ساكن ALWAYS_INLINE فراغ halt_thread(بنية k_thread *thread, uint8_t new_state,
				      k_spinlock_key_t *key)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	אם ((thread->base.thread_state & new_state) == 0U) {
		thread->base.thread_state |= new_state;
		אם (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			dequeue_thread(thread);
		}

		אם (new_state == _THREAD_DEAD) {
			אם (thread->base.pended_on != عدم) {
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
			אם (key != عدم) {
				כלעוד (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*key);
					*key = z_sched_spinlock_lock();
				}
			}
			unpend_all(&thread->join_queue);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			אם (thread == _current && arch_is_in_isr()) {
				dummify = true;
			}
		}
#ifdef CONFIG_SMP
		unpend_all(&thread->halt_queue);
#endif /* CONFIG_SMP */
		update_cache(1);

		אם (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			החזר;
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
		k_object_uninit(thread->stack_obj);
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
		אם (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#ifdef CONFIG_USE_SWITCH
			_current->switch_handle = (فراغ *)1;
#endif
#ifdef CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			אם (thread->base.swap_data ==
			    (فراغ *)&z_spinlock_abort_sentinel) {
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

فراغ z_thread_suspend_current(بنية k_thread *thread)
{
	k_spinlock_key_t key = z_sched_spinlock_lock();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	dequeue_thread(thread);
	update_cache(1);
	z_swap_locked(key);
}
