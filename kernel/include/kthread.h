/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#شامل <zephyr/kernel.h>
#شامل <kernel_internal.h>
#شامل <timeout_q.h>


#تعریف_کرو Z_STATE_STR_DUMMY       "dummy"
#تعریف_کرو Z_STATE_STR_PENDING     "pending"
#تعریف_کرو Z_STATE_STR_SLEEPING    "sleeping"
#تعریف_کرو Z_STATE_STR_DEAD        "dead"
#تعریف_کرو Z_STATE_STR_SUSPENDED   "suspended"
#تعریف_کرو Z_STATE_STR_ABORTING    "aborting"
#تعریف_کرو Z_STATE_STR_SUSPENDING  "suspending"
#تعریف_کرو Z_STATE_STR_QUEUED      "queued"

#اگر_تعریف_شدہ CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
خارجی ساخت k_spinlock z_thread_monitor_تالا;
#اگر_ختم /* CONFIG_THREAD_MONITOR */

#اگر_تعریف_شدہ CONFIG_MULTITHREADING
خارجی ساخت k_thread z_idle_دھاگے[CONFIG_MP_MAX_عدد_CPUS];
#اگر_ختم /* CONFIG_MULTITHREADING */

خالی idle(خالی *unused1, خالی *unused2, خالی *unused3);

/* clean up when a thread is aborted */

#اگر defined(CONFIG_THREAD_MONITOR)
خالی z_thread_monitor_exit(ساخت k_thread *thread);
#ورنہ
#تعریف_کرو z_thread_monitor_exit(thread) \
	کرو {/* nothing */    \
	} جب (false)
#اگر_ختم /* CONFIG_THREAD_MONITOR */

خالی z_thread_abort(ساخت k_thread *thread);

ساکن اندرونی خالی thread_جدول_وقت_new(ساخت k_thread *thread, k_timeout_t delay)
{
#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
	اگر (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_آغاز(thread);
	} ورنہ {
		z_جوڑو_thread_timeout(thread, delay);
	}
#ورنہ
	اگر (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_آغاز(thread);
	}
#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */
}

ساکن اندرونی صحیح thread_is_preemptible(ثابت ساخت k_thread *thread)
{
	/* explanation in kernel_struct.h */
	واپس thread->بنیاد.preempt <= PREEMPT_حد;
}


ساکن اندرونی صحیح thread_is_metairq(ثابت ساخت k_thread *thread)
{
#اگر CONFIG_عدد_METAIRQ_PRIORITIES > 0
	واپس (thread->بنیاد.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_عدد_METAIRQ_PRIORITIES;
#ورنہ
	ARG_غیر_مستعمل(thread);
	واپس 0;
#اگر_ختم /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ساکن اندرونی bool is_thread_dummy(ثابت ساخت k_thread *thread)
{
	واپس (thread->بنیاد.thread_state & _THREAD_DUMMY) != 0U;
}

ساکن اندرونی bool z_is_thread_suspended(ثابت ساخت k_thread *thread)
{
	واپس (thread->بنیاد.thread_state & _THREAD_SUSPENDED) != 0U;
}

ساکن اندرونی bool z_is_thread_pending(ثابت ساخت k_thread *thread)
{
	واپس (thread->بنیاد.thread_state & _THREAD_PENDING) != 0U;
}

ساکن اندرونی bool z_is_thread_dead(ثابت ساخت k_thread *thread)
{
	واپس (thread->بنیاد.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ساکن اندرونی bool z_is_thread_aborting(ثابت ساخت k_thread *thread)
{
	واپس (thread->بنیاد.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ساکن اندرونی bool z_is_thread_halting(ثابت ساخت k_thread *thread)
{
	واپس (thread->بنیاد.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ساکن اندرونی bool z_is_thread_prevented_from_running(ثابت ساخت k_thread *thread)
{
	uint8_t state = thread->بنیاد.thread_state;

	واپس (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ساکن اندرونی bool z_is_thread_timeout_فعال(ثابت ساخت k_thread *thread)
{
	واپس !z_is_inactive_timeout(&thread->بنیاد.timeout);
}

ساکن اندرونی bool z_is_thread_ready(ثابت ساخت k_thread *thread)
{
	واپس !z_is_thread_prevented_from_running(thread);
}

ساکن اندرونی bool z_is_thread_state_سیٹ(ثابت ساخت k_thread *thread, uint32_t state)
{
	واپس (thread->بنیاد.thread_state & state) != 0U;
}

ساکن اندرونی bool z_is_thread_queued(ثابت ساخت k_thread *thread)
{
	واپس z_is_thread_state_سیٹ(thread, _THREAD_QUEUED);
}

ساکن اندرونی خالی z_mark_thread_as_queued(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state |= _THREAD_QUEUED;
}

ساکن اندرونی خالی z_mark_thread_as_not_queued(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state &= ~_THREAD_QUEUED;
}

ساکن اندرونی خالی z_mark_thread_as_suspended(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state |= _THREAD_SUSPENDED;

	SYS_پورٹ_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ساکن اندرونی خالی z_mark_thread_as_not_suspended(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state &= ~_THREAD_SUSPENDED;

	SYS_پورٹ_TRACING_FUNC(k_thread, sched_resume, thread);
}

ساکن اندرونی خالی z_mark_thread_as_pending(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state |= _THREAD_PENDING;
}

ساکن اندرونی خالی z_mark_thread_as_not_pending(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state &= ~_THREAD_PENDING;
}

ساکن اندرونی bool z_is_thread_sleeping(ساخت k_thread *thread)
{
	واپس (thread->بنیاد.thread_state & _THREAD_SLEEPING) != 0U;
}

ساکن اندرونی خالی z_mark_thread_as_sleeping(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state |= _THREAD_SLEEPING;
}

ساکن اندرونی خالی z_mark_thread_as_not_sleeping(ساخت k_thread *thread)
{
	thread->بنیاد.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ساکن اندرونی خالی z_thread_essential_سیٹ(ساخت k_thread *thread)
{
	thread->بنیاد.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ساکن اندرونی خالی z_thread_essential_clear(ساخت k_thread *thread)
{
	thread->بنیاد.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ساکن اندرونی bool z_is_thread_essential(ثابت ساخت k_thread *thread)
{
	واپس (thread->بنیاد.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ساکن ALWAYS_INLINE bool should_preempt(ثابت ساخت k_thread *thread,
					 صحیح preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	اگر (preempt_ok != 0) {
		واپس true;
	}

	__ASSERT(_current != تہی, "");

	/* Or if we're pended/suspended/dummy (duh) */
	اگر (z_is_thread_prevented_from_running(_current)) {
		واپس true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	اگر (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		واپس true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	اگر (IS_ENABLED(CONFIG_تبادلہ_NONATOMIC)
	    && z_is_thread_timeout_فعال(thread)) {
		واپس true;
	}

	واپس false;
}


ساکن اندرونی bool z_is_idle_thread_اندراج(k_thread_اندراج_t اندراج_نقطہ)
{
	واپس اندراج_نقطہ == idle;
}

ساکن اندرونی bool z_is_idle_thread_object(ثابت ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_MULTITHREADING
#اگر_تعریف_شدہ CONFIG_SMP
	واپس thread->بنیاد.is_idle;
#ورنہ
	واپس thread == &z_idle_دھاگے[0];
#اگر_ختم /* CONFIG_SMP */
#ورنہ
	واپس false;
#اگر_ختم /* CONFIG_MULTITHREADING */
}


#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
