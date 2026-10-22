/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>
#ಅನ್ತರ್ಭಾವ <timeslicing.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
ಸ್ಥಿರ ಸಂರಚನಾ _timeout slice_timeouts[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ಸ್ಥಿರ atomic_t slice_expired[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
ಸಂರಚನಾ k_thread *pending_current;
#endif

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ slice_time(ಸಂರಚನಾ k_thread *thread)
{
	ಪೂರ್ಣಾಙ್ಕ ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	ಯದಿ (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ z_time_slice_size(ಸಂರಚನಾ k_thread *thread)
{
	ಯದಿ (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	ಯದಿ (thread->base.slice_ticks != 0) {
		ಪ್ರತ್ಯಾಗಮ thread->base.slice_ticks;
	}
#endif

	ಯದಿ (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		ಪ್ರತ್ಯಾಗಮ slice_ticks;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ slice_timeout(ಸಂರಚನಾ _timeout *timeout)
{
	ಪೂರ್ಣಾಙ್ಕ cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	ಯದಿ (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ slice_reset(ಪೂರ್ಣಾಙ್ಕ slice_size)
{
	ಪೂರ್ಣಾಙ್ಕ cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(ಶೂನ್ಯ)z_try_abort_timeout(&slice_timeouts[cpu]);
	ಯದಿ (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		ಪೂರ್ಣಾಙ್ಕ delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

ಶೂನ್ಯ z_time_slice_reset(ಸಂರಚನಾ k_thread *thread)
{
	slice_reset(z_time_slice_size(thread));
}

ಸ್ಥಿರ ALWAYS_INLINE bool thread_defines_time_slice_size(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	ಪ್ರತ್ಯಾಗಮ (thread->base.slice_ticks != 0);
#ಅನ್ಯಥಾ  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	ಪ್ರತ್ಯಾಗಮ false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

ಶೂನ್ಯ k_sched_time_slice_set(int32_t slice, ಪೂರ್ಣಾಙ್ಕ prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		ಯದಿ (!thread_defines_time_slice_size(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
ಶೂನ್ಯ k_thread_time_slice_set(ಸಂರಚನಾ k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, ಶೂನ್ಯ *data)
{
	Z_SCHED_SPINLOCK {
		thread->base.slice_ticks = thread_slice_ticks;
		thread->base.slice_expired = expired;
		thread->base.slice_data = data;
		z_time_slice_reset(thread);
	}
}
#endif

/* Called out of each timer and IPI interrupt */
ಶೂನ್ಯ z_time_slice(ಶೂನ್ಯ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	ಯದಿ (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_lock();
	ಸಂರಚನಾ k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	ಯದಿ (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}
	pending_current = ರಿಕ್ತ;
#endif

	ಪೂರ್ಣಾಙ್ಕ slice_size = 0;

	ಯದಿ (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	ಯದಿ (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		ಯದಿ (handler != ರಿಕ್ತ) {
			z_sched_spinlock_unlock(ಕೀಲಿ);
			handler(curr, curr->base.slice_data);
			ಕೀಲಿ = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_size = z_time_slice_size(curr);
		}
#endif
		ಯದಿ (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			ಸಂರಚನಾ k_thread *next = runq_best();

			ಯದಿ (next == ರಿಕ್ತ || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#ಅನ್ಯಥಾ
			ಯದಿ (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(ಕೀಲಿ);
}
