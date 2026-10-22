/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <kspinlock.h>
#include <kswap.h>
#include <ksched.h>
#include <ipi.h>
#include <timeslicing.h>

static පූර්ණ_සංඛ්‍යා slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
static පූර්ණ_සංඛ්‍යා slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
static sa_nracanaa _timeout slice_timeouts[CONFIG_MP_MAX_NUM_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
static atomic_t slice_expired[CONFIG_MP_MAX_NUM_CPUS];

#ifdef CONFIG_SWAP_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
sa_nracanaa k_thread *pending_current;
#endif

static anta_hstha පූර්ණ_සංඛ්‍යා slice_time(sa_nracanaa k_thread *thread)
{
	පූර්ණ_සංඛ්‍යා ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	නම් (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#නැතහොත්
	ARG_UNUSED(thread);
#endif
	ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා z_time_slice_size(sa_nracanaa k_thread *thread)
{
	නම් (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		ආපසු 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	නම් (thread->base.slice_ticks != 0) {
		ආපසු thread->base.slice_ticks;
	}
#endif

	නම් (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		ආපසු slice_ticks;
	}

	ආපසු 0;
}

static void slice_timeout(sa_nracanaa _timeout *timeout)
{
	පූර්ණ_සංඛ්‍යා cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	නම් (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

static void slice_reset(පූර්ණ_සංඛ්‍යා slice_size)
{
	පූර්ණ_සංඛ්‍යා cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(void)z_try_abort_timeout(&slice_timeouts[cpu]);
	නම් (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		පූර්ණ_සංඛ්‍යා delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

		z_add_timeout(&slice_timeouts[cpu], slice_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&slice_expired[cpu]);
}

void z_time_slice_reset(sa_nracanaa k_thread *thread)
{
	slice_reset(z_time_slice_size(thread));
}

static ALWAYS_INLINE bool thread_defines_time_slice_size(sa_nracanaa k_thread *thread)
{
#ifdef CONFIG_TIMESLICE_PER_THREAD
	ආපසු (thread->base.slice_ticks != 0);
#නැතහොත්  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	ආපසු false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

void k_sched_time_slice_set(int32_t slice, පූර්ණ_සංඛ්‍යා prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		නම් (!thread_defines_time_slice_size(_current)) {
			z_time_slice_reset(_current);
		}
	}
}

#ifdef CONFIG_TIMESLICE_PER_THREAD
void k_thread_time_slice_set(sa_nracanaa k_thread *thread, int32_t thread_slice_ticks,
			     k_thread_timeslice_fn_t expired, void *data)
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
void z_time_slice(void)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	නම් (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		ආපසු;
	}

	k_spinlock_key_t key = z_sched_spinlock_lock();
	sa_nracanaa k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	නම් (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(key);
		ආපසු;
	}
	pending_current = NULL;
#endif

	පූර්ණ_සංඛ්‍යා slice_size = 0;

	නම් (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	නම් (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		නම් (handler != NULL) {
			z_sched_spinlock_unlock(key);
			handler(curr, curr->base.slice_data);
			key = z_sched_spinlock_lock();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			slice_size = z_time_slice_size(curr);
		}
#endif
		නම් (!z_is_thread_prevented_from_running(curr)) {
			move_current_to_end_of_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ifdef CONFIG_SMP
			sa_nracanaa k_thread *next = runq_best();

			නම් (next == NULL || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#නැතහොත්
			නම් (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(key);
}
