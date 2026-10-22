/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_KSWAP_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_KSWAP_H_

#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <run_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/barrier.h>
#ಅನ್ತರ್ಭಾವ <kernel_arch_func.h>
#ಅನ್ತರ್ಭಾವ <timeslicing.h>
#ಅನ್ತರ್ಭಾವ <usage.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ರಾಶಿ_SENTINEL
ಬಾಹ್ಯ ಶೂನ್ಯ z_check_ರಾಶಿ_sentinel(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು z_check_ರಾಶಿ_sentinel() /**/
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_SENTINEL */

/* In SMP, the irq_lock() is a spinlock which is implicitly released
 * and reacquired on context switch to preserve the existing
 * semantics.  This means that whenever we are about to return to a
 * thread (via either z_swap() or interrupt/exception return!) we need
 * to restore the lock state to whatever the thread's counter
 * expects.
 */
ಶೂನ್ಯ z_smp_release_global_ಬೀಗ(ರಚನೆ k_thread *thread);

/* context switching and scheduling-related routines */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USE_SWITCH

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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_sched_switch_spin(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	ಅಸ್ಥಿರ ಶೂನ್ಯ **shp = (ಶೂನ್ಯ *)&thread->switch_handle;

	ಆಗ (*shp == ಖಾಲಿ) {
		arch_spin_relax();
	}
	/* Read barrier: don't allow any subsequent loads in the
	 * calling code to reorder before we saw switch_handle go
	 * non-null.
	 */
	barrier_dmem_fence_ತುಂಬಿದ();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}

/* New style context switching.  arch_switch() is a lower level
 * primitive that doesn't know about the scheduler or return value.
 * Needed for SMP, where the scheduler requires spinlocking that we
 * don't want to have to do in per-architecture assembly.
 *
 * Note that is_spinlock is a compile-time construct which will be
 * optimized out when this function is expanded.
 */
ಸ್ಥಿರ ALWAYS_INLINE ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ do_ಅದಲುಬದಲು(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ,
					  ರಚನೆ k_spinlock *ಬೀಗ,
					  bool is_spinlock)
{
	ರಚನೆ k_thread *new_thread, *old_thread;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPIN_VALIDATE
	z_assert_can_ಅದಲುಬದಲು(ಕೀಲಿ, ಬೀಗ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SPIN_VALIDATE */

	old_thread = _current;

	z_check_ರಾಶಿ_sentinel();

	old_thread->ಅದಲುಬದಲು_retval = -EAGAIN;

	/* We always take the scheduler spinlock if we don't already
	 * have it.  We "release" other spinlocks here.  But we never
	 * drop the interrupt lock.
	 */
	ಆದರೆ (is_spinlock && (ಬೀಗ != ಖಾಲಿ) && !z_is_sched_spinlock(ಬೀಗ)) {
		k_spin_release(ಬೀಗ);
	}
	ಆದರೆ (IS_ENABLED(CONFIG_SMP) || IS_ENABLED(CONFIG_SPIN_VALIDATE)) {
		/* Taking a nested uniprocessor lock in void context is a noop */
		ಆದರೆ (!is_spinlock || !z_is_sched_spinlock(ಬೀಗ)) {
			(ಶೂನ್ಯ)z_sched_spinlock_ಬೀಗ();
		}
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	new_thread = z_ಅದಲುಬದಲು_ಮುಂದಿನ_thread();
#ಇಲ್ಲದಿದ್ದರೆ
	new_thread = _kernel.ready_q.cache;
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (new_thread != old_thread) {
		z_sched_usage_switch(new_thread);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
		new_thread->ತಳ.cpu = arch_curr_cpu()->id;

		ಆದರೆ (!is_spinlock) {
			z_smp_release_global_ಬೀಗ(new_thread);
		}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
		z_thread_mark_switched_ಹೊರಹೋಗು();
		z_sched_switch_spin(new_thread);
		z_current_thread_ಗಣ(new_thread);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICING
		z_ಸಮಯ_ತುಂಡು_reset(new_thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICING */

		z_sched_spinlock_transfer_owner(new_thread);

		arch_cohere_stacks(old_thread, ಖಾಲಿ, new_thread);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
		/* Now add _current back to the run queue, once we are
		 * guaranteed to reach the context switch in finite time.  See
		 * z_sched_switch_spin().  The current thread can never live in
		 * the run queue until we are inexorably on the context switch
		 * path on SMP, otherwise there is a deadlock condition where a
		 * set of CPUs pick a cycle of threads to run and wait for them
		 * all to context switch forever.
		 */
		ಆದರೆ (z_is_thread_queued(old_thread)) {
			runq_ಕೂಡಿಸು(old_thread);
		}
		signal_pending_ipi();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
		ಶೂನ್ಯ *newsh = new_thread->switch_handle;

		ಆದರೆ (IS_ENABLED(CONFIG_SMP)) {
			/* Active threads must have a null here.  And
			 * it must be seen before the scheduler lock
			 * is released!
			 */
			new_thread->switch_handle = ಖಾಲಿ;
			barrier_dmem_fence_ತುಂಬಿದ(); /* write barrier */
		}
		z_sched_spinlock_release();
		arch_switch(newsh, &old_thread->switch_handle);
	} ಇಲ್ಲದಿದ್ದರೆ {
		z_sched_spinlock_release();
	}

	ಆದರೆ (is_spinlock) {
		arch_irq_unlock(ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		irq_unlock(ಕೀಲಿ);
	}

	ಹಿಂತಿರುಗಿ _current->ಅದಲುಬದಲು_retval;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_ಅದಲುಬದಲು_irqlock(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ)
{
	ಹಿಂತಿರುಗಿ do_ಅದಲುಬದಲು(ಕೀಲಿ, ಖಾಲಿ, false);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_ಅದಲುಬದಲು(ರಚನೆ k_spinlock *ಬೀಗ, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ಹಿಂತಿರುಗಿ do_ಅದಲುಬದಲು(ಕೀಲಿ.ಕೀಲಿ, ಬೀಗ, true);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_ಅದಲುಬದಲು_unlocked(ಶೂನ್ಯ)
{
	(ಶೂನ್ಯ) do_ಅದಲುಬದಲು(arch_irq_ಬೀಗ(), ಖಾಲಿ, true);
}

#ಇಲ್ಲದಿದ್ದರೆ /* !CONFIG_USE_SWITCH */


ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_sched_switch_spin(ರಚನೆ k_thread *thread)
{
	ARG_ಬಳಸದ(thread);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_ಅದಲುಬದಲು_irqlock(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ)
{
	ಪೂರ್ಣಾಂಕ ret;
	z_check_ರಾಶಿ_sentinel();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPIN_VALIDATE
	z_assert_can_ಅದಲುಬದಲು(ಕೀಲಿ, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SPIN_VALIDATE */

	ret = arch_ಅದಲುಬದಲು(ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ ret;
}

/* If !USE_SWITCH, then spinlocks are guaranteed degenerate as we
 * can't be in SMP.  The k_spin_release() call is just for validation
 * handling.
 */
ಸ್ಥಿರ ALWAYS_INLINE ಪೂರ್ಣಾಂಕ z_ಅದಲುಬದಲು(ರಚನೆ k_spinlock *ಬೀಗ, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	k_spin_release(ಬೀಗ);
	ಹಿಂತಿರುಗಿ z_ಅದಲುಬದಲು_irqlock(ಕೀಲಿ.ಕೀಲಿ);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_ಅದಲುಬದಲು_unlocked(ಶೂನ್ಯ)
{
	(ಶೂನ್ಯ) z_ಅದಲುಬದಲು_irqlock(arch_irq_ಬೀಗ());
}

#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_USE_SWITCH */

/**
 * @brief - Like z_swap() but it is known that the scheduler's spinlock is already held.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_ಅದಲುಬದಲು_locked(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ಹಿಂತಿರುಗಿ z_ಅದಲುಬದಲು(&_sched_spinlock, ಕೀಲಿ);
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
ಶೂನ್ಯ z_dummy_thread_init(ರಚನೆ k_thread *dummy_thread);

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_KSWAP_H_ */
