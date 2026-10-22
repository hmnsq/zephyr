/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/sys/atomic.h>
#അന്തര്ഭാവ <kspinlock.h>
#അന്തര്ഭാവ <kswap.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <ipi.h>
#അന്തര്ഭാവ <timeslicing.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
സ്ഥിതം പൂർണ്ണസംഖ്യ slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
സ്ഥിതം ഘടന _timeout slice_timeouts[CONFIG_MP_MAX_അക്കം_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
സ്ഥിതം atomic_t slice_expired[CONFIG_MP_MAX_അക്കം_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
ഘടന k_thread *pending_current;
#endif

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ slice_time(ഘടന k_thread *thread)
{
	പൂർണ്ണസംഖ്യ ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	എങ്കിൽ (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif
	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ z_time_slice_size(ഘടന k_thread *thread)
{
	എങ്കിൽ (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		തിരികെ 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	എങ്കിൽ (thread->base.slice_ticks != 0) {
		തിരികെ thread->base.slice_ticks;
	}
#endif

	എങ്കിൽ (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		തിരികെ slice_ticks;
	}

	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം slice_timeout(ഘടന _timeout *timeout)
{
	പൂർണ്ണസംഖ്യ cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	എങ്കിൽ (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

സ്ഥിതം ശൂന്യം slice_reset(പൂർണ്ണസംഖ്യ slice_size)
{
	പൂർണ്ണസംഖ്യ cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(ശൂന്യം)z_try_abort_timeout(&slice_timeouts[cpu]);
	എങ്കിൽ (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		പൂർണ്ണസംഖ്യ delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

ശൂന്യം z_time_slice_reset(ഘടന k_thread *thread)
{
	slice_reset(z_time_slice_size(thread));
}

സ്ഥിതം ALWAYS_INLINE bool thread_defines_time_slice_size(ഘടന k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	തിരികെ (thread->base.slice_ticks != 0);
#അല്ലെങ്കിൽ  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	തിരികെ false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

ശൂന്യം k_sched_time_slice_set(int32_t slice, പൂർണ്ണസംഖ്യ prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		എങ്കിൽ (!thread_defines_time_slice_size(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
ശൂന്യം k_thread_time_slice_set(ഘടന k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, ശൂന്യം *data)
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
ശൂന്യം z_time_slice(ശൂന്യം)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	എങ്കിൽ (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		തിരികെ;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();
	ഘടന k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	എങ്കിൽ (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(താക്കോൽ);
		തിരികെ;
	}
	pending_current = ശൂന്യ;
#endif

	പൂർണ്ണസംഖ്യ slice_size = 0;

	എങ്കിൽ (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	എങ്കിൽ (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		എങ്കിൽ (handler != ശൂന്യ) {
			z_sched_spinlock_unlock(താക്കോൽ);
			handler(curr, curr->base.slice_data);
			താക്കോൽ = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_size = z_time_slice_size(curr);
		}
#endif
		എങ്കിൽ (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			ഘടന k_thread *next = runq_best();

			എങ്കിൽ (next == ശൂന്യ || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#അല്ലെങ്കിൽ
			എങ്കിൽ (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(താക്കോൽ);
}
