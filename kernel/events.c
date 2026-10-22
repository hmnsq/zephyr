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

#समावेश <zephyr/kernel.h>

#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/sys/dlist.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/tracing/tracing.h>
#समावेश <zephyr/sys/check.h>
/* private kernel APIs */
#समावेश <wait_q.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>

#घोषणा K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#घोषणा K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#घोषणा K_EVENT_WAIT_MASK     0x01

#घोषणा K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#घोषणा K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

काष्ठा event_walk_तथ्य {
#अगर_घोषित CONFIG_WAITQ_SCALABLE
	काष्ठा k_thread  *शीर्ष;
#पूर्ण_अगर /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#अगर_घोषित CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_EVENT */

व्योम z_impl_k_event_init(काष्ठा k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_in_isr());

	event->events = 0;
	event->lock = (काष्ठा k_spinlock) {};

	SYS_PORT_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#अगर_घोषित CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_EVENT */
}

#अगर_घोषित CONFIG_USERSPACE
व्योम z_vrfy_k_event_init(काष्ठा k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#समावेश <zephyr/syscalls/k_event_init_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

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
अटल uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					अचिन्हित पूर्णांक wait_condition)
{
	uint32_t match = current & desired;

	अगर ((wait_condition == K_EVENT_WAIT_ALL) && (match != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		वापस 0;
	}

	/* return the matched events for any wait condition */
	वापस match;
}

#अगर_घोषित CONFIG_WAITQ_SCALABLE
अटल व्योम event_post_walk_op(पूर्णांक status, व्योम *तथ्य)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_UNUSED(status);
	काष्ठा event_walk_तथ्य *walk_तथ्य = तथ्य;
	काष्ठा k_thread *thread, *अग्र;

	thread = walk_तथ्य->शीर्ष;

	जबतक (thread != शून्य) {
		अग्र = thread->अग्र_wake_link;

		arch_thread_return_मूल्य_set(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = अग्र;
	}
}
#घोषणा EVENT_POST_WALK_OP_FN event_post_walk_op
#अन्यथा /* CONFIG_WAITQ_SCALABLE */
#घोषणा EVENT_POST_WALK_OP_FN शून्य
#पूर्ण_अगर /* CONFIG_WAITQ_SCALABLE */

अटल पूर्णांक event_walk_op(काष्ठा k_thread *thread, व्योम *तथ्य)
{
	uint32_t match;
	अचिन्हित पूर्णांक wait_condition;
	काष्ठा event_walk_तथ्य *event_तथ्य = तथ्य;

	wait_condition = thread->event_options & K_EVENT_WAIT_MASK;

	match = are_wait_conditions_met(thread->events, event_तथ्य->events,
					wait_condition);
	अगर (match != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = match;
		अगर (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_तथ्य->clear_events |= match;
		}
		(व्योम)z_try_abort_thread_timeout(thread);

#अगर_अघोषित CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_मूल्य_set(thread, 0);
		z_sched_wake_thread_locked(thread);
#अन्यथा /* !CONFIG_WAITQ_SCALABLE */
		thread->अग्र_wake_link = event_तथ्य->शीर्ष;
		event_तथ्य->शीर्ष = thread;
#पूर्ण_अगर /* !CONFIG_WAITQ_SCALABLE */
	}

	वापस 0;
}

अटल uint32_t k_event_post_internal(काष्ठा k_event *event, uint32_t events,
				  uint32_t events_mask)
{
	k_spinlock_कुंजी_t  कुंजी;
	काष्ठा event_walk_तथ्य तथ्य;
	uint32_t previous_events;

	कुंजी = k_spin_lock(&event->lock);

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

#अगर_घोषित CONFIG_WAITQ_SCALABLE
	तथ्य.शीर्ष = शून्य;
#पूर्ण_अगर /* CONFIG_WAITQ_SCALABLE */
	तथ्य.events = events;
	तथ्य.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &तथ्य);

	/* stash any events not consumed */
	event->events = तथ्य.events & ~तथ्य.clear_events;

	z_reschedule(&event->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_mask);

	वापस previous_events;
}

uint32_t z_impl_k_event_post(काष्ठा k_event *event, uint32_t events)
{
	वापस k_event_post_internal(event, events, events);
}

#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(काष्ठा k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_post(event, events);
}
#समावेश <zephyr/syscalls/k_event_post_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_set(काष्ठा k_event *event, uint32_t events)
{
	वापस k_event_post_internal(event, events, ~0);
}

#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_set(काष्ठा k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_set(event, events);
}
#समावेश <zephyr/syscalls/k_event_set_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_set_masked(काष्ठा k_event *event, uint32_t events,
			       uint32_t events_mask)
{
	वापस k_event_post_internal(event, events, events_mask);
}

#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_set_masked(काष्ठा k_event *event, uint32_t events,
			       uint32_t events_mask)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_set_masked(event, events, events_mask);
}
#समावेश <zephyr/syscalls/k_event_set_masked_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(काष्ठा k_event *event, uint32_t events)
{
	वापस k_event_post_internal(event, 0, events);
}

#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(काष्ठा k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_clear(event, events);
}
#समावेश <zephyr/syscalls/k_event_clear_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

अटल uint32_t k_event_wait_internal(काष्ठा k_event *event, uint32_t events,
				      अचिन्हित पूर्णांक options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_event, रुको, event, events,
					options, timeout);

	अगर ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, रुको, event, events, 0);
		वापस 0;
	}

	k_spinlock_कुंजी_t  कुंजी = k_spin_lock(&event->lock);

	अगर (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	अगर (events == 0) {
		k_spin_unlock(&event->lock, कुंजी);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, रुको, event, events, 0);
		वापस 0;
	}

	अचिन्हित पूर्णांक wait_condition = options & K_EVENT_WAIT_MASK;
	काष्ठा k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	अगर (rv != 0) {
		/* clear the events that are matched */
		अगर (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->lock, कुंजी);
		जाओ out;
	}

	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->lock, कुंजी);
		जाओ out;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_event, रुको, event, events,
					   options, timeout);

	अगर (z_pend_curr(&event->lock, कुंजी, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_event, रुको, event, events, rv);

	वापस rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(काष्ठा k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	वापस k_event_wait_internal(event, events, options, timeout);
}
#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(काष्ठा k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_wait(event, events, reset, timeout);
}
#समावेश <zephyr/syscalls/k_event_wait_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(काष्ठा k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	वापस k_event_wait_internal(event, events, options, timeout);
}

#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(काष्ठा k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_wait_all(event, events, reset, timeout);
}
#समावेश <zephyr/syscalls/k_event_wait_all_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_safe(काष्ठा k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	वापस k_event_wait_internal(event, events, options, timeout);
}

#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_safe(काष्ठा k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_wait_safe(event, events, reset, timeout);
}
#समावेश <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_safe(काष्ठा k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	वापस k_event_wait_internal(event, events, options, timeout);
}

#अगर_घोषित CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_safe(काष्ठा k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	वापस z_impl_k_event_wait_all_safe(event, events, reset, timeout);
}
#समावेश <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
