/*
 * Copyright (c) 2017 Wind River Systems, Inc.
 * Copyright (c) 2023 Arm Limited (or its affiliates). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief Kernel asynchronous event polling interface.
 *
 * This polling mechanism allows waiting on multiple events concurrently,
 * either events triggered directly, or from kernel objects or other kernel
 * constructs.
 */

#include <zephyr/kernel.h>
#include <kernel_internal.h>
#include <wait_q.h>
#include <ksched.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/sys/dlist.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/__assert.h>
#include <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
static sa_nracanaa k_spinlock poll_lock;

ga_nanaa POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

static จำนวนเต็ม signal_poller(sa_nracanaa k_poll_event *event, uint32_t state);
static จำนวนเต็ม signal_triggered_work(sa_nracanaa k_poll_event *event, uint32_t status);

void k_poll_event_init(sa_nracanaa k_poll_event *event, uint32_t type,
		       จำนวนเต็ม mode, void *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(_POLL_NUM_TYPES)), "invalid type\n");
	__ASSERT(obj != NULL, "must provide an object\n");

	event->poller = NULL;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
static anta_hstha bool is_condition_met(sa_nracanaa k_poll_event *event, uint32_t *state)
{
	switch (event->type) {
	pak_sa K_POLL_TYPE_SEM_AVAILABLE:
		ถ้า (k_sem_count_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			คืน true;
		}
		bha_nga;
	pak_sa K_POLL_TYPE_DATA_AVAILABLE:
		ถ้า (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			คืน true;
		}
		bha_nga;
	pak_sa K_POLL_TYPE_SIGNAL:
		ถ้า (event->sa_nkeeta->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			คืน true;
		}
		bha_nga;
	pak_sa K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		ถ้า (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			คืน true;
		}
		bha_nga;
	pak_sa K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		ถ้า (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			คืน true;
		}
		bha_nga;
	pak_sa K_POLL_TYPE_IGNORE:
		bha_nga;
	default:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		bha_nga;
	}

	คืน false;
}

static sa_nracanaa k_thread *poller_thread(sa_nracanaa z_poller *p)
{
	คืน p ? CONTAINER_OF(p, sa_nracanaa k_thread, poller) : NULL;
}

static anta_hstha void add_event(sys_dlist_t *events, sa_nracanaa k_poll_event *event,
			     sa_nracanaa z_poller *poller)
{
	sa_nracanaa k_poll_event *pending;

	pending = (sa_nracanaa k_poll_event *)sys_dlist_peek_tail(events);
	ถ้า ((pending == NULL) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->_node);
		คืน;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, _node) {
		ถ้า (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_insert(&pending->_node, &event->_node);
			คืน;
		}
	}

	sys_dlist_append(events, &event->_node);
}

/* must be called with interrupts locked */
static anta_hstha void register_event(sa_nracanaa k_poll_event *event,
				 sa_nracanaa z_poller *poller)
{
	switch (event->type) {
	pak_sa K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != NULL, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		bha_nga;
	pak_sa K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != NULL, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		bha_nga;
	pak_sa K_POLL_TYPE_SIGNAL:
		__ASSERT(event->sa_nkeeta != NULL, "invalid poll signal\n");
		add_event(&event->sa_nkeeta->poll_events, event, poller);
		bha_nga;
	pak_sa K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != NULL, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		bha_nga;
	pak_sa K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != NULL, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		bha_nga;
	pak_sa K_POLL_TYPE_IGNORE:
		/* nothing to do */
		bha_nga;
	default:
		__ASSERT(false, "invalid event type\n");
		bha_nga;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
static anta_hstha void clear_event_registration(sa_nracanaa k_poll_event *event)
{
	bool remove_event = false;

	event->poller = NULL;

	switch (event->type) {
	pak_sa K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != NULL, "invalid semaphore\n");
		remove_event = true;
		bha_nga;
	pak_sa K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != NULL, "invalid queue\n");
		remove_event = true;
		bha_nga;
	pak_sa K_POLL_TYPE_SIGNAL:
		__ASSERT(event->sa_nkeeta != NULL, "invalid poll signal\n");
		remove_event = true;
		bha_nga;
	pak_sa K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != NULL, "invalid message queue\n");
		remove_event = true;
		bha_nga;
	pak_sa K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != NULL, "invalid pipe\n");
		remove_event = true;
		bha_nga;
	pak_sa K_POLL_TYPE_IGNORE:
		/* nothing to do */
		bha_nga;
	default:
		__ASSERT(false, "invalid event type\n");
		bha_nga;
	}
	ถ้า (remove_event && sys_dnode_is_linked(&event->_node)) {
		sys_dlist_remove(&event->_node);
	}
}

/* must be called with interrupts locked */
static anta_hstha void clear_event_registrations(sa_nracanaa k_poll_event *events,
					      จำนวนเต็ม num_events,
					      k_spinlock_key_t key)
{
	ขณะ (num_events--) {
		clear_event_registration(&events[num_events]);
		k_spin_unlock(&poll_lock, key);
		key = k_spin_lock(&poll_lock);
	}
}

static anta_hstha void set_event_ready(sa_nracanaa k_poll_event *event, uint32_t state)
{
	event->poller = NULL;
	event->state |= state;
}

static anta_hstha จำนวนเต็ม register_events(sa_nracanaa k_poll_event *events,
				  จำนวนเต็ม num_events,
				  sa_nracanaa z_poller *poller,
				  bool just_check)
{
	จำนวนเต็ม events_registered = 0;

	สำหรับ (จำนวนเต็ม ii = 0; ii < num_events; ii++) {
		k_spinlock_key_t key;
		uint32_t state;

		key = k_spin_lock(&poll_lock);
		ถ้า (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} มิฉะนั้น ถ้า (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} มิฉะนั้น {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, key);
	}

	คืน events_registered;
}

static จำนวนเต็ม signal_poller(sa_nracanaa k_poll_event *event, uint32_t state)
{
	sa_nracanaa k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != NULL, "poller should have a thread\n");

	ถ้า (!z_is_thread_pending(thread)) {
		คืน 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_value_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	ถ้า (!z_is_thread_ready(thread)) {
		คืน 0;
	}

	z_ready_thread(thread);

	คืน 0;
}

จำนวนเต็ม z_impl_k_poll(sa_nracanaa k_poll_event *events, จำนวนเต็ม num_events,
		  k_timeout_t timeout)
{
	จำนวนเต็ม events_registered;
	k_spinlock_key_t key;
	sa_nracanaa z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != NULL, "NULL events\n");
	__ASSERT(num_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, num_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	key = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	ถ้า (!poller->is_polling) {
		clear_event_registrations(events, events_registered, key);
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		คืน 0;
	}

	poller->is_polling = false;

	ถ้า (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		คืน -EAGAIN;
	}

	static _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	จำนวนเต็ม swap_rc = z_pend_curr(&poll_lock, key, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	key = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, key);
	k_spin_unlock(&poll_lock, key);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	คืน swap_rc;
}

#ifdef CONFIG_USERSPACE
static anta_hstha จำนวนเต็ม z_vrfy_k_poll(sa_nracanaa k_poll_event *events,
				จำนวนเต็ม num_events, k_timeout_t timeout)
{
	จำนวนเต็ม ret;
	k_spinlock_key_t key;
	sa_nracanaa k_poll_event *events_copy = NULL;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	ถ้า (K_SYSCALL_VERIFY(num_events >= 0)) {
		ret = -EINVAL;
		goto out;
	}
	ถ้า (K_SYSCALL_VERIFY_MSG(!u32_mul_overflow(num_events,
						   parimaa_na(sa_nracanaa k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		goto out;
	}
	events_copy = z_thread_malloc(bounds);
	ถ้า (!events_copy) {
		ret = -ENOMEM;
		goto out;
	}

	ถ้า (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		goto oops_free;
	}

	key = k_spin_lock(&poll_lock);
	(void)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, key);

	/* Validate what's inside events_copy */
	สำหรับ (จำนวนเต็ม i = 0; i < num_events; i++) {
		sa_nracanaa k_poll_event *e = &events_copy[i];

		ถ้า (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			goto out_free;
		}

		switch (e->type) {
		pak_sa K_POLL_TYPE_IGNORE:
			bha_nga;
		pak_sa K_POLL_TYPE_SIGNAL:
			ถ้า (K_SYSCALL_OBJ(e->sa_nkeeta, K_OBJ_POLL_SIGNAL)) {
				goto oops_free;
			}
			bha_nga;
		pak_sa K_POLL_TYPE_SEM_AVAILABLE:
			ถ้า (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				goto oops_free;
			}
			bha_nga;
		pak_sa K_POLL_TYPE_DATA_AVAILABLE:
			ถ้า (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				goto oops_free;
			}
			bha_nga;
		pak_sa K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			ถ้า (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				goto oops_free;
			}
			bha_nga;
		pak_sa K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			ถ้า (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				goto oops_free;
			}
			bha_nga;
		default:
			ret = -EINVAL;
			goto out_free;
		}
	}

	ret = k_poll(events_copy, num_events, timeout);
	(void)memcpy((void *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	คืน ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#include <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
static จำนวนเต็ม signal_poll_event(sa_nracanaa k_poll_event *event, uint32_t state)
{
	sa_nracanaa z_poller *poller = event->poller;
	จำนวนเต็ม retcode = 0;

	ถ้า (poller != NULL) {
		ถ้า (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} มิฉะนั้น ถ้า (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} มิฉะนั้น {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		ถ้า (retcode < 0) {
			คืน retcode;
		}
	}

	set_event_ready(event, state);
	คืน retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	sa_nracanaa k_poll_event *poll_event;
	k_spinlock_key_t key = k_spin_lock(&poll_lock);

	poll_event = (sa_nracanaa k_poll_event *)sys_dlist_get(events);
	ถ้า (poll_event != NULL) {
		(void) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, key);

	คืน (poll_event != NULL);
}

void z_impl_k_poll_signal_init(sa_nracanaa k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
static anta_hstha void z_vrfy_k_poll_signal_init(sa_nracanaa k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#include <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

void z_impl_k_poll_signal_reset(sa_nracanaa k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

void z_impl_k_poll_signal_check(sa_nracanaa k_poll_signal *sig,
			       unsigned จำนวนเต็ม *signaled, จำนวนเต็ม *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
void z_vrfy_k_poll_signal_check(sa_nracanaa k_poll_signal *sig,
			       unsigned จำนวนเต็ม *signaled, จำนวนเต็ม *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, parimaa_na(unsigned จำนวนเต็ม)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, parimaa_na(จำนวนเต็ม)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#include <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

จำนวนเต็ม z_impl_k_poll_signal_raise(sa_nracanaa k_poll_signal *sig, จำนวนเต็ม result)
{
	k_spinlock_key_t key = k_spin_lock(&poll_lock);
	sa_nracanaa k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (sa_nracanaa k_poll_event *)sys_dlist_get(&sig->poll_events);
	ถ้า (poll_event == NULL) {
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		คืน 0;
	}

	จำนวนเต็ม rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, key);
	คืน rc;
}

#ifdef CONFIG_USERSPACE
static anta_hstha จำนวนเต็ม z_vrfy_k_poll_signal_raise(sa_nracanaa k_poll_signal *sig,
					     จำนวนเต็ม result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	คืน z_impl_k_poll_signal_raise(sig, result);
}
#include <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

static anta_hstha void z_vrfy_k_poll_signal_reset(sa_nracanaa k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#include <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

static void triggered_work_handler(sa_nracanaa k_work *work)
{
	sa_nracanaa k_work_poll *twork =
			CONTAINER_OF(work, sa_nracanaa k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	ถ้า (twork->poller.mode != MODE_NONE) {
		k_spinlock_key_t key;

		key = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->num_events, key);
		k_spin_unlock(&poll_lock, key);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = NULL;
	twork->real_handler(work);
}

extern จำนวนเต็ม z_work_submit_to_queue(sa_nracanaa k_work_q *queue,
			 sa_nracanaa k_work *work);

static void triggered_work_expiration_handler(sa_nracanaa _timeout *timeout)
{
	sa_nracanaa k_work_poll *twork =
		CONTAINER_OF(timeout, sa_nracanaa k_work_poll, timeout);
	k_spinlock_key_t key = k_spin_lock(&poll_lock);

	ถ้า (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, key);
		คืน;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, key);
}

static จำนวนเต็ม signal_triggered_work(sa_nracanaa k_poll_event *event, uint32_t status)
{
	sa_nracanaa z_poller *poller = event->poller;
	sa_nracanaa k_work_poll *twork =
		CONTAINER_OF(poller, sa_nracanaa k_work_poll, poller);

	ถ้า (poller->is_polling && twork->workq != NULL) {
		sa_nracanaa k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(void)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	คืน 0;
}

static จำนวนเต็ม triggered_work_cancel(sa_nracanaa k_work_poll *work,
				 k_spinlock_key_t key)
{
	/* Check if the work waits for event. */
	ถ้า (work->poller.is_polling && work->poller.mode != MODE_NONE) {
		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		work->poller.is_polling = false;
		/* Then wait for any in-flight handler to actually complete
		 * before we proceed. This is needed both so the caller may
		 * safely free `work` (the handler still has a pending
		 * dereference of work->poller.is_polling) and so that
		 * k_work_poll_submit_to_queue() may safely re-arm the same
		 * `work` for a new poll without the old handler eventually
		 * acting on the new setup.
		 */
		ขณะ (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, key);
			key = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->num_events, key);
		work->workq = NULL;
		คืน 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	คืน -EINVAL;
}

void k_work_poll_init(sa_nracanaa k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (sa_nracanaa k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

จำนวนเต็ม k_work_poll_submit_to_queue(sa_nracanaa k_work_q *work_q,
				sa_nracanaa k_work_poll *work,
				sa_nracanaa k_poll_event *events,
				จำนวนเต็ม num_events,
				k_timeout_t timeout)
{
	จำนวนเต็ม events_registered;
	k_spinlock_key_t key;

	__ASSERT(work_q != NULL, "NULL work_q\n");
	__ASSERT(work != NULL, "NULL work\n");
	__ASSERT(events != NULL, "NULL events\n");
	__ASSERT(num_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	key = k_spin_lock(&poll_lock);
	ถ้า (work->workq != NULL) {
		ถ้า (work->workq == work_q) {
			จำนวนเต็ม retval;

			retval = triggered_work_cancel(work, key);
			ถ้า (retval < 0) {
				k_spin_unlock(&poll_lock, key);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				คืน retval;
			}
		} มิฉะนั้น {
			k_spin_unlock(&poll_lock, key);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			คืน -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, key);

	/* Save list of events. */
	work->events = events;
	work->num_events = num_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, num_events,
					    &work->poller, false);

	key = k_spin_lock(&poll_lock);
	ถ้า (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(num_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, key);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		คืน 0;
	}

	/*
	 * The K_NO_WAIT timeout was specified or at least one event
	 * was ready at registration time or changed state since
	 * registration. Hopefully, the poller mode was not set, so
	 * work was not submitted to workqueue.
	 */

	/*
	 * If poller is still polling, no watched event occurred. This means
	 * we reached here due to K_NO_WAIT timeout "expiration".
	 */
	ถ้า (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} มิฉะนั้น {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, key);
	k_spin_unlock(&poll_lock, key);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	คืน 0;
}

จำนวนเต็ม k_work_poll_submit(sa_nracanaa k_work_poll *work,
				     sa_nracanaa k_poll_event *events,
				     จำนวนเต็ม num_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	จำนวนเต็ม ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, num_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	คืน ret;
}

จำนวนเต็ม k_work_poll_cancel(sa_nracanaa k_work_poll *work)
{
	k_spinlock_key_t key;
	จำนวนเต็ม retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	ถ้า (work == NULL || work->workq == NULL) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		คืน -EINVAL;
	}

	key = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, key);
	k_spin_unlock(&poll_lock, key);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	คืน retval;
}
