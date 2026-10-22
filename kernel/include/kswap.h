/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_KSWAP_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_KSWAP_H_

#شامل <kspinlock.h>
#شامل <ksched.h>
#شامل <run_q.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/sys/barrier.h>
#شامل <kernel_arch_func.h>
#شامل <timeslicing.h>
#شامل <usage.h>

#اگر_تعریف_شدہ CONFIG_ڈھیر_SENTINEL
خارجی خالی z_check_ڈھیر_sentinel(خالی);
#ورنہ
#تعریف_کرو z_check_ڈھیر_sentinel() /**/
#اگر_ختم /* CONFIG_STACK_SENTINEL */

/* In SMP, the irq_lock() is a spinlock which is implicitly released
 * and reacquired on context switch to preserve the existing
 * semantics.  This means that whenever we are about to return to a
 * thread (via either z_swap() or interrupt/exception return!) we need
 * to restore the lock state to whatever the thread's counter
 * expects.
 */
خالی z_smp_release_global_تالا(ساخت k_thread *thread);

/* context switching and scheduling-related routines */
#اگر_تعریف_شدہ CONFIG_USE_SWITCH

/* Spin, with the scheduler lock held (!), on a thread that is known
 * (!!) to have released the lock and be on a path where it will
 * deterministically (!!!) reach arch_switch() in very small constant
 * time.
 *
 * This exists to treat an unavoidable SMP race when threads swap --
 * their thread record is in the queue (and visible to other CPUs)
 * before arch_switch() finishes saving state.  We must spin for the
 * switch handle before entering a new thread.  See docs on
 * arch_switch().
 *
 * Stated differently: there's a chicken and egg bug with the question
 * of "is a thread running or not?".  The thread needs to mark itself
 * "not running" from its own context, but at that moment it obviously
 * is still running until it reaches arch_switch()!  Locking can't
 * treat this because the scheduler lock can't be released by the
 * switched-to thread, which is going to (obviously) be running its
 * own code and doesn't know it was switched out.
 */
ساکن اندرونی خالی z_sched_switch_spin(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SMP
	متلون خالی **shp = (خالی *)&thread->switch_handle;

	جب (*shp == تہی) {
		arch_spin_relax();
	}
	/* Read barrier: don't allow any subsequent loads in the
	 * calling code to reorder before we saw switch_handle go
	 * non-null.
	 */
	barrier_dmem_fence_بھرا();
#اگر_ختم /* CONFIG_SMP */
}

/* New style context switching.  arch_switch() is a lower level
 * primitive that doesn't know about the scheduler or return value.
 * Needed for SMP, where the scheduler requires spinlocking that we
 * don't want to have to do in per-architecture assembly.
 *
 * Note that is_spinlock is a compile-time construct which will be
 * optimized out when this function is expanded.
 */
ساکن ALWAYS_INLINE غیرعلامتی صحیح do_تبادلہ(غیرعلامتی صحیح کلید,
					  ساخت k_spinlock *تالا,
					  bool is_spinlock)
{
	ساخت k_thread *new_thread, *old_thread;

#اگر_تعریف_شدہ CONFIG_SPIN_VALIDATE
	z_assert_can_تبادلہ(کلید, تالا);
#اگر_ختم /* CONFIG_SPIN_VALIDATE */

	old_thread = _current;

	z_check_ڈھیر_sentinel();

	old_thread->تبادلہ_retval = -EAGAIN;

	/* We always take the scheduler spinlock if we don't already
	 * have it.  We "release" other spinlocks here.  But we never
	 * drop the interrupt lock.
	 */
	اگر (is_spinlock && (تالا != تہی) && !z_is_sched_spinlock(تالا)) {
		k_spin_release(تالا);
	}
	اگر (IS_ENABLED(CONFIG_SMP) || IS_ENABLED(CONFIG_SPIN_VALIDATE)) {
		/* Taking a nested uniprocessor lock in void context is a noop */
		اگر (!is_spinlock || !z_is_sched_spinlock(تالا)) {
			(خالی)z_sched_spinlock_تالا();
		}
	}

#اگر_تعریف_شدہ CONFIG_SMP
	new_thread = z_تبادلہ_اگلا_thread();
#ورنہ
	new_thread = _kernel.ready_q.cache;
#اگر_ختم

	اگر (new_thread != old_thread) {
		z_sched_usage_switch(new_thread);

#اگر_تعریف_شدہ CONFIG_SMP
		new_thread->بنیاد.cpu = arch_curr_cpu()->id;

		اگر (!is_spinlock) {
			z_smp_release_global_تالا(new_thread);
		}
#اگر_ختم /* CONFIG_SMP */
		z_thread_mark_switched_برآمد();
		z_sched_switch_spin(new_thread);
		z_current_thread_سیٹ(new_thread);

#اگر_تعریف_شدہ CONFIG_TIMESLICING
		z_وقت_قاش_reset(new_thread);
#اگر_ختم /* CONFIG_TIMESLICING */

		z_sched_spinlock_transfer_owner(new_thread);

		arch_cohere_stacks(old_thread, تہی, new_thread);

#اگر_تعریف_شدہ CONFIG_SMP
		/* Now add _current back to the run queue, once we are
		 * guaranteed to reach the context switch in finite time.  See
		 * z_sched_switch_spin().  The current thread can never live in
		 * the run queue until we are inexorably on the context switch
		 * path on SMP, otherwise there is a deadlock condition where a
		 * set of CPUs pick a cycle of threads to run and wait for them
		 * all to context switch forever.
		 */
		اگر (z_is_thread_queued(old_thread)) {
			runq_جوڑو(old_thread);
		}
		signal_pending_ipi();
#اگر_ختم /* CONFIG_SMP */
		خالی *newsh = new_thread->switch_handle;

		اگر (IS_ENABLED(CONFIG_SMP)) {
			/* Active threads must have a null here.  And
			 * it must be seen before the scheduler lock
			 * is released!
			 */
			new_thread->switch_handle = تہی;
			barrier_dmem_fence_بھرا(); /* write barrier */
		}
		z_sched_spinlock_release();
		arch_switch(newsh, &old_thread->switch_handle);
	} ورنہ {
		z_sched_spinlock_release();
	}

	اگر (is_spinlock) {
		arch_irq_unlock(کلید);
	} ورنہ {
		irq_unlock(کلید);
	}

	واپس _current->تبادلہ_retval;
}

ساکن اندرونی صحیح z_تبادلہ_irqlock(غیرعلامتی صحیح کلید)
{
	واپس do_تبادلہ(کلید, تہی, false);
}

ساکن اندرونی صحیح z_تبادلہ(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید)
{
	واپس do_تبادلہ(کلید.کلید, تالا, true);
}

ساکن اندرونی خالی z_تبادلہ_unlocked(خالی)
{
	(خالی) do_تبادلہ(arch_irq_تالا(), تہی, true);
}

#ورنہ /* !CONFIG_USE_SWITCH */


ساکن اندرونی خالی z_sched_switch_spin(ساخت k_thread *thread)
{
	ARG_غیر_مستعمل(thread);
}

ساکن اندرونی صحیح z_تبادلہ_irqlock(غیرعلامتی صحیح کلید)
{
	صحیح ret;
	z_check_ڈھیر_sentinel();

#اگر_تعریف_شدہ CONFIG_SPIN_VALIDATE
	z_assert_can_تبادلہ(کلید, تہی);
#اگر_ختم /* CONFIG_SPIN_VALIDATE */

	ret = arch_تبادلہ(کلید);
	واپس ret;
}

/* If !USE_SWITCH, then spinlocks are guaranteed degenerate as we
 * can't be in SMP.  The k_spin_release() call is just for validation
 * handling.
 */
ساکن ALWAYS_INLINE صحیح z_تبادلہ(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید)
{
	k_spin_release(تالا);
	واپس z_تبادلہ_irqlock(کلید.کلید);
}

ساکن اندرونی خالی z_تبادلہ_unlocked(خالی)
{
	(خالی) z_تبادلہ_irqlock(arch_irq_تالا());
}

#اگر_ختم /* !CONFIG_USE_SWITCH */

/**
 * @brief - Like z_swap() but it is known that the scheduler's spinlock is already held.
 */
ساکن اندرونی صحیح z_تبادلہ_locked(k_spinlock_کلید_t کلید)
{
	واپس z_تبادلہ(&_sched_spinlock, کلید);
}

/**
 * Set up a "dummy" thread, used at early initialization to launch the
 * first thread on a CPU.
 *
 * Needs to set enough fields such that the context switching code can
 * use it to properly store state, which will just be discarded.
 *
 * The memory of the dummy thread can be completely uninitialized.
 */
خالی z_dummy_thread_init(ساخت k_thread *dummy_thread);

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_KSWAP_H_ */
