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

static ကိန်းပြည့် slice_ticks = DIV_ROUND_UP(CONFIG_TIMESLICE_SIZE * Z_HZ_ticks, Z_HZ_ms);
static ကိန်းပြည့် slice_max_prio = CONFIG_TIMESLICE_PRIORITY;
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

static anta_hstha ကိန်းပြည့် slice_time(sa_nracanaa k_thread *thread)
{
	ကိန်းပြည့် ret = slice_ticks;

#ifdef CONFIG_TIMESLICE_PER_THREAD
	အကယ်၍ (thread->base.slice_ticks != 0) {
		ret = thread->base.slice_ticks;
	}
#မဟုတ်ပါက
	ARG_UNUSED(thread);
#endif
	ပြန်ပေး ret;
}

static ကိန်းပြည့် z_time_slice_size(sa_nracanaa k_thread *thread)
{
	အကယ်၍ (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (slice_time(thread) == 0)) {
		ပြန်ပေး 0;
	}

#ifdef CONFIG_TIMESLICE_PER_THREAD
	အကယ်၍ (thread->base.slice_ticks != 0) {
		ပြန်ပေး thread->base.slice_ticks;
	}
#endif

	အကယ်၍ (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->base.prio, slice_max_prio)) {
		ပြန်ပေး slice_ticks;
	}

	ပြန်ပေး 0;
}

static void slice_timeout(sa_nracanaa _timeout *timeout)
{
	ကိန်းပြည့် cpu = ARRAY_INDEX(slice_timeouts, timeout);

	atomic_set(&slice_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	အကယ်၍ (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_MASK(cpu));
	}
}

static void slice_reset(ကိန်းပြည့် slice_size)
{
	ကိန်းပြည့် cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(void)z_try_abort_timeout(&slice_timeouts[cpu]);
	အကယ်၍ (slice_size != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		ကိန်းပြည့် delay = atomic_get(&slice_expired[cpu]) ? slice_size - 1 : slice_size;

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
	ပြန်ပေး (thread->base.slice_ticks != 0);
#မဟုတ်ပါက  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_UNUSED(thread);
	ပြန်ပေး false;
#endif /* !CONFIG_TIMESLICE_PER_THREAD */
}

void k_sched_time_slice_set(int32_t slice, ကိန်းပြည့် prio)
{
	Z_SCHED_SPINLOCK {

		slice_ticks = k_ms_to_ticks_ceil32(slice);
		slice_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		အကယ်၍ (!thread_defines_time_slice_size(_current)) {
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
	အကယ်၍ (!IS_ENABLED(CONFIG_SWAP_NONATOMIC) &&
	    !atomic_get(&slice_expired[_current_cpu->id])) {
		ပြန်ပေး;
	}

	k_spinlock_key_t key = z_sched_spinlock_lock();
	sa_nracanaa k_thread *curr = _current;

#ifdef CONFIG_SWAP_NONATOMIC
	အကယ်၍ (pending_current == curr) {
		z_time_slice_reset(curr);
		z_sched_spinlock_unlock(key);
		ပြန်ပေး;
	}
	pending_current = NULL;
#endif

	ကိန်းပြည့် slice_size = 0;

	အကယ်၍ (atomic_get(&slice_expired[_current_cpu->id])) {
		slice_size = z_time_slice_size(curr);
	}

	အကယ်၍ (slice_size != 0) {
#ifdef CONFIG_TIMESLICE_PER_THREAD
		k_thread_timeslice_fn_t handler = curr->base.slice_expired;

		အကယ်၍ (handler != NULL) {
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
		အကယ်၍ (!z_is_thread_prevented_from_running(curr)) {
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

			အကယ်၍ (next == NULL || z_is_idle_thread_object(next)) {
				slice_reset(slice_size);
			}
#မဟုတ်ပါက
			အကယ်၍ (_kernel.ready_q.cache == curr) {
				slice_reset(slice_size);
			}
#endif
		}
	}
	z_sched_spinlock_unlock(key);
}
