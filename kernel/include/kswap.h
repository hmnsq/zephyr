/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_KSWAP_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_KSWAP_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <run_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/barrier.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_arch_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeslicing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <usage.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਢੇਰ_SENTINEL
ਬਾਹਰੀ ਖਾਲੀ z_check_ਢੇਰ_sentinel(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_check_ਢੇਰ_sentinel() /**/
#ਜੇ_ਅੰਤ /* CONFIG_STACK_SENTINEL */

/* In SMP, the irq_lock() is a spinlock which is implicitly released
 * and reacquired on context switch to preserve the existing
 * semantics.  This means that whenever we are about to return to a
 * thread (via either z_swap() or interrupt/exception return!) we need
 * to restore the lock state to whatever the thread's counter
 * expects.
 */
ਖਾਲੀ z_smp_release_global_ਤਾਲਾ(ਬਣਤਰ k_thread *thread);

/* context switching and scheduling-related routines */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USE_SWITCH

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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_sched_switch_spin(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਅਸਥਿਰ ਖਾਲੀ **shp = (ਖਾਲੀ *)&thread->switch_handle;

	ਜਦੋਂ (*shp == ਨਿਰਾ) {
		arch_spin_relax();
	}
	/* Read barrier: don't allow any subsequent loads in the
	 * calling code to reorder before we saw switch_handle go
	 * non-null.
	 */
	barrier_dmem_fence_ਭਰਿਆ();
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}

/* New style context switching.  arch_switch() is a lower level
 * primitive that doesn't know about the scheduler or return value.
 * Needed for SMP, where the scheduler requires spinlocking that we
 * don't want to have to do in per-architecture assembly.
 *
 * Note that is_spinlock is a compile-time construct which will be
 * optimized out when this function is expanded.
 */
ਸਥਿਰ ALWAYS_INLINE ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ do_ਅਦਲਾ_ਬਦਲੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ,
					  ਬਣਤਰ k_spinlock *ਤਾਲਾ,
					  bool is_spinlock)
{
	ਬਣਤਰ k_thread *new_thread, *old_thread;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPIN_VALIDATE
	z_assert_can_ਅਦਲਾ_ਬਦਲੀ(ਕੁੰਜੀ, ਤਾਲਾ);
#ਜੇ_ਅੰਤ /* CONFIG_SPIN_VALIDATE */

	old_thread = _current;

	z_check_ਢੇਰ_sentinel();

	old_thread->ਅਦਲਾ_ਬਦਲੀ_retval = -EAGAIN;

	/* We always take the scheduler spinlock if we don't already
	 * have it.  We "release" other spinlocks here.  But we never
	 * drop the interrupt lock.
	 */
	ਜੇ (is_spinlock && (ਤਾਲਾ != ਨਿਰਾ) && !z_is_sched_spinlock(ਤਾਲਾ)) {
		k_spin_release(ਤਾਲਾ);
	}
	ਜੇ (IS_ENABLED(CONFIG_SMP) || IS_ENABLED(CONFIG_SPIN_VALIDATE)) {
		/* Taking a nested uniprocessor lock in void context is a noop */
		ਜੇ (!is_spinlock || !z_is_sched_spinlock(ਤਾਲਾ)) {
			(ਖਾਲੀ)z_sched_spinlock_ਤਾਲਾ();
		}
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	new_thread = z_ਅਦਲਾ_ਬਦਲੀ_ਅਗਲਾ_thread();
#ਨਹੀਂਤਾਂ
	new_thread = _kernel.ready_q.cache;
#ਜੇ_ਅੰਤ

	ਜੇ (new_thread != old_thread) {
		z_sched_usage_switch(new_thread);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
		new_thread->ਅਧਾਰ.cpu = arch_curr_cpu()->id;

		ਜੇ (!is_spinlock) {
			z_smp_release_global_ਤਾਲਾ(new_thread);
		}
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
		z_thread_mark_switched_ਬਾਹਰਲਾ();
		z_sched_switch_spin(new_thread);
		z_current_thread_ਸੈੱਟ(new_thread);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICING
		z_ਸਮਾਂ_ਟੁਕੜਾ_reset(new_thread);
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICING */

		z_sched_spinlock_transfer_owner(new_thread);

		arch_cohere_stacks(old_thread, ਨਿਰਾ, new_thread);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
		/* Now add _current back to the run queue, once we are
		 * guaranteed to reach the context switch in finite time.  See
		 * z_sched_switch_spin().  The current thread can never live in
		 * the run queue until we are inexorably on the context switch
		 * path on SMP, otherwise there is a deadlock condition where a
		 * set of CPUs pick a cycle of threads to run and wait for them
		 * all to context switch forever.
		 */
		ਜੇ (z_is_thread_queued(old_thread)) {
			runq_ਜੋੜੋ(old_thread);
		}
		signal_pending_ipi();
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
		ਖਾਲੀ *newsh = new_thread->switch_handle;

		ਜੇ (IS_ENABLED(CONFIG_SMP)) {
			/* Active threads must have a null here.  And
			 * it must be seen before the scheduler lock
			 * is released!
			 */
			new_thread->switch_handle = ਨਿਰਾ;
			barrier_dmem_fence_ਭਰਿਆ(); /* write barrier */
		}
		z_sched_spinlock_release();
		arch_switch(newsh, &old_thread->switch_handle);
	} ਨਹੀਂਤਾਂ {
		z_sched_spinlock_release();
	}

	ਜੇ (is_spinlock) {
		arch_irq_unlock(ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		irq_unlock(ਕੁੰਜੀ);
	}

	ਵਾਪਸ _current->ਅਦਲਾ_ਬਦਲੀ_retval;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_ਅਦਲਾ_ਬਦਲੀ_irqlock(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ)
{
	ਵਾਪਸ do_ਅਦਲਾ_ਬਦਲੀ(ਕੁੰਜੀ, ਨਿਰਾ, false);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_ਅਦਲਾ_ਬਦਲੀ(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	ਵਾਪਸ do_ਅਦਲਾ_ਬਦਲੀ(ਕੁੰਜੀ.ਕੁੰਜੀ, ਤਾਲਾ, true);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_ਅਦਲਾ_ਬਦਲੀ_unlocked(ਖਾਲੀ)
{
	(ਖਾਲੀ) do_ਅਦਲਾ_ਬਦਲੀ(arch_irq_ਤਾਲਾ(), ਨਿਰਾ, true);
}

#ਨਹੀਂਤਾਂ /* !CONFIG_USE_SWITCH */


ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_sched_switch_spin(ਬਣਤਰ k_thread *thread)
{
	ARG_ਅਣਵਰਤਿਆ(thread);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_ਅਦਲਾ_ਬਦਲੀ_irqlock(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ)
{
	ਪੂਰਨ ret;
	z_check_ਢੇਰ_sentinel();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPIN_VALIDATE
	z_assert_can_ਅਦਲਾ_ਬਦਲੀ(ਕੁੰਜੀ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_SPIN_VALIDATE */

	ret = arch_ਅਦਲਾ_ਬਦਲੀ(ਕੁੰਜੀ);
	ਵਾਪਸ ret;
}

/* If !USE_SWITCH, then spinlocks are guaranteed degenerate as we
 * can't be in SMP.  The k_spin_release() call is just for validation
 * handling.
 */
ਸਥਿਰ ALWAYS_INLINE ਪੂਰਨ z_ਅਦਲਾ_ਬਦਲੀ(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	k_spin_release(ਤਾਲਾ);
	ਵਾਪਸ z_ਅਦਲਾ_ਬਦਲੀ_irqlock(ਕੁੰਜੀ.ਕੁੰਜੀ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_ਅਦਲਾ_ਬਦਲੀ_unlocked(ਖਾਲੀ)
{
	(ਖਾਲੀ) z_ਅਦਲਾ_ਬਦਲੀ_irqlock(arch_irq_ਤਾਲਾ());
}

#ਜੇ_ਅੰਤ /* !CONFIG_USE_SWITCH */

/**
 * @brief - Like z_swap() but it is known that the scheduler's spinlock is already held.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_ਅਦਲਾ_ਬਦਲੀ_locked(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	ਵਾਪਸ z_ਅਦਲਾ_ਬਦਲੀ(&_sched_spinlock, ਕੁੰਜੀ);
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
ਖਾਲੀ z_dummy_thread_init(ਬਣਤਰ k_thread *dummy_thread);

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_KSWAP_H_ */
