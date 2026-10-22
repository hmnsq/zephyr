/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/atomic.h>
#ضمن <kspinlock.h>
#ضمن <kswap.h>
#ضمن <ksched.h>
#ضمن <ipi.h>
#ضمن <timeslicing.h>

ساكن صحيح ټوټه_ticks = DIV_پړاو_پورته(CONFIG_TIMESLICE_اندازه * Z_HZ_ticks, Z_HZ_ms);
ساكن صحيح ټوټه_max_prio = CONFIG_TIMESLICE_لومړیتوب;
ساكن بنية _timeout ټوټه_timeouts[CONFIG_MP_MAX_عدد_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ساكن atomic_t ټوټه_expired[CONFIG_MP_MAX_عدد_CPUS];

#که_تعریف_شوی CONFIG_بدلون_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
بنية k_thread *pending_current;
#د_که_پای

ساكن مضمن صحيح ټوټه_وخت(بنية k_thread *thread)
{
	صحيح ret = ټوټه_ticks;

#که_تعریف_شوی CONFIG_TIMESLICE_فی_THREAD
	إذا (thread->بنسټ.ټوټه_ticks != 0) {
		ret = thread->بنسټ.ټوټه_ticks;
	}
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای
	أرجع ret;
}

ساكن صحيح z_وخت_ټوټه_اندازه(بنية k_thread *thread)
{
	إذا (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (ټوټه_وخت(thread) == 0)) {
		أرجع 0;
	}

#که_تعریف_شوی CONFIG_TIMESLICE_فی_THREAD
	إذا (thread->بنسټ.ټوټه_ticks != 0) {
		أرجع thread->بنسټ.ټوټه_ticks;
	}
#د_که_پای

	إذا (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->بنسټ.prio, ټوټه_max_prio)) {
		أرجع ټوټه_ticks;
	}

	أرجع 0;
}

ساكن فراغ ټوټه_timeout(بنية _timeout *timeout)
{
	صحيح cpu = ARRAY_شاخص(ټوټه_timeouts, timeout);

	atomic_سېټ(&ټوټه_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	إذا (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_ماسک(cpu));
	}
}

ساكن فراغ ټوټه_reset(صحيح ټوټه_اندازه)
{
	صحيح cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(فراغ)z_try_abort_timeout(&ټوټه_timeouts[cpu]);
	إذا (ټوټه_اندازه != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		صحيح delay = atomic_get(&ټوټه_expired[cpu]) ? ټوټه_اندازه - 1 : ټوټه_اندازه;

		z_جمع_کړه_timeout(&ټوټه_timeouts[cpu], ټوټه_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&ټوټه_expired[cpu]);
}

فراغ z_وخت_ټوټه_reset(بنية k_thread *thread)
{
	ټوټه_reset(z_وخت_ټوټه_اندازه(thread));
}

ساكن ALWAYS_INLINE bool thread_تعریفوي_وخت_ټوټه_اندازه(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_TIMESLICE_فی_THREAD
	أرجع (thread->بنسټ.ټوټه_ticks != 0);
#وإلا  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_ناکارول_شوی(thread);
	أرجع false;
#د_که_پای /* !CONFIG_TIMESLICE_PER_THREAD */
}

فراغ k_sched_وخت_ټوټه_سېټ(int32_t ټوټه, صحيح prio)
{
	Z_SCHED_SPINLOCK {

		ټوټه_ticks = k_ms_منزل_ticks_ceil32(ټوټه);
		ټوټه_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		إذا (!thread_تعریفوي_وخت_ټوټه_اندازه(_current)) {
			z_وخت_ټوټه_reset(_current);
		}
	}
}

#که_تعریف_شوی CONFIG_TIMESLICE_فی_THREAD
فراغ k_thread_وخت_ټوټه_سېټ(بنية k_thread *thread, int32_t thread_ټوټه_ticks,
			     k_thread_timeslice_fn_t expired, فراغ *معلومات)
{
	Z_SCHED_SPINLOCK {
		thread->بنسټ.ټوټه_ticks = thread_ټوټه_ticks;
		thread->بنسټ.ټوټه_expired = expired;
		thread->بنسټ.ټوټه_معلومات = معلومات;
		z_وخت_ټوټه_reset(thread);
	}
}
#د_که_پای

/* Called out of each timer and IPI interrupt */
فراغ z_وخت_ټوټه(فراغ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	إذا (!IS_ENABLED(CONFIG_بدلون_NONATOMIC) &&
	    !atomic_get(&ټوټه_expired[_current_cpu->id])) {
		أرجع;
	}

	k_spinlock_کلی_t کلی = z_sched_spinlock_قلف();
	بنية k_thread *curr = _current;

#که_تعریف_شوی CONFIG_بدلون_NONATOMIC
	إذا (pending_current == curr) {
		z_وخت_ټوټه_reset(curr);
		z_sched_spinlock_unlock(کلی);
		أرجع;
	}
	pending_current = عدم;
#د_که_پای

	صحيح ټوټه_اندازه = 0;

	إذا (atomic_get(&ټوټه_expired[_current_cpu->id])) {
		ټوټه_اندازه = z_وخت_ټوټه_اندازه(curr);
	}

	إذا (ټوټه_اندازه != 0) {
#که_تعریف_شوی CONFIG_TIMESLICE_فی_THREAD
		k_thread_timeslice_fn_t handler = curr->بنسټ.ټوټه_expired;

		إذا (handler != عدم) {
			z_sched_spinlock_unlock(کلی);
			handler(curr, curr->بنسټ.ټوټه_معلومات);
			کلی = z_sched_spinlock_قلف();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			ټوټه_اندازه = z_وخت_ټوټه_اندازه(curr);
		}
#د_که_پای
		إذا (!z_is_thread_prevented_from_running(curr)) {
			move_current_منزل_پای_ټکی_د_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#که_تعریف_شوی CONFIG_SMP
			بنية k_thread *بل = runq_غوره();

			إذا (بل == عدم || z_is_idle_thread_object(بل)) {
				ټوټه_reset(ټوټه_اندازه);
			}
#وإلا
			إذا (_kernel.ready_q.cache == curr) {
				ټوټه_reset(ټوټه_اندازه);
			}
#د_که_پای
		}
	}
	z_sched_spinlock_unlock(کلی);
}
