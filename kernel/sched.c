/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/kernel.h>
#شامل <kspinlock.h>
#شامل <ksched.h>
#شامل <zephyr/spinlock.h>
#شامل <wait_q.h>
#شامل <kthread.h>
#شامل <priority_q.h>
#شامل <kswap.h>
#شامل <ipi.h>
#شامل <kernel_arch_func.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/drivers/timer/system_timer.h>
#شامل <stdbool.h>
#شامل <kernel_internal.h>
#شامل <zephyr/logging/log.h>
#شامل <zephyr/sys/atomic.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/timing/timing.h>
#شامل <zephyr/sys/util.h>
#شامل <metairq.h>
#شامل <run_q.h>
#شامل <timeslicing.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* pending_current is owned by timeslicing.c; sleep.c also accesses it */
#اگر defined(CONFIG_تبادلہ_NONATOMIC) && defined(CONFIG_TIMESLICING)
خارجی ساخت k_thread *pending_current;
#اگر_ختم

ساخت k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent ساخت k_thread _thread_dummy;

ساکن ALWAYS_INLINE خالی update_cache(صحیح preempt_ok);
ساکن ALWAYS_INLINE خالی halt_thread(ساخت k_thread *thread, uint8_t new_state,
				      k_spinlock_کلید_t *کلید);
ساکن خالی جوڑو_مقصد_waitq_locked(ساخت k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
ساکن اندرونی خالی clear_halting(ساخت k_thread *thread)
{
	اگر (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_عدد_CPUS > 1)) {
		barrier_dmem_fence_بھرا(); /* Other cpus spin on this locklessly! */
		thread->بنیاد.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

ساکن ALWAYS_INLINE ساخت k_thread *اگلا_اوپر(خالی)
{
#اگر_تعریف_شدہ CONFIG_SMP
	bool ipi_idle_ہدف_rebound = false;
	ساخت k_thread *ipi_idle_ہدف = ipi_idle_reserved_take();

	اگر (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, تہی);
	}
#اگر_ختم /* CONFIG_SMP */

	ساخت k_thread *thread = runq_بہترین();

	thread = metairq_preempt_recover(thread);

#اگر_غیر_تعریف_شدہ CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	واپس (thread != تہی) ? thread : _current_cpu->idle_thread;
#ورنہ
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
	bool فعال = z_is_thread_ready(_current);

	اگر (thread == تہی) {
		thread = _current_cpu->idle_thread;
	}

	اگر (فعال) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		اگر ((cmp > 0) || ((cmp == 0) && !_current_cpu->تبادلہ_ok)) {
			thread = _current;
		}

		اگر (!should_preempt(thread, _current_cpu->تبادلہ_ok)) {
			thread = _current;
		}
	}

	اگر (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		اگر (فعال && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			قطار_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	اگر (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		اگر (ipi_idle_ہدف != تہی &&
		    thread != ipi_idle_ہدف &&
		    z_is_thread_queued(ipi_idle_ہدف)) {
			ipi_idle_ہدف_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_ہدف);
		} ورنہ {
			ipi_idle_thread_unreserve(thread);
		}
		قطار_سے_نکالو_thread(thread);
	}

	_current_cpu->تبادلہ_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	اگر (!ipi_idle_ہدف_rebound &&
	    ipi_idle_ہدف != تہی && z_is_thread_queued(ipi_idle_ہدف)) {
		flag_ipi(ipi_نقاب_create(ipi_idle_ہدف));
	}
	واپس thread;
#اگر_ختم /* CONFIG_SMP */
}

خالی move_current_مقصد_انتہا_کا_prio_q(خالی)
{
	runq_yield();

	update_cache(1);
}

ساکن ALWAYS_INLINE خالی update_cache(صحیح preempt_ok)
{
#اگر_غیر_تعریف_شدہ CONFIG_SMP
	ساخت k_thread *thread = اگلا_اوپر();

	اگر (should_preempt(thread, preempt_ok)) {
#اگر_تعریف_شدہ CONFIG_TIMESLICING
		اگر (thread != _current) {
			z_وقت_قاش_reset(thread);
		}
#اگر_ختم /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} ورنہ {
		_kernel.ready_q.cache = _current;
	}

#ورنہ
	/* The way this works is that the CPU record keeps its
	 * "cooperative swapping is OK" flag until the next reschedule
	 * call or context switch.  It doesn't need to be tracked per
	 * thread because if the thread gets preempted for whatever
	 * reason the scheduler will make the same decision anyway.
	 */
	_current_cpu->تبادلہ_ok = preempt_ok;
#اگر_ختم /* CONFIG_SMP */
}

/**
 * Returns pointer to _cpu if the thread is currently running on
 * another CPU.
 */
ساکن ساخت _cpu *thread_فعال_elsewhere(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SMP
	صحیح thread_cpu_id = thread->بنیاد.cpu;
	ساخت _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_عدد_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	اگر ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		واپس thread_cpu;
	}

#اگر_ختم /* CONFIG_SMP */
	ARG_غیر_مستعمل(thread);
	واپس تہی;
}

ساکن اندرونی خالی ready_thread(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	اگر (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		قطار_thread(thread);
		update_cache(0);

		flag_ipi(ipi_نقاب_create(thread));
	}
}

خالی z_ready_thread(ساخت k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		اگر (thread_فعال_elsewhere(thread) == تہی) {
			ready_thread(thread);
		}
	}
}

خالی z_sched_ready_locked(ساخت k_thread *thread) ALIAS_کا(ready_thread);

ساکن خالی unready_thread(ساخت k_thread *thread)
{
	اگر (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		قطار_سے_نکالو_thread(thread);
	}
	update_cache(thread == _current);
}

/* This routine exists for benchmarking purposes. It is not used in
 * general production code.
 */
خالی z_unready_thread(ساخت k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		unready_thread(thread);
	}
}


خالی z_sched_unready_locked(ساخت k_thread *thread) ALIAS_کا(unready_thread);

/* This routine only used for testing purposes */
خالی z_yield_testing_only(خالی)
{
	Z_SCHED_SPINLOCK {
		move_current_مقصد_انتہا_کا_prio_q();
	}
}

/* Spins in ISR context, waiting for a thread known to be running on
 * another CPU to catch the IPI we sent and halt.  Note that we check
 * for ourselves being asynchronously halted first to prevent simple
 * deadlocks (but not complex ones involving cycles of 3+ threads!).
 * Acts to release the provided lock before returning.
 */
ساکن خالی thread_halt_spin(ساخت k_thread *thread, k_spinlock_کلید_t کلید)
{
	اگر (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &کلید);
	}
	z_sched_spinlock_unlock(کلید);
	جب (z_is_thread_halting(thread)) {
		غیرعلامتی صحیح k = arch_irq_تالا();

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
خالی z_thread_halt(ساخت k_thread *thread, k_spinlock_کلید_t کلید,
					bool terminate)
{
	_wait_q_t *wq = &thread->join_قطار;
#اگر_تعریف_شدہ CONFIG_SMP
	wq = terminate ? wq : &thread->halt_قطار;
#اگر_ختم

	z_metairq_preempted_clear(thread);

	/* If the target is a thread running on another CPU, flag and
	 * poke (note that we might spin to wait, so a true
	 * synchronous IPI is needed here, not deferred!), it will
	 * halt itself in the IPI.  Otherwise it's unscheduled, so we
	 * can clean it up directly.
	 */

	ساخت _cpu *cpu = thread_فعال_elsewhere(thread);

	اگر (cpu != تہی) {
		thread->بنیاد.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#اگر defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#اگر_تعریف_شدہ CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_نقاب(cpu->id));
#ورنہ
		arch_sched_نشر_ipi();
#اگر_ختم /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#اگر_ختم /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		اگر (arch_is_اندر_isr()) {
			thread_halt_spin(thread, کلید);
		} ورنہ  {
			جوڑو_مقصد_waitq_locked(_current, wq);
			z_تبادلہ_locked(کلید);
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
		جب (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} ورنہ {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &کلید);
		اگر ((thread == _current) && !arch_is_اندر_isr()) {
			اگر (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(کلید);
				k_panic();
				کلید = z_sched_spinlock_تالا();
			}
			z_تبادلہ_locked(کلید);
			__ASSERT(!terminate, "aborted _current back from dead");
		} ورنہ {
			z_sched_spinlock_unlock(کلید);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

ساکن اندرونی bool resched(uint32_t کلید)
{
#اگر_تعریف_شدہ CONFIG_SMP
	_current_cpu->تبادلہ_ok = 0;
#اگر_ختم /* CONFIG_SMP */

	واپس arch_irq_unlocked(کلید) && !arch_is_اندر_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
ساکن اندرونی bool need_تبادلہ(خالی)
{
	/* the SMP case will be handled in C based z_swap() */
#اگر_تعریف_شدہ CONFIG_SMP
	واپس true;
#ورنہ
	ساخت k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	واپس new_thread != _current;
#اگر_ختم /* CONFIG_SMP */
}

ساکن خالی reschedule(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید)
{
	اگر (resched(کلید.کلید) && need_تبادلہ()) {
		z_تبادلہ(تالا, کلید);
	} ورنہ {
		signal_pending_ipi();
		k_spin_unlock(تالا, کلید);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
ساکن خالی reschedule_locked(k_spinlock_کلید_t کلید)
{
	واپس reschedule(&_sched_spinlock, کلید);
}

خالی z_sched_تالا_reschedule(k_spinlock_کلید_t کلید)
{
	update_cache(0);
	reschedule_locked(کلید);
}

خالی z_sched_yield(خالی)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	runq_yield();
	update_cache(1);
	z_تبادلہ_locked(کلید);
}

/* The scheduler's spinlock must be held */
ساکن خالی جوڑو_مقصد_waitq_locked(ساخت k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->بنیاد.pended_on == تہی);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_پورٹ_TRACING_FUNC(k_thread, sched_pend, thread);

	اگر (wait_q != تہی) {
		thread->بنیاد.pended_on = wait_q;
		priq_wait_جوڑو(&wait_q->waitq, thread);
	}
}

خالی z_sched_جوڑو_مقصد_waitq_locked(ساخت k_thread *thread, _wait_q_t *wait_q)
	ALIAS_کا(جوڑو_مقصد_waitq_locked);

ساکن خالی pend_locked(ساخت k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == تہی || sys_cache_is_mem_coherent(wait_q));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */
	جوڑو_مقصد_waitq_locked(thread, wait_q);
	z_جوڑو_thread_timeout(thread, timeout);
}

خالی z_pend_thread(ساخت k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout)
{
	__ASSERT_NO_MSG(thread == _current || is_thread_dummy(thread));
	Z_SCHED_SPINLOCK {
		pend_locked(thread, wait_q, timeout);
	}
}

خالی z_unpend_thread_no_timeout(ساخت k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		اگر (thread->بنیاد.pended_on != تہی) {
			unpend_thread_no_timeout(thread);
		}
	}
}

خالی z_sched_wake_thread_locked(ساخت k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->بنیاد.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	اگر (!killed) {
		/* The thread is not being killed */
		اگر (thread->بنیاد.pended_on != تہی) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
خالی z_thread_timeout(ساخت _timeout *timeout)
{
	ساخت k_thread *thread = CONTAINER_کا(timeout,
					       ساخت k_thread, بنیاد.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		اگر (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */

صحیح z_pend_curr(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	اگر (arch_is_اندر_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#اگر defined(CONFIG_TIMESLICING) && defined(CONFIG_تبادلہ_NONATOMIC)
	pending_current = _current;
#اگر_ختم /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((حجم(ساخت k_spinlock) == 0) || !z_is_sched_spinlock(تالا));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(خالی) z_sched_spinlock_تالا();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(تالا);
	واپس z_تبادلہ_locked(کلید);
}

ساخت k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	ساخت k_thread *thread = تہی;

	Z_SCHED_SPINLOCK {
		thread = priq_wait_بہترین(&wait_q->waitq);

		اگر (thread != تہی) {
			unpend_thread_no_timeout(thread);
		}
	}

	واپس thread;
}

خالی z_unpend_thread(ساخت k_thread *thread)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	اگر (thread->بنیاد.pended_on != تہی) {
		unpend_thread_no_timeout(thread);
	}
	جب (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(کلید);
		کلید = z_sched_spinlock_تالا();
	}
	z_sched_spinlock_unlock(کلید);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_سیٹ(ساخت k_thread *thread, صحیح prio)
{
	bool need_sched = false;
	صحیح old_prio = thread->بنیاد.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		اگر (need_sched) {
			اگر (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				قطار_سے_نکالو_thread(thread);
				thread->بنیاد.prio = prio;
				قطار_thread(thread);

				اگر (old_prio > prio) {
					flag_ipi(ipi_نقاب_create(thread));
				}
			} ورنہ {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->بنیاد.prio = prio;

				ساخت _cpu *cpu;

				cpu = thread_فعال_elsewhere(thread);
				اگر ((cpu != تہی) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_نقاب(cpu->id));
				}
			}

			update_cache(1);
		} ورنہ اگر (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->بنیاد.prio = prio;
			priq_wait_جوڑو(&wait_q->waitq, thread);
		} ورنہ {
			thread->بنیاد.prio = prio;
		}
	}

	SYS_پورٹ_TRACING_OBJ_FUNC(k_thread, sched_ترجیح_سیٹ, thread, prio);

	واپس need_sched;
}

خالی z_reschedule(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید) ALIAS_کا(reschedule);

خالی z_reschedule_locked(k_spinlock_کلید_t کلید) ALIAS_کا(reschedule_locked);

خالی z_reschedule_irqlock(uint32_t کلید)
{
	اگر (resched(کلید) && need_تبادلہ()) {
		z_تبادلہ_irqlock(کلید);
	} ورنہ {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(کلید);
	}
}

ساخت k_thread *z_تبادلہ_اگلا_thread(خالی)
{
	ساخت k_thread *ret = اگلا_اوپر();

	اگر (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
ساکن اندرونی خالی سیٹ_current(ساخت k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	اگر (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
		z_thread_mark_switched_برآمد();
	}
	z_current_thread_سیٹ(new_thread);
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
خالی *z_get_اگلا_switch_handle(خالی *interrupted)
{
	z_check_ڈھیر_sentinel();

#اگر_تعریف_شدہ CONFIG_SMP
	خالی *ret = تہی;

	Z_SCHED_SPINLOCK {
		ساخت k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == تہی || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = اگلا_اوپر();

		z_sched_usage_switch(new_thread);

		اگر (old_thread != new_thread) {
			uint8_t  cpu_id;

			z_sched_switch_spin(new_thread);
			arch_cohere_stacks(old_thread, interrupted, new_thread);

			_current_cpu->تبادلہ_ok = 0;
			cpu_id = arch_curr_cpu()->id;
			new_thread->بنیاد.cpu = cpu_id;
			سیٹ_current(new_thread);

#اگر_تعریف_شدہ CONFIG_TIMESLICING
			z_وقت_قاش_reset(new_thread);
#اگر_ختم /* CONFIG_TIMESLICING */

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
			اگر (z_is_thread_queued(old_thread)) {
#اگر_تعریف_شدہ CONFIG_SCHED_IPI_CASCADE
				اگر ((new_thread->بنیاد.cpu_نقاب != -1) &&
				    (old_thread->بنیاد.cpu_نقاب != ثنائی_ہندسہ(cpu_id))) {
					flag_ipi(ipi_نقاب_create(old_thread));
				}
#اگر_ختم
				runq_جوڑو(old_thread);
			}
		}
		old_thread->switch_handle = interrupted;
		ret = new_thread->switch_handle;
		/* Active threads MUST have a null here */
		new_thread->switch_handle = تہی;

		/* Check for IPIs under the lock to avoid silently consuming a
		 * rescheduling IPI flagged by another CPU for ourselves.
		 */
		signal_pending_ipi();
	}
	واپس ret;
#ورنہ
	ساخت k_thread *اگلا = _kernel.ready_q.cache;

	z_sched_usage_switch(اگلا);
	_current->switch_handle = interrupted;
	سیٹ_current(اگلا);
	واپس اگلا->switch_handle;
#اگر_ختم /* CONFIG_SMP */
}
#اگر_ختم /* CONFIG_USE_SWITCH */

صحیح z_unpend_all(_wait_q_t *wait_q)
{
	صحیح need_sched = 0;
	ساخت k_thread *thread;

	Z_SCHED_SPINLOCK {
		برائے (thread = z_waitq_سر_locked(wait_q);
		     thread != تہی;
		     thread = z_waitq_سر_locked(wait_q)) {
			unpend_thread_no_timeout(thread);
			/* Abort the timeout and ready the thread unconditionally. If
			 * the timeout handler is in flight on another CPU, the abort
			 * flags it superseded and z_thread_timeout() bails when it
			 * runs -- so it will NOT ready the thread, we must.
			 */
			(خالی)z_try_abort_thread_timeout(thread);
			ready_thread(thread);
			need_sched = 1;
		}
	}

	واپس need_sched;
}

ساکن اندرونی خالی unpend_all(_wait_q_t *wait_q)
{
	ساخت k_thread *thread;

	برائے (thread = z_waitq_سر_locked(wait_q);
	     thread != تہی;
	     thread = z_waitq_سر_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_قدر_سیٹ(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(خالی)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#اگر_تعریف_شدہ CONFIG_THREAD_ABORT_HOOK
خارجی خالی thread_abort_hook(ساخت k_thread *thread);
#اگر_ختم /* CONFIG_THREAD_ABORT_HOOK */

/**
 * @brief Dequeues the specified thread
 *
 * Dequeues the specified thread and move it into the specified new state.
 *
 * @param thread Identify the thread to halt
 * @param new_state New thread state (_THREAD_DEAD or _THREAD_SUSPENDED)
 * @param key Pointer to the scheduler spinlock key held by the caller
 */
ساکن ALWAYS_INLINE خالی halt_thread(ساخت k_thread *thread, uint8_t new_state,
				      k_spinlock_کلید_t *کلید)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	اگر ((thread->بنیاد.thread_state & new_state) == 0U) {
		thread->بنیاد.thread_state |= new_state;
		اگر (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			قطار_سے_نکالو_thread(thread);
		}

		اگر (new_state == _THREAD_DEAD) {
			اگر (thread->بنیاد.pended_on != تہی) {
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
			اگر (کلید != تہی) {
				جب (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*کلید);
					*کلید = z_sched_spinlock_تالا();
				}
			}
			unpend_all(&thread->join_قطار);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			اگر (thread == _current && arch_is_اندر_isr()) {
				dummify = true;
			}
		}
#اگر_تعریف_شدہ CONFIG_SMP
		unpend_all(&thread->halt_قطار);
#اگر_ختم /* CONFIG_SMP */
		update_cache(1);

		اگر (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			واپس;
		}

		arch_coprocessors_disable(thread);

		SYS_پورٹ_TRACING_FUNC(k_thread, sched_abort, thread);

		z_thread_monitor_exit(thread);
#اگر_تعریف_شدہ CONFIG_THREAD_ABORT_HOOK
		thread_abort_hook(thread);
#اگر_ختم /* CONFIG_THREAD_ABORT_HOOK */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_THREAD
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
		k_obj_core_stats_deregister(K_OBJ_CORE(thread));
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_THREAD */
		k_obj_core_unlink(K_OBJ_CORE(thread));
#اگر_ختم /* CONFIG_OBJ_CORE_THREAD */

#اگر_تعریف_شدہ CONFIG_USERSPACE
		z_mem_domain_exit_thread(thread);
		k_thread_perms_all_clear(thread);
		k_object_uninit(thread->ڈھیر_obj);
		k_object_uninit(thread);
#اگر_ختم /* CONFIG_USERSPACE */

#اگر_تعریف_شدہ CONFIG_THREAD_ABORT_NEED_CLEANUP
		k_thread_abort_cleanup(thread);
#اگر_ختم /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

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
		اگر (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#اگر_تعریف_شدہ CONFIG_USE_SWITCH
			_current->switch_handle = (خالی *)1;
#اگر_ختم
#اگر_تعریف_شدہ CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			اگر (thread->بنیاد.تبادلہ_ڈیٹا ==
			    (خالی *)&z_spinlock_abort_sentinel) {
				z_spin_validate_reset(true);
			}
#اگر_ختم
			z_dummy_thread_init(&_thread_dummy);

		}

		/* Finally update the halting thread state, on which
		 * other CPUs might be spinning (see
		 * thread_halt_spin()).
		 */
		clear_halting(thread);
	}
}

خالی z_thread_suspend_current(ساخت k_thread *thread)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	قطار_سے_نکالو_thread(thread);
	update_cache(1);
	z_تبادلہ_locked(کلید);
}
