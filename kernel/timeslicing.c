/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/sys/atomic.h>
#समावेश <kspinlock.h>
#समावेश <kswap.h>
#समावेश <ksched.h>
#समावेश <ipi.h>
#समावेश <timeslicing.h>

अटल पूर्णांक slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_आकार * Z_HZ_ticks, Z_HZ_ms);
अटल पूर्णांक slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
अटल काष्ठा _timeout slice_timeouts[CONFIG_MP_MAX_अंक_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
अटल atomic_t slice_expired[CONFIG_MP_MAX_अंक_CPUS];

#अगर_घोषित CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
काष्ठा k_thread *pending_current;
#पूर्ण_अगर

अटल अंतरभूत पूर्णांक slice_time(काष्ठा k_thread *thread)
{
	पूर्णांक ret = slice_ticks;

#अगर_घोषित CONFIG_TIMESLICE_PER_THREAD
	अगर (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर
	वापस ret;
}

अटल पूर्णांक z_time_slice_आकार(काष्ठा k_thread *thread)
{
	अगर (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		वापस 0;
	}

#अगर_घोषित CONFIG_TIMESLICE_PER_THREAD
	अगर (thread->base.slice_ticks != 0) {
		वापस thread->base.slice_ticks;
	}
#पूर्ण_अगर

	अगर (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		वापस slice_ticks;
	}

	वापस 0;
}

अटल व्योम slice_timeout(काष्ठा _timeout *timeout)
{
	पूर्णांक cpu = ARRAY_सूचकांक(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	अगर (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

अटल व्योम slice_reset(पूर्णांक slice_आकार)
{
	पूर्णांक cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(व्योम)z_try_abort_timeout(&slice_timeouts[cpu]);
	अगर (slice_आकार != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		पूर्णांक delay = atomic_get(&slice_expired[cpu]) ? slice_आकार - 1 : slice_आकार;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

व्योम z_time_slice_reset(काष्ठा k_thread *thread)
{
	slice_reset(z_time_slice_आकार(thread));
}

अटल ALWAYS_INLINE bool thread_defines_time_slice_आकार(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_TIMESLICE_PER_THREAD
	वापस (thread->base.slice_ticks != 0);
#अन्यथा  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	वापस false;
#पूर्ण_अगर /* !CONFIG_TIMESLICE_PER_THREAD */
}

व्योम k_sched_time_slice_set(int32_t slice, पूर्णांक prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		अगर (!thread_defines_time_slice_आकार(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#अगर_घोषित CONFIG_TIMESLICE_PER_THREAD
व्योम k_thread_time_slice_set(काष्ठा k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, व्योम *तथ्य)
{
	Z_SCHED_SPINLOCK {
		thread->base.slice_ticks = thread_slice_ticks;
		thread->base.slice_expired = expired;
		thread->base.slice_तथ्य = तथ्य;
		z_time_slice_reset(thread);
	}
}
#पूर्ण_अगर

/* Called out of each timer and IPI interrupt */
व्योम z_time_slice(व्योम)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	अगर (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		वापस;
	}

	k_spinlock_कुंजी_t कुंजी = z_sched_spinlock_lock();
	काष्ठा k_thread *curr = _current;

#अगर_घोषित CONFIG_SWAP_NONATOMIC
	अगर (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(कुंजी);
		वापस;
	}
	pending_current = शून्य;
#पूर्ण_अगर

	पूर्णांक slice_आकार = 0;

	अगर (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_आकार = z_time_slice_आकार(curr);
	}

	अगर (slice_आकार != 0) {
#अगर_घोषित CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		अगर (handler != शून्य) {
			z_sched_spinlock_unlock(कुंजी);
			handler(curr, curr->base.slice_तथ्य);
			कुंजी = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_आकार = z_time_slice_आकार(curr);
		}
#पूर्ण_अगर
		अगर (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#अगर_घोषित CONFIG_SMP
			काष्ठा k_thread *अग्र = runq_best();

			अगर (अग्र == शून्य || z_is_idle_thread_object(अग्र)) {
				slice_reset(slice_आकार);
			}
#अन्यथा
			अगर (_kernel.ready_q.cache == curr) {
				slice_reset(slice_आकार);
			}
#पूर्ण_अगर
		}
	}
	z_sched_spinlock_unlock(कुंजी);
}
