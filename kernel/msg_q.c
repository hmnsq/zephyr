/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/linker/sections.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/sys/check.h>

#నిర్వచితమైతే CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MSGQ */

స్థిర అంతఃవరుస bool msgq_handle_poll_events(నిర్మాణం k_msgq *msgq)
{
#నిర్వచితమైతే CONFIG_POLL
	తిరిగి z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_దత్తాంశం_అందుబాటు);
#లేకపోతే
	ARG_వాడని(msgq);
	తిరిగి false;
#అయితే_ముగింపు /* CONFIG_POLL */
}

శూన్య k_msgq_init(నిర్మాణం k_msgq *msgq, అక్షరం *బఫరు, size_t సందేశం_కొలత,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!కొలత_mul_అధికప్రవాహం(max_msgs, సందేశం_కొలత, &(size_t){0}));
	__ASSERT_NO_MSG(!కొలత_కూడు_అధికప్రవాహం((size_t)(uintptr_t)బఫరు, max_msgs * సందేశం_కొలత,
					&(size_t){0}));

	msgq->సందేశం_కొలత = సందేశం_కొలత;
	msgq->max_msgs = max_msgs;
	msgq->బఫరు_ప్రారంభం = బఫరు;
	msgq->బఫరు_చివర = బఫరు + (max_msgs * సందేశం_కొలత);
	msgq->read_ptr = బఫరు;
	msgq->write_ptr = బఫరు;
	msgq->వాడిన_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->తాళం = (నిర్మాణం k_spinlock) {};
#నిర్వచితమైతే CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#అయితే_ముగింపు	/* CONFIG_POLL */

#నిర్వచితమైతే CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MSGQ */

	SYS_పోర్టు_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

పూర్ణాంకం z_impl_k_msgq_alloc_init(నిర్మాణం k_msgq *msgq, size_t సందేశం_కొలత,
			    uint32_t max_msgs)
{
	శూన్య *బఫరు;
	పూర్ణాంకం ret;
	size_t మొత్తం_కొలత;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	ఒకవేళ (కొలత_mul_అధికప్రవాహం(సందేశం_కొలత, max_msgs, &మొత్తం_కొలత)) {
		ret = -EINVAL;
	} లేకపోతే {
		బఫరు = z_thread_malloc(మొత్తం_కొలత);
		ఒకవేళ (బఫరు != ఖాళీ) {
			k_msgq_init(msgq, బఫరు, సందేశం_కొలత, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} లేకపోతే {
			ret = -ENOMEM;
		}
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_msgq_alloc_init(నిర్మాణం k_msgq *msgq, size_t సందేశం_కొలత,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	తిరిగి z_impl_k_msgq_alloc_init(msgq, సందేశం_కొలత, max_msgs);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_msgq_cleanup(నిర్మాణం k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	పూర్ణాంకం ret = 0;
	bool freed = false;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&msgq->తాళం);

	CHECKIF(locked && (z_waitq_తల_locked(&msgq->wait_q) != ఖాళీ)) {
		ret = -EBUSY;
		వెళ్ళు బహిర్గతం;
	}

	CHECKIF(!locked && (z_waitq_తల(&msgq->wait_q) != ఖాళీ)) {
		ret = -EBUSY;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->బఫరు_ప్రారంభం);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

బహిర్గతం:
	k_spin_unlock(&msgq->తాళం, కీలకం);

#నిర్వచితమైతే CONFIG_OBJ_CORE_MSGQ
	ఒకవేళ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MSGQ */

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	తిరిగి ret;
}

పూర్ణాంకం k_msgq_cleanup(నిర్మాణం k_msgq *msgq)
{
	తిరిగి z_msgq_cleanup(msgq, false);
}

స్థిర అంతఃవరుస పూర్ణాంకం put_సందేశం_లోపల_వరుస(నిర్మాణం k_msgq *msgq, స్థిరరాశి శూన్య *దత్తాంశం,
			k_timeout_t timeout, bool put_at_వెనుక)
{
	__ASSERT(!arch_is_లోపల_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	నిర్మాణం k_thread *pending_thread = ఖాళీ;
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ఫలితం;
	bool resched = false;

	కీలకం = k_spin_తాళం(&msgq->తాళం);

	ఒకవేళ (put_at_వెనుక) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} లేకపోతే {
		SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	ఒకవేళ (msgq->వాడిన_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		తాళం_SCHED_SPINLOCK {
			pending_thread = z_unpend_మొదటి_thread_locked(&msgq->wait_q);
			ఒకవేళ (pending_thread != ఖాళీ) {
				/* copy into the receiver's buffer */
				(శూన్య)జ్ఞాపకం_నకలు(pending_thread->పునాది.మార్పిడి_దత్తాంశం, దత్తాంశం,
					     msgq->సందేశం_కొలత);
				arch_thread_return_విలువ_సమితి(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ఒకవేళ (pending_thread == ఖాళీ) {
			size_t సందేశం_కొలత = msgq->సందేశం_కొలత;
			అక్షరం *స్థానం;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->బఫరు_ప్రారంభం) &&
					(msgq->write_ptr <= (msgq->బఫరు_చివర - 1)) &&
					((size_t)(uintptr_t)(msgq->బఫరు_చివర - msgq->write_ptr) >=
						msgq->సందేశం_కొలత));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			ఒకవేళ (put_at_వెనుక) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				అక్షరం *తదుపరి;

				స్థానం = msgq->write_ptr;
				తదుపరి = స్థానం + సందేశం_కొలత;
				ఒకవేళ (తదుపరి == msgq->బఫరు_చివర) {
					తదుపరి = msgq->బఫరు_ప్రారంభం;
				}
				msgq->write_ptr = తదుపరి;
			} లేకపోతే {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				స్థానం = msgq->read_ptr;
				ఒకవేళ (స్థానం == msgq->బఫరు_ప్రారంభం) {
					స్థానం = msgq->బఫరు_చివర;
				}
				స్థానం -= సందేశం_కొలత;
				msgq->read_ptr = స్థానం;
			}
			msgq->వాడిన_msgs++;
			(శూన్య)జ్ఞాపకం_నకలు(స్థానం, దత్తాంశం, సందేశం_కొలత);
			resched = msgq_handle_poll_events(msgq);
		}
		ఫలితం = 0;
	} లేకపోతే ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		ఫలితం = -ENOMSG;
	} లేకపోతే {
		ఒకవేళ (put_at_వెనుక) {
			SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} లేకపోతే {
			SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->పునాది.మార్పిడి_దత్తాంశం = (శూన్య *) దత్తాంశం;

		ఫలితం = z_pend_curr(&msgq->తాళం, కీలకం, &msgq->wait_q, timeout);

		ఒకవేళ (put_at_వెనుక) {
			SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ఫలితం);
		} లేకపోతే {
			SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ఫలితం);
		}

		తిరిగి ఫలితం;
	}

	ఒకవేళ (put_at_వెనుక) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ఫలితం);
	} లేకపోతే {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ఫలితం);
	}

	ఒకవేళ (resched) {
		z_reschedule(&msgq->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&msgq->తాళం, కీలకం);
	}

	తిరిగి ఫలితం;
}


పూర్ణాంకం z_impl_k_msgq_put(నిర్మాణం k_msgq *msgq, స్థిరరాశి శూన్య *దత్తాంశం, k_timeout_t timeout)
{
	తిరిగి put_సందేశం_లోపల_వరుస(msgq, దత్తాంశం, timeout, true);
}

పూర్ణాంకం z_impl_k_msgq_put_front(నిర్మాణం k_msgq *msgq, స్థిరరాశి శూన్య *దత్తాంశం)
{
	తిరిగి put_సందేశం_లోపల_వరుస(msgq, దత్తాంశం, K_NO_WAIT, false);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_msgq_put(నిర్మాణం k_msgq *msgq, స్థిరరాశి శూన్య *దత్తాంశం,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_READ(దత్తాంశం, msgq->సందేశం_కొలత));

	తిరిగి z_impl_k_msgq_put(msgq, దత్తాంశం, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_put_mrsh.c>

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_msgq_put_front(నిర్మాణం k_msgq *msgq, స్థిరరాశి శూన్య *దత్తాంశం)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_READ(దత్తాంశం, msgq->సందేశం_కొలత));

	తిరిగి z_impl_k_msgq_put_front(msgq, దత్తాంశం);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_impl_k_msgq_get_attrs(నిర్మాణం k_msgq *msgq, నిర్మాణం k_msgq_attrs *attrs)
{
	attrs->సందేశం_కొలత = msgq->సందేశం_కొలత;
	attrs->max_msgs = msgq->max_msgs;
	attrs->వాడిన_msgs = msgq->వాడిన_msgs;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_msgq_get_attrs(నిర్మాణం k_msgq *msgq,
					   నిర్మాణం k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(attrs, పరిమాణం(నిర్మాణం k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_msgq_get(నిర్మాణం k_msgq *msgq, శూన్య *దత్తాంశం, k_timeout_t timeout)
{
	__ASSERT(!arch_is_లోపల_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_కీలకం_t కీలకం;
	నిర్మాణం k_thread *pending_thread;
	పూర్ణాంకం ఫలితం;
	bool resched = false;

	కీలకం = k_spin_తాళం(&msgq->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	ఒకవేళ (likely(msgq->వాడిన_msgs > 0U)) {
		size_t సందేశం_కొలత = msgq->సందేశం_కొలత;
		అక్షరం *స్థానం = msgq->read_ptr;
		అక్షరం *తదుపరి = స్థానం + సందేశం_కొలత;

		/* drop the message from the queue, then copy it out */
		ఒకవేళ (తదుపరి == msgq->బఫరు_చివర) {
			తదుపరి = msgq->బఫరు_ప్రారంభం;
		}
		msgq->read_ptr = తదుపరి;
		msgq->వాడిన_msgs--;

		(శూన్య)జ్ఞాపకం_నకలు(దత్తాంశం, స్థానం, సందేశం_కొలత);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->బఫరు_ప్రారంభం) &&
				(msgq->write_ptr <= (msgq->బఫరు_చివర - 1)) &&
				((size_t)(uintptr_t)(msgq->బఫరు_చివర - msgq->write_ptr) >=
					msgq->సందేశం_కొలత));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		తాళం_SCHED_SPINLOCK {
			pending_thread = z_unpend_మొదటి_thread_locked(&msgq->wait_q);
			ఒకవేళ (pending_thread != ఖాళీ) {
				SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				స్థానం = msgq->write_ptr;
				తదుపరి = స్థానం + సందేశం_కొలత;
				ఒకవేళ (తదుపరి == msgq->బఫరు_చివర) {
					తదుపరి = msgq->బఫరు_ప్రారంభం;
				}
				msgq->write_ptr = తదుపరి;
				msgq->వాడిన_msgs++;

				(శూన్య)జ్ఞాపకం_నకలు(స్థానం, pending_thread->పునాది.మార్పిడి_దత్తాంశం, సందేశం_కొలత);

				arch_thread_return_విలువ_సమితి(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ఫలితం = 0;
	} లేకపోతే ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		ఫలితం = -ENOMSG;
	} లేకపోతే {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->పునాది.మార్పిడి_దత్తాంశం = దత్తాంశం;

		ఫలితం = z_pend_curr(&msgq->తాళం, కీలకం, &msgq->wait_q, timeout);
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ఫలితం);
		తిరిగి ఫలితం;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ఫలితం);

	ఒకవేళ (resched) {
		z_reschedule(&msgq->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&msgq->తాళం, కీలకం);
	}

	తిరిగి ఫలితం;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_msgq_get(నిర్మాణం k_msgq *msgq, శూన్య *దత్తాంశం,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(దత్తాంశం, msgq->సందేశం_కొలత));

	తిరిగి z_impl_k_msgq_get(msgq, దత్తాంశం, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_msgq_peek(నిర్మాణం k_msgq *msgq, శూన్య *దత్తాంశం)
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ఫలితం;

	కీలకం = k_spin_తాళం(&msgq->తాళం);

	ఒకవేళ (msgq->వాడిన_msgs > 0U) {
		/* take first available message from queue */
		(శూన్య)జ్ఞాపకం_నకలు(దత్తాంశం, msgq->read_ptr, msgq->సందేశం_కొలత);
		ఫలితం = 0;
	} లేకపోతే {
		/* don't wait for a message to become available */
		ఫలితం = -ENOMSG;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ఫలితం);

	k_spin_unlock(&msgq->తాళం, కీలకం);

	తిరిగి ఫలితం;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_msgq_peek(నిర్మాణం k_msgq *msgq, శూన్య *దత్తాంశం)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(దత్తాంశం, msgq->సందేశం_కొలత));

	తిరిగి z_impl_k_msgq_peek(msgq, దత్తాంశం);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_peek_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_msgq_peek_at(నిర్మాణం k_msgq *msgq, శూన్య *దత్తాంశం, uint32_t idx)
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ఫలితం;
	uint32_t బైట్లు_గమ్యం_చివర;
	uint32_t byte_స్థానభ్రంశం;
	అక్షరం *ప్రారంభం_addr;

	కీలకం = k_spin_తాళం(&msgq->తాళం);

	ఒకవేళ (msgq->వాడిన_msgs > idx) {
		బైట్లు_గమ్యం_చివర = (msgq->బఫరు_చివర - msgq->read_ptr);
		byte_స్థానభ్రంశం = idx * msgq->సందేశం_కొలత;
		ప్రారంభం_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		ఒకవేళ (బైట్లు_గమ్యం_చివర <= byte_స్థానభ్రంశం) {
			/* Tweak the values in case */
			byte_స్థానభ్రంశం -= బైట్లు_గమ్యం_చివర;
			/* wrap-around is required */
			ప్రారంభం_addr = msgq->బఫరు_ప్రారంభం;
		}
		(శూన్య)జ్ఞాపకం_నకలు(దత్తాంశం, ప్రారంభం_addr + byte_స్థానభ్రంశం, msgq->సందేశం_కొలత);
		ఫలితం = 0;
	} లేకపోతే {
		/* don't wait for a message to become available */
		ఫలితం = -ENOMSG;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ఫలితం);

	k_spin_unlock(&msgq->తాళం, కీలకం);

	తిరిగి ఫలితం;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_msgq_peek_at(నిర్మాణం k_msgq *msgq, శూన్య *దత్తాంశం, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(దత్తాంశం, msgq->సందేశం_కొలత));

	తిరిగి z_impl_k_msgq_peek_at(msgq, దత్తాంశం, idx);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

శూన్య z_impl_k_msgq_purge(నిర్మాణం k_msgq *msgq)
{
	k_spinlock_కీలకం_t కీలకం;
	bool resched = false;

	కీలకం = k_spin_తాళం(&msgq->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	అయితే (z_sched_wake(&msgq->wait_q, -ENOMSG, ఖాళీ)) {
		resched = true;
	}

	msgq->వాడిన_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	ఒకవేళ (resched) {
		z_reschedule(&msgq->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&msgq->తాళం, కీలకం);
	}
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_msgq_purge(నిర్మాణం k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_purge_mrsh.c>

స్థిర అంతఃవరుస uint32_t z_vrfy_k_msgq_అంకె_free_get(నిర్మాణం k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	తిరిగి z_impl_k_msgq_అంకె_free_get(msgq);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

స్థిర అంతఃవరుస uint32_t z_vrfy_k_msgq_అంకె_వాడిన_get(నిర్మాణం k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	తిరిగి z_impl_k_msgq_అంకె_వాడిన_get(msgq);
}
#అన్తర్భావ <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#అయితే_ముగింపు /* CONFIG_USERSPACE */
