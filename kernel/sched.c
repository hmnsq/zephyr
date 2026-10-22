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
#إذا defined(CONFIG_جابجا_NONATOMIC) && defined(CONFIG_TIMESLICING)
خارجي بنية k_thread *pending_current;
#پایان_اگر

بنية k_spinlock _sched_spinlock;  /* The scheduler's spinlock */

/* Storage to "complete" the context switch from an invalid/incomplete thread
 * context (ex: exiting an ISR that aborted _current)
 */
__incoherent بنية k_thread _thread_dummy;

ساكن ALWAYS_INLINE فراغ update_cache(صحيح preempt_ok);
ساكن ALWAYS_INLINE فراغ halt_thread(بنية k_thread *thread, uint8_t new_state,
				      k_spinlock_کلید_t *کلید);
ساكن فراغ جمع_کن_مقصد_waitq_locked(بنية k_thread *thread, _wait_q_t *wait_q);

/* Clear the halting bits (_THREAD_ABORTING and _THREAD_SUSPENDING) */
ساكن مضمن فراغ clear_halting(بنية k_thread *thread)
{
	إذا (IS_ENABLED(CONFIG_SMP) && (CONFIG_MP_MAX_رقم_CPUS > 1)) {
		barrier_dmem_fence_پر(); /* Other cpus spin on this locklessly! */
		thread->پایه.thread_state &= ~(_THREAD_ABORTING | _THREAD_SUSPENDING);
	}
}

ساكن ALWAYS_INLINE بنية k_thread *بعدی_به_بالا(فراغ)
{
#اگر_تعریف_شده CONFIG_SMP
	bool ipi_idle_هدف_rebound = false;
	بنية k_thread *ipi_idle_هدف = ipi_idle_reserved_take();

	إذا (z_is_thread_halting(_current)) {
		/* NULL key: scheduler context, no retry possible. _current
		 * cannot have an in-flight timeout (a running thread's
		 * timeout already fired and its handler returned), so the
		 * abort inside halt_thread won't see -EAGAIN.
		 */
		halt_thread(_current, z_is_thread_aborting(_current) ?
				      _THREAD_DEAD : _THREAD_SUSPENDED, عدم);
	}
#پایان_اگر /* CONFIG_SMP */

	بنية k_thread *thread = runq_بهترین();

	thread = metairq_preempt_recover(thread);

#اگر_تعریف_نشده CONFIG_SMP
	/* In uniprocessor mode, we can leave the current thread in
	 * the queue (actually we have to, otherwise the assembly
	 * context switch code for all architectures would be
	 * responsible for putting it back in z_swap and ISR return!),
	 * which makes this choice simple.
	 */
	أرجع (thread != عدم) ? thread : _current_cpu->idle_thread;
#وإلا
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

	إذا (thread == عدم) {
		thread = _current_cpu->idle_thread;
	}

	إذا (فعال) {
		int32_t cmp = z_sched_prio_cmp(_current, thread);

		/* Ties only switch if state says we yielded */
		إذا ((cmp > 0) || ((cmp == 0) && !_current_cpu->جابجا_ok)) {
			thread = _current;
		}

		إذا (!should_preempt(thread, _current_cpu->جابجا_ok)) {
			thread = _current;
		}
	}

	إذا (thread != _current) {
		update_metairq_preempt(thread);
		/*
		 * Put _current back into the queue unless it is ..
		 * 1. not active (i.e., blocked, suspended, dead), or
		 * 2. already queued, or
		 * 3. the idle thread, or
		 * 4. preempted by a MetaIRQ thread
		 */
		إذا (فعال && !queued && !z_is_idle_thread_object(_current)
		    && metairq_current_requeue_allowed()) {
			صف_thread(_current);
		}
	}

	/* Take the new _current out of the queue */
	إذا (z_is_thread_queued(thread)) {
		/* Remove or transfer the selected thread's idle CPU coverage. */
		إذا (ipi_idle_هدف != عدم &&
		    thread != ipi_idle_هدف &&
		    z_is_thread_queued(ipi_idle_هدف)) {
			ipi_idle_هدف_rebound =
				ipi_idle_thread_rebind(thread, ipi_idle_هدف);
		} وإلا {
			ipi_idle_thread_unreserve(thread);
		}
		خارج_از_صف_thread(thread);
	}

	_current_cpu->جابجا_ok = false;

	/* If this CPU consumed a different thread, preserve coverage for the
	 * runnable thread covered by this CPU's reservation.
	 */
	إذا (!ipi_idle_هدف_rebound &&
	    ipi_idle_هدف != عدم && z_is_thread_queued(ipi_idle_هدف)) {
		flag_ipi(ipi_ماسک_create(ipi_idle_هدف));
	}
	أرجع thread;
#پایان_اگر /* CONFIG_SMP */
}

فراغ move_current_مقصد_انتها_از_prio_q(فراغ)
{
	runq_yield();

	update_cache(1);
}

ساكن ALWAYS_INLINE فراغ update_cache(صحيح preempt_ok)
{
#اگر_تعریف_نشده CONFIG_SMP
	بنية k_thread *thread = بعدی_به_بالا();

	إذا (should_preempt(thread, preempt_ok)) {
#اگر_تعریف_شده CONFIG_TIMESLICING
		إذا (thread != _current) {
			z_زمان_برش_reset(thread);
		}
#پایان_اگر /* CONFIG_TIMESLICING */
		update_metairq_preempt(thread);
		_kernel.ready_q.cache = thread;
	} وإلا {
		_kernel.ready_q.cache = _current;
	}

#وإلا
	/* The way this works is that the CPU record keeps its
	 * "cooperative swapping is OK" flag until the next reschedule
	 * call or context switch.  It doesn't need to be tracked per
	 * thread because if the thread gets preempted for whatever
	 * reason the scheduler will make the same decision anyway.
	 */
	_current_cpu->جابجا_ok = preempt_ok;
#پایان_اگر /* CONFIG_SMP */
}

/**
 * Returns pointer to _cpu if the thread is currently running on
 * another CPU.
 */
ساكن بنية _cpu *thread_فعال_elsewhere(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_SMP
	صحيح thread_cpu_id = thread->پایه.cpu;
	بنية _cpu *thread_cpu;

	__ASSERT_NO_MSG((thread_cpu_id >= 0) &&
			(thread_cpu_id < arch_رقم_cpus()));

	thread_cpu = &_kernel.cpus[thread_cpu_id];
	إذا ((thread_cpu->current == thread) && (thread_cpu != _current_cpu)) {
		أرجع thread_cpu;
	}

#پایان_اگر /* CONFIG_SMP */
	ARG_استفادهنشده(thread);
	أرجع عدم;
}

ساكن مضمن فراغ ready_thread(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */

	/* If thread is queued already, do not try and added it to the
	 * run queue again
	 */
	إذا (!z_is_thread_queued(thread) && z_is_thread_ready(thread)) {
		SYS_پورت_TRACING_OBJ_FUNC(k_thread, sched_ready, thread);

		صف_thread(thread);
		update_cache(0);

		flag_ipi(ipi_ماسک_create(thread));
	}
}

فراغ z_ready_thread(بنية k_thread *thread)
{
	Z_SCHED_SPINLOCK {
		إذا (thread_فعال_elsewhere(thread) == عدم) {
			ready_thread(thread);
		}
	}
}

فراغ z_sched_ready_locked(بنية k_thread *thread) ALIAS_از(ready_thread);

ساكن فراغ unready_thread(بنية k_thread *thread)
{
	إذا (z_is_thread_queued(thread)) {
		/* Clear idle CPU coverage before removing the thread from the run queue. */
		ipi_idle_thread_unreserve(thread);
		خارج_از_صف_thread(thread);
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


فراغ z_sched_unready_locked(بنية k_thread *thread) ALIAS_از(unready_thread);

/* This routine only used for testing purposes */
فراغ z_yield_testing_only(فراغ)
{
	Z_SCHED_SPINLOCK {
		move_current_مقصد_انتها_از_prio_q();
	}
}

/* Spins in ISR context, waiting for a thread known to be running on
 * another CPU to catch the IPI we sent and halt.  Note that we check
 * for ourselves being asynchronously halted first to prevent simple
 * deadlocks (but not complex ones involving cycles of 3+ threads!).
 * Acts to release the provided lock before returning.
 */
ساكن فراغ thread_halt_spin(بنية k_thread *thread, k_spinlock_کلید_t کلید)
{
	إذا (z_is_thread_halting(_current)) {
		halt_thread(_current,
			    z_is_thread_aborting(_current) ? _THREAD_DEAD : _THREAD_SUSPENDED,
			    &کلید);
	}
	z_sched_spinlock_unlock(کلید);
	طالما (z_is_thread_halting(thread)) {
		بیعلامت صحيح k = arch_irq_قفل();

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
فراغ z_thread_halt(بنية k_thread *thread, k_spinlock_کلید_t کلید,
					bool terminate)
{
	_wait_q_t *wq = &thread->join_صف;
#اگر_تعریف_شده CONFIG_SMP
	wq = terminate ? wq : &thread->halt_صف;
#پایان_اگر

	z_metairq_preempted_clear(thread);

	/* If the target is a thread running on another CPU, flag and
	 * poke (note that we might spin to wait, so a true
	 * synchronous IPI is needed here, not deferred!), it will
	 * halt itself in the IPI.  Otherwise it's unscheduled, so we
	 * can clean it up directly.
	 */

	بنية _cpu *cpu = thread_فعال_elsewhere(thread);

	إذا (cpu != عدم) {
		thread->پایه.thread_state |= (terminate ? _THREAD_ABORTING
					      : _THREAD_SUSPENDING);
#إذا defined(CONFIG_SMP) && defined(CONFIG_SCHED_IPI_SUPPORTED)
#اگر_تعریف_شده CONFIG_ARCH_HAS_DIRECTED_IPIS
		arch_sched_directed_ipi(IPI_CPU_ماسک(cpu->id));
#وإلا
		arch_sched_پخش_ipi();
#پایان_اگر /* CONFIG_ARCH_HAS_DIRECTED_IPIS */
#پایان_اگر /* CONFIG_SMP && CONFIG_SCHED_IPI_SUPPORTED */
		إذا (arch_is_درون_isr()) {
			thread_halt_spin(thread, کلید);
		} وإلا  {
			جمع_کن_مقصد_waitq_locked(_current, wq);
			z_جابجا_locked(کلید);
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
		طالما (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		}
	} وإلا {
		halt_thread(thread, terminate ? _THREAD_DEAD : _THREAD_SUSPENDED, &کلید);
		إذا ((thread == _current) && !arch_is_درون_isr()) {
			إذا (z_is_thread_essential(thread)) {
				z_sched_spinlock_unlock(کلید);
				k_panic();
				کلید = z_sched_spinlock_قفل();
			}
			z_جابجا_locked(کلید);
			__ASSERT(!terminate, "aborted _current back from dead");
		} وإلا {
			z_sched_spinlock_unlock(کلید);
		}
	}
	/* NOTE: the scheduler lock has been released.  Don't put
	 * logic here, it's likely to be racy/deadlocky even if you
	 * re-take the lock!
	 */
}

ساكن مضمن bool resched(uint32_t کلید)
{
#اگر_تعریف_شده CONFIG_SMP
	_current_cpu->جابجا_ok = 0;
#پایان_اگر /* CONFIG_SMP */

	أرجع arch_irq_unlocked(کلید) && !arch_is_درون_isr();
}

/*
 * Check if the next ready thread is the same as the current thread
 * and save the trip if true.
 */
ساكن مضمن bool need_جابجا(فراغ)
{
	/* the SMP case will be handled in C based z_swap() */
#اگر_تعریف_شده CONFIG_SMP
	أرجع true;
#وإلا
	بنية k_thread *new_thread;

	/* Check if the next ready thread is the same as the current thread */
	new_thread = _kernel.ready_q.cache;
	أرجع new_thread != _current;
#پایان_اگر /* CONFIG_SMP */
}

ساكن فراغ reschedule(بنية k_spinlock *قفل, k_spinlock_کلید_t کلید)
{
	إذا (resched(کلید.کلید) && need_جابجا()) {
		z_جابجا(قفل, کلید);
	} وإلا {
		signal_pending_ipi();
		k_spin_unlock(قفل, کلید);
	}
}

/**
 * Like reschedule(), but the scheduler's spinlock is known to be the lock.
 */
ساكن فراغ reschedule_locked(k_spinlock_کلید_t کلید)
{
	أرجع reschedule(&_sched_spinlock, کلید);
}

فراغ z_sched_قفل_reschedule(k_spinlock_کلید_t کلید)
{
	update_cache(0);
	reschedule_locked(کلید);
}

فراغ z_sched_yield(فراغ)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

	runq_yield();
	update_cache(1);
	z_جابجا_locked(کلید);
}

/* The scheduler's spinlock must be held */
ساكن فراغ جمع_کن_مقصد_waitq_locked(بنية k_thread *thread, _wait_q_t *wait_q)
{
	/* A thread must not already be on a wait queue when added to a new one. */
	__ASSERT_NO_MSG(thread->پایه.pended_on == عدم);

	unready_thread(thread);
	z_mark_thread_as_pending(thread);

	SYS_پورت_TRACING_FUNC(k_thread, sched_pend, thread);

	إذا (wait_q != عدم) {
		thread->پایه.pended_on = wait_q;
		priq_wait_جمع_کن(&wait_q->waitq, thread);
	}
}

فراغ z_sched_جمع_کن_مقصد_waitq_locked(بنية k_thread *thread, _wait_q_t *wait_q)
	ALIAS_از(جمع_کن_مقصد_waitq_locked);

ساكن فراغ pend_locked(بنية k_thread *thread, _wait_q_t *wait_q,
			k_timeout_t timeout)
{
#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(wait_q == عدم || sys_cache_is_mem_coherent(wait_q));
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */
	جمع_کن_مقصد_waitq_locked(thread, wait_q);
	z_جمع_کن_thread_timeout(thread, timeout);
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
		إذا (thread->پایه.pended_on != عدم) {
			unpend_thread_no_timeout(thread);
		}
	}
}

فراغ z_sched_wake_thread_locked(بنية k_thread *thread)
{
	/* No K_SPINLOCK: caller must hold the scheduler's spinlock when calling */
	bool killed = (thread->پایه.thread_state &
			(_THREAD_DEAD | _THREAD_ABORTING));

	إذا (!killed) {
		/* The thread is not being killed */
		إذا (thread->پایه.pended_on != عدم) {
			unpend_thread_no_timeout(thread);
		}
		z_mark_thread_as_not_sleeping(thread);
		ready_thread(thread);
	}
}

#اگر_تعریف_شده CONFIG_SYS_CLOCK_EXISTS
/* Timeout handler for *_thread_timeout() APIs */
فراغ z_thread_timeout(بنية _timeout *timeout)
{
	بنية k_thread *thread = CONTAINER_از(timeout,
					       بنية k_thread, پایه.timeout);

	Z_SCHED_SPINLOCK {
		/* A concurrent waker (e.g. a sem give on another CPU) may
		 * have unpended and readied the thread, after which the
		 * thread could run and re-pend elsewhere -- possibly with no
		 * timeout. Such a waker aborts this timeout, flagging it
		 * superseded; bail so we don't wake the thread from its new
		 * wait.
		 */
		إذا (!z_timeout_inflight_superseded(timeout)) {
			z_sched_wake_thread_locked(thread);
		}
	}
}
#پایان_اگر /* CONFIG_SYS_CLOCK_EXISTS */

صحيح z_pend_curr(بنية k_spinlock *قفل, k_spinlock_کلید_t کلید,
	       _wait_q_t *wait_q, k_timeout_t timeout)
{
	/* A blocking pend from ISR context is a programming error with no
	 * safe recovery: it would sleep whatever thread was interrupted and,
	 * on CONFIG_SWAP_NONATOMIC, corrupt the scheduler. Refuse it fatally
	 * in every build rather than degrading into the corruption traced in
	 * #111518. Placed first so the refusal performs no side effect.
	 */
	إذا (arch_is_درون_isr()) {
		__ASSERT(false, "blocking pend from ISR context");
		k_panic();
	}

#إذا defined(CONFIG_TIMESLICING) && defined(CONFIG_جابجا_NONATOMIC)
	pending_current = _current;
#پایان_اگر /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	__ASSERT_NO_MSG((حجم(بنية k_spinlock) == 0) || !z_is_sched_spinlock(قفل));

	/* We do a "lock swap" prior to calling z_swap(), such that
	 * the caller's lock gets released as desired.  But we ensure
	 * that we hold the scheduler lock and leave local interrupts
	 * masked until we reach the context switch.  z_swap() itself
	 * has similar code; the duplication is because it's a legacy
	 * API that doesn't expect to be called with scheduler lock
	 * held.
	 */
	(فراغ) z_sched_spinlock_قفل();
	pend_locked(_current, wait_q, timeout);
	k_spin_release(قفل);
	أرجع z_جابجا_locked(کلید);
}

بنية k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q)
{
	بنية k_thread *thread = عدم;

	Z_SCHED_SPINLOCK {
		thread = priq_wait_بهترین(&wait_q->waitq);

		إذا (thread != عدم) {
			unpend_thread_no_timeout(thread);
		}
	}

	أرجع thread;
}

فراغ z_unpend_thread(بنية k_thread *thread)
{
	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

	إذا (thread->پایه.pended_on != عدم) {
		unpend_thread_no_timeout(thread);
	}
	طالما (z_try_abort_thread_timeout(thread) == -EAGAIN) {
		z_sched_spinlock_unlock(کلید);
		کلید = z_sched_spinlock_قفل();
	}
	z_sched_spinlock_unlock(کلید);
}

/* Priority set utility that does no rescheduling, it just changes the
 * run queue state, returning true if a reschedule is needed later.
 */
bool z_thread_prio_مجموعه(بنية k_thread *thread, صحيح prio)
{
	bool need_sched = false;
	صحيح old_prio = thread->پایه.prio;

	Z_SCHED_SPINLOCK {
		need_sched = z_is_thread_ready(thread);

		إذا (need_sched) {
			إذا (!IS_ENABLED(CONFIG_SMP) || z_is_thread_queued(thread)) {
				خارج_از_صف_thread(thread);
				thread->پایه.prio = prio;
				صف_thread(thread);

				إذا (old_prio > prio) {
					flag_ipi(ipi_ماسک_create(thread));
				}
			} وإلا {
				/*
				 * This is a running thread on SMP. Update its
				 * priority, but do not requeue it. An IPI is
				 * needed if the priority is both being lowered
				 * and it is running on another CPU.
				 */

				thread->پایه.prio = prio;

				بنية _cpu *cpu;

				cpu = thread_فعال_elsewhere(thread);
				إذا ((cpu != عدم) && (old_prio < prio)) {
					flag_ipi(IPI_CPU_ماسک(cpu->id));
				}
			}

			update_cache(1);
		} وإلا إذا (z_is_thread_pending(thread)) {
			/* Thread is pending, remove it from the waitq
			 * and reinsert it with the new priority to avoid
			 * violating waitq ordering and rb assumptions.
			 */
			_wait_q_t *wait_q = pended_on_thread(thread);

			_priq_wait_remove(&wait_q->waitq, thread);
			thread->پایه.prio = prio;
			priq_wait_جمع_کن(&wait_q->waitq, thread);
		} وإلا {
			thread->پایه.prio = prio;
		}
	}

	SYS_پورت_TRACING_OBJ_FUNC(k_thread, sched_اولویت_مجموعه, thread, prio);

	أرجع need_sched;
}

فراغ z_reschedule(بنية k_spinlock *قفل, k_spinlock_کلید_t کلید) ALIAS_از(reschedule);

فراغ z_reschedule_locked(k_spinlock_کلید_t کلید) ALIAS_از(reschedule_locked);

فراغ z_reschedule_irqlock(uint32_t کلید)
{
	إذا (resched(کلید) && need_جابجا()) {
		z_جابجا_irqlock(کلید);
	} وإلا {
		/* TODO: We only hold the IRQ lock here, not the scheduler's
		 * spinlock, violating the locking requirement documented in
		 * signal_pending_ipi(). This can result in added delayed
		 * rescheduling.
		 */
		signal_pending_ipi();
		irq_unlock(کلید);
	}
}

بنية k_thread *z_جابجا_بعدی_thread(فراغ)
{
	بنية k_thread *ret = بعدی_به_بالا();

	إذا (ret == _current) {
		/* When not swapping, have to signal IPIs here.  In
		 * the context switch case it must happen later, after
		 * _current gets requeued.
		 */
		signal_pending_ipi();
	}
	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USE_SWITCH
/* Just a wrapper around z_current_thread_set(xxx) with tracing.
 *
 * _current is not guaranteed to report @a new_thread until the switch to it
 * has happened: see z_current_thread_set(). Code between this call and the
 * switch must use @a new_thread directly.
 */
ساكن مضمن فراغ مجموعه_current(بنية k_thread *new_thread)
{
	/* If the new thread is the same as the current thread, we
	 * don't need to do anything.
	 */
	إذا (IS_ENABLED(CONFIG_INSTRUMENT_THREAD_SWITCHING) && new_thread != _current) {
		z_thread_mark_switched_بیرون();
	}
	z_current_thread_مجموعه(new_thread);
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
فراغ *z_get_بعدی_switch_handle(فراغ *interrupted)
{
	z_check_پشته_sentinel();

#اگر_تعریف_شده CONFIG_SMP
	فراغ *ret = عدم;

	Z_SCHED_SPINLOCK {
		بنية k_thread *old_thread = _current, *new_thread;

		__ASSERT(old_thread->switch_handle == عدم || is_thread_dummy(old_thread),
			"old thread handle should be null.");

		new_thread = بعدی_به_بالا();

		z_sched_usage_switch(new_thread);

		إذا (old_thread != new_thread) {
			uint8_t  cpu_id;

			z_sched_switch_spin(new_thread);
			arch_cohere_stacks(old_thread, interrupted, new_thread);

			_current_cpu->جابجا_ok = 0;
			cpu_id = arch_curr_cpu()->id;
			new_thread->پایه.cpu = cpu_id;
			مجموعه_current(new_thread);

#اگر_تعریف_شده CONFIG_TIMESLICING
			z_زمان_برش_reset(new_thread);
#پایان_اگر /* CONFIG_TIMESLICING */

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
			إذا (z_is_thread_queued(old_thread)) {
#اگر_تعریف_شده CONFIG_SCHED_IPI_CASCADE
				إذا ((new_thread->پایه.cpu_ماسک != -1) &&
				    (old_thread->پایه.cpu_ماسک != بیت(cpu_id))) {
					flag_ipi(ipi_ماسک_create(old_thread));
				}
#پایان_اگر
				runq_جمع_کن(old_thread);
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
	أرجع ret;
#وإلا
	بنية k_thread *بعدی = _kernel.ready_q.cache;

	z_sched_usage_switch(بعدی);
	_current->switch_handle = interrupted;
	مجموعه_current(بعدی);
	أرجع بعدی->switch_handle;
#پایان_اگر /* CONFIG_SMP */
}
#پایان_اگر /* CONFIG_USE_SWITCH */

صحيح z_unpend_all(_wait_q_t *wait_q)
{
	صحيح need_sched = 0;
	بنية k_thread *thread;

	Z_SCHED_SPINLOCK {
		لكل (thread = z_waitq_سر_locked(wait_q);
		     thread != عدم;
		     thread = z_waitq_سر_locked(wait_q)) {
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

	أرجع need_sched;
}

ساكن مضمن فراغ unpend_all(_wait_q_t *wait_q)
{
	بنية k_thread *thread;

	لكل (thread = z_waitq_سر_locked(wait_q);
	     thread != عدم;
	     thread = z_waitq_سر_locked(wait_q)) {
		unpend_thread_no_timeout(thread);
		arch_thread_return_مقدار_مجموعه(thread, 0);
		/* See z_unpend_all(): the in-flight handler bails on the
		 * superseded mark, so we ready the thread unconditionally.
		 */
		(فراغ)z_try_abort_thread_timeout(thread);
		ready_thread(thread);
	}
}

#اگر_تعریف_شده CONFIG_THREAD_ABORT_HOOK
خارجي فراغ thread_abort_hook(بنية k_thread *thread);
#پایان_اگر /* CONFIG_THREAD_ABORT_HOOK */

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
				      k_spinlock_کلید_t *کلید)
{
	bool dummify = false;

	/* We hold the lock, and the thread is known not to be running
	 * anywhere.
	 */
	إذا ((thread->پایه.thread_state & new_state) == 0U) {
		thread->پایه.thread_state |= new_state;
		إذا (z_is_thread_queued(thread)) {
			/* Clear idle CPU coverage before removing the thread from the run queue. */
			ipi_idle_thread_unreserve(thread);
			خارج_از_صف_thread(thread);
		}

		إذا (new_state == _THREAD_DEAD) {
			إذا (thread->پایه.pended_on != عدم) {
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
			إذا (کلید != عدم) {
				طالما (z_try_abort_thread_timeout(thread) == -EAGAIN) {
					z_sched_spinlock_unlock(*کلید);
					*کلید = z_sched_spinlock_قفل();
				}
			}
			unpend_all(&thread->join_صف);

			/* Edge case: aborting _current from within an
			 * ISR that preempted it requires clearing the
			 * _current pointer so the upcoming context
			 * switch doesn't clobber the now-freed
			 * memory
			 */
			إذا (thread == _current && arch_is_درون_isr()) {
				dummify = true;
			}
		}
#اگر_تعریف_شده CONFIG_SMP
		unpend_all(&thread->halt_صف);
#پایان_اگر /* CONFIG_SMP */
		update_cache(1);

		إذا (new_state == _THREAD_SUSPENDED) {
			clear_halting(thread);
			أرجع;
		}

		arch_coprocessors_disable(thread);

		SYS_پورت_TRACING_FUNC(k_thread, sched_abort, thread);

		z_thread_monitor_exit(thread);
#اگر_تعریف_شده CONFIG_THREAD_ABORT_HOOK
		thread_abort_hook(thread);
#پایان_اگر /* CONFIG_THREAD_ABORT_HOOK */

#اگر_تعریف_شده CONFIG_OBJ_CORE_THREAD
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_THREAD
		k_obj_core_stats_deregister(K_OBJ_CORE(thread));
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_THREAD */
		k_obj_core_unlink(K_OBJ_CORE(thread));
#پایان_اگر /* CONFIG_OBJ_CORE_THREAD */

#اگر_تعریف_شده CONFIG_USERSPACE
		z_mem_domain_exit_thread(thread);
		k_thread_perms_all_clear(thread);
		k_object_uninit(thread->پشته_obj);
		k_object_uninit(thread);
#پایان_اگر /* CONFIG_USERSPACE */

#اگر_تعریف_شده CONFIG_THREAD_ABORT_NEED_CLEANUP
		k_thread_abort_cleanup(thread);
#پایان_اگر /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

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
		إذا (dummify && !IS_ENABLED(CONFIG_ARCH_POSIX)) {
#اگر_تعریف_شده CONFIG_USE_SWITCH
			_current->switch_handle = (فراغ *)1;
#پایان_اگر
#اگر_تعریف_شده CONFIG_SPIN_VALIDATE
			/* On arches where exceptions run as ISRs (e.g. Xtensa),
			 * the dying thread's lock tracking is never cleared via the
			 * normal abort path.  Reset it here before _thread_dummy
			 * takes over.  Sentinel-gated so genuine bugs still assert.
			 */
			إذا (thread->پایه.جابجا_داده ==
			    (فراغ *)&z_spinlock_abort_sentinel) {
				z_spin_validate_reset(true);
			}
#پایان_اگر
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
	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

	z_mark_thread_as_suspended(thread);
	z_metairq_preempted_clear(thread);
	خارج_از_صف_thread(thread);
	update_cache(1);
	z_جابجا_locked(کلید);
}
