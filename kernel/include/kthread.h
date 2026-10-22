/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#ضمن <zephyr/kernel.h>
#ضمن <kernel_internal.h>
#ضمن <timeout_q.h>


#تعریف_کړه Z_STATE_STR_DUMMY       "dummy"
#تعریف_کړه Z_STATE_STR_PENDING     "pending"
#تعریف_کړه Z_STATE_STR_SLEEPING    "sleeping"
#تعریف_کړه Z_STATE_STR_DEAD        "dead"
#تعریف_کړه Z_STATE_STR_SUSPENDED   "suspended"
#تعریف_کړه Z_STATE_STR_ABORTING    "aborting"
#تعریف_کړه Z_STATE_STR_SUSPENDING  "suspending"
#تعریف_کړه Z_STATE_STR_QUEUED      "queued"

#که_تعریف_شوی CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
خارجي بنية k_spinlock z_thread_monitor_قلف;
#د_که_پای /* CONFIG_THREAD_MONITOR */

#که_تعریف_شوی CONFIG_MULTITHREADING
خارجي بنية k_thread z_idle_تارونه[CONFIG_MP_MAX_عدد_CPUS];
#د_که_پای /* CONFIG_MULTITHREADING */

فراغ idle(فراغ *unused1, فراغ *unused2, فراغ *unused3);

/* clean up when a thread is aborted */

#إذا defined(CONFIG_THREAD_MONITOR)
فراغ z_thread_monitor_exit(بنية k_thread *thread);
#وإلا
#تعریف_کړه z_thread_monitor_exit(thread) \
	افعل {/* nothing */    \
	} طالما (false)
#د_که_پای /* CONFIG_THREAD_MONITOR */

فراغ z_thread_abort(بنية k_thread *thread);

ساكن مضمن فراغ thread_مهالوېش_new(بنية k_thread *thread, k_timeout_t delay)
{
#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS
	إذا (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_پیل(thread);
	} وإلا {
		z_جمع_کړه_thread_timeout(thread, delay);
	}
#وإلا
	إذا (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_پیل(thread);
	}
#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */
}

ساكن مضمن صحيح thread_is_preemptible(ثابت بنية k_thread *thread)
{
	/* explanation in kernel_struct.h */
	أرجع thread->بنسټ.preempt <= PREEMPT_پوله;
}


ساكن مضمن صحيح thread_is_metairq(ثابت بنية k_thread *thread)
{
#إذا CONFIG_عدد_METAIRQ_PRIORITIES > 0
	أرجع (thread->بنسټ.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_عدد_METAIRQ_PRIORITIES;
#وإلا
	ARG_ناکارول_شوی(thread);
	أرجع 0;
#د_که_پای /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ساكن مضمن bool is_thread_dummy(ثابت بنية k_thread *thread)
{
	أرجع (thread->بنسټ.thread_state & _THREAD_DUMMY) != 0U;
}

ساكن مضمن bool z_is_thread_suspended(ثابت بنية k_thread *thread)
{
	أرجع (thread->بنسټ.thread_state & _THREAD_SUSPENDED) != 0U;
}

ساكن مضمن bool z_is_thread_pending(ثابت بنية k_thread *thread)
{
	أرجع (thread->بنسټ.thread_state & _THREAD_PENDING) != 0U;
}

ساكن مضمن bool z_is_thread_dead(ثابت بنية k_thread *thread)
{
	أرجع (thread->بنسټ.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ساكن مضمن bool z_is_thread_aborting(ثابت بنية k_thread *thread)
{
	أرجع (thread->بنسټ.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ساكن مضمن bool z_is_thread_halting(ثابت بنية k_thread *thread)
{
	أرجع (thread->بنسټ.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ساكن مضمن bool z_is_thread_prevented_from_running(ثابت بنية k_thread *thread)
{
	uint8_t state = thread->بنسټ.thread_state;

	أرجع (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ساكن مضمن bool z_is_thread_timeout_فعال(ثابت بنية k_thread *thread)
{
	أرجع !z_is_inactive_timeout(&thread->بنسټ.timeout);
}

ساكن مضمن bool z_is_thread_ready(ثابت بنية k_thread *thread)
{
	أرجع !z_is_thread_prevented_from_running(thread);
}

ساكن مضمن bool z_is_thread_state_سېټ(ثابت بنية k_thread *thread, uint32_t state)
{
	أرجع (thread->بنسټ.thread_state & state) != 0U;
}

ساكن مضمن bool z_is_thread_queued(ثابت بنية k_thread *thread)
{
	أرجع z_is_thread_state_سېټ(thread, _THREAD_QUEUED);
}

ساكن مضمن فراغ z_mark_thread_as_queued(بنية k_thread *thread)
{
	thread->بنسټ.thread_state |= _THREAD_QUEUED;
}

ساكن مضمن فراغ z_mark_thread_as_not_queued(بنية k_thread *thread)
{
	thread->بنسټ.thread_state &= ~_THREAD_QUEUED;
}

ساكن مضمن فراغ z_mark_thread_as_suspended(بنية k_thread *thread)
{
	thread->بنسټ.thread_state |= _THREAD_SUSPENDED;

	SYS_پورټ_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ساكن مضمن فراغ z_mark_thread_as_not_suspended(بنية k_thread *thread)
{
	thread->بنسټ.thread_state &= ~_THREAD_SUSPENDED;

	SYS_پورټ_TRACING_FUNC(k_thread, sched_resume, thread);
}

ساكن مضمن فراغ z_mark_thread_as_pending(بنية k_thread *thread)
{
	thread->بنسټ.thread_state |= _THREAD_PENDING;
}

ساكن مضمن فراغ z_mark_thread_as_not_pending(بنية k_thread *thread)
{
	thread->بنسټ.thread_state &= ~_THREAD_PENDING;
}

ساكن مضمن bool z_is_thread_sleeping(بنية k_thread *thread)
{
	أرجع (thread->بنسټ.thread_state & _THREAD_SLEEPING) != 0U;
}

ساكن مضمن فراغ z_mark_thread_as_sleeping(بنية k_thread *thread)
{
	thread->بنسټ.thread_state |= _THREAD_SLEEPING;
}

ساكن مضمن فراغ z_mark_thread_as_not_sleeping(بنية k_thread *thread)
{
	thread->بنسټ.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ساكن مضمن فراغ z_thread_essential_سېټ(بنية k_thread *thread)
{
	thread->بنسټ.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ساكن مضمن فراغ z_thread_essential_clear(بنية k_thread *thread)
{
	thread->بنسټ.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ساكن مضمن bool z_is_thread_essential(ثابت بنية k_thread *thread)
{
	أرجع (thread->بنسټ.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ساكن ALWAYS_INLINE bool should_preempt(ثابت بنية k_thread *thread,
					 صحيح preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	إذا (preempt_ok != 0) {
		أرجع true;
	}

	__ASSERT(_current != عدم, "");

	/* Or if we're pended/suspended/dummy (duh) */
	إذا (z_is_thread_prevented_from_running(_current)) {
		أرجع true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	إذا (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		أرجع true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	إذا (IS_ENABLED(CONFIG_بدلون_NONATOMIC)
	    && z_is_thread_timeout_فعال(thread)) {
		أرجع true;
	}

	أرجع false;
}


ساكن مضمن bool z_is_idle_thread_ثبت(k_thread_ثبت_t ثبت_ټکی)
{
	أرجع ثبت_ټکی == idle;
}

ساكن مضمن bool z_is_idle_thread_object(ثابت بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_MULTITHREADING
#که_تعریف_شوی CONFIG_SMP
	أرجع thread->بنسټ.is_idle;
#وإلا
	أرجع thread == &z_idle_تارونه[0];
#د_که_پای /* CONFIG_SMP */
#وإلا
	أرجع false;
#د_که_پای /* CONFIG_MULTITHREADING */
}


#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
