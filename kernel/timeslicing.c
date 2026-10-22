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

ساكن صحيح ٹکڑا_ticks = DIV_گیڑا_اتے(CONFIG_TIMESLICE_ناپ * Z_HZ_ticks, Z_HZ_ms);
ساكن صحيح ٹکڑا_max_prio = CONFIG_TIMESLICE_ترجیح;
ساكن بنية _timeout ٹکڑا_timeouts[CONFIG_MP_MAX_عدد_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ساكن atomic_t ٹکڑا_expired[CONFIG_MP_MAX_عدد_CPUS];

#جے_تعریف_شدہ CONFIG_وٹاندرا_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
بنية k_thread *pending_current;
#جے_انت

ساكن مضمن صحيح ٹکڑا_ویلا(بنية k_thread *thread)
{
	صحيح ret = ٹکڑا_ticks;

#جے_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	إذا (thread->ادھار.ٹکڑا_ticks != 0) {
		ret = thread->ادھار.ٹکڑا_ticks;
	}
#وإلا
	ARG_بن_ورتیا(thread);
#جے_انت
	أرجع ret;
}

ساكن صحيح z_ویلا_ٹکڑا_ناپ(بنية k_thread *thread)
{
	إذا (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (ٹکڑا_ویلا(thread) == 0)) {
		أرجع 0;
	}

#جے_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	إذا (thread->ادھار.ٹکڑا_ticks != 0) {
		أرجع thread->ادھار.ٹکڑا_ticks;
	}
#جے_انت

	إذا (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->ادھار.prio, ٹکڑا_max_prio)) {
		أرجع ٹکڑا_ticks;
	}

	أرجع 0;
}

ساكن فراغ ٹکڑا_timeout(بنية _timeout *timeout)
{
	صحيح cpu = ARRAY_اشاریہ(ٹکڑا_timeouts, timeout);

	atomic_سیٹ(&ٹکڑا_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	إذا (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_نقاب(cpu));
	}
}

ساكن فراغ ٹکڑا_reset(صحيح ٹکڑا_ناپ)
{
	صحيح cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(فراغ)z_try_abort_timeout(&ٹکڑا_timeouts[cpu]);
	إذا (ٹکڑا_ناپ != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		صحيح delay = atomic_get(&ٹکڑا_expired[cpu]) ? ٹکڑا_ناپ - 1 : ٹکڑا_ناپ;

		z_جوڑو_timeout(&ٹکڑا_timeouts[cpu], ٹکڑا_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&ٹکڑا_expired[cpu]);
}

فراغ z_ویلا_ٹکڑا_reset(بنية k_thread *thread)
{
	ٹکڑا_reset(z_ویلا_ٹکڑا_ناپ(thread));
}

ساكن ALWAYS_INLINE bool thread_مقرر_کردا_ویلا_ٹکڑا_ناپ(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	أرجع (thread->ادھار.ٹکڑا_ticks != 0);
#وإلا  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_بن_ورتیا(thread);
	أرجع false;
#جے_انت /* !CONFIG_TIMESLICE_PER_THREAD */
}

فراغ k_sched_ویلا_ٹکڑا_سیٹ(int32_t ٹکڑا, صحيح prio)
{
	Z_SCHED_SPINLOCK {

		ٹکڑا_ticks = k_ms_منزل_ticks_ceil32(ٹکڑا);
		ٹکڑا_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		إذا (!thread_مقرر_کردا_ویلا_ٹکڑا_ناپ(_current)) {
			z_ویلا_ٹکڑا_reset(_current);
		}
	}
}

#جے_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
فراغ k_thread_ویلا_ٹکڑا_سیٹ(بنية k_thread *thread, int32_t thread_ٹکڑا_ticks,
			     k_thread_timeslice_fn_t expired, فراغ *انکڑے)
{
	Z_SCHED_SPINLOCK {
		thread->ادھار.ٹکڑا_ticks = thread_ٹکڑا_ticks;
		thread->ادھار.ٹکڑا_expired = expired;
		thread->ادھار.ٹکڑا_انکڑے = انکڑے;
		z_ویلا_ٹکڑا_reset(thread);
	}
}
#جے_انت

/* Called out of each timer and IPI interrupt */
فراغ z_ویلا_ٹکڑا(فراغ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	إذا (!IS_ENABLED(CONFIG_وٹاندرا_NONATOMIC) &&
	    !atomic_get(&ٹکڑا_expired[_current_cpu->id])) {
		أرجع;
	}

	k_spinlock_کلید_t کلید = z_sched_spinlock_جندرا();
	بنية k_thread *curr = _current;

#جے_تعریف_شدہ CONFIG_وٹاندرا_NONATOMIC
	إذا (pending_current == curr) {
		z_ویلا_ٹکڑا_reset(curr);
		z_sched_spinlock_unlock(کلید);
		أرجع;
	}
	pending_current = عدم;
#جے_انت

	صحيح ٹکڑا_ناپ = 0;

	إذا (atomic_get(&ٹکڑا_expired[_current_cpu->id])) {
		ٹکڑا_ناپ = z_ویلا_ٹکڑا_ناپ(curr);
	}

	إذا (ٹکڑا_ناپ != 0) {
#جے_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
		k_thread_timeslice_fn_t handler = curr->ادھار.ٹکڑا_expired;

		إذا (handler != عدم) {
			z_sched_spinlock_unlock(کلید);
			handler(curr, curr->ادھار.ٹکڑا_انکڑے);
			کلید = z_sched_spinlock_جندرا();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			ٹکڑا_ناپ = z_ویلا_ٹکڑا_ناپ(curr);
		}
#جے_انت
		إذا (!z_is_thread_prevented_from_running(curr)) {
			move_current_منزل_سرا_دا_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#جے_تعریف_شدہ CONFIG_SMP
			بنية k_thread *اگلا = runq_بہترین();

			إذا (اگلا == عدم || z_is_idle_thread_object(اگلا)) {
				ٹکڑا_reset(ٹکڑا_ناپ);
			}
#وإلا
			إذا (_kernel.ready_q.cache == curr) {
				ٹکڑا_reset(ٹکڑا_ناپ);
			}
#جے_انت
		}
	}
	z_sched_spinlock_unlock(کلید);
}
