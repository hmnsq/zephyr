/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_KSWAP_H_
#define ZEPHYR_KERNEL_INCLUDE_KSWAP_H_

#अन्तर्भाव <kspinlock.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <run_q.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/sys/barrier.h>
#अन्तर्भाव <kernel_arch_func.h>
#अन्तर्भाव <timeslicing.h>
#अन्तर्भाव <usage.h>

#ifdef CONFIG_ढेर_SENTINEL
बाह्य शून्य z_check_ढेर_sentinel(शून्य);
#अन्यथा
#define z_check_ढेर_sentinel() /**/
#endif /* CONFIG_STACK_SENTINEL */

/* In SMP, the irq_lock() is a spinlock which is implicitly released
 * and reacquired on context switch to preserve the existing
 * semantics.  This means that whenever we are about to return to a
 * thread (via either z_swap() or interrupt/exception return!) we need
 * to restore the lock state to whatever the thread's counter
 * expects.
 */
शून्य z_smp_release_global_lock(संरचना k_thread *thread);

/* context switching and scheduling-related routines */
#ifdef CONFIG_USE_SWITCH

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
स्थिर अन्तःस्थ शून्य z_sched_switch_spin(संरचना k_thread *thread)
{
#ifdef CONFIG_SMP
	चञ्चल शून्य **shp = (शून्य *)&thread->switch_handle;

	यावत् (*shp == रिक्त) {
		arch_spin_relax();
	}
	/* Read barrier: don't allow any subsequent loads in the
	 * calling code to reorder before we saw switch_handle go
	 * non-null.
	 */
	barrier_dmem_fence_full();
#endif /* CONFIG_SMP */
}

/* New style context switching.  arch_switch() is a lower level
 * primitive that doesn't know about the scheduler or return value.
 * Needed for SMP, where the scheduler requires spinlocking that we
 * don't want to have to do in per-architecture assembly.
 *
 * Note that is_spinlock is a compile-time construct which will be
 * optimized out when this function is expanded.
 */
स्थिर ALWAYS_INLINE अचिन्हित पूर्णाङ्क do_swap(अचिन्हित पूर्णाङ्क कुंजी,
					  संरचना k_spinlock *lock,
					  bool is_spinlock)
{
	संरचना k_thread *new_thread, *old_thread;

#ifdef CONFIG_SPIN_VALIDATE
	z_assert_can_swap(कुंजी, lock);
#endif /* CONFIG_SPIN_VALIDATE */

	old_thread = _current;

	z_check_ढेर_sentinel();

	old_thread->swap_retval = -EAGAIN;

	/* We always take the scheduler spinlock if we don't already
	 * have it.  We "release" other spinlocks here.  But we never
	 * drop the interrupt lock.
	 */
	यदि (is_spinlock && (lock != रिक्त) && !z_is_sched_spinlock(lock)) {
		k_spin_release(lock);
	}
	यदि (IS_ENABLED(CONFIG_SMP) || IS_ENABLED(CONFIG_SPIN_VALIDATE)) {
		/* Taking a nested uniprocessor lock in void context is a noop */
		यदि (!is_spinlock || !z_is_sched_spinlock(lock)) {
			(शून्य)z_sched_spinlock_lock();
		}
	}

#ifdef CONFIG_SMP
	new_thread = z_swap_next_thread();
#अन्यथा
	new_thread = _kernel.ready_q.cache;
#endif

	यदि (new_thread != old_thread) {
		z_sched_usage_switch(new_thread);

#ifdef CONFIG_SMP
		new_thread->base.cpu = arch_curr_cpu()->id;

		यदि (!is_spinlock) {
			z_smp_release_global_lock(new_thread);
		}
#endif /* CONFIG_SMP */
		z_thread_mark_switched_out();
		z_sched_switch_spin(new_thread);
		z_current_thread_set(new_thread);

#ifdef CONFIG_TIMESLICING
		z_time_slice_reset(new_thread);
#endif /* CONFIG_TIMESLICING */

		z_sched_spinlock_transfer_owner(new_thread);

		arch_cohere_stacks(old_thread, रिक्त, new_thread);

#ifdef CONFIG_SMP
		/* Now add _current back to the run queue, once we are
		 * guaranteed to reach the context switch in finite time.  See
		 * z_sched_switch_spin().  The current thread can never live in
		 * the run queue until we are inexorably on the context switch
		 * path on SMP, otherwise there is a deadlock condition where a
		 * set of CPUs pick a cycle of threads to run and wait for them
		 * all to context switch forever.
		 */
		यदि (z_is_thread_queued(old_thread)) {
			runq_add(old_thread);
		}
		signal_pending_ipi();
#endif /* CONFIG_SMP */
		शून्य *newsh = new_thread->switch_handle;

		यदि (IS_ENABLED(CONFIG_SMP)) {
			/* Active threads must have a null here.  And
			 * it must be seen before the scheduler lock
			 * is released!
			 */
			new_thread->switch_handle = रिक्त;
			barrier_dmem_fence_full(); /* write barrier */
		}
		z_sched_spinlock_release();
		arch_switch(newsh, &old_thread->switch_handle);
	} अन्यथा {
		z_sched_spinlock_release();
	}

	यदि (is_spinlock) {
		arch_irq_unlock(कुंजी);
	} अन्यथा {
		irq_unlock(कुंजी);
	}

	प्रत्यागम _current->swap_retval;
}

स्थिर अन्तःस्थ पूर्णाङ्क z_swap_irqlock(अचिन्हित पूर्णाङ्क कुंजी)
{
	प्रत्यागम do_swap(कुंजी, रिक्त, false);
}

स्थिर अन्तःस्थ पूर्णाङ्क z_swap(संरचना k_spinlock *lock, k_spinlock_कुंजी_t कुंजी)
{
	प्रत्यागम do_swap(कुंजी.कुंजी, lock, true);
}

स्थिर अन्तःस्थ शून्य z_swap_unlocked(शून्य)
{
	(शून्य) do_swap(arch_irq_lock(), रिक्त, true);
}

#अन्यथा /* !CONFIG_USE_SWITCH */


स्थिर अन्तःस्थ शून्य z_sched_switch_spin(संरचना k_thread *thread)
{
	ARG_UNUSED(thread);
}

स्थिर अन्तःस्थ पूर्णाङ्क z_swap_irqlock(अचिन्हित पूर्णाङ्क कुंजी)
{
	पूर्णाङ्क ret;
	z_check_ढेर_sentinel();

#ifdef CONFIG_SPIN_VALIDATE
	z_assert_can_swap(कुंजी, रिक्त);
#endif /* CONFIG_SPIN_VALIDATE */

	ret = arch_swap(कुंजी);
	प्रत्यागम ret;
}

/* If !USE_SWITCH, then spinlocks are guaranteed degenerate as we
 * can't be in SMP.  The k_spin_release() call is just for validation
 * handling.
 */
स्थिर ALWAYS_INLINE पूर्णाङ्क z_swap(संरचना k_spinlock *lock, k_spinlock_कुंजी_t कुंजी)
{
	k_spin_release(lock);
	प्रत्यागम z_swap_irqlock(कुंजी.कुंजी);
}

स्थिर अन्तःस्थ शून्य z_swap_unlocked(शून्य)
{
	(शून्य) z_swap_irqlock(arch_irq_lock());
}

#endif /* !CONFIG_USE_SWITCH */

/**
 * @brief - Like z_swap() but it is known that the scheduler's spinlock is already held.
 */
स्थिर अन्तःस्थ पूर्णाङ्क z_swap_locked(k_spinlock_कुंजी_t कुंजी)
{
	प्रत्यागम z_swap(&_sched_spinlock, कुंजी);
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
शून्य z_dummy_thread_init(संरचना k_thread *dummy_thread);

#endif /* ZEPHYR_KERNEL_INCLUDE_KSWAP_H_ */
