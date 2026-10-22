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

#وصف_ڪريو K_EVENT_WAIT_ANY      0x00   /* Wait for any events */
#وصف_ڪريو K_EVENT_WAIT_ALL      0x01   /* Wait for all events */
#وصف_ڪريو K_EVENT_WAIT_نقاب     0x01

#وصف_ڪريو K_EVENT_OPTION_RESET  0x02   /* Reset events prior to waiting */
#وصف_ڪريو K_EVENT_OPTION_CLEAR  0x04   /* Clear events that are received */

بنية event_walk_معلومات {
#جيڪڏهن_وصف_ٿيل CONFIG_WAITQ_SCALABLE
	بنية k_thread  *سر;
#جيڪڏهن_پورو /* CONFIG_WAITQ_SCALABLE */
	uint32_t events;
	uint32_t clear_events;
};

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_EVENT
K_OBJ_TYPE_DEFINE(obj_type_event, k_event, K_OBJ_TYPE_EVENT_ID, عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_EVENT */

فراغ z_impl_k_event_init(بنية k_event *event)
{
	__ASSERT_NO_MSG(!arch_is_اندريون_isr());

	event->events = 0;
	event->تالو = (بنية k_spinlock) {};

	SYS_پورٽ_TRACING_OBJ_INIT(k_event, event);

	z_waitq_init(&event->wait_q);

	k_object_init(event);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_EVENT
	k_obj_core_init_and_link(K_OBJ_CORE(event), &obj_type_event);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_EVENT */
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
فراغ z_vrfy_k_event_init(بنية k_event *event)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(event, K_OBJ_EVENT));
	z_impl_k_event_init(event);
}
#ضمن <zephyr/syscalls/k_event_init_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

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
					بنانشان صحيح wait_condition)
{
	uint32_t ميلاپ = current & desired;

	إذا ((wait_condition == K_EVENT_WAIT_ALL) && (ميلاپ != desired)) {
		/* special case for K_EVENT_WAIT_ALL */
		أرجع 0;
	}

	/* return the matched events for any wait condition */
	أرجع ميلاپ;
}

#جيڪڏهن_وصف_ٿيل CONFIG_WAITQ_SCALABLE
ساكن فراغ event_post_walk_op(صحيح حالت, فراغ *معلومات)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with the scheduler spinlock held.
	 */
	ARG_اڻ_استعمال_ٿيل(حالت);
	بنية event_walk_معلومات *walk_معلومات = معلومات;
	بنية k_thread *thread, *ايندڙ;

	thread = walk_معلومات->سر;

	طالما (thread != عدم) {
		ايندڙ = thread->ايندڙ_wake_link;

		arch_thread_return_قدر_سيٽ(thread, 0);
		z_sched_wake_thread_locked(thread);

		thread = ايندڙ;
	}
}
#وصف_ڪريو EVENT_POST_WALK_OP_FN event_post_walk_op
#وإلا /* CONFIG_WAITQ_SCALABLE */
#وصف_ڪريو EVENT_POST_WALK_OP_FN عدم
#جيڪڏهن_پورو /* CONFIG_WAITQ_SCALABLE */

ساكن صحيح event_walk_op(بنية k_thread *thread, فراغ *معلومات)
{
	uint32_t ميلاپ;
	بنانشان صحيح wait_condition;
	بنية event_walk_معلومات *event_معلومات = معلومات;

	wait_condition = thread->event_options & K_EVENT_WAIT_نقاب;

	ميلاپ = are_wait_conditions_met(thread->events, event_معلومات->events,
					wait_condition);
	إذا (ميلاپ != 0) {
		/*
		 * The wait conditions have been satisfied. Set the
		 * received events then wake thread now if allowed,
		 * else add it to the list of threads to unpend.
		 *
		 * NOTE: thread event options can consume an event
		 */
		thread->events = ميلاپ;
		إذا (thread->event_options & K_EVENT_OPTION_CLEAR) {
			event_معلومات->clear_events |= ميلاپ;
		}
		(فراغ)z_try_abort_thread_timeout(thread);

#جيڪڏهن_وصف_نه_ٿيل CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with the scheduler spinlock held.
		 */
		arch_thread_return_قدر_سيٽ(thread, 0);
		z_sched_wake_thread_locked(thread);
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		thread->ايندڙ_wake_link = event_معلومات->سر;
		event_معلومات->سر = thread;
#جيڪڏهن_پورو /* !CONFIG_WAITQ_SCALABLE */
	}

	أرجع 0;
}

ساكن uint32_t k_event_post_اندروني(بنية k_event *event, uint32_t events,
				  uint32_t events_نقاب)
{
	k_spinlock_ڪنجي_t  ڪنجي;
	بنية event_walk_معلومات معلومات;
	uint32_t اڳيون_events;

	ڪنجي = k_spin_تالو(&event->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_event, post, event, events,
					events_نقاب);

	اڳيون_events = event->events & events_نقاب;
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

#جيڪڏهن_وصف_ٿيل CONFIG_WAITQ_SCALABLE
	معلومات.سر = عدم;
#جيڪڏهن_پورو /* CONFIG_WAITQ_SCALABLE */
	معلومات.events = events;
	معلومات.clear_events = 0;
	z_sched_waitq_walk(&event->wait_q, event_walk_op, EVENT_POST_WALK_OP_FN, &معلومات);

	/* stash any events not consumed */
	event->events = معلومات.events & ~معلومات.clear_events;

	z_reschedule(&event->تالو, ڪنجي);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_event, post, event, events,
				       events_نقاب);

	أرجع اڳيون_events;
}

uint32_t z_impl_k_event_post(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_اندروني(event, events, events);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_post(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_post(event, events);
}
#ضمن <zephyr/syscalls/k_event_post_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سيٽ(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_اندروني(event, events, ~0);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سيٽ(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_سيٽ(event, events);
}
#ضمن <zephyr/syscalls/k_event_set_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_سيٽ_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_نقاب)
{
	أرجع k_event_post_اندروني(event, events, events_نقاب);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_سيٽ_masked(بنية k_event *event, uint32_t events,
			       uint32_t events_نقاب)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_سيٽ_masked(event, events, events_نقاب);
}
#ضمن <zephyr/syscalls/k_event_set_masked_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_clear(بنية k_event *event, uint32_t events)
{
	أرجع k_event_post_اندروني(event, 0, events);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_clear(بنية k_event *event, uint32_t events)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_clear(event, events);
}
#ضمن <zephyr/syscalls/k_event_clear_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

ساكن uint32_t k_event_wait_اندروني(بنية k_event *event, uint32_t events,
				      بنانشان صحيح options, k_timeout_t timeout)
{
	uint32_t  rv = 0;

	__ASSERT(((arch_is_اندريون_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_event, wait, event, events,
					options, timeout);

	إذا ((events == 0U) && !(options & K_EVENT_OPTION_RESET)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	k_spinlock_ڪنجي_t  ڪنجي = k_spin_تالو(&event->تالو);

	إذا (options & K_EVENT_OPTION_RESET) {
		event->events = 0;
	}

	إذا (events == 0) {
		k_spin_unlock(&event->تالو, ڪنجي);
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, 0);
		أرجع 0;
	}

	بنانشان صحيح wait_condition = options & K_EVENT_WAIT_نقاب;
	بنية k_thread *thread = k_sched_current_thread_query();

	/* Test if the wait conditions have already been met. */
	rv = are_wait_conditions_met(events, event->events, wait_condition);
	إذا (rv != 0) {
		/* clear the events that are matched */
		إذا (options & K_EVENT_OPTION_CLEAR) {
			event->events &= ~rv;
		}

		k_spin_unlock(&event->تالو, ڪنجي);
		اذهب ٻاهريون;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&event->تالو, ڪنجي);
		اذهب ٻاهريون;
	}

	/*
	 * The caller must pend to wait for the match. Save the desired
	 * set of events in the k_thread structure.
	 */

	thread->events = events;
	thread->event_options = options;

	SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_event, wait, event, events,
					   options, timeout);

	إذا (z_pend_curr(&event->تالو, ڪنجي, &event->wait_q, timeout) == 0) {
		/* Retrieve the set of events that woke the thread */
		rv = thread->events;
	}

ٻاهريون:
	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_event, wait, event, events, rv);

	أرجع rv;
}

/**
 * Wait for any of the specified events
 */
uint32_t z_impl_k_event_wait(بنية k_event *event, uint32_t events,
			     bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? K_EVENT_OPTION_RESET : 0;

	أرجع k_event_wait_اندروني(event, events, options, timeout);
}
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait(بنية k_event *event, uint32_t events,
				    bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

/**
 * Wait for all of the specified events
 */
uint32_t z_impl_k_event_wait_all(بنية k_event *event, uint32_t events,
				 bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : K_EVENT_WAIT_ALL;

	أرجع k_event_wait_اندروني(event, events, options, timeout);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all(بنية k_event *event, uint32_t events,
					bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_محفوظ(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR | K_EVENT_OPTION_RESET)
				 : K_EVENT_OPTION_CLEAR;

	أرجع k_event_wait_اندروني(event, events, options, timeout);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_محفوظ(بنية k_event *event, uint32_t events,
				  bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_محفوظ(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_safe_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

uint32_t z_impl_k_event_wait_all_محفوظ(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	uint32_t options = reset ? (K_EVENT_OPTION_CLEAR |
				    K_EVENT_OPTION_RESET | K_EVENT_WAIT_ALL)
				 : (K_EVENT_OPTION_CLEAR | K_EVENT_WAIT_ALL);

	أرجع k_event_wait_اندروني(event, events, options, timeout);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
uint32_t z_vrfy_k_event_wait_all_محفوظ(بنية k_event *event, uint32_t events,
				      bool reset, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(event, K_OBJ_EVENT));
	أرجع z_impl_k_event_wait_all_محفوظ(event, events, reset, timeout);
}
#ضمن <zephyr/syscalls/k_event_wait_all_safe_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
