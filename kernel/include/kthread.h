/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#समावेश <zephyr/kernel.h>
#समावेश <kernel_internal.h>
#समावेश <timeout_q.h>


#घोषणा Z_STATE_STR_DUMMY       "dummy"
#घोषणा Z_STATE_STR_PENDING     "pending"
#घोषणा Z_STATE_STR_SLEEPING    "sleeping"
#घोषणा Z_STATE_STR_DEAD        "dead"
#घोषणा Z_STATE_STR_SUSPENDED   "suspended"
#घोषणा Z_STATE_STR_ABORTING    "aborting"
#घोषणा Z_STATE_STR_SUSPENDING  "suspending"
#घोषणा Z_STATE_STR_QUEUED      "queued"

#अगर_घोषित CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
बाह्य काष्ठा k_spinlock z_thread_monitor_lock;
#पूर्ण_अगर /* CONFIG_THREAD_MONITOR */

#अगर_घोषित CONFIG_MULTITHREADING
बाह्य काष्ठा k_thread z_idle_threads[CONFIG_MP_MAX_अंक_CPUS];
#पूर्ण_अगर /* CONFIG_MULTITHREADING */

व्योम idle(व्योम *unused1, व्योम *unused2, व्योम *unused3);

/* clean up when a thread is aborted */

#अगर defined(CONFIG_THREAD_MONITOR)
व्योम z_thread_monitor_exit(काष्ठा k_thread *thread);
#अन्यथा
#घोषणा z_thread_monitor_exit(thread) \
	करो {/* nothing */    \
	} जबतक (false)
#पूर्ण_अगर /* CONFIG_THREAD_MONITOR */

व्योम z_thread_abort(काष्ठा k_thread *thread);

अटल अंतरभूत व्योम thread_schedule_new(काष्ठा k_thread *thread, k_timeout_t delay)
{
#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS
	अगर (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} अन्यथा {
		z_add_thread_timeout(thread, delay);
	}
#अन्यथा
	अगर (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */
}

अटल अंतरभूत पूर्णांक thread_is_preemptible(स्थिर काष्ठा k_thread *thread)
{
	/* explanation in kernel_struct.h */
	वापस thread->base.preempt <= _PREEMPT_THRESHOLD;
}


अटल अंतरभूत पूर्णांक thread_is_metairq(स्थिर काष्ठा k_thread *thread)
{
#अगर CONFIG_अंक_METAIRQ_PRIORITIES > 0
	वापस (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_अंक_METAIRQ_PRIORITIES;
#अन्यथा
	ARG_UNUSED(thread);
	वापस 0;
#पूर्ण_अगर /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

अटल अंतरभूत bool is_thread_dummy(स्थिर काष्ठा k_thread *thread)
{
	वापस (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

अटल अंतरभूत bool z_is_thread_suspended(स्थिर काष्ठा k_thread *thread)
{
	वापस (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

अटल अंतरभूत bool z_is_thread_pending(स्थिर काष्ठा k_thread *thread)
{
	वापस (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

अटल अंतरभूत bool z_is_thread_dead(स्थिर काष्ठा k_thread *thread)
{
	वापस (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
अटल अंतरभूत bool z_is_thread_aborting(स्थिर काष्ठा k_thread *thread)
{
	वापस (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
अटल अंतरभूत bool z_is_thread_halting(स्थिर काष्ठा k_thread *thread)
{
	वापस (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


अटल अंतरभूत bool z_is_thread_prevented_from_running(स्थिर काष्ठा k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	वापस (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

अटल अंतरभूत bool z_is_thread_timeout_active(स्थिर काष्ठा k_thread *thread)
{
	वापस !z_is_inactive_timeout(&thread->base.timeout);
}

अटल अंतरभूत bool z_is_thread_ready(स्थिर काष्ठा k_thread *thread)
{
	वापस !z_is_thread_prevented_from_running(thread);
}

अटल अंतरभूत bool z_is_thread_state_set(स्थिर काष्ठा k_thread *thread, uint32_t state)
{
	वापस (thread->base.thread_state & state) != 0U;
}

अटल अंतरभूत bool z_is_thread_queued(स्थिर काष्ठा k_thread *thread)
{
	वापस z_is_thread_state_set(thread, _THREAD_QUEUED);
}

अटल अंतरभूत व्योम z_mark_thread_as_queued(काष्ठा k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

अटल अंतरभूत व्योम z_mark_thread_as_not_queued(काष्ठा k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

अटल अंतरभूत व्योम z_mark_thread_as_suspended(काष्ठा k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

अटल अंतरभूत व्योम z_mark_thread_as_not_suspended(काष्ठा k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

अटल अंतरभूत व्योम z_mark_thread_as_pending(काष्ठा k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

अटल अंतरभूत व्योम z_mark_thread_as_not_pending(काष्ठा k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

अटल अंतरभूत bool z_is_thread_sleeping(काष्ठा k_thread *thread)
{
	वापस (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

अटल अंतरभूत व्योम z_mark_thread_as_sleeping(काष्ठा k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

अटल अंतरभूत व्योम z_mark_thread_as_not_sleeping(काष्ठा k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
अटल अंतरभूत व्योम z_thread_essential_set(काष्ठा k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
अटल अंतरभूत व्योम z_thread_essential_clear(काष्ठा k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
अटल अंतरभूत bool z_is_thread_essential(स्थिर काष्ठा k_thread *thread)
{
	वापस (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


अटल ALWAYS_INLINE bool should_preempt(स्थिर काष्ठा k_thread *thread,
					 पूर्णांक preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	अगर (preempt_ok != 0) {
		वापस true;
	}

	__ASSERT(_current != शून्य, "");

	/* Or if we're pended/suspended/dummy (duh) */
	अगर (z_is_thread_prevented_from_running(_current)) {
		वापस true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	अगर (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		वापस true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	अगर (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		वापस true;
	}

	वापस false;
}


अटल अंतरभूत bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	वापस entry_point == idle;
}

अटल अंतरभूत bool z_is_idle_thread_object(स्थिर काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_MULTITHREADING
#अगर_घोषित CONFIG_SMP
	वापस thread->base.is_idle;
#अन्यथा
	वापस thread == &z_idle_threads[0];
#पूर्ण_अगर /* CONFIG_SMP */
#अन्यथा
	वापस false;
#पूर्ण_अगर /* CONFIG_MULTITHREADING */
}


#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
