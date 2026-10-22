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

#समावेश <zephyr/kernel.h>
#समावेश <kernel_internal.h>
#समावेश <wait_q.h>
#समावेश <ksched.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/sys/dlist.h>
#समावेश <zephyr/sys/util.h>
#समावेश <zephyr/sys/__assert.h>
#समावेश <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
अटल काष्ठा k_spinlock poll_lock;

क्रमागत POLL_MODE { MODE_NONE, MODE_POLL, MODE_TRIGGERED };

अटल पूर्णांक signal_poller(काष्ठा k_poll_event *event, uint32_t state);
अटल पूर्णांक signal_triggered_work(काष्ठा k_poll_event *event, uint32_t status);

व्योम k_poll_event_init(काष्ठा k_poll_event *event, uint32_t type,
		       पूर्णांक mode, व्योम *obj)
{
	__ASSERT(mode == K_POLL_MODE_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (BIT(POLL_अंक_TYPES)), "invalid type\n");
	__ASSERT(obj != शून्य, "must provide an object\n");

	event->poller = शून्य;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->mode = mode;
	event->unused = 0U;
	event->obj = obj;

	SYS_PORT_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
अटल अंतरभूत bool is_condition_met(काष्ठा k_poll_event *event, uint32_t *state)
{
	चयन (event->type) {
	हाल K_POLL_TYPE_SEM_AVAILABLE:
		अगर (k_sem_गिनती_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_AVAILABLE;
			वापस true;
		}
		अवरोध;
	हाल K_POLL_TYPE_तथ्य_AVAILABLE:
		अगर (!k_queue_is_empty(event->queue)) {
			*state = K_POLL_STATE_FIFO_तथ्य_AVAILABLE;
			वापस true;
		}
		अवरोध;
	हाल K_POLL_TYPE_SIGNAL:
		अगर (event->संकेत->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			वापस true;
		}
		अवरोध;
	हाल K_POLL_TYPE_MSGQ_तथ्य_AVAILABLE:
		अगर (event->msgq->used_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_तथ्य_AVAILABLE;
			वापस true;
		}
		अवरोध;
	हाल K_POLL_TYPE_PIPE_तथ्य_AVAILABLE:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		अगर (!ring_buf_is_empty(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_तथ्य_AVAILABLE;
			वापस true;
		}
		अवरोध;
	हाल K_POLL_TYPE_IGNORE:
		अवरोध;
	शेष:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		अवरोध;
	}

	वापस false;
}

अटल काष्ठा k_thread *poller_thread(काष्ठा z_poller *p)
{
	वापस p ? CONTAINER_OF(p, काष्ठा k_thread, poller) : शून्य;
}

अटल अंतरभूत व्योम add_event(sys_dlist_t *events, काष्ठा k_poll_event *event,
			     काष्ठा z_poller *poller)
{
	काष्ठा k_poll_event *pending;

	pending = (काष्ठा k_poll_event *)sys_dlist_peek_पुच्छ(events);
	अगर ((pending == शून्य) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->गाँठ_2);
		वापस;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, गाँठ_2) {
		अगर (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_जोड़ो(&pending->गाँठ_2, &event->गाँठ_2);
			वापस;
		}
	}

	sys_dlist_append(events, &event->गाँठ_2);
}

/* must be called with interrupts locked */
अटल अंतरभूत व्योम register_event(काष्ठा k_poll_event *event,
				 काष्ठा z_poller *poller)
{
	चयन (event->type) {
	हाल K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != शून्य, "invalid semaphore\n");
		add_event(&event->sem->poll_events, event, poller);
		अवरोध;
	हाल K_POLL_TYPE_तथ्य_AVAILABLE:
		__ASSERT(event->queue != शून्य, "invalid queue\n");
		add_event(&event->queue->poll_events, event, poller);
		अवरोध;
	हाल K_POLL_TYPE_SIGNAL:
		__ASSERT(event->संकेत != शून्य, "invalid poll signal\n");
		add_event(&event->संकेत->poll_events, event, poller);
		अवरोध;
	हाल K_POLL_TYPE_MSGQ_तथ्य_AVAILABLE:
		__ASSERT(event->msgq != शून्य, "invalid message queue\n");
		add_event(&event->msgq->poll_events, event, poller);
		अवरोध;
	हाल K_POLL_TYPE_PIPE_तथ्य_AVAILABLE:
		__ASSERT(event->pipe != शून्य, "invalid pipe\n");
		add_event(&event->pipe->poll_events, event, poller);
		अवरोध;
	हाल K_POLL_TYPE_IGNORE:
		/* nothing to do */
		अवरोध;
	शेष:
		__ASSERT(false, "invalid event type\n");
		अवरोध;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
अटल अंतरभूत व्योम clear_event_registration(काष्ठा k_poll_event *event)
{
	bool remove_event = false;

	event->poller = शून्य;

	चयन (event->type) {
	हाल K_POLL_TYPE_SEM_AVAILABLE:
		__ASSERT(event->sem != शून्य, "invalid semaphore\n");
		remove_event = true;
		अवरोध;
	हाल K_POLL_TYPE_तथ्य_AVAILABLE:
		__ASSERT(event->queue != शून्य, "invalid queue\n");
		remove_event = true;
		अवरोध;
	हाल K_POLL_TYPE_SIGNAL:
		__ASSERT(event->संकेत != शून्य, "invalid poll signal\n");
		remove_event = true;
		अवरोध;
	हाल K_POLL_TYPE_MSGQ_तथ्य_AVAILABLE:
		__ASSERT(event->msgq != शून्य, "invalid message queue\n");
		remove_event = true;
		अवरोध;
	हाल K_POLL_TYPE_PIPE_तथ्य_AVAILABLE:
		__ASSERT(event->pipe != शून्य, "invalid pipe\n");
		remove_event = true;
		अवरोध;
	हाल K_POLL_TYPE_IGNORE:
		/* nothing to do */
		अवरोध;
	शेष:
		__ASSERT(false, "invalid event type\n");
		अवरोध;
	}
	अगर (remove_event && sys_dnode_is_linked(&event->गाँठ_2)) {
		sys_dlist_remove(&event->गाँठ_2);
	}
}

/* must be called with interrupts locked */
अटल अंतरभूत व्योम clear_event_registrations(काष्ठा k_poll_event *events,
					      पूर्णांक अंक_events,
					      k_spinlock_कुंजी_t कुंजी)
{
	जबतक (अंक_events--) {
		clear_event_registration(&events[अंक_events]);
		k_spin_unlock(&poll_lock, कुंजी);
		कुंजी = k_spin_lock(&poll_lock);
	}
}

अटल अंतरभूत व्योम set_event_ready(काष्ठा k_poll_event *event, uint32_t state)
{
	event->poller = शून्य;
	event->state |= state;
}

अटल अंतरभूत पूर्णांक register_events(काष्ठा k_poll_event *events,
				  पूर्णांक अंक_events,
				  काष्ठा z_poller *poller,
				  bool just_check)
{
	पूर्णांक events_registered = 0;

	क्रम (पूर्णांक ii = 0; ii < अंक_events; ii++) {
		k_spinlock_कुंजी_t कुंजी;
		uint32_t state;

		कुंजी = k_spin_lock(&poll_lock);
		अगर (is_condition_met(&events[ii], &state)) {
			set_event_ready(&events[ii], state);
			poller->is_polling = false;
		} अन्यथा अगर (!just_check && poller->is_polling) {
			register_event(&events[ii], poller);
			events_registered += 1;
		} अन्यथा {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_lock, कुंजी);
	}

	वापस events_registered;
}

अटल पूर्णांक signal_poller(काष्ठा k_poll_event *event, uint32_t state)
{
	काष्ठा k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != शून्य, "poller should have a thread\n");

	अगर (!z_is_thread_pending(thread)) {
		वापस 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_मूल्य_set(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	अगर (!z_is_thread_ready(thread)) {
		वापस 0;
	}

	z_ready_thread(thread);

	वापस 0;
}

पूर्णांक z_impl_k_poll(काष्ठा k_poll_event *events, पूर्णांक अंक_events,
		  k_timeout_t timeout)
{
	पूर्णांक events_registered;
	k_spinlock_कुंजी_t कुंजी;
	काष्ठा z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->mode = MODE_POLL;

	__ASSERT(!arch_is_in_isr(), "");
	__ASSERT(events != शून्य, "NULL events\n");
	__ASSERT(अंक_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = register_events(events, अंक_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	कुंजी = k_spin_lock(&poll_lock);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	अगर (!poller->is_polling) {
		clear_event_registrations(events, events_registered, कुंजी);
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		वापस 0;
	}

	poller->is_polling = false;

	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		वापस -EAGAIN;
	}

	अटल _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	पूर्णांक swap_rc = z_pend_curr(&poll_lock, कुंजी, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	कुंजी = k_spin_lock(&poll_lock);
	clear_event_registrations(events, events_registered, कुंजी);
	k_spin_unlock(&poll_lock, कुंजी);

	SYS_PORT_TRACING_FUNC_EXIT(k_poll_api, poll, events, swap_rc);

	वापस swap_rc;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_poll(काष्ठा k_poll_event *events,
				पूर्णांक अंक_events, k_timeout_t timeout)
{
	पूर्णांक ret;
	k_spinlock_कुंजी_t कुंजी;
	काष्ठा k_poll_event *events_copy = शून्य;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	अगर (K_SYSCALL_VERIFY(अंक_events >= 0)) {
		ret = -EINVAL;
		जाओ out;
	}
	अगर (K_SYSCALL_VERIFY_संदेश(!u32_mul_overflow(अंक_events,
						   माप(काष्ठा k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		जाओ out;
	}
	events_copy = z_thread_malloc(bounds);
	अगर (!events_copy) {
		ret = -ENOMEM;
		जाओ out;
	}

	अगर (K_SYSCALL_MEMORY_WRITE(events, bounds)) {
		जाओ oops_free;
	}

	कुंजी = k_spin_lock(&poll_lock);
	(व्योम)स_नकल(events_copy, events, bounds);
	k_spin_unlock(&poll_lock, कुंजी);

	/* Validate what's inside events_copy */
	क्रम (पूर्णांक i = 0; i < अंक_events; i++) {
		काष्ठा k_poll_event *e = &events_copy[i];

		अगर (K_SYSCALL_VERIFY(e->mode == K_POLL_MODE_NOTIFY_ONLY)) {
			ret = -EINVAL;
			जाओ out_free;
		}

		चयन (e->type) {
		हाल K_POLL_TYPE_IGNORE:
			अवरोध;
		हाल K_POLL_TYPE_SIGNAL:
			अगर (K_SYSCALL_OBJ(e->संकेत, K_OBJ_POLL_SIGNAL)) {
				जाओ oops_free;
			}
			अवरोध;
		हाल K_POLL_TYPE_SEM_AVAILABLE:
			अगर (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				जाओ oops_free;
			}
			अवरोध;
		हाल K_POLL_TYPE_तथ्य_AVAILABLE:
			अगर (K_SYSCALL_OBJ(e->queue, K_OBJ_QUEUE)) {
				जाओ oops_free;
			}
			अवरोध;
		हाल K_POLL_TYPE_MSGQ_तथ्य_AVAILABLE:
			अगर (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				जाओ oops_free;
			}
			अवरोध;
		हाल K_POLL_TYPE_PIPE_तथ्य_AVAILABLE:
			अगर (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				जाओ oops_free;
			}
			अवरोध;
		शेष:
			ret = -EINVAL;
			जाओ out_free;
		}
	}

	ret = k_poll(events_copy, अंक_events, timeout);
	(व्योम)स_नकल((व्योम *)events, events_copy, bounds);
out_free:
	k_free(events_copy);
out:
	वापस ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#समावेश <zephyr/syscalls/k_poll_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
अटल पूर्णांक signal_poll_event(काष्ठा k_poll_event *event, uint32_t state)
{
	काष्ठा z_poller *poller = event->poller;
	पूर्णांक retcode = 0;

	अगर (poller != शून्य) {
		अगर (poller->mode == MODE_POLL) {
			retcode = signal_poller(event, state);
		} अन्यथा अगर (poller->mode == MODE_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} अन्यथा {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		अगर (retcode < 0) {
			वापस retcode;
		}
	}

	set_event_ready(event, state);
	वापस retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	काष्ठा k_poll_event *poll_event;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&poll_lock);

	poll_event = (काष्ठा k_poll_event *)sys_dlist_get(events);
	अगर (poll_event != शून्य) {
		(व्योम) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_lock, कुंजी);

	वापस (poll_event != शून्य);
}

व्योम z_impl_k_poll_signal_init(काष्ठा k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_poll_signal_init(काष्ठा k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#समावेश <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_impl_k_poll_signal_reset(काष्ठा k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

व्योम z_impl_k_poll_signal_check(काष्ठा k_poll_signal *sig,
			       अचिन्हित पूर्णांक *signaled, पूर्णांक *परिणाम)
{
	*signaled = sig->signaled;
	*परिणाम = sig->परिणाम;

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#अगर_घोषित CONFIG_USERSPACE
व्योम z_vrfy_k_poll_signal_check(काष्ठा k_poll_signal *sig,
			       अचिन्हित पूर्णांक *signaled, पूर्णांक *परिणाम)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(signaled, माप(अचिन्हित पूर्णांक)));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(परिणाम, माप(पूर्णांक)));
	z_impl_k_poll_signal_check(sig, signaled, परिणाम);
}
#समावेश <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_poll_signal_raise(काष्ठा k_poll_signal *sig, पूर्णांक परिणाम)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&poll_lock);
	काष्ठा k_poll_event *poll_event;

	sig->परिणाम = परिणाम;
	sig->signaled = 1U;

	poll_event = (काष्ठा k_poll_event *)sys_dlist_get(&sig->poll_events);
	अगर (poll_event == शून्य) {
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		वापस 0;
	}

	पूर्णांक rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_PORT_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_lock, कुंजी);
	वापस rc;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_poll_signal_raise(काष्ठा k_poll_signal *sig,
					     पूर्णांक परिणाम)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	वापस z_impl_k_poll_signal_raise(sig, परिणाम);
}
#समावेश <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

अटल अंतरभूत व्योम z_vrfy_k_poll_signal_reset(काष्ठा k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#समावेश <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#पूर्ण_अगर /* CONFIG_USERSPACE */

अटल व्योम triggered_work_handler(काष्ठा k_work *work)
{
	काष्ठा k_work_poll *twork =
			CONTAINER_OF(work, काष्ठा k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	अगर (twork->poller.mode != MODE_NONE) {
		k_spinlock_कुंजी_t कुंजी;

		कुंजी = k_spin_lock(&poll_lock);
		clear_event_registrations(twork->events,
					  twork->अंक_events, कुंजी);
		k_spin_unlock(&poll_lock, कुंजी);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = शून्य;
	twork->real_handler(work);
}

बाह्य पूर्णांक z_work_submit_to_queue(काष्ठा k_work_q *queue,
			 काष्ठा k_work *work);

अटल व्योम triggered_work_expiration_handler(काष्ठा _timeout *timeout)
{
	काष्ठा k_work_poll *twork =
		CONTAINER_OF(timeout, काष्ठा k_work_poll, timeout);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&poll_lock);

	अगर (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_lock, कुंजी);
		वापस;
	}

	twork->poller.is_polling = false;
	twork->poll_परिणाम = -EAGAIN;
	z_work_submit_to_queue(twork->workq, &twork->work);

	k_spin_unlock(&poll_lock, कुंजी);
}

अटल पूर्णांक signal_triggered_work(काष्ठा k_poll_event *event, uint32_t status)
{
	काष्ठा z_poller *poller = event->poller;
	काष्ठा k_work_poll *twork =
		CONTAINER_OF(poller, काष्ठा k_work_poll, poller);

	अगर (poller->is_polling && twork->workq != शून्य) {
		काष्ठा k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(व्योम)z_try_abort_timeout(&twork->timeout);
		twork->poll_परिणाम = 0;
		z_work_submit_to_queue(work_q, &twork->work);
	}

	वापस 0;
}

अटल पूर्णांक triggered_work_cancel(काष्ठा k_work_poll *work,
				 k_spinlock_कुंजी_t कुंजी)
{
	/* Check if the work waits for event. */
	अगर (work->poller.is_polling && work->poller.mode != MODE_NONE) {
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
		जबतक (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_lock, कुंजी);
			कुंजी = k_spin_lock(&poll_lock);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.mode = MODE_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->अंक_events, कुंजी);
		work->workq = शून्य;
		वापस 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	वापस -EINVAL;
}

व्योम k_work_poll_init(काष्ठा k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (काष्ठा k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

पूर्णांक k_work_poll_submit_to_queue(काष्ठा k_work_q *work_q,
				काष्ठा k_work_poll *work,
				काष्ठा k_poll_event *events,
				पूर्णांक अंक_events,
				k_timeout_t timeout)
{
	पूर्णांक events_registered;
	k_spinlock_कुंजी_t कुंजी;

	__ASSERT(work_q != शून्य, "NULL work_q\n");
	__ASSERT(work != शून्य, "NULL work\n");
	__ASSERT(events != शून्य, "NULL events\n");
	__ASSERT(अंक_events >= 0, "<0 events\n");

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit_to_queue, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	कुंजी = k_spin_lock(&poll_lock);
	अगर (work->workq != शून्य) {
		अगर (work->workq == work_q) {
			पूर्णांक retval;

			retval = triggered_work_cancel(work, कुंजी);
			अगर (retval < 0) {
				k_spin_unlock(&poll_lock, कुंजी);

				SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
					work, timeout, retval);

				वापस retval;
			}
		} अन्यथा {
			k_spin_unlock(&poll_lock, कुंजी);

			SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q,
				work, timeout, -EADDRINUSE);

			वापस -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.mode = MODE_NONE;
	k_spin_unlock(&poll_lock, कुंजी);

	/* Save list of events. */
	work->events = events;
	work->अंक_events = अंक_events;

	/* Clear result */
	work->poll_परिणाम = -EINPROGRESS;

	/* Register events */
	events_registered = register_events(events, अंक_events,
					    &work->poller, false);

	कुंजी = k_spin_lock(&poll_lock);
	अगर (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(अंक_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_add_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.mode = MODE_TRIGGERED;
		k_spin_unlock(&poll_lock, कुंजी);

		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

		वापस 0;
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
	अगर (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_परिणाम = -EAGAIN;
	} अन्यथा {
		work->poll_परिणाम = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, कुंजी);
	k_spin_unlock(&poll_lock, कुंजी);

	/* Submit work. */
	k_work_submit_to_queue(work_q, &work->work);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit_to_queue, work_q, work, timeout, 0);

	वापस 0;
}

पूर्णांक k_work_poll_submit(काष्ठा k_work_poll *work,
				     काष्ठा k_poll_event *events,
				     पूर्णांक अंक_events,
				     k_timeout_t timeout)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	पूर्णांक ret = k_work_poll_submit_to_queue(&k_sys_work_q, work,
								events, अंक_events, timeout);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	वापस ret;
}

पूर्णांक k_work_poll_cancel(काष्ठा k_work_poll *work)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक retval;

	SYS_PORT_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	अगर (work == शून्य || work->workq == शून्य) {
		SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		वापस -EINVAL;
	}

	कुंजी = k_spin_lock(&poll_lock);
	retval = triggered_work_cancel(work, कुंजी);
	k_spin_unlock(&poll_lock, कुंजी);

	SYS_PORT_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	वापस retval;
}
