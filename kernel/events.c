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

#تعریف_کړه K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#تعریف_کړه K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#تعریف_کړه K_EVENT_WAIT_ماسک     0x01

#تعریف_کړه K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#تعریف_کړه K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

بنية event_walk_معلومات {
#که_تعریف_شوی CONFIG_WAITQ_SCALABLE
	بنية k_thread  *سر;
#د_که_پای /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#که_تعریف_شوی CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_EVENT */

فراغ z_impl_k_event_init(بنية k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_دننه_isr());

	event->events = 0;
	event->قلف = (بنية k_spinlock) {};

	SYS_پورټ_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#که_تعریف_شوی CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#د_که_پای /* CONFIG_OBJ_CORE_EVENT */
}

#که_تعریف_شوی CONFIG_USERSPACE
فراغ z_vrfy_k_event_init(بنية k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#ضمن <zephyr/syscalls/k_event_init_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

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
					بینښه صحيح wait_condition)
{
	uint32_t سمون = current & desired;

	إذا ((wait_condition == K_EVENT_WAIT_ALL) && (سمون != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		أرجع 0;
	}

	/* return the matched events for any wait condition */
	أرجع سمون;
}

#که_تعریف_شوی CONFIG_WAITQ_SCALABLE
ساكن فراغ event_post_walk_op(صحيح حالت, فراغ *معلومات)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_ناکارول_شوی(حالت);
	بنية event_walk_معلومات *walk_معلومات = معلومات;
	بنية k_thread *thread, *بل;

	thread = walk_معلومات->سر;

	طالما (thread != عدم) {
		بل = thread->بل_wake_link;

		arch_thread_return_ارزښت_سېټ(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = بل;
	}
}
#تعریف_کړه EVENT_POST_WALK_OP_FN event_post_walk_op
#وإلا /* CONFIG_WAITQ_SCALABLE */
#تعریف_کړه EVENT_POST_WALK_OP_FN عدم
#د_که_پای /* CONFIG_WAITQ_SCALABLE */

ساكن صحيح event_walk_op(بنية k_thread *thread, فراغ *معلومات)
{
	uint32_t سمون;
	بینښه صحيح wait_condition;
	بنية event_walk_معلومات *event_معلومات = معلومات;

	wait_condition = thread->event_options & K_EVENT_WAIT_ماسک;

	سمون = are_wait_conditions_met(thread->events, event_معلومات->events,
					wait_condition);
	إذا (سمون != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = سمون;
		إذا (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_معلومات->clear_events |= سمون;
		}
		(فراغ)z_try_abort_thread_timeout(thread);

#که_نه_تعریف_شوی CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_ارزښت_سېټ(thread, 0);
		z_sched_wake_thread_locked(thread);
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		thread->بل_wake_link = event_معلومات->سر;
		event_معلومات->سر = thread;
#د_که_پای /* !CONFIG_WAITQ_SCALABLE */
	}

	أرجع 0;
}

ساكن uint32_t k_event_post_دننی(بنية k_event *event, uint32_t events,
				  uint32_t events_ماسک)
{
	k_spinlock_کلی_t  کلی;
	بنية event_walk_معلومات معلومات;
	uint32_t پخوانی_events;

	کلی = k_spin_قلف(&event->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_ماسک);

	پخوانی_events = event->events & events_ماسک;
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

#که_تعریف_شوی CONFIG_WAITQ_SCALABLE
	معلومات.سر = عدم;
#د_که_پای /* CONFIG_WAITQ_SCALABLE */
	معلومات.events = events;
	معلومات.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &معلومات);

	/* stash any events not consumed */
	event->events = معلومات.events & ~معلومات.clear_events;

	z_reschedule(&event->قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_ماسک);

	أرجع پخوانی_events;
}

uint32_t z_impl_k_event_post(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_دننی(event, events, events);
}

#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_post(event, events);
}
#ضمن <zephyr/syscalls/k_event_post_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سېټ(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_دننی(event, events, ~0);
}

#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سېټ(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_سېټ(event, events);
}
#ضمن <zephyr/syscalls/k_event_set_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سېټ_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_ماسک)
{
	أرجع k_event_post_دننی(event, events, events_ماسک);
}

#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سېټ_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_ماسک)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_سېټ_masked(event, events, events_ماسک);
}
#ضمن <zephyr/syscalls/k_event_set_masked_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_دننی(event, 0, events);
}

#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_clear(event, events);
}
#ضمن <zephyr/syscalls/k_event_clear_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

ساكن uint32_t k_event_wait_دننی(بنية k_event *event, uint32_t events,
				      بینښه صحيح options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_دننه_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	إذا ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	k_spinlock_کلی_t  کلی = k_spin_قلف(&event->قلف);

	إذا (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	إذا (events == 0) {
		k_spin_unlock(&event->قلف, کلی);
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	بینښه صحيح wait_condition = options & K_EVENT_WAIT_ماسک;
	بنية k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	إذا (rv != 0) {
		/* clear the events that are matched */
		إذا (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->قلف, کلی);
		اذهب بهر;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->قلف, کلی);
		اذهب بهر;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	إذا (z_pend_curr(&event->قلف, کلی, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

بهر:
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	أرجع rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(بنية k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	أرجع k_event_wait_دننی(event, events, options, timeout);
}
#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(بنية k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(بنية k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	أرجع k_event_wait_دننی(event, events, options, timeout);
}

#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(بنية k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_خوندي(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	أرجع k_event_wait_دننی(event, events, options, timeout);
}

#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_خوندي(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_خوندي(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_خوندي(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	أرجع k_event_wait_دننی(event, events, options, timeout);
}

#که_تعریف_شوی CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_خوندي(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all_خوندي(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
