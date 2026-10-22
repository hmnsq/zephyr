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

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
#అన్తర్భావ <zephyr/sys/util.h>
#అన్తర్భావ <zephyr/sys/__assert.h>
#అన్తర్భావ <stdbool.h>

/* Single subsystem lock.  Locking per-event would be better on highly
 * contended SMP systems, but the original locking scheme here is
 * subtle (it relies on releasing/reacquiring the lock in areas for
 * latency control and it's sometimes hard to see exactly what data is
 * "inside" a given critical section).  Do the synchronization port
 * later as an optimization.
 */
స్థిర నిర్మాణం k_spinlock poll_తాళం;

గణన POLL_బహుళకం { బహుళకం_NONE, బహుళకం_POLL, బహుళకం_TRIGGERED };

స్థిర పూర్ణాంకం signal_poller(నిర్మాణం k_poll_event *event, uint32_t state);
స్థిర పూర్ణాంకం signal_triggered_work(నిర్మాణం k_poll_event *event, uint32_t స్థితి_2);

శూన్య k_poll_event_init(నిర్మాణం k_poll_event *event, uint32_t type,
		       పూర్ణాంకం బహుళకం, శూన్య *obj)
{
	__ASSERT(బహుళకం == K_POLL_బహుళకం_NOTIFY_ONLY,
		 "only NOTIFY_ONLY mode is supported\n");
	__ASSERT(type < (బిట్టు(POLL_అంకె_TYPES)), "invalid type\n");
	__ASSERT(obj != ఖాళీ, "must provide an object\n");

	event->poller = ఖాళీ;
	/* event->tag is left uninitialized: the user will set it if needed */
	event->type = type;
	event->state = K_POLL_STATE_NOT_READY;
	event->బహుళకం = బహుళకం;
	event->వాడని = 0U;
	event->obj = obj;

	SYS_పోర్టు_TRACING_FUNC(k_poll_api, event_init, event);
}

/* must be called with interrupts locked */
స్థిర అంతఃవరుస bool is_condition_met(నిర్మాణం k_poll_event *event, uint32_t *state)
{
	ఎంపిక (event->type) {
	స్థితి K_POLL_TYPE_SEM_అందుబాటు:
		ఒకవేళ (k_sem_లెక్క_get(event->sem) > 0U) {
			*state = K_POLL_STATE_SEM_అందుబాటు;
			తిరిగి true;
		}
		ఆపు;
	స్థితి K_POLL_TYPE_దత్తాంశం_అందుబాటు:
		ఒకవేళ (!k_వరుస_is_ఖాళీలు(event->వరుస)) {
			*state = K_POLL_STATE_FIFO_దత్తాంశం_అందుబాటు;
			తిరిగి true;
		}
		ఆపు;
	స్థితి K_POLL_TYPE_SIGNAL:
		ఒకవేళ (event->సంకేతం->signaled != 0U) {
			*state = K_POLL_STATE_SIGNALED;
			తిరిగి true;
		}
		ఆపు;
	స్థితి K_POLL_TYPE_MSGQ_దత్తాంశం_అందుబాటు:
		ఒకవేళ (event->msgq->వాడిన_msgs > 0) {
			*state = K_POLL_STATE_MSGQ_దత్తాంశం_అందుబాటు;
			తిరిగి true;
		}
		ఆపు;
	స్థితి K_POLL_TYPE_PIPE_దత్తాంశం_అందుబాటు:
		/*
		 * A closed pipe is readable: k_pipe_read() returns any
		 * remaining bytes, then -EPIPE. Without the closed check,
		 * k_poll() waits forever on a closed empty pipe.
		 */
		ఒకవేళ (!ring_buf_is_ఖాళీలు(&event->pipe->buf) ||
		    (event->pipe->flags & PIPE_FLAG_OPEN) == 0U) {
			*state = K_POLL_STATE_PIPE_దత్తాంశం_అందుబాటు;
			తిరిగి true;
		}
		ఆపు;
	స్థితి K_POLL_TYPE_IGNORE:
		ఆపు;
	అప్రమేయ:
		__ASSERT(false, "invalid event type (0x%x)\n", event->type);
		ఆపు;
	}

	తిరిగి false;
}

స్థిర నిర్మాణం k_thread *poller_thread(నిర్మాణం z_poller *p)
{
	తిరిగి p ? CONTAINER_యొక్క(p, నిర్మాణం k_thread, poller) : ఖాళీ;
}

స్థిర అంతఃవరుస శూన్య కూడు_event(sys_dlist_t *events, నిర్మాణం k_poll_event *event,
			     నిర్మాణం z_poller *poller)
{
	నిర్మాణం k_poll_event *pending;

	pending = (నిర్మాణం k_poll_event *)sys_dlist_peek_తోక(events);
	ఒకవేళ ((pending == ఖాళీ) ||
		(z_sched_prio_cmp(poller_thread(pending->poller),
							   poller_thread(poller)) > 0)) {
		sys_dlist_append(events, &event->కణుపు_2);
		తిరిగి;
	}

	SYS_DLIST_FOR_EACH_CONTAINER(events, pending, కణుపు_2) {
		ఒకవేళ (z_sched_prio_cmp(poller_thread(poller),
					poller_thread(pending->poller)) > 0) {
			sys_dlist_చొప్పించు(&pending->కణుపు_2, &event->కణుపు_2);
			తిరిగి;
		}
	}

	sys_dlist_append(events, &event->కణుపు_2);
}

/* must be called with interrupts locked */
స్థిర అంతఃవరుస శూన్య రిజిస్టరు_event(నిర్మాణం k_poll_event *event,
				 నిర్మాణం z_poller *poller)
{
	ఎంపిక (event->type) {
	స్థితి K_POLL_TYPE_SEM_అందుబాటు:
		__ASSERT(event->sem != ఖాళీ, "invalid semaphore\n");
		కూడు_event(&event->sem->poll_events, event, poller);
		ఆపు;
	స్థితి K_POLL_TYPE_దత్తాంశం_అందుబాటు:
		__ASSERT(event->వరుస != ఖాళీ, "invalid queue\n");
		కూడు_event(&event->వరుస->poll_events, event, poller);
		ఆపు;
	స్థితి K_POLL_TYPE_SIGNAL:
		__ASSERT(event->సంకేతం != ఖాళీ, "invalid poll signal\n");
		కూడు_event(&event->సంకేతం->poll_events, event, poller);
		ఆపు;
	స్థితి K_POLL_TYPE_MSGQ_దత్తాంశం_అందుబాటు:
		__ASSERT(event->msgq != ఖాళీ, "invalid message queue\n");
		కూడు_event(&event->msgq->poll_events, event, poller);
		ఆపు;
	స్థితి K_POLL_TYPE_PIPE_దత్తాంశం_అందుబాటు:
		__ASSERT(event->pipe != ఖాళీ, "invalid pipe\n");
		కూడు_event(&event->pipe->poll_events, event, poller);
		ఆపు;
	స్థితి K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ఆపు;
	అప్రమేయ:
		__ASSERT(false, "invalid event type\n");
		ఆపు;
	}

	event->poller = poller;
}

/* must be called with interrupts locked */
స్థిర అంతఃవరుస శూన్య clear_event_registration(నిర్మాణం k_poll_event *event)
{
	bool remove_event = false;

	event->poller = ఖాళీ;

	ఎంపిక (event->type) {
	స్థితి K_POLL_TYPE_SEM_అందుబాటు:
		__ASSERT(event->sem != ఖాళీ, "invalid semaphore\n");
		remove_event = true;
		ఆపు;
	స్థితి K_POLL_TYPE_దత్తాంశం_అందుబాటు:
		__ASSERT(event->వరుస != ఖాళీ, "invalid queue\n");
		remove_event = true;
		ఆపు;
	స్థితి K_POLL_TYPE_SIGNAL:
		__ASSERT(event->సంకేతం != ఖాళీ, "invalid poll signal\n");
		remove_event = true;
		ఆపు;
	స్థితి K_POLL_TYPE_MSGQ_దత్తాంశం_అందుబాటు:
		__ASSERT(event->msgq != ఖాళీ, "invalid message queue\n");
		remove_event = true;
		ఆపు;
	స్థితి K_POLL_TYPE_PIPE_దత్తాంశం_అందుబాటు:
		__ASSERT(event->pipe != ఖాళీ, "invalid pipe\n");
		remove_event = true;
		ఆపు;
	స్థితి K_POLL_TYPE_IGNORE:
		/* nothing to do */
		ఆపు;
	అప్రమేయ:
		__ASSERT(false, "invalid event type\n");
		ఆపు;
	}
	ఒకవేళ (remove_event && sys_dnode_is_linked(&event->కణుపు_2)) {
		sys_dlist_remove(&event->కణుపు_2);
	}
}

/* must be called with interrupts locked */
స్థిర అంతఃవరుస శూన్య clear_event_registrations(నిర్మాణం k_poll_event *events,
					      పూర్ణాంకం అంకె_events,
					      k_spinlock_కీలకం_t కీలకం)
{
	అయితే (అంకె_events--) {
		clear_event_registration(&events[అంకె_events]);
		k_spin_unlock(&poll_తాళం, కీలకం);
		కీలకం = k_spin_తాళం(&poll_తాళం);
	}
}

స్థిర అంతఃవరుస శూన్య సమితి_event_ready(నిర్మాణం k_poll_event *event, uint32_t state)
{
	event->poller = ఖాళీ;
	event->state |= state;
}

స్థిర అంతఃవరుస పూర్ణాంకం రిజిస్టరు_events(నిర్మాణం k_poll_event *events,
				  పూర్ణాంకం అంకె_events,
				  నిర్మాణం z_poller *poller,
				  bool just_check)
{
	పూర్ణాంకం events_registered = 0;

	కొరకు (పూర్ణాంకం ii = 0; ii < అంకె_events; ii++) {
		k_spinlock_కీలకం_t కీలకం;
		uint32_t state;

		కీలకం = k_spin_తాళం(&poll_తాళం);
		ఒకవేళ (is_condition_met(&events[ii], &state)) {
			సమితి_event_ready(&events[ii], state);
			poller->is_polling = false;
		} లేకపోతే ఒకవేళ (!just_check && poller->is_polling) {
			రిజిస్టరు_event(&events[ii], poller);
			events_registered += 1;
		} లేకపోతే {
			/* Event is not one of those identified in is_condition_met()
			 * catching non-polling events, or is marked for just check,
			 * or not marked for polling. No action needed.
			 */
			;
		}
		k_spin_unlock(&poll_తాళం, కీలకం);
	}

	తిరిగి events_registered;
}

స్థిర పూర్ణాంకం signal_poller(నిర్మాణం k_poll_event *event, uint32_t state)
{
	నిర్మాణం k_thread *thread = poller_thread(event->poller);

	__ASSERT(thread != ఖాళీ, "poller should have a thread\n");

	ఒకవేళ (!z_is_thread_pending(thread)) {
		తిరిగి 0;
	}

	z_unpend_thread(thread);
	arch_thread_return_విలువ_సమితి(thread,
		state == K_POLL_STATE_CANCELLED ? -EINTR : 0);

	ఒకవేళ (!z_is_thread_ready(thread)) {
		తిరిగి 0;
	}

	z_ready_thread(thread);

	తిరిగి 0;
}

పూర్ణాంకం z_impl_k_poll(నిర్మాణం k_poll_event *events, పూర్ణాంకం అంకె_events,
		  k_timeout_t timeout)
{
	పూర్ణాంకం events_registered;
	k_spinlock_కీలకం_t కీలకం;
	నిర్మాణం z_poller *poller = &_current->poller;

	poller->is_polling = true;
	poller->బహుళకం = బహుళకం_POLL;

	__ASSERT(!arch_is_లోపల_isr(), "");
	__ASSERT(events != ఖాళీ, "NULL events\n");
	__ASSERT(అంకె_events >= 0, "<0 events\n");

	SYS_పోర్టు_TRACING_FUNC_ENTER(k_poll_api, poll, events);

	events_registered = రిజిస్టరు_events(events, అంకె_events, poller,
					    K_TIMEOUT_EQ(timeout, K_NO_WAIT));

	కీలకం = k_spin_తాళం(&poll_తాళం);

	/*
	 * If we're not polling anymore, it means that at least one event
	 * condition is met, either when looping through the events here or
	 * because one of the events registered has had its state changed.
	 */
	ఒకవేళ (!poller->is_polling) {
		clear_event_registrations(events, events_registered, కీలకం);
		k_spin_unlock(&poll_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_FUNC_EXIT(k_poll_api, poll, events, 0);

		తిరిగి 0;
	}

	poller->is_polling = false;

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&poll_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_FUNC_EXIT(k_poll_api, poll, events, -EAGAIN);

		తిరిగి -EAGAIN;
	}

	స్థిర _wait_q_t wait_q = Z_WAIT_Q_INIT(&wait_q);

	పూర్ణాంకం మార్పిడి_rc = z_pend_curr(&poll_తాళం, కీలకం, &wait_q, timeout);

	/*
	 * Clear all event registrations. If events happen while we're in this
	 * loop, and we already had one that triggered, that's OK: they will
	 * end up in the list of events that are ready; if we timed out, and
	 * events happen while we're in this loop, that is OK as well since
	 * we've already know the return code (-EAGAIN), and even if they are
	 * added to the list of events that occurred, the user has to check the
	 * return code first, which invalidates the whole list of event states.
	 */
	కీలకం = k_spin_తాళం(&poll_తాళం);
	clear_event_registrations(events, events_registered, కీలకం);
	k_spin_unlock(&poll_తాళం, కీలకం);

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_poll_api, poll, events, మార్పిడి_rc);

	తిరిగి మార్పిడి_rc;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_poll(నిర్మాణం k_poll_event *events,
				పూర్ణాంకం అంకె_events, k_timeout_t timeout)
{
	పూర్ణాంకం ret;
	k_spinlock_కీలకం_t కీలకం;
	నిర్మాణం k_poll_event *events_copy = ఖాళీ;
	uint32_t bounds;

	/* Validate the events buffer and make a copy of it in an
	 * allocated kernel-side buffer.
	 */
	ఒకవేళ (K_SYSCALL_VERIFY(అంకె_events >= 0)) {
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}
	ఒకవేళ (K_SYSCALL_VERIFY_సందేశం(!u32_mul_అధికప్రవాహం(అంకె_events,
						   పరిమాణం(నిర్మాణం k_poll_event),
						   &bounds),
				 "num_events too large")) {
		ret = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}
	events_copy = z_thread_malloc(bounds);
	ఒకవేళ (!events_copy) {
		ret = -ENOMEM;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (K_SYSCALL_జ్ఞాపకం_WRITE(events, bounds)) {
		వెళ్ళు oops_free;
	}

	కీలకం = k_spin_తాళం(&poll_తాళం);
	(శూన్య)జ్ఞాపకం_నకలు(events_copy, events, bounds);
	k_spin_unlock(&poll_తాళం, కీలకం);

	/* Validate what's inside events_copy */
	కొరకు (పూర్ణాంకం i = 0; i < అంకె_events; i++) {
		నిర్మాణం k_poll_event *e = &events_copy[i];

		ఒకవేళ (K_SYSCALL_VERIFY(e->బహుళకం == K_POLL_బహుళకం_NOTIFY_ONLY)) {
			ret = -EINVAL;
			వెళ్ళు బహిర్గతం_free;
		}

		ఎంపిక (e->type) {
		స్థితి K_POLL_TYPE_IGNORE:
			ఆపు;
		స్థితి K_POLL_TYPE_SIGNAL:
			ఒకవేళ (K_SYSCALL_OBJ(e->సంకేతం, K_OBJ_POLL_SIGNAL)) {
				వెళ్ళు oops_free;
			}
			ఆపు;
		స్థితి K_POLL_TYPE_SEM_అందుబాటు:
			ఒకవేళ (K_SYSCALL_OBJ(e->sem, K_OBJ_SEM)) {
				వెళ్ళు oops_free;
			}
			ఆపు;
		స్థితి K_POLL_TYPE_దత్తాంశం_అందుబాటు:
			ఒకవేళ (K_SYSCALL_OBJ(e->వరుస, K_OBJ_వరుస)) {
				వెళ్ళు oops_free;
			}
			ఆపు;
		స్థితి K_POLL_TYPE_MSGQ_దత్తాంశం_అందుబాటు:
			ఒకవేళ (K_SYSCALL_OBJ(e->msgq, K_OBJ_MSGQ)) {
				వెళ్ళు oops_free;
			}
			ఆపు;
		స్థితి K_POLL_TYPE_PIPE_దత్తాంశం_అందుబాటు:
			ఒకవేళ (K_SYSCALL_OBJ(e->pipe, K_OBJ_PIPE)) {
				వెళ్ళు oops_free;
			}
			ఆపు;
		అప్రమేయ:
			ret = -EINVAL;
			వెళ్ళు బహిర్గతం_free;
		}
	}

	ret = k_poll(events_copy, అంకె_events, timeout);
	(శూన్య)జ్ఞాపకం_నకలు((శూన్య *)events, events_copy, bounds);
బహిర్గతం_free:
	k_free(events_copy);
బహిర్గతం:
	తిరిగి ret;
oops_free:
	k_free(events_copy);
	K_OOPS(1);
}
#అన్తర్భావ <zephyr/syscalls/k_poll_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

/* must be called with interrupts locked */
స్థిర పూర్ణాంకం signal_poll_event(నిర్మాణం k_poll_event *event, uint32_t state)
{
	నిర్మాణం z_poller *poller = event->poller;
	పూర్ణాంకం retcode = 0;

	ఒకవేళ (poller != ఖాళీ) {
		ఒకవేళ (poller->బహుళకం == బహుళకం_POLL) {
			retcode = signal_poller(event, state);
		} లేకపోతే ఒకవేళ (poller->బహుళకం == బహుళకం_TRIGGERED) {
			retcode = signal_triggered_work(event, state);
		} లేకపోతే {
			/* Poller is not poll or triggered mode. No action needed.*/
			;
		}

		poller->is_polling = false;

		ఒకవేళ (retcode < 0) {
			తిరిగి retcode;
		}
	}

	సమితి_event_ready(event, state);
	తిరిగి retcode;
}

bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state)
{
	నిర్మాణం k_poll_event *poll_event;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&poll_తాళం);

	poll_event = (నిర్మాణం k_poll_event *)sys_dlist_get(events);
	ఒకవేళ (poll_event != ఖాళీ) {
		(శూన్య) signal_poll_event(poll_event, state);
	}

	k_spin_unlock(&poll_తాళం, కీలకం);

	తిరిగి (poll_event != ఖాళీ);
}

శూన్య z_impl_k_poll_signal_init(నిర్మాణం k_poll_signal *sig)
{
	sys_dlist_init(&sig->poll_events);
	sig->signaled = 0U;
	/* signal->result is left uninitialized */
	k_object_init(sig);

	SYS_పోర్టు_TRACING_FUNC(k_poll_api, signal_init, sig);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_poll_signal_init(నిర్మాణం k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_init(sig);
}
#అన్తర్భావ <zephyr/syscalls/k_poll_signal_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_impl_k_poll_signal_reset(నిర్మాణం k_poll_signal *sig)
{
	sig->signaled = 0U;

	SYS_పోర్టు_TRACING_FUNC(k_poll_api, signal_reset, sig);
}

శూన్య z_impl_k_poll_signal_check(నిర్మాణం k_poll_signal *sig,
			       గుర్తులేని పూర్ణాంకం *signaled, పూర్ణాంకం *ఫలితం)
{
	*signaled = sig->signaled;
	*ఫలితం = sig->ఫలితం;

	SYS_పోర్టు_TRACING_FUNC(k_poll_api, signal_check, sig);
}

#నిర్వచితమైతే CONFIG_USERSPACE
శూన్య z_vrfy_k_poll_signal_check(నిర్మాణం k_poll_signal *sig,
			       గుర్తులేని పూర్ణాంకం *signaled, పూర్ణాంకం *ఫలితం)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(signaled, పరిమాణం(గుర్తులేని పూర్ణాంకం)));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(ఫలితం, పరిమాణం(పూర్ణాంకం)));
	z_impl_k_poll_signal_check(sig, signaled, ఫలితం);
}
#అన్తర్భావ <zephyr/syscalls/k_poll_signal_check_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_poll_signal_raise(నిర్మాణం k_poll_signal *sig, పూర్ణాంకం ఫలితం)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&poll_తాళం);
	నిర్మాణం k_poll_event *poll_event;

	sig->ఫలితం = ఫలితం;
	sig->signaled = 1U;

	poll_event = (నిర్మాణం k_poll_event *)sys_dlist_get(&sig->poll_events);
	ఒకవేళ (poll_event == ఖాళీ) {
		k_spin_unlock(&poll_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_FUNC(k_poll_api, signal_raise, sig, 0);

		తిరిగి 0;
	}

	పూర్ణాంకం rc = signal_poll_event(poll_event, K_POLL_STATE_SIGNALED);

	SYS_పోర్టు_TRACING_FUNC(k_poll_api, signal_raise, sig, rc);

	z_reschedule(&poll_తాళం, కీలకం);
	తిరిగి rc;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_poll_signal_raise(నిర్మాణం k_poll_signal *sig,
					     పూర్ణాంకం ఫలితం)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	తిరిగి z_impl_k_poll_signal_raise(sig, ఫలితం);
}
#అన్తర్భావ <zephyr/syscalls/k_poll_signal_raise_mrsh.c>

స్థిర అంతఃవరుస శూన్య z_vrfy_k_poll_signal_reset(నిర్మాణం k_poll_signal *sig)
{
	K_OOPS(K_SYSCALL_OBJ(sig, K_OBJ_POLL_SIGNAL));
	z_impl_k_poll_signal_reset(sig);
}
#అన్తర్భావ <zephyr/syscalls/k_poll_signal_reset_mrsh.c>

#అయితే_ముగింపు /* CONFIG_USERSPACE */

స్థిర శూన్య triggered_work_handler(నిర్మాణం k_work *work)
{
	నిర్మాణం k_work_poll *twork =
			CONTAINER_యొక్క(work, నిర్మాణం k_work_poll, work);

	/*
	 * If callback is not set, the k_work_poll_submit_to_queue()
	 * already cleared event registrations.
	 */
	ఒకవేళ (twork->poller.బహుళకం != బహుళకం_NONE) {
		k_spinlock_కీలకం_t కీలకం;

		కీలకం = k_spin_తాళం(&poll_తాళం);
		clear_event_registrations(twork->events,
					  twork->అంకె_events, కీలకం);
		k_spin_unlock(&poll_తాళం, కీలకం);
	}

	/* Drop work ownership and execute real handler. */
	twork->workq = ఖాళీ;
	twork->real_handler(work);
}

బాహ్య పూర్ణాంకం z_work_submit_గమ్యం_వరుస(నిర్మాణం k_work_q *వరుస,
			 నిర్మాణం k_work *work);

స్థిర శూన్య triggered_work_expiration_handler(నిర్మాణం _timeout *timeout)
{
	నిర్మాణం k_work_poll *twork =
		CONTAINER_యొక్క(timeout, నిర్మాణం k_work_poll, timeout);
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&poll_తాళం);

	ఒకవేళ (!twork->poller.is_polling) {
		/* signal_triggered_work() or triggered_work_cancel() has
		 * already claimed this wake under the same lock; nothing
		 * for us to do.
		 */
		k_spin_unlock(&poll_తాళం, కీలకం);
		తిరిగి;
	}

	twork->poller.is_polling = false;
	twork->poll_ఫలితం = -EAGAIN;
	z_work_submit_గమ్యం_వరుస(twork->workq, &twork->work);

	k_spin_unlock(&poll_తాళం, కీలకం);
}

స్థిర పూర్ణాంకం signal_triggered_work(నిర్మాణం k_poll_event *event, uint32_t స్థితి_2)
{
	నిర్మాణం z_poller *poller = event->poller;
	నిర్మాణం k_work_poll *twork =
		CONTAINER_యొక్క(poller, నిర్మాణం k_work_poll, poller);

	ఒకవేళ (poller->is_polling && twork->workq != ఖాళీ) {
		నిర్మాణం k_work_q *work_q = twork->workq;

		/* Claim the wake before aborting so a racing in-flight
		 * handler observes is_polling=false and bails.
		 */
		poller->is_polling = false;
		(శూన్య)z_try_abort_timeout(&twork->timeout);
		twork->poll_ఫలితం = 0;
		z_work_submit_గమ్యం_వరుస(work_q, &twork->work);
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం triggered_work_cancel(నిర్మాణం k_work_poll *work,
				 k_spinlock_కీలకం_t కీలకం)
{
	/* Check if the work waits for event. */
	ఒకవేళ (work->poller.is_polling && work->poller.బహుళకం != బహుళకం_NONE) {
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
		అయితే (z_try_abort_timeout(&work->timeout) == -EAGAIN) {
			k_spin_unlock(&poll_తాళం, కీలకం);
			కీలకం = k_spin_తాళం(&poll_తాళం);
		}

		/*
		 * Prevent work execution if event arrives while we will be
		 * clearing registrations.
		 */
		work->poller.బహుళకం = బహుళకం_NONE;

		/* Clear registrations and work ownership. */
		clear_event_registrations(work->events, work->అంకె_events, కీలకం);
		work->workq = ఖాళీ;
		తిరిగి 0;
	}

	/*
	 * If we reached here, the work is either being registered in
	 * the k_work_poll_submit_to_queue(), executed or is pending.
	 * Only in the last case we have a chance to cancel it, but
	 * unfortunately there is no public API performing this task.
	 */

	తిరిగి -EINVAL;
}

శూన్య k_work_poll_init(నిర్మాణం k_work_poll *work,
		      k_work_handler_t handler)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work_poll, init, work);

	*work = (నిర్మాణం k_work_poll) {};
	k_work_init(&work->work, triggered_work_handler);
	work->real_handler = handler;
	z_init_timeout(&work->timeout);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_poll, init, work);
}

పూర్ణాంకం k_work_poll_submit_గమ్యం_వరుస(నిర్మాణం k_work_q *work_q,
				నిర్మాణం k_work_poll *work,
				నిర్మాణం k_poll_event *events,
				పూర్ణాంకం అంకె_events,
				k_timeout_t timeout)
{
	పూర్ణాంకం events_registered;
	k_spinlock_కీలకం_t కీలకం;

	__ASSERT(work_q != ఖాళీ, "NULL work_q\n");
	__ASSERT(work != ఖాళీ, "NULL work\n");
	__ASSERT(events != ఖాళీ, "NULL events\n");
	__ASSERT(అంకె_events >= 0, "<0 events\n");

	SYS_పోర్టు_TRACING_FUNC_ENTER(k_work_poll, submit_గమ్యం_వరుస, work_q, work, timeout);

	/* Take ownership of the work if it is possible. */
	కీలకం = k_spin_తాళం(&poll_తాళం);
	ఒకవేళ (work->workq != ఖాళీ) {
		ఒకవేళ (work->workq == work_q) {
			పూర్ణాంకం retval;

			retval = triggered_work_cancel(work, కీలకం);
			ఒకవేళ (retval < 0) {
				k_spin_unlock(&poll_తాళం, కీలకం);

				SYS_పోర్టు_TRACING_FUNC_EXIT(k_work_poll, submit_గమ్యం_వరుస, work_q,
					work, timeout, retval);

				తిరిగి retval;
			}
		} లేకపోతే {
			k_spin_unlock(&poll_తాళం, కీలకం);

			SYS_పోర్టు_TRACING_FUNC_EXIT(k_work_poll, submit_గమ్యం_వరుస, work_q,
				work, timeout, -EADDRINUSE);

			తిరిగి -EADDRINUSE;
		}
	}


	work->poller.is_polling = true;
	work->workq = work_q;
	work->poller.బహుళకం = బహుళకం_NONE;
	k_spin_unlock(&poll_తాళం, కీలకం);

	/* Save list of events. */
	work->events = events;
	work->అంకె_events = అంకె_events;

	/* Clear result */
	work->poll_ఫలితం = -EINPROGRESS;

	/* Register events */
	events_registered = రిజిస్టరు_events(events, అంకె_events,
					    &work->poller, false);

	కీలకం = k_spin_తాళం(&poll_తాళం);
	ఒకవేళ (work->poller.is_polling && !K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/*
		 * Poller is still polling.
		 * No event is ready and all are watched.
		 */
		__ASSERT(అంకె_events == events_registered,
			 "Some events were not registered!\n");

		/* Setup timeout if such action is requested */
		z_కూడు_timeout(&work->timeout, triggered_work_expiration_handler, timeout);

		/* From now, any event will result in submitted work. */
		work->poller.బహుళకం = బహుళకం_TRIGGERED;
		k_spin_unlock(&poll_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_FUNC_EXIT(k_work_poll, submit_గమ్యం_వరుస, work_q, work, timeout, 0);

		తిరిగి 0;
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
	ఒకవేళ (work->poller.is_polling) {
		work->poller.is_polling = false;
		work->poll_ఫలితం = -EAGAIN;
	} లేకపోతే {
		work->poll_ఫలితం = 0;
	}

	/* Clear registrations. */
	clear_event_registrations(events, events_registered, కీలకం);
	k_spin_unlock(&poll_తాళం, కీలకం);

	/* Submit work. */
	k_work_submit_గమ్యం_వరుస(work_q, &work->work);

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_work_poll, submit_గమ్యం_వరుస, work_q, work, timeout, 0);

	తిరిగి 0;
}

పూర్ణాంకం k_work_poll_submit(నిర్మాణం k_work_poll *work,
				     నిర్మాణం k_poll_event *events,
				     పూర్ణాంకం అంకె_events,
				     k_timeout_t timeout)
{
	SYS_పోర్టు_TRACING_FUNC_ENTER(k_work_poll, submit, work, timeout);

	పూర్ణాంకం ret = k_work_poll_submit_గమ్యం_వరుస(&k_sys_work_q, work,
								events, అంకె_events, timeout);

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_work_poll, submit, work, timeout, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_poll_cancel(నిర్మాణం k_work_poll *work)
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం retval;

	SYS_పోర్టు_TRACING_FUNC_ENTER(k_work_poll, cancel, work);

	/* Check if the work was submitted. */
	ఒకవేళ (work == ఖాళీ || work->workq == ఖాళీ) {
		SYS_పోర్టు_TRACING_FUNC_EXIT(k_work_poll, cancel, work, -EINVAL);

		తిరిగి -EINVAL;
	}

	కీలకం = k_spin_తాళం(&poll_తాళం);
	retval = triggered_work_cancel(work, కీలకం);
	k_spin_unlock(&poll_తాళం, కీలకం);

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_work_poll, cancel, work, retval);

	తిరిగి retval;
}
