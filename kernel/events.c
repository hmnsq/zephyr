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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/tracing/tracing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>
/* private kernel APIs */
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_EVENT_WAIT_ਨਕਾਬ     0x01

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

ਬਣਤਰ event_walk_ਅੰਕੜੇ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
	ਬਣਤਰ k_thread  *ਸਿਰ;
#ਜੇ_ਅੰਤ /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_EVENT */

ਖਾਲੀ z_impl_k_event_init(ਬਣਤਰ k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_ਵਿੱਚ_isr());

	event->events = 0;
	event->ਤਾਲਾ = (ਬਣਤਰ k_spinlock) {};

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_EVENT */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਖਾਲੀ z_vrfy_k_event_init(ਬਣਤਰ k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

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
ਸਥਿਰ uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ wait_condition)
{
	uint32_t ਮੇਲ = current & desired;

	ਜੇ ((wait_condition == K_EVENT_WAIT_ALL) && (ਮੇਲ != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		ਵਾਪਸ 0;
	}

	/* return the matched events for any wait condition */
	ਵਾਪਸ ਮੇਲ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
ਸਥਿਰ ਖਾਲੀ event_post_walk_op(ਪੂਰਨ ਸਥਿਤੀ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_ਅਣਵਰਤਿਆ(ਸਥਿਤੀ);
	ਬਣਤਰ event_walk_ਅੰਕੜੇ *walk_ਅੰਕੜੇ = ਅੰਕੜੇ;
	ਬਣਤਰ k_thread *thread, *ਅਗਲਾ;

	thread = walk_ਅੰਕੜੇ->ਸਿਰ;

	ਜਦੋਂ (thread != ਨਿਰਾ) {
		ਅਗਲਾ = thread->ਅਗਲਾ_wake_link;

		arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = ਅਗਲਾ;
	}
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EVENT_POST_WALK_OP_FN event_post_walk_op
#ਨਹੀਂਤਾਂ /* CONFIG_WAITQ_SCALABLE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EVENT_POST_WALK_OP_FN ਨਿਰਾ
#ਜੇ_ਅੰਤ /* CONFIG_WAITQ_SCALABLE */

ਸਥਿਰ ਪੂਰਨ event_walk_op(ਬਣਤਰ k_thread *thread, ਖਾਲੀ *ਅੰਕੜੇ)
{
	uint32_t ਮੇਲ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ wait_condition;
	ਬਣਤਰ event_walk_ਅੰਕੜੇ *event_ਅੰਕੜੇ = ਅੰਕੜੇ;

	wait_condition = thread->event_options & K_EVENT_WAIT_ਨਕਾਬ;

	ਮੇਲ = are_wait_conditions_met(thread->events, event_ਅੰਕੜੇ->events,
					wait_condition);
	ਜੇ (ਮੇਲ != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = ਮੇਲ;
		ਜੇ (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_ਅੰਕੜੇ->clear_events |= ਮੇਲ;
		}
		(ਖਾਲੀ)z_try_abort_thread_timeout(thread);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, 0);
		z_sched_wake_thread_locked(thread);
#ਨਹੀਂਤਾਂ /* !CONFIG_WAITQ_SCALABLE */
		thread->ਅਗਲਾ_wake_link = event_ਅੰਕੜੇ->ਸਿਰ;
		event_ਅੰਕੜੇ->ਸਿਰ = thread;
#ਜੇ_ਅੰਤ /* !CONFIG_WAITQ_SCALABLE */
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ uint32_t k_event_post_ਅੰਦਰੂਨੀ(ਬਣਤਰ k_event *event, uint32_t events,
				  uint32_t events_ਨਕਾਬ)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;
	ਬਣਤਰ event_walk_ਅੰਕੜੇ ਅੰਕੜੇ;
	uint32_t ਪਿਛਲਾ_events;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&event->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_ਨਕਾਬ);

	ਪਿਛਲਾ_events = event->events & events_ਨਕਾਬ;
	events = (event->events & ~events_ਨਕਾਬ) |
		 (events & events_ਨਕਾਬ);

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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
	ਅੰਕੜੇ.ਸਿਰ = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_WAITQ_SCALABLE */
	ਅੰਕੜੇ.events = events;
	ਅੰਕੜੇ.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &ਅੰਕੜੇ);

	/* stash any events not consumed */
	event->events = ਅੰਕੜੇ.events & ~ਅੰਕੜੇ.clear_events;

	z_reschedule(&event->ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_ਨਕਾਬ);

	ਵਾਪਸ ਪਿਛਲਾ_events;
}

uint32_t z_impl_k_event_post(ਬਣਤਰ k_event *event, uint32_t events)
{
	ਵਾਪਸ k_event_post_ਅੰਦਰੂਨੀ(event, events, events);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(ਬਣਤਰ k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_post(event, events);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_post_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_ਸੈੱਟ(ਬਣਤਰ k_event *event, uint32_t events)
{
	ਵਾਪਸ k_event_post_ਅੰਦਰੂਨੀ(event, events, ~0);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_ਸੈੱਟ(ਬਣਤਰ k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_ਸੈੱਟ(event, events);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_set_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_ਸੈੱਟ_masked(ਬਣਤਰ k_event *event, uint32_t events,
			       uint32_t events_ਨਕਾਬ)
{
	ਵਾਪਸ k_event_post_ਅੰਦਰੂਨੀ(event, events, events_ਨਕਾਬ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_ਸੈੱਟ_masked(ਬਣਤਰ k_event *event, uint32_t events,
			       uint32_t events_ਨਕਾਬ)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_ਸੈੱਟ_masked(event, events, events_ਨਕਾਬ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_set_masked_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(ਬਣਤਰ k_event *event, uint32_t events)
{
	ਵਾਪਸ k_event_post_ਅੰਦਰੂਨੀ(event, 0, events);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(ਬਣਤਰ k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_clear(event, events);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_clear_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਸਥਿਰ uint32_t k_event_wait_ਅੰਦਰੂਨੀ(ਬਣਤਰ k_event *event, uint32_t events,
				      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_ਵਿੱਚ_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	ਜੇ ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		ਵਾਪਸ 0;
	}

	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&event->ਤਾਲਾ);

	ਜੇ (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	ਜੇ (events == 0) {
		k_spin_unlock(&event->ਤਾਲਾ, ਕੁੰਜੀ);
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		ਵਾਪਸ 0;
	}

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ wait_condition = options & K_EVENT_WAIT_ਨਕਾਬ;
	ਬਣਤਰ k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	ਜੇ (rv != 0) {
		/* clear the events that are matched */
		ਜੇ (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->ਤਾਲਾ, ਕੁੰਜੀ);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->ਤਾਲਾ, ਕੁੰਜੀ);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	ਜੇ (z_pend_curr(&event->ਤਾਲਾ, ਕੁੰਜੀ, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

ਬਾਹਰਲਾ:
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	ਵਾਪਸ rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(ਬਣਤਰ k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	ਵਾਪਸ k_event_wait_ਅੰਦਰੂਨੀ(event, events, options, timeout);
}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(ਬਣਤਰ k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_wait(event, events, reset, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_wait_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(ਬਣਤਰ k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	ਵਾਪਸ k_event_wait_ਅੰਦਰੂਨੀ(event, events, options, timeout);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(ਬਣਤਰ k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_wait_all(event, events, reset, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_wait_all_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_ਸੁਰੱਖਿਅਤ(ਬਣਤਰ k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	ਵਾਪਸ k_event_wait_ਅੰਦਰੂਨੀ(event, events, options, timeout);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_ਸੁਰੱਖਿਅਤ(ਬਣਤਰ k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_wait_ਸੁਰੱਖਿਅਤ(event, events, reset, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_ਸੁਰੱਖਿਅਤ(ਬਣਤਰ k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	ਵਾਪਸ k_event_wait_ਅੰਦਰੂਨੀ(event, events, options, timeout);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_ਸੁਰੱਖਿਅਤ(ਬਣਤਰ k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ਵਾਪਸ z_impl_k_event_wait_all_ਸੁਰੱਖਿਅਤ(event, events, reset, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
