/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_KSWAP_H_
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_KSWAP_H_

#ضمن <kspinlock.h>
#ضمن <ksched.h>
#ضمن <run_q.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/sys/barrier.h>
#ضمن <kernel_arch_func.h>
#ضمن <timeslicing.h>
#ضمن <usage.h>

#جيڪڏهن_وصف_ٿيل CONFIG_ڍير_SENTINEL
خارجي فراغ z_check_ڍير_sentinel(فراغ);
#وإلا
#وصف_ڪريو z_check_ڍير_sentinel() /**/
#جيڪڏهن_پورو /* CONFIG_STACK_SENTINEL */

/* In SMP, the irq_lock() is a spinlock which is implicitly released
 * and reacquired on context switch to preserve the existing
 * semantics.  This means that whenever we are about to return to a
 * thread (via either z_swap() or interrupt/exception return!) we need
 * to restore the lock state to whatever the thread's counter
 * expects.
 */
فراغ z_smp_release_global_تالو(بنية k_thread *thread);

/* context switching and scheduling-related routines */
#جيڪڏهن_وصف_ٿيل CONFIG_USE_SWITCH

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
ساكن مضمن فراغ z_sched_switch_spin(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	متطاير فراغ **shp = (فراغ *)&thread->switch_handle;

	طالما (*shp == عدم) {
		arch_spin_relax();
	}
	/* Read barrier: don't allow any subsequent loads in the
	 * calling code to reorder before we saw switch_handle go
	 * non-null.
	 */
	barrier_dmem_fence_ڀريل();
#جيڪڏهن_پورو /* CONFIG_SMP */
}

/* New style context switching.  arch_switch() is a lower level
 * primitive that doesn't know about the scheduler or return value.
 * Needed for SMP, where the scheduler requires spinlocking that we
 * don't want to have to do in per-architecture assembly.
 *
 * Note that is_spinlock is a compile-time construct which will be
 * optimized out when this function is expanded.
 */
ساكن ALWAYS_INLINE بنانشان صحيح do_مٽا_سٽا(بنانشان صحيح ڪنجي,
					  بنية k_spinlock *تالو,
					  bool is_spinlock)
{
	بنية k_thread *new_thread, *old_thread;

#جيڪڏهن_وصف_ٿيل CONFIG_SPIN_VALIDATE
	z_assert_can_مٽا_سٽا(ڪنجي, تالو);
#جيڪڏهن_پورو /* CONFIG_SPIN_VALIDATE */

	old_thread = _current;

	z_check_ڍير_sentinel();

	old_thread->مٽا_سٽا_retval = -EAGAIN;

	/* We always take the scheduler spinlock if we don't already
	 * have it.  We "release" other spinlocks here.  But we never
	 * drop the interrupt lock.
	 */
	إذا (is_spinlock && (تالو != عدم) && !z_is_sched_spinlock(تالو)) {
		k_spin_release(تالو);
	}
	إذا (IS_ENABLED(CONFIG_SMP) || IS_ENABLED(CONFIG_SPIN_VALIDATE)) {
		/* Taking a nested uniprocessor lock in void context is a noop */
		إذا (!is_spinlock || !z_is_sched_spinlock(تالو)) {
			(فراغ)z_sched_spinlock_تالو();
		}
	}

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	new_thread = z_مٽا_سٽا_ايندڙ_thread();
#وإلا
	new_thread = _kernel.ready_q.cache;
#جيڪڏهن_پورو

	إذا (new_thread != old_thread) {
		z_sched_usage_switch(new_thread);

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
		new_thread->بنياد.cpu = arch_curr_cpu()->id;

		إذا (!is_spinlock) {
			z_smp_release_global_تالو(new_thread);
		}
#جيڪڏهن_پورو /* CONFIG_SMP */
		z_thread_mark_switched_ٻاهريون();
		z_sched_switch_spin(new_thread);
		z_current_thread_سيٽ(new_thread);

#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICING
		z_وقت_ٽڪرو_reset(new_thread);
#جيڪڏهن_پورو /* CONFIG_TIMESLICING */

		z_sched_spinlock_transfer_owner(new_thread);

		arch_cohere_stacks(old_thread, عدم, new_thread);

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
		/* Now add _current back to the run queue, once we are
		 * guaranteed to reach the context switch in finite time.  See
		 * z_sched_switch_spin().  The current thread can never live in
		 * the run queue until we are inexorably on the context switch
		 * path on SMP, otherwise there is a deadlock condition where a
		 * set of CPUs pick a cycle of threads to run and wait for them
		 * all to context switch forever.
		 */
		إذا (z_is_thread_queued(old_thread)) {
			runq_جوڙيو(old_thread);
		}
		signal_pending_ipi();
#جيڪڏهن_پورو /* CONFIG_SMP */
		فراغ *newsh = new_thread->switch_handle;

		إذا (IS_ENABLED(CONFIG_SMP)) {
			/* Active threads must have a null here.  And
			 * it must be seen before the scheduler lock
			 * is released!
			 */
			new_thread->switch_handle = عدم;
			barrier_dmem_fence_ڀريل(); /* write barrier */
		}
		z_sched_spinlock_release();
		arch_switch(newsh, &old_thread->switch_handle);
	} وإلا {
		z_sched_spinlock_release();
	}

	إذا (is_spinlock) {
		arch_irq_unlock(ڪنجي);
	} وإلا {
		irq_unlock(ڪنجي);
	}

	أرجع _current->مٽا_سٽا_retval;
}

ساكن مضمن صحيح z_مٽا_سٽا_irqlock(بنانشان صحيح ڪنجي)
{
	أرجع do_مٽا_سٽا(ڪنجي, عدم, false);
}

ساكن مضمن صحيح z_مٽا_سٽا(بنية k_spinlock *تالو, k_spinlock_ڪنجي_t ڪنجي)
{
	أرجع do_مٽا_سٽا(ڪنجي.ڪنجي, تالو, true);
}

ساكن مضمن فراغ z_مٽا_سٽا_unlocked(فراغ)
{
	(فراغ) do_مٽا_سٽا(arch_irq_تالو(), عدم, true);
}

#وإلا /* !CONFIG_USE_SWITCH */


ساكن مضمن فراغ z_sched_switch_spin(بنية k_thread *thread)
{
	ARG_اڻ_استعمال_ٿيل(thread);
}

ساكن مضمن صحيح z_مٽا_سٽا_irqlock(بنانشان صحيح ڪنجي)
{
	صحيح ret;
	z_check_ڍير_sentinel();

#جيڪڏهن_وصف_ٿيل CONFIG_SPIN_VALIDATE
	z_assert_can_مٽا_سٽا(ڪنجي, عدم);
#جيڪڏهن_پورو /* CONFIG_SPIN_VALIDATE */

	ret = arch_مٽا_سٽا(ڪنجي);
	أرجع ret;
}

/* If !USE_SWITCH, then spinlocks are guaranteed degenerate as we
 * can't be in SMP.  The k_spin_release() call is just for validation
 * handling.
 */
ساكن ALWAYS_INLINE صحيح z_مٽا_سٽا(بنية k_spinlock *تالو, k_spinlock_ڪنجي_t ڪنجي)
{
	k_spin_release(تالو);
	أرجع z_مٽا_سٽا_irqlock(ڪنجي.ڪنجي);
}

ساكن مضمن فراغ z_مٽا_سٽا_unlocked(فراغ)
{
	(فراغ) z_مٽا_سٽا_irqlock(arch_irq_تالو());
}

#جيڪڏهن_پورو /* !CONFIG_USE_SWITCH */

/**
 * @brief - Like z_swap() but it is known that the scheduler's spinlock is already held.
 */
ساكن مضمن صحيح z_مٽا_سٽا_locked(k_spinlock_ڪنجي_t ڪنجي)
{
	أرجع z_مٽا_سٽا(&_sched_spinlock, ڪنجي);
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
فراغ z_dummy_thread_init(بنية k_thread *dummy_thread);

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_KSWAP_H_ */
