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

#define K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#define K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#define K_EVENT_WAIT_MASK     0x01

#define K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#define K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

ساختار event_walk_data {
#ifdef CONFIG_WAITQ_SCALABLE
	ساختار k_thread  *head;
#endif /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#ifdef CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_EVENT */

تهی z_impl_k_event_init(ساختار k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_in_isr());

	event->events = 0;
	event->lock = (ساختار k_spinlock) {};

	SYS_PORT_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#ifdef CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#endif /* CONFIG_OBJ_CORE_EVENT */
}

#ifdef CONFIG_USERSPACE
تهی z_vrfy_k_event_init(ساختار k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#شامل <zephyr/syscalls/k_event_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

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
ایستا uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					unsigned صحیح wait_condition)
{
	uint32_t match = current & desired;

	اگر ((wait_condition == K_EVENT_WAIT_ALL) && (match != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		بازگشت 0;
	}

	/* return the matched events for any wait condition */
	بازگشت match;
}

#ifdef CONFIG_WAITQ_SCALABLE
ایستا تهی event_post_walk_op(صحیح status, تهی *data)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_UNUSED(status);
	ساختار event_walk_data *walk_data = data;
	ساختار k_thread *thread, *next;

	thread = walk_data->head;

	وقتی (thread != پوچ) {
		next = thread->next_wake_link;

		arch_thread_return_مقدار_set(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = next;
	}
}
#define EVENT_POST_WALK_OP_FN event_post_walk_op
#وگرنه /* CONFIG_WAITQ_SCALABLE */
#define EVENT_POST_WALK_OP_FN پوچ
#endif /* CONFIG_WAITQ_SCALABLE */

ایستا صحیح event_walk_op(ساختار k_thread *thread, تهی *data)
{
	uint32_t match;
	unsigned صحیح wait_condition;
	ساختار event_walk_data *event_data = data;

	wait_condition = thread->event_options & K_EVENT_WAIT_MASK;

	match = are_wait_conditions_met(thread->events, event_data->events,
					wait_condition);
	اگر (match != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = match;
		اگر (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_data->clear_events |= match;
		}
		(تهی)z_try_abort_thread_timeout(thread);

#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_مقدار_set(thread, 0);
		z_sched_wake_thread_locked(thread);
#وگرنه /* !CONFIG_WAITQ_SCALABLE */
		thread->next_wake_link = event_data->head;
		event_data->head = thread;
#endif /* !CONFIG_WAITQ_SCALABLE */
	}

	بازگشت 0;
}

ایستا uint32_t k_event_post_internal(ساختار k_event *event, uint32_t events,
				  uint32_t events_mask)
{
	k_spinlock_کلید_t  کلید;
	ساختار event_walk_data data;
	uint32_t previous_events;

	کلید = k_spin_lock(&event->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_mask);

	previous_events = event->events & events_mask;
	events = (event->events & ~events_mask) |
		 (events & events_mask);

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

#ifdef CONFIG_WAITQ_SCALABLE
	data.head = پوچ;
#endif /* CONFIG_WAITQ_SCALABLE */
	data.events = events;
	data.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &data);

	/* stash any events not consumed */
	event->events = data.events & ~data.clear_events;

	z_reschedule(&event->lock, کلید);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_mask);

	بازگشت previous_events;
}

uint32_t z_impl_k_event_post(ساختار k_event *event, uint32_t events)
{
	بازگشت k_event_post_internal(event, events, events);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(ساختار k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_post(event, events);
}
#شامل <zephyr/syscalls/k_event_post_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_set(ساختار k_event *event, uint32_t events)
{
	بازگشت k_event_post_internal(event, events, ~0);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_set(ساختار k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_set(event, events);
}
#شامل <zephyr/syscalls/k_event_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_set_masked(ساختار k_event *event, uint32_t events,
			       uint32_t events_mask)
{
	بازگشت k_event_post_internal(event, events, events_mask);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_set_masked(ساختار k_event *event, uint32_t events,
			       uint32_t events_mask)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_set_masked(event, events, events_mask);
}
#شامل <zephyr/syscalls/k_event_set_masked_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(ساختار k_event *event, uint32_t events)
{
	بازگشت k_event_post_internal(event, 0, events);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(ساختار k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_clear(event, events);
}
#شامل <zephyr/syscalls/k_event_clear_mrsh.c>
#endif /* CONFIG_USERSPACE */

ایستا uint32_t k_event_wait_internal(ساختار k_event *event, uint32_t events,
				      unsigned صحیح options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	اگر ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		بازگشت 0;
	}

	k_spinlock_کلید_t  کلید = k_spin_lock(&event->lock);

	اگر (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	اگر (events == 0) {
		k_spin_unlock(&event->lock, کلید);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		بازگشت 0;
	}

	unsigned صحیح wait_condition = options & K_EVENT_WAIT_MASK;
	ساختار k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	اگر (rv != 0) {
		/* clear the events that are matched */
		اگر (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->lock, کلید);
		برو out;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->lock, کلید);
		برو out;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	اگر (z_pend_curr(&event->lock, کلید, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	بازگشت rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(ساختار k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	بازگشت k_event_wait_internal(event, events, options, timeout);
}
#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(ساختار k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_wait(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(ساختار k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	بازگشت k_event_wait_internal(event, events, options, timeout);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(ساختار k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_wait_all(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_all_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_safe(ساختار k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	بازگشت k_event_wait_internal(event, events, options, timeout);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_safe(ساختار k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_wait_safe(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_safe(ساختار k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	بازگشت k_event_wait_internal(event, events, options, timeout);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_safe(ساختار k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	بازگشت z_impl_k_event_wait_all_safe(event, events, reset, timeout);
}
#شامل <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#endif /* CONFIG_USERSPACE */
