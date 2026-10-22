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

ساكن صحيح برش_ticks = DIV_دور_به_بالا(CONFIG_TIMESLICE_اندازه * Z_HZ_ticks, Z_HZ_ms);
ساكن صحيح برش_max_prio = CONFIG_TIMESLICE_اولویت;
ساكن بنية _timeout برش_timeouts[CONFIG_MP_MAX_رقم_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ساكن atomic_t برش_expired[CONFIG_MP_MAX_رقم_CPUS];

#اگر_تعریف_شده CONFIG_جابجا_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
بنية k_thread *pending_current;
#پایان_اگر

ساكن مضمن صحيح برش_زمان(بنية k_thread *thread)
{
	صحيح ret = برش_ticks;

#اگر_تعریف_شده CONFIG_TIMESLICE_در_هر_THREAD
	إذا (thread->پایه.برش_ticks != 0) {
		ret = thread->پایه.برش_ticks;
	}
#وإلا
	ARG_استفادهنشده(thread);
#پایان_اگر
	أرجع ret;
}

ساكن صحيح z_زمان_برش_اندازه(بنية k_thread *thread)
{
	إذا (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (برش_زمان(thread) == 0)) {
		أرجع 0;
	}

#اگر_تعریف_شده CONFIG_TIMESLICE_در_هر_THREAD
	إذا (thread->پایه.برش_ticks != 0) {
		أرجع thread->پایه.برش_ticks;
	}
#پایان_اگر

	إذا (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->پایه.prio, برش_max_prio)) {
		أرجع برش_ticks;
	}

	أرجع 0;
}

ساكن فراغ برش_timeout(بنية _timeout *timeout)
{
	صحيح cpu = ARRAY_شاخص(برش_timeouts, timeout);

	atomic_مجموعه(&برش_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	إذا (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_ماسک(cpu));
	}
}

ساكن فراغ برش_reset(صحيح برش_اندازه)
{
	صحيح cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(فراغ)z_try_abort_timeout(&برش_timeouts[cpu]);
	إذا (برش_اندازه != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		صحيح delay = atomic_get(&برش_expired[cpu]) ? برش_اندازه - 1 : برش_اندازه;

		z_جمع_کن_timeout(&برش_timeouts[cpu], برش_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&برش_expired[cpu]);
}

فراغ z_زمان_برش_reset(بنية k_thread *thread)
{
	برش_reset(z_زمان_برش_اندازه(thread));
}

ساكن ALWAYS_INLINE bool thread_تعریف_میکند_زمان_برش_اندازه(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_TIMESLICE_در_هر_THREAD
	أرجع (thread->پایه.برش_ticks != 0);
#وإلا  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_استفادهنشده(thread);
	أرجع false;
#پایان_اگر /* !CONFIG_TIMESLICE_PER_THREAD */
}

فراغ k_sched_زمان_برش_مجموعه(int32_t برش, صحيح prio)
{
	Z_SCHED_SPINLOCK {

		برش_ticks = k_ms_مقصد_ticks_ceil32(برش);
		برش_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		إذا (!thread_تعریف_میکند_زمان_برش_اندازه(_current)) {
			z_زمان_برش_reset(_current);
		}
	}
}

#اگر_تعریف_شده CONFIG_TIMESLICE_در_هر_THREAD
فراغ k_thread_زمان_برش_مجموعه(بنية k_thread *thread, int32_t thread_برش_ticks,
			     k_thread_timeslice_fn_t expired, فراغ *داده)
{
	Z_SCHED_SPINLOCK {
		thread->پایه.برش_ticks = thread_برش_ticks;
		thread->پایه.برش_expired = expired;
		thread->پایه.برش_داده = داده;
		z_زمان_برش_reset(thread);
	}
}
#پایان_اگر

/* Called out of each timer and IPI interrupt */
فراغ z_زمان_برش(فراغ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	إذا (!IS_ENABLED(CONFIG_جابجا_NONATOMIC) &&
	    !atomic_get(&برش_expired[_current_cpu->id])) {
		أرجع;
	}

	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();
	بنية k_thread *curr = _current;

#اگر_تعریف_شده CONFIG_جابجا_NONATOMIC
	إذا (pending_current == curr) {
		z_زمان_برش_reset(curr);
		z_sched_spinlock_unlock(کلید);
		أرجع;
	}
	pending_current = عدم;
#پایان_اگر

	صحيح برش_اندازه = 0;

	إذا (atomic_get(&برش_expired[_current_cpu->id])) {
		برش_اندازه = z_زمان_برش_اندازه(curr);
	}

	إذا (برش_اندازه != 0) {
#اگر_تعریف_شده CONFIG_TIMESLICE_در_هر_THREAD
		k_thread_timeslice_fn_t handler = curr->پایه.برش_expired;

		إذا (handler != عدم) {
			z_sched_spinlock_unlock(کلید);
			handler(curr, curr->پایه.برش_داده);
			کلید = z_sched_spinlock_قفل();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			برش_اندازه = z_زمان_برش_اندازه(curr);
		}
#پایان_اگر
		إذا (!z_is_thread_prevented_from_running(curr)) {
			move_current_مقصد_انتها_از_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#اگر_تعریف_شده CONFIG_SMP
			بنية k_thread *بعدی = runq_بهترین();

			إذا (بعدی == عدم || z_is_idle_thread_object(بعدی)) {
				برش_reset(برش_اندازه);
			}
#وإلا
			إذا (_kernel.ready_q.cache == curr) {
				برش_reset(برش_اندازه);
			}
#پایان_اگر
		}
	}
	z_sched_spinlock_unlock(کلید);
}
