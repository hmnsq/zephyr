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

स्थिर पूर्णाङ्क खण्डिका_ticks = DIV_आवर्तनम्_ऊर्ध्वम्(CONFIG_TIMESLICE_आकार * Z_HZ_ticks, Z_HZ_ms);
स्थिर पूर्णाङ्क खण्डिका_max_prio = CONFIG_TIMESLICE_प्राथमिकता;
स्थिर संरचना _timeout खण्डिका_timeouts[CONFIG_MP_MAX_अङ्कः_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
स्थिर atomic_t खण्डिका_expired[CONFIG_MP_MAX_अङ्कः_CPUS];

#यदि_परिभाषितम् CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
संरचना k_thread *pending_current;
#यदि_अन्तः

स्थिर अन्तःस्थ पूर्णाङ्क खण्डिका_time(संरचना k_thread *thread)
{
	पूर्णाङ्क ret = खण्डिका_ticks;

#यदि_परिभाषितम् CONFIG_TIMESLICE_प्रत्येकम्_THREAD
	यदि (thread->आधारः.खण्डिका_ticks != 0) {
		ret = thread->आधारः.खण्डिका_ticks;
	}
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः
	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क z_time_खण्डिका_आकार(संरचना k_thread *thread)
{
	यदि (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (खण्डिका_time(thread) == 0)) {
		प्रत्यागम 0;
	}

#यदि_परिभाषितम् CONFIG_TIMESLICE_प्रत्येकम्_THREAD
	यदि (thread->आधारः.खण्डिका_ticks != 0) {
		प्रत्यागम thread->आधारः.खण्डिका_ticks;
	}
#यदि_अन्तः

	यदि (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->आधारः.prio, खण्डिका_max_prio)) {
		प्रत्यागम खण्डिका_ticks;
	}

	प्रत्यागम 0;
}

स्थिर शून्य खण्डिका_timeout(संरचना _timeout *timeout)
{
	पूर्णाङ्क cpu = ARRAY_सूचकांक(खण्डिका_timeouts, timeout);

	atomic_समुच्चयः(&खण्डिका_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	यदि (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_आवरणम्(cpu));
	}
}

स्थिर शून्य खण्डिका_reset(पूर्णाङ्क खण्डिका_आकार)
{
	पूर्णाङ्क cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(शून्य)z_try_abort_timeout(&खण्डिका_timeouts[cpu]);
	यदि (खण्डिका_आकार != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		पूर्णाङ्क delay = atomic_get(&खण्डिका_expired[cpu]) ? खण्डिका_आकार - 1 : खण्डिका_आकार;

		z_सङ्कलय_timeout(&खण्डिका_timeouts[cpu], खण्डिका_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&खण्डिका_expired[cpu]);
}

शून्य z_time_खण्डिका_reset(संरचना k_thread *thread)
{
	खण्डिका_reset(z_time_खण्डिका_आकार(thread));
}

स्थिर ALWAYS_INLINE bool thread_परिभाषते_time_खण्डिका_आकार(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_TIMESLICE_प्रत्येकम्_THREAD
	प्रत्यागम (thread->आधारः.खण्डिका_ticks != 0);
#अन्यथा  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_अप्रयुक्तम्(thread);
	प्रत्यागम false;
#यदि_अन्तः /* !CONFIG_TIMESLICE_PER_THREAD */
}

शून्य k_sched_time_खण्डिका_समुच्चयः(int32_t खण्डिका, पूर्णाङ्क prio)
{
	Z_SCHED_SPINLOCK {

		खण्डिका_ticks = k_ms_गन्तव्यम्_ticks_ceil32(खण्डिका);
		खण्डिका_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		यदि (!thread_परिभाषते_time_खण्डिका_आकार(_current)) {
			z_time_खण्डिका_reset(_current);
		}
	}
}

#यदि_परिभाषितम् CONFIG_TIMESLICE_प्रत्येकम्_THREAD
शून्य k_thread_time_खण्डिका_समुच्चयः(संरचना k_thread *thread, int32_t thread_खण्डिका_ticks,
			     k_thread_timeslice_fn_t expired, शून्य *दत्त)
{
	Z_SCHED_SPINLOCK {
		thread->आधारः.खण्डिका_ticks = thread_खण्डिका_ticks;
		thread->आधारः.खण्डिका_expired = expired;
		thread->आधारः.खण्डिका_दत्त = दत्त;
		z_time_खण्डिका_reset(thread);
	}
}
#यदि_अन्तः

/* Called out of each timer and IPI interrupt */
शून्य z_time_खण्डिका(शून्य)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	यदि (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&खण्डिका_expired[_current_cpu->id])) {
		प्रत्यागम;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_तालकम्();
	संरचना k_thread *curr = _current;

#यदि_परिभाषितम् CONFIG_SWAP_NONATOMIC
	यदि (pending_current == curr) {
		z_time_खण्डिका_reset(curr);
		z_sched_spinlock_unlock(कुञ्चिका);
		प्रत्यागम;
	}
	pending_current = रिक्त;
#यदि_अन्तः

	पूर्णाङ्क खण्डिका_आकार = 0;

	यदि (atomic_get(&खण्डिका_expired[_current_cpu->id])) {
		खण्डिका_आकार = z_time_खण्डिका_आकार(curr);
	}

	यदि (खण्डिका_आकार != 0) {
#यदि_परिभाषितम् CONFIG_TIMESLICE_प्रत्येकम्_THREAD
		k_thread_timeslice_fn_t handler = curr->आधारः.खण्डिका_expired;

		यदि (handler != रिक्त) {
			z_sched_spinlock_unlock(कुञ्चिका);
			handler(curr, curr->आधारः.खण्डिका_दत्त);
			कुञ्चिका = z_sched_spinlock_तालकम्();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			खण्डिका_आकार = z_time_खण्डिका_आकार(curr);
		}
#यदि_अन्तः
		यदि (!z_is_thread_prevented_from_running(curr)) {
			move_current_गन्तव्यम्_अन्तः_सम्बन्धि_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#यदि_परिभाषितम् CONFIG_SMP
			संरचना k_thread *अग्र = runq_best();

			यदि (अग्र == रिक्त || z_is_idle_thread_object(अग्र)) {
				खण्डिका_reset(खण्डिका_आकार);
			}
#अन्यथा
			यदि (_kernel.ready_q.cache == curr) {
				खण्डिका_reset(खण्डिका_आकार);
			}
#यदि_अन्तः
		}
	}
	z_sched_spinlock_unlock(कुञ्चिका);
}
