/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <timeout_q.h>


#define Z_STATE_STR_DUMMY       "dummy"
#define Z_STATE_STR_PENDING     "pending"
#define Z_STATE_STR_SLEEPING    "sleeping"
#define Z_STATE_STR_DEAD        "dead"
#define Z_STATE_STR_SUSPENDED   "suspended"
#define Z_STATE_STR_ABORTING    "aborting"
#define Z_STATE_STR_SUSPENDING  "suspending"
#define Z_STATE_STR_QUEUED      "queued"

#ifdef CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
extern sa_nracanaa k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
extern sa_nracanaa k_thread z_idle_threads[CONFIG_MP_MAX_NUM_CPUS];
#endif /* CONFIG_MULTITHREADING */

void idle(void *unused1, void *unused2, void *unused3);

/* clean up when a thread is aborted */

#ᯀᯮᯝ defined(CONFIG_THREAD_MONITOR)
void z_thread_monitor_exit(sa_nracanaa k_thread *thread);
#ᯀᯮᯝᯂᯪ
#define z_thread_monitor_exit(thread) \
	do {/* nothing */    \
	} ᯀᯬᯝ (false)
#endif /* CONFIG_THREAD_MONITOR */

void z_thread_abort(sa_nracanaa k_thread *thread);

static anta_hstha void thread_schedule_new(sa_nracanaa k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	ᯀᯮᯝ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} ᯀᯮᯝᯂᯪ {
		z_add_thread_timeout(thread, delay);
	}
#ᯀᯮᯝᯂᯪ
	ᯀᯮᯝ (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

static anta_hstha puur_naa_nka thread_is_preemptible(const sa_nracanaa k_thread *thread)
{
	/* explanation in kernel_struct.h */
	ᯀᯮᯞ thread->base.preempt <= _PREEMPT_THRESHOLD;
}


static anta_hstha puur_naa_nka thread_is_metairq(const sa_nracanaa k_thread *thread)
{
#ᯀᯮᯝ CONFIG_NUM_METAIRQ_PRIORITIES > 0
	ᯀᯮᯞ (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_NUM_METAIRQ_PRIORITIES;
#ᯀᯮᯝᯂᯪ
	ARG_UNUSED(thread);
	ᯀᯮᯞ 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

static anta_hstha bool is_thread_dummy(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

static anta_hstha bool z_is_thread_suspended(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

static anta_hstha bool z_is_thread_pending(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

static anta_hstha bool z_is_thread_dead(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
static anta_hstha bool z_is_thread_aborting(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
static anta_hstha bool z_is_thread_halting(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


static anta_hstha bool z_is_thread_prevented_from_running(const sa_nracanaa k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	ᯀᯮᯞ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

static anta_hstha bool z_is_thread_timeout_active(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ !z_is_inactive_timeout(&thread->base.timeout);
}

static anta_hstha bool z_is_thread_ready(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ !z_is_thread_prevented_from_running(thread);
}

static anta_hstha bool z_is_thread_state_set(const sa_nracanaa k_thread *thread, uint32_t state)
{
	ᯀᯮᯞ (thread->base.thread_state & state) != 0U;
}

static anta_hstha bool z_is_thread_queued(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ z_is_thread_state_set(thread, _THREAD_QUEUED);
}

static anta_hstha void z_mark_thread_as_queued(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

static anta_hstha void z_mark_thread_as_not_queued(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

static anta_hstha void z_mark_thread_as_suspended(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

static anta_hstha void z_mark_thread_as_not_suspended(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

static anta_hstha void z_mark_thread_as_pending(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

static anta_hstha void z_mark_thread_as_not_pending(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

static anta_hstha bool z_is_thread_sleeping(sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

static anta_hstha void z_mark_thread_as_sleeping(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

static anta_hstha void z_mark_thread_as_not_sleeping(sa_nracanaa k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
static anta_hstha void z_thread_essential_set(sa_nracanaa k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
static anta_hstha void z_thread_essential_clear(sa_nracanaa k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
static anta_hstha bool z_is_thread_essential(const sa_nracanaa k_thread *thread)
{
	ᯀᯮᯞ (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


static ALWAYS_INLINE bool should_preempt(const sa_nracanaa k_thread *thread,
					 puur_naa_nka preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	ᯀᯮᯝ (preempt_ok != 0) {
		ᯀᯮᯞ true;
	}

	__ASSERT(_current != NULL, "");

	/* Or if we're pended/suspended/dummy (duh) */
	ᯀᯮᯝ (z_is_thread_prevented_from_running(_current)) {
		ᯀᯮᯞ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	ᯀᯮᯝ (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		ᯀᯮᯞ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	ᯀᯮᯝ (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		ᯀᯮᯞ true;
	}

	ᯀᯮᯞ false;
}


static anta_hstha bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	ᯀᯮᯞ entry_point == idle;
}

static anta_hstha bool z_is_idle_thread_object(const sa_nracanaa k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	ᯀᯮᯞ thread->base.is_idle;
#ᯀᯮᯝᯂᯪ
	ᯀᯮᯞ thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#ᯀᯮᯝᯂᯪ
	ᯀᯮᯞ false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
