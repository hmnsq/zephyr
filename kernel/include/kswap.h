/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_KSWAP_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_KSWAP_H_

#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <run_q.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/sys/barrier.h>
#అన్తర్భావ <kernel_arch_func.h>
#అన్తర్భావ <timeslicing.h>
#అన్తర్భావ <usage.h>

#నిర్వచితమైతే CONFIG_దొంతర_SENTINEL
బాహ్య శూన్య z_check_దొంతర_sentinel(శూన్య);
#లేకపోతే
#నిర్వచించు z_check_దొంతర_sentinel() /**/
#అయితే_ముగింపు /* CONFIG_STACK_SENTINEL */

/* In SMP, the irq_lock() is a spinlock which is implicitly released
 * and reacquired on context switch to preserve the existing
 * semantics.  This means that whenever we are about to return to a
 * thread (via either z_swap() or interrupt/exception return!) we need
 * to restore the lock state to whatever the thread's counter
 * expects.
 */
శూన్య z_smp_release_global_తాళం(నిర్మాణం k_thread *thread);

/* context switching and scheduling-related routines */
#నిర్వచితమైతే CONFIG_USE_SWITCH

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
స్థిర అంతఃవరుస శూన్య z_sched_switch_spin(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SMP
	అస్థిర శూన్య **shp = (శూన్య *)&thread->switch_handle;

	అయితే (*shp == ఖాళీ) {
		arch_spin_relax();
	}
	/* Read barrier: don't allow any subsequent loads in the
	 * calling code to reorder before we saw switch_handle go
	 * non-null.
	 */
	barrier_dmem_fence_నిండిన();
#అయితే_ముగింపు /* CONFIG_SMP */
}

/* New style context switching.  arch_switch() is a lower level
 * primitive that doesn't know about the scheduler or return value.
 * Needed for SMP, where the scheduler requires spinlocking that we
 * don't want to have to do in per-architecture assembly.
 *
 * Note that is_spinlock is a compile-time construct which will be
 * optimized out when this function is expanded.
 */
స్థిర ALWAYS_INLINE గుర్తులేని పూర్ణాంకం do_మార్పిడి(గుర్తులేని పూర్ణాంకం కీలకం,
					  నిర్మాణం k_spinlock *తాళం,
					  bool is_spinlock)
{
	నిర్మాణం k_thread *new_thread, *old_thread;

#నిర్వచితమైతే CONFIG_SPIN_VALIDATE
	z_assert_can_మార్పిడి(కీలకం, తాళం);
#అయితే_ముగింపు /* CONFIG_SPIN_VALIDATE */

	old_thread = _current;

	z_check_దొంతర_sentinel();

	old_thread->మార్పిడి_retval = -EAGAIN;

	/* We always take the scheduler spinlock if we don't already
	 * have it.  We "release" other spinlocks here.  But we never
	 * drop the interrupt lock.
	 */
	ఒకవేళ (is_spinlock && (తాళం != ఖాళీ) && !z_is_sched_spinlock(తాళం)) {
		k_spin_release(తాళం);
	}
	ఒకవేళ (IS_ENABLED(CONFIG_SMP) || IS_ENABLED(CONFIG_SPIN_VALIDATE)) {
		/* Taking a nested uniprocessor lock in void context is a noop */
		ఒకవేళ (!is_spinlock || !z_is_sched_spinlock(తాళం)) {
			(శూన్య)z_sched_spinlock_తాళం();
		}
	}

#నిర్వచితమైతే CONFIG_SMP
	new_thread = z_మార్పిడి_తదుపరి_thread();
#లేకపోతే
	new_thread = _kernel.ready_q.cache;
#అయితే_ముగింపు

	ఒకవేళ (new_thread != old_thread) {
		z_sched_usage_switch(new_thread);

#నిర్వచితమైతే CONFIG_SMP
		new_thread->పునాది.cpu = arch_curr_cpu()->id;

		ఒకవేళ (!is_spinlock) {
			z_smp_release_global_తాళం(new_thread);
		}
#అయితే_ముగింపు /* CONFIG_SMP */
		z_thread_mark_switched_బహిర్గతం();
		z_sched_switch_spin(new_thread);
		z_current_thread_సమితి(new_thread);

#నిర్వచితమైతే CONFIG_TIMESLICING
		z_సమయం_ముక్క_reset(new_thread);
#అయితే_ముగింపు /* CONFIG_TIMESLICING */

		z_sched_spinlock_transfer_owner(new_thread);

		arch_cohere_stacks(old_thread, ఖాళీ, new_thread);

#నిర్వచితమైతే CONFIG_SMP
		/* Now add _current back to the run queue, once we are
		 * guaranteed to reach the context switch in finite time.  See
		 * z_sched_switch_spin().  The current thread can never live in
		 * the run queue until we are inexorably on the context switch
		 * path on SMP, otherwise there is a deadlock condition where a
		 * set of CPUs pick a cycle of threads to run and wait for them
		 * all to context switch forever.
		 */
		ఒకవేళ (z_is_thread_queued(old_thread)) {
			runq_కూడు(old_thread);
		}
		signal_pending_ipi();
#అయితే_ముగింపు /* CONFIG_SMP */
		శూన్య *newsh = new_thread->switch_handle;

		ఒకవేళ (IS_ENABLED(CONFIG_SMP)) {
			/* Active threads must have a null here.  And
			 * it must be seen before the scheduler lock
			 * is released!
			 */
			new_thread->switch_handle = ఖాళీ;
			barrier_dmem_fence_నిండిన(); /* write barrier */
		}
		z_sched_spinlock_release();
		arch_switch(newsh, &old_thread->switch_handle);
	} లేకపోతే {
		z_sched_spinlock_release();
	}

	ఒకవేళ (is_spinlock) {
		arch_irq_unlock(కీలకం);
	} లేకపోతే {
		irq_unlock(కీలకం);
	}

	తిరిగి _current->మార్పిడి_retval;
}

స్థిర అంతఃవరుస పూర్ణాంకం z_మార్పిడి_irqlock(గుర్తులేని పూర్ణాంకం కీలకం)
{
	తిరిగి do_మార్పిడి(కీలకం, ఖాళీ, false);
}

స్థిర అంతఃవరుస పూర్ణాంకం z_మార్పిడి(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం)
{
	తిరిగి do_మార్పిడి(కీలకం.కీలకం, తాళం, true);
}

స్థిర అంతఃవరుస శూన్య z_మార్పిడి_unlocked(శూన్య)
{
	(శూన్య) do_మార్పిడి(arch_irq_తాళం(), ఖాళీ, true);
}

#లేకపోతే /* !CONFIG_USE_SWITCH */


స్థిర అంతఃవరుస శూన్య z_sched_switch_spin(నిర్మాణం k_thread *thread)
{
	ARG_వాడని(thread);
}

స్థిర అంతఃవరుస పూర్ణాంకం z_మార్పిడి_irqlock(గుర్తులేని పూర్ణాంకం కీలకం)
{
	పూర్ణాంకం ret;
	z_check_దొంతర_sentinel();

#నిర్వచితమైతే CONFIG_SPIN_VALIDATE
	z_assert_can_మార్పిడి(కీలకం, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_SPIN_VALIDATE */

	ret = arch_మార్పిడి(కీలకం);
	తిరిగి ret;
}

/* If !USE_SWITCH, then spinlocks are guaranteed degenerate as we
 * can't be in SMP.  The k_spin_release() call is just for validation
 * handling.
 */
స్థిర ALWAYS_INLINE పూర్ణాంకం z_మార్పిడి(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం)
{
	k_spin_release(తాళం);
	తిరిగి z_మార్పిడి_irqlock(కీలకం.కీలకం);
}

స్థిర అంతఃవరుస శూన్య z_మార్పిడి_unlocked(శూన్య)
{
	(శూన్య) z_మార్పిడి_irqlock(arch_irq_తాళం());
}

#అయితే_ముగింపు /* !CONFIG_USE_SWITCH */

/**
 * @brief - Like z_swap() but it is known that the scheduler's spinlock is already held.
 */
స్థిర అంతఃవరుస పూర్ణాంకం z_మార్పిడి_locked(k_spinlock_కీలకం_t కీలకం)
{
	తిరిగి z_మార్పిడి(&_sched_spinlock, కీలకం);
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
శూన్య z_dummy_thread_init(నిర్మాణం k_thread *dummy_thread);

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_KSWAP_H_ */
