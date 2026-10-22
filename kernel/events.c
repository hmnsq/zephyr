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

#تعریف_کرو K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#تعریف_کرو K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#تعریف_کرو K_EVENT_WAIT_نقاب     0x01

#تعریف_کرو K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#تعریف_کرو K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

بنية event_walk_انکڑے {
#جے_تعریف_شدہ CONFIG_WAITQ_SCALABLE
	بنية k_thread  *سر;
#جے_انت /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#جے_تعریف_شدہ CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, عدم);
#جے_انت /* CONFIG_OBJ_CORE_EVENT */

فراغ z_impl_k_event_init(بنية k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_وچ_isr());

	event->events = 0;
	event->جندرا = (بنية k_spinlock) {};

	SYS_پورٹ_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#جے_تعریف_شدہ CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#جے_انت /* CONFIG_OBJ_CORE_EVENT */
}

#جے_تعریف_شدہ CONFIG_USERSPACE
فراغ z_vrfy_k_event_init(بنية k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#ضمن <zephyr/syscalls/k_event_init_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

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
					بنانشانی صحيح wait_condition)
{
	uint32_t میل = current & desired;

	إذا ((wait_condition == K_EVENT_WAIT_ALL) && (میل != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		أرجع 0;
	}

	/* return the matched events for any wait condition */
	أرجع میل;
}

#جے_تعریف_شدہ CONFIG_WAITQ_SCALABLE
ساكن فراغ event_post_walk_op(صحيح حالت, فراغ *انکڑے)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_بن_ورتیا(حالت);
	بنية event_walk_انکڑے *walk_انکڑے = انکڑے;
	بنية k_thread *thread, *اگلا;

	thread = walk_انکڑے->سر;

	طالما (thread != عدم) {
		اگلا = thread->اگلا_wake_link;

		arch_thread_return_قدر_سیٹ(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = اگلا;
	}
}
#تعریف_کرو EVENT_POST_WALK_OP_FN event_post_walk_op
#وإلا /* CONFIG_WAITQ_SCALABLE */
#تعریف_کرو EVENT_POST_WALK_OP_FN عدم
#جے_انت /* CONFIG_WAITQ_SCALABLE */

ساكن صحيح event_walk_op(بنية k_thread *thread, فراغ *انکڑے)
{
	uint32_t میل;
	بنانشانی صحيح wait_condition;
	بنية event_walk_انکڑے *event_انکڑے = انکڑے;

	wait_condition = thread->event_options & K_EVENT_WAIT_نقاب;

	میل = are_wait_conditions_met(thread->events, event_انکڑے->events,
					wait_condition);
	إذا (میل != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = میل;
		إذا (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_انکڑے->clear_events |= میل;
		}
		(فراغ)z_try_abort_thread_timeout(thread);

#جے_بنا_تعریف CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_قدر_سیٹ(thread, 0);
		z_sched_wake_thread_locked(thread);
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		thread->اگلا_wake_link = event_انکڑے->سر;
		event_انکڑے->سر = thread;
#جے_انت /* !CONFIG_WAITQ_SCALABLE */
	}

	أرجع 0;
}

ساكن uint32_t k_event_post_اندرونی(بنية k_event *event, uint32_t events,
				  uint32_t events_نقاب)
{
	k_spinlock_کلید_t  کلید;
	بنية event_walk_انکڑے انکڑے;
	uint32_t پچھلا_events;

	کلید = k_spin_جندرا(&event->جندرا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_نقاب);

	پچھلا_events = event->events & events_نقاب;
	events = (event->events & ~events_نقاب) |
		 (events & events_نقاب);

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

#جے_تعریف_شدہ CONFIG_WAITQ_SCALABLE
	انکڑے.سر = عدم;
#جے_انت /* CONFIG_WAITQ_SCALABLE */
	انکڑے.events = events;
	انکڑے.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &انکڑے);

	/* stash any events not consumed */
	event->events = انکڑے.events & ~انکڑے.clear_events;

	z_reschedule(&event->جندرا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_نقاب);

	أرجع پچھلا_events;
}

uint32_t z_impl_k_event_post(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_اندرونی(event, events, events);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_post(event, events);
}
#ضمن <zephyr/syscalls/k_event_post_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سیٹ(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_اندرونی(event, events, ~0);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سیٹ(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_سیٹ(event, events);
}
#ضمن <zephyr/syscalls/k_event_set_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سیٹ_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_نقاب)
{
	أرجع k_event_post_اندرونی(event, events, events_نقاب);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سیٹ_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_نقاب)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_سیٹ_masked(event, events, events_نقاب);
}
#ضمن <zephyr/syscalls/k_event_set_masked_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_اندرونی(event, 0, events);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_clear(event, events);
}
#ضمن <zephyr/syscalls/k_event_clear_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

ساكن uint32_t k_event_wait_اندرونی(بنية k_event *event, uint32_t events,
				      بنانشانی صحيح options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_وچ_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	إذا ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	k_spinlock_کلید_t  کلید = k_spin_جندرا(&event->جندرا);

	إذا (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	إذا (events == 0) {
		k_spin_unlock(&event->جندرا, کلید);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	بنانشانی صحيح wait_condition = options & K_EVENT_WAIT_نقاب;
	بنية k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	إذا (rv != 0) {
		/* clear the events that are matched */
		إذا (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->جندرا, کلید);
		اذهب باہرلا;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->جندرا, کلید);
		اذهب باہرلا;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	إذا (z_pend_curr(&event->جندرا, کلید, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

باہرلا:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	أرجع rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(بنية k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	أرجع k_event_wait_اندرونی(event, events, options, timeout);
}
#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(بنية k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(بنية k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	أرجع k_event_wait_اندرونی(event, events, options, timeout);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(بنية k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_محفوظ(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	أرجع k_event_wait_اندرونی(event, events, options, timeout);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_محفوظ(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_محفوظ(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_محفوظ(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	أرجع k_event_wait_اندرونی(event, events, options, timeout);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_محفوظ(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all_محفوظ(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */
