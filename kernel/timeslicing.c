/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/atomic.h>
#ଅନ୍ତର୍ଭାଵ <kspinlock.h>
#ଅନ୍ତର୍ଭାଵ <kswap.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <ipi.h>
#ଅନ୍ତର୍ଭାଵ <timeslicing.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
ସ୍ଥିର ସଂରଚନା _timeout slice_timeouts[CONFIG_MP_MAX_ଅଙ୍କ_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ସ୍ଥିର atomic_t slice_expired[CONFIG_MP_MAX_ଅଙ୍କ_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
ସଂରଚନା k_thread *pending_current;
#endif

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା slice_time(ସଂରଚନା k_thread *thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	ଯଦି (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_time_slice_size(ସଂରଚନା k_thread *thread)
{
	ଯଦି (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		ଫେରନ୍ତୁ 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	ଯଦି (thread->base.slice_ticks != 0) {
		ଫେରନ୍ତୁ thread->base.slice_ticks;
	}
#endif

	ଯଦି (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		ଫେରନ୍ତୁ slice_ticks;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ slice_timeout(ସଂରଚନା _timeout *timeout)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	ଯଦି (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

ସ୍ଥିର ଶୂନ୍ୟ slice_reset(ପୂର୍ଣ୍ଣସଂଖ୍ୟା slice_size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(ଶୂନ୍ୟ)z_try_abort_timeout(&slice_timeouts[cpu]);
	ଯଦି (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

ଶୂନ୍ୟ z_time_slice_reset(ସଂରଚନା k_thread *thread)
{
	slice_reset(z_time_slice_size(thread));
}

ସ୍ଥିର ALWAYS_INLINE bool thread_defines_time_slice_size(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	ଫେରନ୍ତୁ (thread->base.slice_ticks != 0);
#ଅନ୍ୟଥା  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	ଫେରନ୍ତୁ false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

ଶୂନ୍ୟ k_sched_time_slice_set(int32_t slice, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		ଯଦି (!thread_defines_time_slice_size(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
ଶୂନ୍ୟ k_thread_time_slice_set(ସଂରଚନା k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, ଶୂନ୍ୟ *data)
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
ଶୂନ୍ୟ z_time_slice(ଶୂନ୍ୟ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	ଯଦି (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		ଫେରନ୍ତୁ;
	}

	k_spinlock_ଚାବି_t ଚାବି = z_sched_spinlock_lock();
	ସଂରଚନା k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	ଯଦି (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(ଚାବି);
		ଫେରନ୍ତୁ;
	}
	pending_current = ଖାଲି;
#endif

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା slice_size = 0;

	ଯଦି (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	ଯଦି (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		ଯଦି (handler != ଖାଲି) {
			z_sched_spinlock_unlock(ଚାବି);
			handler(curr, curr->base.slice_data);
			ଚାବି = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_size = z_time_slice_size(curr);
		}
#endif
		ଯଦି (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			ସଂରଚନା k_thread *next = runq_best();

			ଯଦି (next == ଖାଲି || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#ଅନ୍ୟଥା
			ଯଦି (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(ଚାବି);
}
