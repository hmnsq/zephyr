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

ساكن صحيح ٽڪرو_ticks = DIV_ڦيرو_مٿي(CONFIG_TIMESLICE_سائيز * Z_HZ_ticks, Z_HZ_ms);
ساكن صحيح ٽڪرو_max_prio = CONFIG_TIMESLICE_ترجيح;
ساكن بنية _timeout ٽڪرو_timeouts[CONFIG_MP_MAX_انگ_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ساكن atomic_t ٽڪرو_expired[CONFIG_MP_MAX_انگ_CPUS];

#جيڪڏهن_وصف_ٿيل CONFIG_مٽا_سٽا_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
بنية k_thread *pending_current;
#جيڪڏهن_پورو

ساكن مضمن صحيح ٽڪرو_وقت(بنية k_thread *thread)
{
	صحيح ret = ٽڪرو_ticks;

#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICE_في_THREAD
	إذا (thread->بنياد.ٽڪرو_ticks != 0) {
		ret = thread->بنياد.ٽڪرو_ticks;
	}
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_پورو
	أرجع ret;
}

ساكن صحيح z_وقت_ٽڪرو_سائيز(بنية k_thread *thread)
{
	إذا (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (ٽڪرو_وقت(thread) == 0)) {
		أرجع 0;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICE_في_THREAD
	إذا (thread->بنياد.ٽڪرو_ticks != 0) {
		أرجع thread->بنياد.ٽڪرو_ticks;
	}
#جيڪڏهن_پورو

	إذا (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->بنياد.prio, ٽڪرو_max_prio)) {
		أرجع ٽڪرو_ticks;
	}

	أرجع 0;
}

ساكن فراغ ٽڪرو_timeout(بنية _timeout *timeout)
{
	صحيح cpu = ARRAY_اشاريو(ٽڪرو_timeouts, timeout);

	atomic_سيٽ(&ٽڪرو_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	إذا (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_نقاب(cpu));
	}
}

ساكن فراغ ٽڪرو_reset(صحيح ٽڪرو_سائيز)
{
	صحيح cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(فراغ)z_try_abort_timeout(&ٽڪرو_timeouts[cpu]);
	إذا (ٽڪرو_سائيز != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		صحيح delay = atomic_get(&ٽڪرو_expired[cpu]) ? ٽڪرو_سائيز - 1 : ٽڪرو_سائيز;

		z_جوڙيو_timeout(&ٽڪرو_timeouts[cpu], ٽڪرو_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&ٽڪرو_expired[cpu]);
}

فراغ z_وقت_ٽڪرو_reset(بنية k_thread *thread)
{
	ٽڪرو_reset(z_وقت_ٽڪرو_سائيز(thread));
}

ساكن ALWAYS_INLINE bool thread_مقرر_ڪري_وقت_ٽڪرو_سائيز(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICE_في_THREAD
	أرجع (thread->بنياد.ٽڪرو_ticks != 0);
#وإلا  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_اڻ_استعمال_ٿيل(thread);
	أرجع false;
#جيڪڏهن_پورو /* !CONFIG_TIMESLICE_PER_THREAD */
}

فراغ k_sched_وقت_ٽڪرو_سيٽ(int32_t ٽڪرو, صحيح prio)
{
	Z_SCHED_SPINLOCK {

		ٽڪرو_ticks = k_ms_منزل_ticks_ceil32(ٽڪرو);
		ٽڪرو_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		إذا (!thread_مقرر_ڪري_وقت_ٽڪرو_سائيز(_current)) {
			z_وقت_ٽڪرو_reset(_current);
		}
	}
}

#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICE_في_THREAD
فراغ k_thread_وقت_ٽڪرو_سيٽ(بنية k_thread *thread, int32_t thread_ٽڪرو_ticks,
			     k_thread_timeslice_fn_t expired, فراغ *معلومات)
{
	Z_SCHED_SPINLOCK {
		thread->بنياد.ٽڪرو_ticks = thread_ٽڪرو_ticks;
		thread->بنياد.ٽڪرو_expired = expired;
		thread->بنياد.ٽڪرو_معلومات = معلومات;
		z_وقت_ٽڪرو_reset(thread);
	}
}
#جيڪڏهن_پورو

/* Called out of each timer and IPI interrupt */
فراغ z_وقت_ٽڪرو(فراغ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	إذا (!IS_ENABLED(CONFIG_مٽا_سٽا_NONATOMIC) &&
	    !atomic_get(&ٽڪرو_expired[_current_cpu->id])) {
		أرجع;
	}

	k_spinlock_ڪنجي_t ڪنجي = z_sched_spinlock_تالو();
	بنية k_thread *curr = _current;

#جيڪڏهن_وصف_ٿيل CONFIG_مٽا_سٽا_NONATOMIC
	إذا (pending_current == curr) {
		z_وقت_ٽڪرو_reset(curr);
		z_sched_spinlock_unlock(ڪنجي);
		أرجع;
	}
	pending_current = عدم;
#جيڪڏهن_پورو

	صحيح ٽڪرو_سائيز = 0;

	إذا (atomic_get(&ٽڪرو_expired[_current_cpu->id])) {
		ٽڪرو_سائيز = z_وقت_ٽڪرو_سائيز(curr);
	}

	إذا (ٽڪرو_سائيز != 0) {
#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICE_في_THREAD
		k_thread_timeslice_fn_t handler = curr->بنياد.ٽڪرو_expired;

		إذا (handler != عدم) {
			z_sched_spinlock_unlock(ڪنجي);
			handler(curr, curr->بنياد.ٽڪرو_معلومات);
			ڪنجي = z_sched_spinlock_تالو();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			ٽڪرو_سائيز = z_وقت_ٽڪرو_سائيز(curr);
		}
#جيڪڏهن_پورو
		إذا (!z_is_thread_prevented_from_running(curr)) {
			move_current_منزل_پڇاڙي_جو_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
			بنية k_thread *ايندڙ = runq_بهترين();

			إذا (ايندڙ == عدم || z_is_idle_thread_object(ايندڙ)) {
				ٽڪرو_reset(ٽڪرو_سائيز);
			}
#وإلا
			إذا (_kernel.ready_q.cache == curr) {
				ٽڪرو_reset(ٽڪرو_سائيز);
			}
#جيڪڏهن_پورو
		}
	}
	z_sched_spinlock_unlock(ڪنجي);
}
