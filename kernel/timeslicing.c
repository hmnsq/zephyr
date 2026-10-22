/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/atomic.h>
#अन्तर्भाव <kspinlock.h>
#अन्तर्भाव <kswap.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <ipi.h>
#अन्तर्भाव <timeslicing.h>

स्थिर पूर्णाङ्क slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
स्थिर पूर्णाङ्क slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
स्थिर संरचना _timeout slice_timeouts[CONFIG_MP_MAX_अंक_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
स्थिर atomic_t slice_expired[CONFIG_MP_MAX_अंक_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
संरचना k_thread *pending_current;
#endif

स्थिर अन्तःस्थ पूर्णाङ्क slice_time(संरचना k_thread *thread)
{
	पूर्णाङ्क ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	यदि (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#अन्यथा
	ARG_UNUSED(thread);
#endif
	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क z_time_slice_size(संरचना k_thread *thread)
{
	यदि (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		प्रत्यागम 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	यदि (thread->base.slice_ticks != 0) {
		प्रत्यागम thread->base.slice_ticks;
	}
#endif

	यदि (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		प्रत्यागम slice_ticks;
	}

	प्रत्यागम 0;
}

स्थिर शून्य slice_timeout(संरचना _timeout *timeout)
{
	पूर्णाङ्क cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	यदि (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

स्थिर शून्य slice_reset(पूर्णाङ्क slice_size)
{
	पूर्णाङ्क cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(शून्य)z_try_abort_timeout(&slice_timeouts[cpu]);
	यदि (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		पूर्णाङ्क delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

शून्य z_time_slice_reset(संरचना k_thread *thread)
{
	slice_reset(z_time_slice_size(thread));
}

स्थिर ALWAYS_INLINE bool thread_defines_time_slice_size(संरचना k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	प्रत्यागम (thread->base.slice_ticks != 0);
#अन्यथा  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	प्रत्यागम false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

शून्य k_sched_time_slice_set(int32_t slice, पूर्णाङ्क prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		यदि (!thread_defines_time_slice_size(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
शून्य k_thread_time_slice_set(संरचना k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, शून्य *data)
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
शून्य z_time_slice(शून्य)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	यदि (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		प्रत्यागम;
	}

	k_spinlock_किल्ली_t किल्ली = z_sched_spinlock_lock();
	संरचना k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	यदि (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(किल्ली);
		प्रत्यागम;
	}
	pending_current = रिक्त;
#endif

	पूर्णाङ्क slice_size = 0;

	यदि (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	यदि (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		यदि (handler != रिक्त) {
			z_sched_spinlock_unlock(किल्ली);
			handler(curr, curr->base.slice_data);
			किल्ली = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_size = z_time_slice_size(curr);
		}
#endif
		यदि (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			संरचना k_thread *next = runq_best();

			यदि (next == रिक्त || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#अन्यथा
			यदि (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(किल्ली);
}
