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

#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <zephyr/sys/dlist.h>
#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/tracing/tracing.h>
#شامل <zephyr/sys/check.h>
/* private kernel APIs */
#شامل <wait_q.h>
#شامل <ksched.h>
#شامل <scheduler.h>

#تعریف_کرو K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#تعریف_کرو K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#تعریف_کرو K_EVENT_WAIT_نقاب     0x01

#تعریف_کرو K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#تعریف_کرو K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

ساخت event_walk_ڈیٹا {
#اگر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
	ساخت k_thread  *سر;
#اگر_ختم /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_EVENT */

خالی z_impl_k_event_init(ساخت k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_اندر_isr());

	event->events = 0;
	event->تالا = (ساخت k_spinlock) {};

	SYS_پورٹ_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#اگر_ختم /* CONFIG_OBJ_CORE_EVENT */
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
خالی z_vrfy_k_event_init(ساخت k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#شامل <zephyr/syscalls/k_event_init_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

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
ساکن uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					غیرعلامتی صحیح wait_condition)
{
	uint32_t مطابقت = current & desired;

	اگر ((wait_condition == K_EVENT_WAIT_ALL) && (مطابقت != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		واپس 0;
	}

	/* return the matched events for any wait condition */
	واپس مطابقت;
}

#اگر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
ساکن خالی event_post_walk_op(صحیح حالت_2, خالی *ڈیٹا)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_غیر_مستعمل(حالت_2);
	ساخت event_walk_ڈیٹا *walk_ڈیٹا = ڈیٹا;
	ساخت k_thread *thread, *اگلا;

	thread = walk_ڈیٹا->سر;

	جب (thread != تہی) {
		اگلا = thread->اگلا_wake_link;

		arch_thread_return_قدر_سیٹ(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = اگلا;
	}
}
#تعریف_کرو EVENT_POST_WALK_OP_FN event_post_walk_op
#ورنہ /* CONFIG_WAITQ_SCALABLE */
#تعریف_کرو EVENT_POST_WALK_OP_FN تہی
#اگر_ختم /* CONFIG_WAITQ_SCALABLE */

ساکن صحیح event_walk_op(ساخت k_thread *thread, خالی *ڈیٹا)
{
	uint32_t مطابقت;
	غیرعلامتی صحیح wait_condition;
	ساخت event_walk_ڈیٹا *event_ڈیٹا = ڈیٹا;

	wait_condition = thread->event_options & K_EVENT_WAIT_نقاب;

	مطابقت = are_wait_conditions_met(thread->events, event_ڈیٹا->events,
					wait_condition);
	اگر (مطابقت != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = مطابقت;
		اگر (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_ڈیٹا->clear_events |= مطابقت;
		}
		(خالی)z_try_abort_thread_timeout(thread);

#اگر_غیر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_قدر_سیٹ(thread, 0);
		z_sched_wake_thread_locked(thread);
#ورنہ /* !CONFIG_WAITQ_SCALABLE */
		thread->اگلا_wake_link = event_ڈیٹا->سر;
		event_ڈیٹا->سر = thread;
#اگر_ختم /* !CONFIG_WAITQ_SCALABLE */
	}

	واپس 0;
}

ساکن uint32_t k_event_post_اندرونی(ساخت k_event *event, uint32_t events,
				  uint32_t events_نقاب)
{
	k_spinlock_کلید_t  کلید;
	ساخت event_walk_ڈیٹا ڈیٹا;
	uint32_t پچھلا_events;

	کلید = k_spin_تالا(&event->تالا);

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

#اگر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
	ڈیٹا.سر = تہی;
#اگر_ختم /* CONFIG_WAITQ_SCALABLE */
	ڈیٹا.events = events;
	ڈیٹا.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &ڈیٹا);

	/* stash any events not consumed */
	event->events = ڈیٹا.events & ~ڈیٹا.clear_events;

	z_reschedule(&event->تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_نقاب);

	واپس پچھلا_events;
}

uint32_t z_impl_k_event_post(ساخت k_event *event, uint32_t events)
{
	واپس k_event_post_اندرونی(event, events, events);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(ساخت k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_post(event, events);
}
#شامل <zephyr/syscalls/k_event_post_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سیٹ(ساخت k_event *event, uint32_t events)
{
	واپس k_event_post_اندرونی(event, events, ~0);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سیٹ(ساخت k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_سیٹ(event, events);
}
#شامل <zephyr/syscalls/k_event_set_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سیٹ_masked(ساخت k_event *event, uint32_t events,
			       uint32_t events_نقاب)
{
	واپس k_event_post_اندرونی(event, events, events_نقاب);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سیٹ_masked(ساخت k_event *event, uint32_t events,
			       uint32_t events_نقاب)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_سیٹ_masked(event, events, events_نقاب);
}
#شامل <zephyr/syscalls/k_event_set_masked_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(ساخت k_event *event, uint32_t events)
{
	واپس k_event_post_اندرونی(event, 0, events);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(ساخت k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_clear(event, events);
}
#شامل <zephyr/syscalls/k_event_clear_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

ساکن uint32_t k_event_wait_اندرونی(ساخت k_event *event, uint32_t events,
				      غیرعلامتی صحیح options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_اندر_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	اگر ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		واپس 0;
	}

	k_spinlock_کلید_t  کلید = k_spin_تالا(&event->تالا);

	اگر (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	اگر (events == 0) {
		k_spin_unlock(&event->تالا, کلید);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		واپس 0;
	}

	غیرعلامتی صحیح wait_condition = options & K_EVENT_WAIT_نقاب;
	ساخت k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	اگر (rv != 0) {
		/* clear the events that are matched */
		اگر (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->تالا, کلید);
		جاؤ برآمد;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->تالا, کلید);
		جاؤ برآمد;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	اگر (z_pend_curr(&event->تالا, کلید, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

برآمد:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	واپس rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(ساخت k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	واپس k_event_wait_اندرونی(event, events, options, timeout);
}
#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(ساخت k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_wait(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(ساخت k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	واپس k_event_wait_اندرونی(event, events, options, timeout);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(ساخت k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_wait_all(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_all_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_محفوظ(ساخت k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	واپس k_event_wait_اندرونی(event, events, options, timeout);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_محفوظ(ساخت k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_wait_محفوظ(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_محفوظ(ساخت k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	واپس k_event_wait_اندرونی(event, events, options, timeout);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_محفوظ(ساخت k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	واپس z_impl_k_event_wait_all_محفوظ(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
