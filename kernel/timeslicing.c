/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/atomic.h>
#شامل <kspinlock.h>
#شامل <kswap.h>
#شامل <ksched.h>
#شامل <ipi.h>
#شامل <timeslicing.h>

ساکن صحیح قاش_ticks = DIV_باری_اوپر(CONFIG_TIMESLICE_سائز * Z_HZ_ticks, Z_HZ_ms);
ساکن صحیح قاش_max_prio = CONFIG_TIMESLICE_ترجیح;
ساکن ساخت _timeout قاش_timeouts[CONFIG_MP_MAX_عدد_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ساکن atomic_t قاش_expired[CONFIG_MP_MAX_عدد_CPUS];

#اگر_تعریف_شدہ CONFIG_تبادلہ_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
ساخت k_thread *pending_current;
#اگر_ختم

ساکن اندرونی صحیح قاش_وقت(ساخت k_thread *thread)
{
	صحیح ret = قاش_ticks;

#اگر_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	اگر (thread->بنیاد.قاش_ticks != 0) {
		ret = thread->بنیاد.قاش_ticks;
	}
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم
	واپس ret;
}

ساکن صحیح z_وقت_قاش_سائز(ساخت k_thread *thread)
{
	اگر (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (قاش_وقت(thread) == 0)) {
		واپس 0;
	}

#اگر_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	اگر (thread->بنیاد.قاش_ticks != 0) {
		واپس thread->بنیاد.قاش_ticks;
	}
#اگر_ختم

	اگر (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->بنیاد.prio, قاش_max_prio)) {
		واپس قاش_ticks;
	}

	واپس 0;
}

ساکن خالی قاش_timeout(ساخت _timeout *timeout)
{
	صحیح cpu = ARRAY_اشاریہ(قاش_timeouts, timeout);

	atomic_سیٹ(&قاش_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	اگر (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_نقاب(cpu));
	}
}

ساکن خالی قاش_reset(صحیح قاش_سائز)
{
	صحیح cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(خالی)z_try_abort_timeout(&قاش_timeouts[cpu]);
	اگر (قاش_سائز != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		صحیح delay = atomic_get(&قاش_expired[cpu]) ? قاش_سائز - 1 : قاش_سائز;

		z_جوڑو_timeout(&قاش_timeouts[cpu], قاش_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&قاش_expired[cpu]);
}

خالی z_وقت_قاش_reset(ساخت k_thread *thread)
{
	قاش_reset(z_وقت_قاش_سائز(thread));
}

ساکن ALWAYS_INLINE bool thread_متعین_کرتا_وقت_قاش_سائز(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
	واپس (thread->بنیاد.قاش_ticks != 0);
#ورنہ  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_غیر_مستعمل(thread);
	واپس false;
#اگر_ختم /* !CONFIG_TIMESLICE_PER_THREAD */
}

خالی k_sched_وقت_قاش_سیٹ(int32_t قاش, صحیح prio)
{
	Z_SCHED_SPINLOCK {

		قاش_ticks = k_ms_مقصد_ticks_ceil32(قاش);
		قاش_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		اگر (!thread_متعین_کرتا_وقت_قاش_سائز(_current)) {
			z_وقت_قاش_reset(_current);
		}
	}
}

#اگر_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
خالی k_thread_وقت_قاش_سیٹ(ساخت k_thread *thread, int32_t thread_قاش_ticks,
			     k_thread_timeslice_fn_t expired, خالی *ڈیٹا)
{
	Z_SCHED_SPINLOCK {
		thread->بنیاد.قاش_ticks = thread_قاش_ticks;
		thread->بنیاد.قاش_expired = expired;
		thread->بنیاد.قاش_ڈیٹا = ڈیٹا;
		z_وقت_قاش_reset(thread);
	}
}
#اگر_ختم

/* Called out of each timer and IPI interrupt */
خالی z_وقت_قاش(خالی)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	اگر (!IS_ENABLED(CONFIG_تبادلہ_NONATOMIC) &&
	    !atomic_get(&قاش_expired[_current_cpu->id])) {
		واپس;
	}

	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();
	ساخت k_thread *curr = _current;

#اگر_تعریف_شدہ CONFIG_تبادلہ_NONATOMIC
	اگر (pending_current == curr) {
		z_وقت_قاش_reset(curr);
		z_sched_spinlock_unlock(کلید);
		واپس;
	}
	pending_current = تہی;
#اگر_ختم

	صحیح قاش_سائز = 0;

	اگر (atomic_get(&قاش_expired[_current_cpu->id])) {
		قاش_سائز = z_وقت_قاش_سائز(curr);
	}

	اگر (قاش_سائز != 0) {
#اگر_تعریف_شدہ CONFIG_TIMESLICE_فی_THREAD
		k_thread_timeslice_fn_t handler = curr->بنیاد.قاش_expired;

		اگر (handler != تہی) {
			z_sched_spinlock_unlock(کلید);
			handler(curr, curr->بنیاد.قاش_ڈیٹا);
			کلید = z_sched_spinlock_تالا();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			قاش_سائز = z_وقت_قاش_سائز(curr);
		}
#اگر_ختم
		اگر (!z_is_thread_prevented_from_running(curr)) {
			move_current_مقصد_انتہا_کا_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#اگر_تعریف_شدہ CONFIG_SMP
			ساخت k_thread *اگلا = runq_بہترین();

			اگر (اگلا == تہی || z_is_idle_thread_object(اگلا)) {
				قاش_reset(قاش_سائز);
			}
#ورنہ
			اگر (_kernel.ready_q.cache == curr) {
				قاش_reset(قاش_سائز);
			}
#اگر_ختم
		}
	}
	z_sched_spinlock_unlock(کلید);
}
