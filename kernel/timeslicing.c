/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/sys/atomic.h>
#அந்தர்பாவ <kspinlock.h>
#அந்தர்பாவ <kswap.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <ipi.h>
#அந்தர்பாவ <timeslicing.h>

நிலையான முழு slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
நிலையான முழு slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
நிலையான கட்டமைப்பு _timeout slice_timeouts[CONFIG_MP_MAX_இலக்கம்_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
நிலையான atomic_t slice_expired[CONFIG_MP_MAX_இலக்கம்_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
கட்டமைப்பு k_thread *pending_current;
#endif

நிலையான உள்வரி முழு slice_time(கட்டமைப்பு k_thread *thread)
{
	முழு ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	எனில் (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif
	திரும்பு ret;
}

நிலையான முழு z_time_slice_size(கட்டமைப்பு k_thread *thread)
{
	எனில் (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		திரும்பு 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	எனில் (thread->base.slice_ticks != 0) {
		திரும்பு thread->base.slice_ticks;
	}
#endif

	எனில் (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		திரும்பு slice_ticks;
	}

	திரும்பு 0;
}

நிலையான வெறுமை slice_timeout(கட்டமைப்பு _timeout *timeout)
{
	முழு cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	எனில் (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

நிலையான வெறுமை slice_reset(முழு slice_size)
{
	முழு cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(வெறுமை)z_try_abort_timeout(&slice_timeouts[cpu]);
	எனில் (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		முழு delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

வெறுமை z_time_slice_reset(கட்டமைப்பு k_thread *thread)
{
	slice_reset(z_time_slice_size(thread));
}

நிலையான ALWAYS_INLINE bool thread_defines_time_slice_size(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	திரும்பு (thread->base.slice_ticks != 0);
#இல்லையெனில்  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	திரும்பு false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

வெறுமை k_sched_time_slice_set(int32_t slice, முழு prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		எனில் (!thread_defines_time_slice_size(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
வெறுமை k_thread_time_slice_set(கட்டமைப்பு k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, வெறுமை *data)
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
வெறுமை z_time_slice(வெறுமை)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	எனில் (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		திரும்பு;
	}

	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();
	கட்டமைப்பு k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	எனில் (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(சாவி);
		திரும்பு;
	}
	pending_current = வெற்று;
#endif

	முழு slice_size = 0;

	எனில் (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	எனில் (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		எனில் (handler != வெற்று) {
			z_sched_spinlock_unlock(சாவி);
			handler(curr, curr->base.slice_data);
			சாவி = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_size = z_time_slice_size(curr);
		}
#endif
		எனில் (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			கட்டமைப்பு k_thread *next = runq_best();

			எனில் (next == வெற்று || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#இல்லையெனில்
			எனில் (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(சாவி);
}
