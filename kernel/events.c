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

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/tracing/tracing.h>
#అన్తర్భావ <zephyr/sys/check.h>
/* private kernel APIs */
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>

#నిర్వచించు K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#నిర్వచించు K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#నిర్వచించు K_EVENT_WAIT_ముసుగు     0x01

#నిర్వచించు K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#నిర్వచించు K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

నిర్మాణం event_walk_దత్తాంశం {
#నిర్వచితమైతే CONFIG_WAITQ_SCALABLE
	నిర్మాణం k_thread  *తల;
#అయితే_ముగింపు /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#నిర్వచితమైతే CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_EVENT */

శూన్య z_impl_k_event_init(నిర్మాణం k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_లోపల_isr());

	event->events = 0;
	event->తాళం = (నిర్మాణం k_spinlock) {};

	SYS_పోర్టు_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#నిర్వచితమైతే CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_EVENT */
}

#నిర్వచితమైతే CONFIG_USERSPACE
శూన్య z_vrfy_k_event_init(నిర్మాణం k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#అన్తర్భావ <zephyr/syscalls/k_event_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

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
స్థిర uint32_t are_wait_conditions_met(uint32_t desired, uint32_t current,
					గుర్తులేని పూర్ణాంకం wait_condition)
{
	uint32_t సరిపోలిక = current & desired;

	ఒకవేళ ((wait_condition == K_EVENT_WAIT_ALL) && (సరిపోలిక != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		తిరిగి 0;
	}

	/* return the matched events for any wait condition */
	తిరిగి సరిపోలిక;
}

#నిర్వచితమైతే CONFIG_WAITQ_SCALABLE
స్థిర శూన్య event_post_walk_op(పూర్ణాంకం స్థితి_2, శూన్య *దత్తాంశం)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_వాడని(స్థితి_2);
	నిర్మాణం event_walk_దత్తాంశం *walk_దత్తాంశం = దత్తాంశం;
	నిర్మాణం k_thread *thread, *తదుపరి;

	thread = walk_దత్తాంశం->తల;

	అయితే (thread != ఖాళీ) {
		తదుపరి = thread->తదుపరి_wake_link;

		arch_thread_return_విలువ_సమితి(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = తదుపరి;
	}
}
#నిర్వచించు EVENT_POST_WALK_OP_FN event_post_walk_op
#లేకపోతే /* CONFIG_WAITQ_SCALABLE */
#నిర్వచించు EVENT_POST_WALK_OP_FN ఖాళీ
#అయితే_ముగింపు /* CONFIG_WAITQ_SCALABLE */

స్థిర పూర్ణాంకం event_walk_op(నిర్మాణం k_thread *thread, శూన్య *దత్తాంశం)
{
	uint32_t సరిపోలిక;
	గుర్తులేని పూర్ణాంకం wait_condition;
	నిర్మాణం event_walk_దత్తాంశం *event_దత్తాంశం = దత్తాంశం;

	wait_condition = thread->event_options & K_EVENT_WAIT_ముసుగు;

	సరిపోలిక = are_wait_conditions_met(thread->events, event_దత్తాంశం->events,
					wait_condition);
	ఒకవేళ (సరిపోలిక != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = సరిపోలిక;
		ఒకవేళ (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_దత్తాంశం->clear_events |= సరిపోలిక;
		}
		(శూన్య)z_try_abort_thread_timeout(thread);

#నిర్వచితం_కాకపోతే CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_విలువ_సమితి(thread, 0);
		z_sched_wake_thread_locked(thread);
#లేకపోతే /* !CONFIG_WAITQ_SCALABLE */
		thread->తదుపరి_wake_link = event_దత్తాంశం->తల;
		event_దత్తాంశం->తల = thread;
#అయితే_ముగింపు /* !CONFIG_WAITQ_SCALABLE */
	}

	తిరిగి 0;
}

స్థిర uint32_t k_event_post_అంతర్గత(నిర్మాణం k_event *event, uint32_t events,
				  uint32_t events_ముసుగు)
{
	k_spinlock_కీలకం_t  కీలకం;
	నిర్మాణం event_walk_దత్తాంశం దత్తాంశం;
	uint32_t మునుపటి_events;

	కీలకం = k_spin_తాళం(&event->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_ముసుగు);

	మునుపటి_events = event->events & events_ముసుగు;
	events = (event->events & ~events_ముసుగు) |
		 (events & events_ముసుగు);

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

#నిర్వచితమైతే CONFIG_WAITQ_SCALABLE
	దత్తాంశం.తల = ఖాళీ;
#అయితే_ముగింపు /* CONFIG_WAITQ_SCALABLE */
	దత్తాంశం.events = events;
	దత్తాంశం.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &దత్తాంశం);

	/* stash any events not consumed */
	event->events = దత్తాంశం.events & ~దత్తాంశం.clear_events;

	z_reschedule(&event->తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_ముసుగు);

	తిరిగి మునుపటి_events;
}

uint32_t z_impl_k_event_post(నిర్మాణం k_event *event, uint32_t events)
{
	తిరిగి k_event_post_అంతర్గత(event, events, events);
}

#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(నిర్మాణం k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_post(event, events);
}
#అన్తర్భావ <zephyr/syscalls/k_event_post_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_సమితి(నిర్మాణం k_event *event, uint32_t events)
{
	తిరిగి k_event_post_అంతర్గత(event, events, ~0);
}

#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_సమితి(నిర్మాణం k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_సమితి(event, events);
}
#అన్తర్భావ <zephyr/syscalls/k_event_set_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_సమితి_masked(నిర్మాణం k_event *event, uint32_t events,
			       uint32_t events_ముసుగు)
{
	తిరిగి k_event_post_అంతర్గత(event, events, events_ముసుగు);
}

#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_సమితి_masked(నిర్మాణం k_event *event, uint32_t events,
			       uint32_t events_ముసుగు)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_సమితి_masked(event, events, events_ముసుగు);
}
#అన్తర్భావ <zephyr/syscalls/k_event_set_masked_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(నిర్మాణం k_event *event, uint32_t events)
{
	తిరిగి k_event_post_అంతర్గత(event, 0, events);
}

#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(నిర్మాణం k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_clear(event, events);
}
#అన్తర్భావ <zephyr/syscalls/k_event_clear_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

స్థిర uint32_t k_event_wait_అంతర్గత(నిర్మాణం k_event *event, uint32_t events,
				      గుర్తులేని పూర్ణాంకం options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_లోపల_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	ఒకవేళ ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		తిరిగి 0;
	}

	k_spinlock_కీలకం_t  కీలకం = k_spin_తాళం(&event->తాళం);

	ఒకవేళ (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	ఒకవేళ (events == 0) {
		k_spin_unlock(&event->తాళం, కీలకం);
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		తిరిగి 0;
	}

	గుర్తులేని పూర్ణాంకం wait_condition = options & K_EVENT_WAIT_ముసుగు;
	నిర్మాణం k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	ఒకవేళ (rv != 0) {
		/* clear the events that are matched */
		ఒకవేళ (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->తాళం, కీలకం);
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->తాళం, కీలకం);
		వెళ్ళు బహిర్గతం;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	ఒకవేళ (z_pend_curr(&event->తాళం, కీలకం, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

బహిర్గతం:
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	తిరిగి rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(నిర్మాణం k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	తిరిగి k_event_wait_అంతర్గత(event, events, options, timeout);
}
#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(నిర్మాణం k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_wait(event, events, reset, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_event_wait_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(నిర్మాణం k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	తిరిగి k_event_wait_అంతర్గత(event, events, options, timeout);
}

#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(నిర్మాణం k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_wait_all(event, events, reset, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_event_wait_all_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_సురక్షిత(నిర్మాణం k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	తిరిగి k_event_wait_అంతర్గత(event, events, options, timeout);
}

#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_సురక్షిత(నిర్మాణం k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_wait_సురక్షిత(event, events, reset, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_సురక్షిత(నిర్మాణం k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	తిరిగి k_event_wait_అంతర్గత(event, events, options, timeout);
}

#నిర్వచితమైతే CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_సురక్షిత(నిర్మాణం k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	తిరిగి z_impl_k_event_wait_all_సురక్షిత(event, events, reset, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
