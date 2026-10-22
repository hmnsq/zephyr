/*
 * Copyright (c) 2021 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief event objects library
 *
 * Event objects are used to signal one or more threads that a custom set of
 * events has occurred. Threads wait on event objects until another thread or
 * ISR posts the desired set of events to the event object. Each time events
 * are posted to an event object, all threads waiting on that event object are
 * processed to determine if there is a match. All threads that whose wait
 * conditions match the current set of events now belonging to the event object
 * are awakened.
 *
 * Threads waiting on an event object have the option of either waking once
 * any or all of the events it desires have been posted to the event object.
 *
 * @brief Kernel event object
 */

#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/tracing/tracing.h>
#ضمن <zephyr/sys/check.h>
/* private kernel APIs */
#ضمن <wait_q.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>

#تعریف_کن K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#تعریف_کن K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#تعریف_کن K_EVENT_WAIT_ماسک     0x01

#تعریف_کن K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#تعریف_کن K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

بنية event_walk_داده {
#اگر_تعریف_شده CONFIG_WAITQ_SCALABLE
	بنية k_thread  *سر;
#پایان_اگر /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#اگر_تعریف_شده CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, عدم);
#پایان_اگر /* CONFIG_OBJ_CORE_EVENT */

فراغ z_impl_k_event_init(بنية k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_درون_isr());

	event->events = 0;
	event->قفل = (بنية k_spinlock) {};

	SYS_پورت_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#اگر_تعریف_شده CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#پایان_اگر /* CONFIG_OBJ_CORE_EVENT */
}

#اگر_تعریف_شده CONFIG_USERSPACE
فراغ z_vrfy_k_event_init(بنية k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#ضمن <zephyr/syscalls/k_event_init_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

/**
 * @brief determine the set of events that have been satisfied
 *
 * This routine determines if the current set of events satisfies the desired
 * set of events. If @a wait_condition is K_EVENT_WAIT_ALL, then at least
 * all the desired events must be present to satisfy the request. If @a
 * wait_condition is not K_EVENT_WAIT_ALL, it is assumed to be K_EVENT_WAIT_ANY.
 * In the K_EVENT_WAIT_ANY case, the request is satisfied when any of the
 * current set of events are present in the desired set of events.
 *
 * @return event bits that satisfy the wait condition or zero
 */
ساكن uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					بیعلامت صحيح wait_condition)
{
	uint32_t تطابق = current & desired;

	إذا ((wait_condition == K_EVENT_WAIT_ALL) && (تطابق != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		أرجع 0;
	}

	/* return the matched events for any wait condition */
	أرجع تطابق;
}

#اگر_تعریف_شده CONFIG_WAITQ_SCALABLE
ساكن فراغ event_post_walk_op(صحيح وضعیت, فراغ *داده)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_استفادهنشده(وضعیت);
	بنية event_walk_داده *walk_داده = داده;
	بنية k_thread *thread, *بعدی;

	thread = walk_داده->سر;

	طالما (thread != عدم) {
		بعدی = thread->بعدی_wake_link;

		arch_thread_return_مقدار_مجموعه(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = بعدی;
	}
}
#تعریف_کن EVENT_POST_WALK_OP_FN event_post_walk_op
#وإلا /* CONFIG_WAITQ_SCALABLE */
#تعریف_کن EVENT_POST_WALK_OP_FN عدم
#پایان_اگر /* CONFIG_WAITQ_SCALABLE */

ساكن صحيح event_walk_op(بنية k_thread *thread, فراغ *داده)
{
	uint32_t تطابق;
	بیعلامت صحيح wait_condition;
	بنية event_walk_داده *event_داده = داده;

	wait_condition = thread->event_options & K_EVENT_WAIT_ماسک;

	تطابق = are_wait_conditions_met(thread->events, event_داده->events,
					wait_condition);
	إذا (تطابق != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = تطابق;
		إذا (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_داده->clear_events |= تطابق;
		}
		(فراغ)z_try_abort_thread_timeout(thread);

#اگر_تعریف_نشده CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_مقدار_مجموعه(thread, 0);
		z_sched_wake_thread_locked(thread);
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		thread->بعدی_wake_link = event_داده->سر;
		event_داده->سر = thread;
#پایان_اگر /* !CONFIG_WAITQ_SCALABLE */
	}

	أرجع 0;
}

ساكن uint32_t k_event_post_داخلی(بنية k_event *event, uint32_t events,
				  uint32_t events_ماسک)
{
	k_spinlock_کلید_t  کلید;
	بنية event_walk_داده داده;
	uint32_t قبلی_events;

	کلید = k_spin_قفل(&event->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_ماسک);

	قبلی_events = event->events & events_ماسک;
	events = (event->events & ~events_ماسک) |
		 (events & events_ماسک);

	/*
	 * Posting an event has the potential to wake multiple pended threads.
	 * It is desirable to wake all affected threads simultaneously. When
	 * z_sched_waitq_walk() allows removal of nodes from the wait queue,
	 * we wake (unpend and ready) each thread as part of the callback.
	 * Otherwise, proceed in two steps:
	 *
	 * 1. Walk the waitq and create a linked list of threads to wake.
	 * 2. Walk the resulting linked list and wake each of the threads.
	 */

#اگر_تعریف_شده CONFIG_WAITQ_SCALABLE
	داده.سر = عدم;
#پایان_اگر /* CONFIG_WAITQ_SCALABLE */
	داده.events = events;
	داده.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &داده);

	/* stash any events not consumed */
	event->events = داده.events & ~داده.clear_events;

	z_reschedule(&event->قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_ماسک);

	أرجع قبلی_events;
}

uint32_t z_impl_k_event_post(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_داخلی(event, events, events);
}

#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_post(event, events);
}
#ضمن <zephyr/syscalls/k_event_post_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_مجموعه(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_داخلی(event, events, ~0);
}

#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_مجموعه(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_مجموعه(event, events);
}
#ضمن <zephyr/syscalls/k_event_set_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_مجموعه_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_ماسک)
{
	أرجع k_event_post_داخلی(event, events, events_ماسک);
}

#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_مجموعه_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_ماسک)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_مجموعه_masked(event, events, events_ماسک);
}
#ضمن <zephyr/syscalls/k_event_set_masked_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_داخلی(event, 0, events);
}

#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_clear(event, events);
}
#ضمن <zephyr/syscalls/k_event_clear_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

ساكن uint32_t k_event_wait_داخلی(بنية k_event *event, uint32_t events,
				      بیعلامت صحيح options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_درون_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	إذا ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	k_spinlock_کلید_t  کلید = k_spin_قفل(&event->قفل);

	إذا (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	إذا (events == 0) {
		k_spin_unlock(&event->قفل, کلید);
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	بیعلامت صحيح wait_condition = options & K_EVENT_WAIT_ماسک;
	بنية k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	إذا (rv != 0) {
		/* clear the events that are matched */
		إذا (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->قفل, کلید);
		اذهب بیرون;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->قفل, کلید);
		اذهب بیرون;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	إذا (z_pend_curr(&event->قفل, کلید, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

بیرون:
	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	أرجع rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(بنية k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	أرجع k_event_wait_داخلی(event, events, options, timeout);
}
#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(بنية k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(بنية k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	أرجع k_event_wait_داخلی(event, events, options, timeout);
}

#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(بنية k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_امن(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	أرجع k_event_wait_داخلی(event, events, options, timeout);
}

#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_امن(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_امن(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_امن(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	أرجع k_event_wait_داخلی(event, events, options, timeout);
}

#اگر_تعریف_شده CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_امن(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all_امن(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
