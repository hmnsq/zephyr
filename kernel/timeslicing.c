/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/sys/atomic.h>
#અન્તર્ભાવ <kspinlock.h>
#અન્તર્ભાવ <kswap.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <ipi.h>
#અન્તર્ભાવ <timeslicing.h>

સ્થિર પૂર્ણાંક slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
સ્થિર પૂર્ણાંક slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
સ્થિર સંરચના _timeout slice_timeouts[CONFIG_MP_MAX_અંક_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
સ્થિર atomic_t slice_expired[CONFIG_MP_MAX_અંક_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
સંરચના k_thread *pending_current;
#endif

સ્થિર અંતઃરેખ પૂર્ણાંક slice_time(સંરચના k_thread *thread)
{
	પૂર્ણાંક ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	જો (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#નહીંતર
	ARG_UNUSED(thread);
#endif
	પાછા ret;
}

સ્થિર પૂર્ણાંક z_time_slice_size(સંરચના k_thread *thread)
{
	જો (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		પાછા 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	જો (thread->base.slice_ticks != 0) {
		પાછા thread->base.slice_ticks;
	}
#endif

	જો (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		પાછા slice_ticks;
	}

	પાછા 0;
}

સ્થિર શૂન્ય slice_timeout(સંરચના _timeout *timeout)
{
	પૂર્ણાંક cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	જો (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

સ્થિર શૂન્ય slice_reset(પૂર્ણાંક slice_size)
{
	પૂર્ણાંક cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(શૂન્ય)z_try_abort_timeout(&slice_timeouts[cpu]);
	જો (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		પૂર્ણાંક delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

શૂન્ય z_time_slice_reset(સંરચના k_thread *thread)
{
	slice_reset(z_time_slice_size(thread));
}

સ્થિર ALWAYS_INLINE bool thread_defines_time_slice_size(સંરચના k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	પાછા (thread->base.slice_ticks != 0);
#નહીંતર  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	પાછા false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

શૂન્ય k_sched_time_slice_set(int32_t slice, પૂર્ણાંક prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		જો (!thread_defines_time_slice_size(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
શૂન્ય k_thread_time_slice_set(સંરચના k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, શૂન્ય *data)
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
શૂન્ય z_time_slice(શૂન્ય)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	જો (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		પાછા;
	}

	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();
	સંરચના k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	જો (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(ચાવી);
		પાછા;
	}
	pending_current = ખાલી;
#endif

	પૂર્ણાંક slice_size = 0;

	જો (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	જો (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		જો (handler != ખાલી) {
			z_sched_spinlock_unlock(ચાવી);
			handler(curr, curr->base.slice_data);
			ચાવી = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_size = z_time_slice_size(curr);
		}
#endif
		જો (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			સંરચના k_thread *next = runq_best();

			જો (next == ખાલી || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#નહીંતર
			જો (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(ચાવી);
}
