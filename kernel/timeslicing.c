/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/sys/atomic.h>
#অন্তর্ভাব <kspinlock.h>
#অন্তর্ভাব <kswap.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <ipi.h>
#অন্তর্ভাব <timeslicing.h>

স্থির পূর্ণসংখ্যা slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_আকার * Z_HZ_ticks, Z_HZ_ms);
স্থির পূর্ণসংখ্যা slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
স্থির কাঠামো _timeout slice_timeouts[CONFIG_MP_MAX_অঙ্ক_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
স্থির atomic_t slice_expired[CONFIG_MP_MAX_অঙ্ক_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
কাঠামো k_thread *pending_current;
#endif

স্থির অন্তঃরেখ পূর্ণসংখ্যা slice_time(কাঠামো k_thread *thread)
{
	পূর্ণসংখ্যা ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	যদি (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#অন্যথা
	ARG_UNUSED(thread);
#endif
	ফেরত ret;
}

স্থির পূর্ণসংখ্যা z_time_slice_আকার(কাঠামো k_thread *thread)
{
	যদি (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		ফেরত 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	যদি (thread->base.slice_ticks != 0) {
		ফেরত thread->base.slice_ticks;
	}
#endif

	যদি (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		ফেরত slice_ticks;
	}

	ফেরত 0;
}

স্থির শূন্য slice_timeout(কাঠামো _timeout *timeout)
{
	পূর্ণসংখ্যা cpu = ARRAY_সূচক(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	যদি (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

স্থির শূন্য slice_reset(পূর্ণসংখ্যা slice_আকার)
{
	পূর্ণসংখ্যা cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(শূন্য)z_try_abort_timeout(&slice_timeouts[cpu]);
	যদি (slice_আকার != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		পূর্ণসংখ্যা delay = atomic_get(&slice_expired[cpu]) ? slice_আকার - 1 : slice_আকার;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

শূন্য z_time_slice_reset(কাঠামো k_thread *thread)
{
	slice_reset(z_time_slice_আকার(thread));
}

স্থির ALWAYS_INLINE bool thread_defines_time_slice_আকার(কাঠামো k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	ফেরত (thread->base.slice_ticks != 0);
#অন্যথা  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	ফেরত false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

শূন্য k_sched_time_slice_set(int32_t slice, পূর্ণসংখ্যা prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		যদি (!thread_defines_time_slice_আকার(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
শূন্য k_thread_time_slice_set(কাঠামো k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, শূন্য *তথ্য)
{
	Z_SCHED_SPINLOCK {
		thread->base.slice_ticks = thread_slice_ticks;
		thread->base.slice_expired = expired;
		thread->base.slice_তথ্য = তথ্য;
		z_time_slice_reset(thread);
	}
}
#endif

/* Called out of each timer and IPI interrupt */
শূন্য z_time_slice(শূন্য)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	যদি (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		ফেরত;
	}

	k_spinlock_চাবি_t চাবি = z_sched_spinlock_lock();
	কাঠামো k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	যদি (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(চাবি);
		ফেরত;
	}
	pending_current = ফাঁকা;
#endif

	পূর্ণসংখ্যা slice_আকার = 0;

	যদি (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_আকার = z_time_slice_আকার(curr);
	}

	যদি (slice_আকার != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		যদি (handler != ফাঁকা) {
			z_sched_spinlock_unlock(চাবি);
			handler(curr, curr->base.slice_তথ্য);
			চাবি = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_আকার = z_time_slice_আকার(curr);
		}
#endif
		যদি (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			কাঠামো k_thread *পরবর্তী = runq_best();

			যদি (পরবর্তী == ফাঁকা || z_is_idle_thread_object(পরবর্তী)) {
				slice_reset(slice_আকার);
			}
#অন্যথা
			যদি (_kernel.ready_q.cache == curr) {
				slice_reset(slice_আকার);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(চাবি);
}
