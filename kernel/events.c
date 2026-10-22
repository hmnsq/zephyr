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

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/tracing/tracing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>
/* private kernel APIs */
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>

#ವ್ಯಾಖ್ಯಾನಿಸು K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#ವ್ಯಾಖ್ಯಾನಿಸು K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#ವ್ಯಾಖ್ಯಾನಿಸು K_EVENT_WAIT_ಮುಖವಾಡ     0x01

#ವ್ಯಾಖ್ಯಾನಿಸು K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#ವ್ಯಾಖ್ಯಾನಿಸು K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

ರಚನೆ event_walk_ದತ್ತಾಂಶ {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_WAITQ_SCALABLE
	ರಚನೆ k_thread  *ತಲೆ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_EVENT */

ಶೂನ್ಯ z_impl_k_event_init(ರಚನೆ k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_ಒಳ_isr());

	event->events = 0;
	event->ಬೀಗ = (ರಚನೆ k_spinlock) {};

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_EVENT */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಶೂನ್ಯ z_vrfy_k_event_init(ರಚನೆ k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

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
ಸ್ಥಿರ uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ wait_condition)
{
	uint32_t ಹೊಂದಿಕೆ = current & desired;

	ಆದರೆ ((wait_condition == K_EVENT_WAIT_ALL) && (ಹೊಂದಿಕೆ != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		ಹಿಂತಿರುಗಿ 0;
	}

	/* return the matched events for any wait condition */
	ಹಿಂತಿರುಗಿ ಹೊಂದಿಕೆ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_WAITQ_SCALABLE
ಸ್ಥಿರ ಶೂನ್ಯ event_post_walk_op(ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_ಬಳಸದ(ಸ್ಥಿತಿ_2);
	ರಚನೆ event_walk_ದತ್ತಾಂಶ *walk_ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
	ರಚನೆ k_thread *thread, *ಮುಂದಿನ;

	thread = walk_ದತ್ತಾಂಶ->ತಲೆ;

	ಆಗ (thread != ಖಾಲಿ) {
		ಮುಂದಿನ = thread->ಮುಂದಿನ_wake_link;

		arch_thread_return_ಮೌಲ್ಯ_ಗಣ(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = ಮುಂದಿನ;
	}
}
#ವ್ಯಾಖ್ಯಾನಿಸು EVENT_POST_WALK_OP_FN event_post_walk_op
#ಇಲ್ಲದಿದ್ದರೆ /* CONFIG_WAITQ_SCALABLE */
#ವ್ಯಾಖ್ಯಾನಿಸು EVENT_POST_WALK_OP_FN ಖಾಲಿ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_WAITQ_SCALABLE */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ event_walk_op(ರಚನೆ k_thread *thread, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	uint32_t ಹೊಂದಿಕೆ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ wait_condition;
	ರಚನೆ event_walk_ದತ್ತಾಂಶ *event_ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;

	wait_condition = thread->event_options & K_EVENT_WAIT_ಮುಖವಾಡ;

	ಹೊಂದಿಕೆ = are_wait_conditions_met(thread->events, event_ದತ್ತಾಂಶ->events,
					wait_condition);
	ಆದರೆ (ಹೊಂದಿಕೆ != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = ಹೊಂದಿಕೆ;
		ಆದರೆ (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_ದತ್ತಾಂಶ->clear_events |= ಹೊಂದಿಕೆ;
		}
		(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_ಮೌಲ್ಯ_ಗಣ(thread, 0);
		z_sched_wake_thread_locked(thread);
#ಇಲ್ಲದಿದ್ದರೆ /* !CONFIG_WAITQ_SCALABLE */
		thread->ಮುಂದಿನ_wake_link = event_ದತ್ತಾಂಶ->ತಲೆ;
		event_ದತ್ತಾಂಶ->ತಲೆ = thread;
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_WAITQ_SCALABLE */
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ uint32_t k_event_post_ಆಂತರಿಕ(ರಚನೆ k_event *event, uint32_t events,
				  uint32_t events_ಮುಖವಾಡ)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;
	ರಚನೆ event_walk_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;
	uint32_t ಹಿಂದಿನ_events;

	ಕೀಲಿ = k_spin_ಬೀಗ(&event->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_ಮುಖವಾಡ);

	ಹಿಂದಿನ_events = event->events & events_ಮುಖವಾಡ;
	events = (event->events & ~events_ಮುಖವಾಡ) |
		 (events & events_ಮುಖವಾಡ);

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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_WAITQ_SCALABLE
	ದತ್ತಾಂಶ.ತಲೆ = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_WAITQ_SCALABLE */
	ದತ್ತಾಂಶ.events = events;
	ದತ್ತಾಂಶ.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &ದತ್ತಾಂಶ);

	/* stash any events not consumed */
	event->events = ದತ್ತಾಂಶ.events & ~ದತ್ತಾಂಶ.clear_events;

	z_reschedule(&event->ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_ಮುಖವಾಡ);

	ಹಿಂತಿರುಗಿ ಹಿಂದಿನ_events;
}

uint32_t z_impl_k_event_post(ರಚನೆ k_event *event, uint32_t events)
{
	ಹಿಂತಿರುಗಿ k_event_post_ಆಂತರಿಕ(event, events, events);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(ರಚನೆ k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_post(event, events);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_post_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_ಗಣ(ರಚನೆ k_event *event, uint32_t events)
{
	ಹಿಂತಿರುಗಿ k_event_post_ಆಂತರಿಕ(event, events, ~0);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_ಗಣ(ರಚನೆ k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_ಗಣ(event, events);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_set_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_ಗಣ_masked(ರಚನೆ k_event *event, uint32_t events,
			       uint32_t events_ಮುಖವಾಡ)
{
	ಹಿಂತಿರುಗಿ k_event_post_ಆಂತರಿಕ(event, events, events_ಮುಖವಾಡ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_ಗಣ_masked(ರಚನೆ k_event *event, uint32_t events,
			       uint32_t events_ಮುಖವಾಡ)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_ಗಣ_masked(event, events, events_ಮುಖವಾಡ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_set_masked_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(ರಚನೆ k_event *event, uint32_t events)
{
	ಹಿಂತಿರುಗಿ k_event_post_ಆಂತರಿಕ(event, 0, events);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(ರಚನೆ k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_clear(event, events);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_clear_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಸ್ಥಿರ uint32_t k_event_wait_ಆಂತರಿಕ(ರಚನೆ k_event *event, uint32_t events,
				      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_ಒಳ_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	ಆದರೆ ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		ಹಿಂತಿರುಗಿ 0;
	}

	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ = k_spin_ಬೀಗ(&event->ಬೀಗ);

	ಆದರೆ (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	ಆದರೆ (events == 0) {
		k_spin_unlock(&event->ಬೀಗ, ಕೀಲಿ);
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		ಹಿಂತಿರುಗಿ 0;
	}

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ wait_condition = options & K_EVENT_WAIT_ಮುಖವಾಡ;
	ರಚನೆ k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	ಆದರೆ (rv != 0) {
		/* clear the events that are matched */
		ಆದರೆ (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->ಬೀಗ, ಕೀಲಿ);
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->ಬೀಗ, ಕೀಲಿ);
		ಹೋಗು ಹೊರಹೋಗು;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	ಆದರೆ (z_pend_curr(&event->ಬೀಗ, ಕೀಲಿ, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

ಹೊರಹೋಗು:
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	ಹಿಂತಿರುಗಿ rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(ರಚನೆ k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	ಹಿಂತಿರುಗಿ k_event_wait_ಆಂತರಿಕ(event, events, options, timeout);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(ರಚನೆ k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_wait(event, events, reset, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_wait_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(ರಚನೆ k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	ಹಿಂತಿರುಗಿ k_event_wait_ಆಂತರಿಕ(event, events, options, timeout);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(ರಚನೆ k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_wait_all(event, events, reset, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_wait_all_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_ಸುರಕ್ಷಿತ(ರಚನೆ k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	ಹಿಂತಿರುಗಿ k_event_wait_ಆಂತರಿಕ(event, events, options, timeout);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_ಸುರಕ್ಷಿತ(ರಚನೆ k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_wait_ಸುರಕ್ಷಿತ(event, events, reset, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_ಸುರಕ್ಷಿತ(ರಚನೆ k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	ಹಿಂತಿರುಗಿ k_event_wait_ಆಂತರಿಕ(event, events, options, timeout);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_ಸುರಕ್ಷಿತ(ರಚನೆ k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	ಹಿಂತಿರುಗಿ z_impl_k_event_wait_all_ಸುರಕ್ಷಿತ(event, events, reset, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
