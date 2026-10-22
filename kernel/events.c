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

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/tracing/tracing.h>
#அந்தர்பாவ <zephyr/sys/check.h>
/* private kernel APIs */
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <scheduler.h>

#define K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#define K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#define K_EVENT_WAIT_MASK     0x01

#define K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#define K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

கட்டமைப்பு event_walk_data {
#ifdef CONFIG_WAITQ_SCALABLE
	கட்டமைப்பு k_thread  *head;
#endif /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#ifdef CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_EVENT */

வெறுமை z_impl_k_event_init(கட்டமைப்பு k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_in_isr());

	event->events = 0;
	event->lock = (கட்டமைப்பு k_spinlock) {};

	SYS_PORT_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#ifdef CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#endif /* CONFIG_OBJ_CORE_EVENT */
}

#ifdef CONFIG_USERSPACE
வெறுமை z_vrfy_k_event_init(கட்டமைப்பு k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_init_mrsh.c>
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
நிலையான uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					unsigned முழு wait_condition)
{
	uint32_t match = current & desired;

	எனில் ((wait_condition == K_EVENT_WAIT_ALL) && (match != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		திரும்பு 0;
	}

	/* return the matched events for any wait condition */
	திரும்பு match;
}

#ifdef CONFIG_WAITQ_SCALABLE
நிலையான வெறுமை event_post_walk_op(முழு status, வெறுமை *data)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_UNUSED(status);
	கட்டமைப்பு event_walk_data *walk_data = data;
	கட்டமைப்பு k_thread *thread, *next;

	thread = walk_data->head;

	போது (thread != வெற்று) {
		next = thread->next_wake_link;

		arch_thread_return_மதிப்பு_set(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = next;
	}
}
#define EVENT_POST_WALK_OP_FN event_post_walk_op
#இல்லையெனில் /* CONFIG_WAITQ_SCALABLE */
#define EVENT_POST_WALK_OP_FN வெற்று
#endif /* CONFIG_WAITQ_SCALABLE */

நிலையான முழு event_walk_op(கட்டமைப்பு k_thread *thread, வெறுமை *data)
{
	uint32_t match;
	unsigned முழு wait_condition;
	கட்டமைப்பு event_walk_data *event_data = data;

	wait_condition = thread->event_options & K_EVENT_WAIT_MASK;

	match = are_wait_conditions_met(thread->events, event_data->events,
					wait_condition);
	எனில் (match != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = match;
		எனில் (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_data->clear_events |= match;
		}
		(வெறுமை)z_try_abort_thread_timeout(thread);

#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_மதிப்பு_set(thread, 0);
		z_sched_wake_thread_locked(thread);
#இல்லையெனில் /* !CONFIG_WAITQ_SCALABLE */
		thread->next_wake_link = event_data->head;
		event_data->head = thread;
#endif /* !CONFIG_WAITQ_SCALABLE */
	}

	திரும்பு 0;
}

நிலையான uint32_t k_event_post_internal(கட்டமைப்பு k_event *event, uint32_t events,
				  uint32_t events_mask)
{
	k_spinlock_சாவி_t  சாவி;
	கட்டமைப்பு event_walk_data data;
	uint32_t previous_events;

	சாவி = k_spin_lock(&event->lock);

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
	data.head = வெற்று;
#endif /* CONFIG_WAITQ_SCALABLE */
	data.events = events;
	data.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &data);

	/* stash any events not consumed */
	event->events = data.events & ~data.clear_events;

	z_reschedule(&event->lock, சாவி);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_mask);

	திரும்பு previous_events;
}

uint32_t z_impl_k_event_post(கட்டமைப்பு k_event *event, uint32_t events)
{
	திரும்பு k_event_post_internal(event, events, events);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(கட்டமைப்பு k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_post(event, events);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_post_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_set(கட்டமைப்பு k_event *event, uint32_t events)
{
	திரும்பு k_event_post_internal(event, events, ~0);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_set(கட்டமைப்பு k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_set(event, events);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_set_masked(கட்டமைப்பு k_event *event, uint32_t events,
			       uint32_t events_mask)
{
	திரும்பு k_event_post_internal(event, events, events_mask);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_set_masked(கட்டமைப்பு k_event *event, uint32_t events,
			       uint32_t events_mask)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_set_masked(event, events, events_mask);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_set_masked_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(கட்டமைப்பு k_event *event, uint32_t events)
{
	திரும்பு k_event_post_internal(event, 0, events);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(கட்டமைப்பு k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_clear(event, events);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_clear_mrsh.c>
#endif /* CONFIG_USERSPACE */

நிலையான uint32_t k_event_wait_internal(கட்டமைப்பு k_event *event, uint32_t events,
				      unsigned முழு options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	எனில் ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		திரும்பு 0;
	}

	k_spinlock_சாவி_t  சாவி = k_spin_lock(&event->lock);

	எனில் (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	எனில் (events == 0) {
		k_spin_unlock(&event->lock, சாவி);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		திரும்பு 0;
	}

	unsigned முழு wait_condition = options & K_EVENT_WAIT_MASK;
	கட்டமைப்பு k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	எனில் (rv != 0) {
		/* clear the events that are matched */
		எனில் (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->lock, சாவி);
		செல் out;
	}

	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->lock, சாவி);
		செல் out;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	எனில் (z_pend_curr(&event->lock, சாவி, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	திரும்பு rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(கட்டமைப்பு k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	திரும்பு k_event_wait_internal(event, events, options, timeout);
}
#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(கட்டமைப்பு k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_wait(event, events, reset, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(கட்டமைப்பு k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	திரும்பு k_event_wait_internal(event, events, options, timeout);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(கட்டமைப்பு k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_wait_all(event, events, reset, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_wait_all_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_safe(கட்டமைப்பு k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	திரும்பு k_event_wait_internal(event, events, options, timeout);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_safe(கட்டமைப்பு k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_wait_safe(event, events, reset, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_safe(கட்டமைப்பு k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	திரும்பு k_event_wait_internal(event, events, options, timeout);
}

#ifdef CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_safe(கட்டமைப்பு k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	திரும்பு z_impl_k_event_wait_all_safe(event, events, reset, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#endif /* CONFIG_USERSPACE */
