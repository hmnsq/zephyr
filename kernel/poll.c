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

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>
#અન્તર્ભાવ <zephyr/sys/util.h>
#અન્તર્ભાવ <zephyr/sys/__assert.h>
#અન્તર્ભાવ <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
સ્થિર સંરચના k_spinlock poll_lock;

ગણના POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

સ્થિર પૂર્ણાંક signal_poller(સંરચના k_poll_event *event, uint32_t state);
સ્થિર પૂર્ણાંક signal_triggered_work(સંરચના k_poll_event *event, uint32_t status);

શૂન્ય k_poll_event_init(સંરચના k_poll_event *event, uint32_t type,
		       પૂર્ણાંક mode, શૂન્ય *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_અંક_TYPES)), "invalid type\n");
	__ASSERT(obj != ખાલી, "must provide an object\n");

	event->poller = ખાલી;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
સ્થિર અંતઃરેખ bool is_condition_met(સંરચના k_poll_event *event, uint32_t *state)
{
	પસંદગી (event->type) {
	સ્થિતિ K_POLL_TYPE_SEM_AVAILABLE:
		જો (k_sem_ગણતરી_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			પાછા true;
		}
		તોડો;
	સ્થિતિ K_POLL_TYPE_DATA_AVAILABLE:
		જો (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
			પાછા true;
		}
		તોડો;
	સ્થિતિ K_POLL_TYPE_SIGNAL:
		જો (event->સંકેત->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			પાછા true;
		}
		તોડો;
	સ્થિતિ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		જો (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_DATA_AVAILABLE;
			પાછા true;
		}
		તોડો;
	સ્થિતિ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		જો (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_DATA_AVAILABLE;
			પાછા true;
		}
		તોડો;
	સ્થિતિ K_POLL_TYPE_IGNORE:
		તોડો;
	મૂળભૂત:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		તોડો;
	}

	પાછા false;
}

સ્થિર સંરચના k_thread *poller_thread(સંરચના z_poller *p)
{
	પાછા p ? CONTAINER_OF(p, સંરચના k_thread, poller) : ખાલી;
}

સ્થિર અંતઃરેખ શૂન્ય add_event(sys_dlist_t *events, સંરચના k_poll_event *event,
			     સંરચના z_poller *poller)
{
	સંરચના k_poll_event *pending;

	pending = (સંરચના k_poll_event *)sys_dlist_peek_tail(events);
	જો ((pending == ખાલી) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->ગાંઠ_2);
		પાછા;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, ગાંઠ_2) {
		જો (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_ઉમેરો(&pending->ગાંઠ_2, &event->ગાંઠ_2);
			પાછા;
		}
	}

	sys_dlist_append(events, &event->ગાંઠ_2);
}

/* must be called with interrupts locked */
સ્થિર અંતઃરેખ શૂન્ય register_event(સંરચના k_poll_event *event,
				 સંરચના z_poller *poller)
{
	પસંદગી (event->type) {
	સ્થિતિ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ખાલી, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		તોડો;
	સ્થિતિ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != ખાલી, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		તોડો;
	સ્થિતિ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->સંકેત != ખાલી, "invalid poll signal\n");
		add_event(&event->સંકેત->poll_events, event, poller);
		તોડો;
	સ્થિતિ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != ખાલી, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		તોડો;
	સ્થિતિ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != ખાલી, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		તોડો;
	સ્થિતિ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		તોડો;
	મૂળભૂત:
		__ASSERT(false, "invalid event type\n");
		તોડો;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
સ્થિર અંતઃરેખ શૂન્ય clear_event_registration(સંરચના k_poll_event *event)
{
	bool remove_event = false;

	event->poller = ખાલી;

	પસંદગી (event->type) {
	સ્થિતિ K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != ખાલી, "invalid semaphore\n");
		remove_event = true;
		તોડો;
	સ્થિતિ K_POLL_TYPE_DATA_AVAILABLE:
		__ASSERT(event->queue != ખાલી, "invalid queue\n");
		remove_event = true;
		તોડો;
	સ્થિતિ K_POLL_TYPE_SIGNAL:
		__ASSERT(event->સંકેત != ખાલી, "invalid poll signal\n");
		remove_event = true;
		તોડો;
	સ્થિતિ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
		__ASSERT(event->msgq != ખાલી, "invalid message queue\n");
		remove_event = true;
		તોડો;
	સ્થિતિ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
		__ASSERT(event->pipe != ખાલી, "invalid pipe\n");
		remove_event = true;
		તોડો;
	સ્થિતિ K_POLL_TYPE_IGNORE:
		/* nothing to do */
		તોડો;
	મૂળભૂત:
		__ASSERT(false, "invalid event type\n");
		તોડો;
	}
	જો (remove_event && sys_dnode_is_linked(&event->ગાંઠ_2)) {
		sys_dlist_remove(&event->ગાંઠ_2);
	}
}

/* must be called with interrupts locked */
સ્થિર અંતઃરેખ શૂન્ય clear_event_registrations(સંરચના k_poll_event *events,
					      પૂર્ણાંક અંક_events,
					      k_spinlock_ચાવી_t ચાવી)
{
	જ્યારે (અંક_events--) {
		clear_event_registration(&events[અંક_events]);
		k_spin_unlock(&poll_lock, ચાવી);
		ચાવી = k_spin_lock(&poll_lock);
	}
}

સ્થિર અંતઃરેખ શૂન્ય set_event_ready(સંરચના k_poll_event *event, uint32_t state)
{
	event->poller = ખાલી;
	event->state |= state;
}

સ્થિર અંતઃરેખ પૂર્ણાંક register_events(સંરચના k_poll_event *events,
				  પૂર્ણાંક અંક_events,
				  સંરચના z_poller *poller,
				  bool just_check)
{
	પૂર્ણાંક events_registered = 0;

	માટે (પૂર્ણાંક ii = 0; ii < અંક_events; ii++) {
		k_spinlock_ચાવી_t ચાવી;
		uint32_t state;

		ચાવી = k_spin_lock(&poll_lock);
		જો (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} નહીંતર જો (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} નહીંતર {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, ચાવી);
	}

	પાછા events_registered;
}

સ્થિર પૂર્ણાંક signal_poller(સંરચના k_poll_event *event, uint32_t state)
{
	સંરચના k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != ખાલી, "poller should have a thread\n");

	જો (!z_is_thread_pending(thread)) {
		પાછા 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_કિંમત_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	જો (!z_is_thread_ready(thread)) {
		પાછા 0;
	}

	z_ready_thread(thread);

	પાછા 0;
}

પૂર્ણાંક z_impl_k_poll(સંરચના k_poll_event *events, પૂર્ણાંક અંક_events,
		  k_timeout_t timeout)
{
	પૂર્ણાંક events_registered;
	k_spinlock_ચાવી_t ચાવી;
	સંરચના z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != ખાલી, "NULL events\n");
	__ASSERT(અંક_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, અંક_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	ચાવી = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	જો (!poller->is_polling) {
		clear_event_registrations(events, events_registered, ચાવી);
		k_spin_unlock(&poll_lock, ચાવી);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		પાછા 0;
	}

	poller->is_polling = false;

	જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, ચાવી);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		પાછા -EAGAIN;
	}

	સ્થિર _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	પૂર્ણાંક swap_rc = z_pend_curr(&poll_lock, ચાવી, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	ચાવી = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, ચાવી);
	k_spin_unlock(&poll_lock, ચાવી);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	પાછા swap_rc;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_poll(સંરચના k_poll_event *events,
				પૂર્ણાંક અંક_events, k_timeout_t timeout)
{
	પૂર્ણાંક ret;
	k_spinlock_ચાવી_t ચાવી;
	સંરચના k_poll_event *events_copy = ખાલી;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	જો (K_SYSCALL_VERIFY(અંક_events >= 0)) {
		ret = -EINVAL;
		જાઓ out;
	}
	જો (K_SYSCALL_VERIFY_સંદેશ(!u32_mul_overflow(અંક_events,
						   કદ(સંરચના k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		જાઓ out;
	}
	events_copy = z_thread_malloc(bounds);
	જો (!events_copy) {
		ret = -ENOMEM;
		જાઓ out;
	}

	જો (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		જાઓ oops_free;
	}

	ચાવી = k_spin_lock(&poll_lock);
	(શૂન્ય)memcpy(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, ચાવી);

	/* Validate what's inside events_copy */
	માટે (પૂર્ણાંક i = 0; i < અંક_events; i++) {
		સંરચના k_poll_event *e = &events_copy[i];

		જો (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			જાઓ out_free;
		}

		પસંદગી (e->type) {
		સ્થિતિ K_POLL_TYPE_IGNORE:
			તોડો;
		સ્થિતિ K_POLL_TYPE_SIGNAL:
			જો (K_SYSCALL_OBJ(e->સંકેત, K_OBJ_POLL_SIGNAL)) {
				જાઓ oops_free;
			}
			તોડો;
		સ્થિતિ K_POLL_TYPE_SEM_AVAILABLE:
			જો (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				જાઓ oops_free;
			}
			તોડો;
		સ્થિતિ K_POLL_TYPE_DATA_AVAILABLE:
			જો (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				જાઓ oops_free;
			}
			તોડો;
		સ્થિતિ K_POLL_TYPE_MSGQ_DATA_AVAILABLE:
			જો (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				જાઓ oops_free;
			}
			તોડો;
		સ્થિતિ K_POLL_TYPE_PIPE_DATA_AVAILABLE:
			જો (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				જાઓ oops_free;
			}
			તોડો;
		મૂળભૂત:
			ret = -EINVAL;
			જાઓ out_free;
		}
	}

	ret = k_poll(events_copy, અંક_events, timeout);
	(શૂન્ય)memcpy((શૂન્ય *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	પાછા ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#અન્તર્ભાવ <zephyr/syscalls/k_poll_mrsh.c>
#endif /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
સ્થિર પૂર્ણાંક signal_poll_event(સંરચના k_poll_event *event, uint32_t state)
{
	સંરચના z_poller *poller = event->poller;
	પૂર્ણાંક retcode = 0;

	જો (poller != ખાલી) {
		જો (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} નહીંતર જો (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} નહીંતર {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		જો (retcode < 0) {
			પાછા retcode;
		}
	}

	set_event_ready(event, state);
	પાછા retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	સંરચના k_poll_event *poll_event;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&poll_lock);

	poll_event = (સંરચના k_poll_event *)sys_dlist_get(events);
	જો (poll_event != ખાલી) {
		(શૂન્ય) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, ચાવી);

	પાછા (poll_event != ખાલી);
}

શૂન્ય z_impl_k_poll_signal_init(સંરચના k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_poll_signal_init(સંરચના k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#અન્તર્ભાવ <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_impl_k_poll_signal_reset(સંરચના k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

શૂન્ય z_impl_k_poll_signal_check(સંરચના k_poll_signal *sig,
			       unsigned પૂર્ણાંક *signaled, પૂર્ણાંક *result)
{
	*signaled = sig->signaled;
	*result = sig->result;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#ifdef CONFIG_USERSPACE
શૂન્ય z_vrfy_k_poll_signal_check(સંરચના k_poll_signal *sig,
			       unsigned પૂર્ણાંક *signaled, પૂર્ણાંક *result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, કદ(unsigned પૂર્ણાંક)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(result, કદ(પૂર્ણાંક)));
	z_impl_k_poll_signal_check(sig, signaled, result);
}
#અન્તર્ભાવ <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_poll_signal_raise(સંરચના k_poll_signal *sig, પૂર્ણાંક result)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&poll_lock);
	સંરચના k_poll_event *poll_event;

	sig->result = result;
	sig->signaled = 1U;

	poll_event = (સંરચના k_poll_event *)sys_dlist_get(&sig->poll_events);
	જો (poll_event == ખાલી) {
		k_spin_unlock(&poll_lock, ચાવી);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		પાછા 0;
	}

	પૂર્ણાંક rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, ચાવી);
	પાછા rc;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_poll_signal_raise(સંરચના k_poll_signal *sig,
					     પૂર્ણાંક result)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	પાછા z_impl_k_poll_signal_raise(sig, result);
}
#અન્તર્ભાવ <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_poll_signal_reset(સંરચના k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#અન્તર્ભાવ <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#endif /* CONFIG_USERSPACE */

સ્થિર શૂન્ય triggered_work_handler(સંરચના k_work *work)
{
	સંરચના k_work_poll *twork =
			CONTAINER_OF(work, સંરચના k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	જો (twork->poller.mode != MODE_NONE) {
		k_spinlock_ચાવી_t ચાવી;

		ચાવી = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->અંક_events, ચાવી);
		k_spin_unlock(&poll_lock, ચાવી);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = ખાલી;
	twork->real_handler(work);
}

બાહ્ય પૂર્ણાંક z_work_submit_to_queue(સંરચના k_work_q *queue,
			 સંરચના k_work *work);

સ્થિર શૂન્ય triggered_work_expiration_handler(સંરચના _timeout *timeout)
{
	સંરચના k_work_poll *twork =
		CONTAINER_OF(timeout, સંરચના k_work_poll, timeout);
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&poll_lock);

	જો (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, ચાવી);
		પાછા;
	}

	twork->poller.is_polling = false;
	twork->poll_result = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, ચાવી);
}

સ્થિર પૂર્ણાંક signal_triggered_work(સંરચના k_poll_event *event, uint32_t status)
{
	સંરચના z_poller *poller = event->poller;
	સંરચના k_work_poll *twork =
		CONTAINER_OF(poller, સંરચના k_work_poll, poller);

	જો (poller->is_polling && twork->workq != ખાલી) {
		સંરચના k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(શૂન્ય)z_try_abort_timeout(&twork->timeout);
		twork->poll_result = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક triggered_work_cancel(સંરચના k_work_poll *work,
				 k_spinlock_ચાવી_t ચાવી)
{
	/* Check if the work waits for event. */
	જો (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		જ્યારે (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, ચાવી);
			ચાવી = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->અંક_events, ચાવી);
		work->workq = ખાલી;
		પાછા 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	પાછા -EINVAL;
}

શૂન્ય k_work_poll_init(સંરચના k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (સંરચના k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

પૂર્ણાંક k_work_poll_submit_to_queue(સંરચના k_work_q *work_q,
				સંરચના k_work_poll *work,
				સંરચના k_poll_event *events,
				પૂર્ણાંક અંક_events,
				k_timeout_t timeout)
{
	પૂર્ણાંક events_registered;
	k_spinlock_ચાવી_t ચાવી;

	__ASSERT(work_q != ખાલી, "NULL work_q\n");
	__ASSERT(work != ખાલી, "NULL work\n");
	__ASSERT(events != ખાલી, "NULL events\n");
	__ASSERT(અંક_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	ચાવી = k_spin_lock(&poll_lock);
	જો (work->workq != ખાલી) {
		જો (work->workq == work_q) {
			પૂર્ણાંક retval;

			retval = triggered_work_cancel(work, ચાવી);
			જો (retval < 0) {
				k_spin_unlock(&poll_lock, ચાવી);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				પાછા retval;
			}
		} નહીંતર {
			k_spin_unlock(&poll_lock, ચાવી);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			પાછા -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, ચાવી);

	/* Save list of events. */
	work->events = events;
	work->અંક_events = અંક_events;

	/* Clear result */
	work->poll_result = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, અંક_events,
					    &work->poller, false);

	ચાવી = k_spin_lock(&poll_lock);
	જો (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(અંક_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, ચાવી);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		પાછા 0;
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
	જો (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_result = -EAGAIN;
	} નહીંતર {
		work->poll_result = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, ચાવી);
	k_spin_unlock(&poll_lock, ચાવી);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	પાછા 0;
}

પૂર્ણાંક k_work_poll_submit(સંરચના k_work_poll *work,
				     સંરચના k_poll_event *events,
				     પૂર્ણાંક અંક_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	પૂર્ણાંક ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, અંક_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	પાછા ret;
}

પૂર્ણાંક k_work_poll_cancel(સંરચના k_work_poll *work)
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	જો (work == ખાલી || work->workq == ખાલી) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		પાછા -EINVAL;
	}

	ચાવી = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, ચાવી);
	k_spin_unlock(&poll_lock, ચાવી);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	પાછા retval;
}
