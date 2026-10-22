/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <zephyr/linker/sections.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <zephyr/sys/dlist.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <zephyr/sys/check.h>

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_MSGQ */

स्थिर अन्तःस्थ bool msgq_handle_poll_events(संरचना k_msgq *msgq)
{
#यदि_परिभाषितम् CONFIG_POLL
	प्रत्यागम z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_दत्त_उपलब्धम्);
#अन्यथा
	ARG_अप्रयुक्तम्(msgq);
	प्रत्यागम false;
#यदि_अन्तः /* CONFIG_POLL */
}

शून्य k_msgq_init(संरचना k_msgq *msgq, वर्ण *बफर, size_t सन्देशः_आकार,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!आकार_mul_अतिप्रवाहः(max_msgs, सन्देशः_आकार, &(size_t){0}));
	__ASSERT_NO_MSG(!आकार_सङ्कलय_अतिप्रवाहः((size_t)(uintptr_t)बफर, max_msgs * सन्देशः_आकार,
					&(size_t){0}));

	msgq->सन्देशः_आकार = सन्देशः_आकार;
	msgq->max_msgs = max_msgs;
	msgq->बफर_start = बफर;
	msgq->बफर_अन्तः = बफर + (max_msgs * सन्देशः_आकार);
	msgq->read_ptr = बफर;
	msgq->write_ptr = बफर;
	msgq->प्रयुक्तम्_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->तालकम् = (संरचना k_spinlock) {};
#यदि_परिभाषितम् CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#यदि_अन्तः	/* CONFIG_POLL */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#यदि_अन्तः /* CONFIG_OBJ_CORE_MSGQ */

	SYS_पत्तनम्_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

पूर्णाङ्क z_impl_k_msgq_alloc_init(संरचना k_msgq *msgq, size_t सन्देशः_आकार,
			    uint32_t max_msgs)
{
	शून्य *बफर;
	पूर्णाङ्क ret;
	size_t योग_आकार;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	यदि (आकार_mul_अतिप्रवाहः(सन्देशः_आकार, max_msgs, &योग_आकार)) {
		ret = -EINVAL;
	} अन्यथा {
		बफर = z_thread_malloc(योग_आकार);
		यदि (बफर != रिक्त) {
			k_msgq_init(msgq, बफर, सन्देशः_आकार, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_msgq_alloc_init(संरचना k_msgq *msgq, size_t सन्देशः_आकार,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	प्रत्यागम z_impl_k_msgq_alloc_init(msgq, सन्देशः_आकार, max_msgs);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_msgq_cleanup(संरचना k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	पूर्णाङ्क ret = 0;
	bool freed = false;
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&msgq->तालकम्);

	CHECKIF(locked && (z_waitq_शीर्ष_locked(&msgq->wait_q) != रिक्त)) {
		ret = -EBUSY;
		गच्छ बहिर्गमनम्;
	}

	CHECKIF(!locked && (z_waitq_शीर्ष(&msgq->wait_q) != रिक्त)) {
		ret = -EBUSY;
		गच्छ बहिर्गमनम्;
	}

	यदि ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->बफर_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

बहिर्गमनम्:
	k_spin_unlock(&msgq->तालकम्, कुञ्चिका);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MSGQ
	यदि (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#यदि_अन्तः /* CONFIG_OBJ_CORE_MSGQ */

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	प्रत्यागम ret;
}

पूर्णाङ्क k_msgq_cleanup(संरचना k_msgq *msgq)
{
	प्रत्यागम z_msgq_cleanup(msgq, false);
}

स्थिर अन्तःस्थ पूर्णाङ्क put_सन्देशः_अन्तर्_queue(संरचना k_msgq *msgq, ध्रुव शून्य *दत्त,
			k_timeout_t timeout, bool put_at_पृष्ठभागः)
{
	__ASSERT(!arch_is_अन्तर्_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	संरचना k_thread *pending_thread = रिक्त;
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क परिणाम;
	bool resched = false;

	कुञ्चिका = k_spin_तालकम्(&msgq->तालकम्);

	यदि (put_at_पृष्ठभागः) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} अन्यथा {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	यदि (msgq->प्रयुक्तम्_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		तालकम्_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			यदि (pending_thread != रिक्त) {
				/* copy into the receiver's buffer */
				(शून्य)स्मृतिं_प्रतिलिख(pending_thread->आधारः.swap_दत्त, दत्त,
					     msgq->सन्देशः_आकार);
				arch_thread_return_मानम्_समुच्चयः(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		यदि (pending_thread == रिक्त) {
			size_t सन्देशः_आकार = msgq->सन्देशः_आकार;
			वर्ण *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->बफर_start) &&
					(msgq->write_ptr <= (msgq->बफर_अन्तः - 1)) &&
					((size_t)(uintptr_t)(msgq->बफर_अन्तः - msgq->write_ptr) >=
						msgq->सन्देशः_आकार));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			यदि (put_at_पृष्ठभागः) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				वर्ण *अग्र;

				slot = msgq->write_ptr;
				अग्र = slot + सन्देशः_आकार;
				यदि (अग्र == msgq->बफर_अन्तः) {
					अग्र = msgq->बफर_start;
				}
				msgq->write_ptr = अग्र;
			} अन्यथा {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				यदि (slot == msgq->बफर_start) {
					slot = msgq->बफर_अन्तः;
				}
				slot -= सन्देशः_आकार;
				msgq->read_ptr = slot;
			}
			msgq->प्रयुक्तम्_msgs++;
			(शून्य)स्मृतिं_प्रतिलिख(slot, दत्त, सन्देशः_आकार);
			resched = msgq_handle_poll_events(msgq);
		}
		परिणाम = 0;
	} अन्यथा यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		परिणाम = -ENOMSG;
	} अन्यथा {
		यदि (put_at_पृष्ठभागः) {
			SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} अन्यथा {
			SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->आधारः.swap_दत्त = (शून्य *) दत्त;

		परिणाम = z_pend_curr(&msgq->तालकम्, कुञ्चिका, &msgq->wait_q, timeout);

		यदि (put_at_पृष्ठभागः) {
			SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, परिणाम);
		} अन्यथा {
			SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, परिणाम);
		}

		प्रत्यागम परिणाम;
	}

	यदि (put_at_पृष्ठभागः) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, परिणाम);
	} अन्यथा {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, परिणाम);
	}

	यदि (resched) {
		z_reschedule(&msgq->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&msgq->तालकम्, कुञ्चिका);
	}

	प्रत्यागम परिणाम;
}


पूर्णाङ्क z_impl_k_msgq_put(संरचना k_msgq *msgq, ध्रुव शून्य *दत्त, k_timeout_t timeout)
{
	प्रत्यागम put_सन्देशः_अन्तर्_queue(msgq, दत्त, timeout, true);
}

पूर्णाङ्क z_impl_k_msgq_put_front(संरचना k_msgq *msgq, ध्रुव शून्य *दत्त)
{
	प्रत्यागम put_सन्देशः_अन्तर्_queue(msgq, दत्त, K_NO_WAIT, false);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_msgq_put(संरचना k_msgq *msgq, ध्रुव शून्य *दत्त,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_स्मृतिः_READ(दत्त, msgq->सन्देशः_आकार));

	प्रत्यागम z_impl_k_msgq_put(msgq, दत्त, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_put_mrsh.c>

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_msgq_put_front(संरचना k_msgq *msgq, ध्रुव शून्य *दत्त)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_स्मृतिः_READ(दत्त, msgq->सन्देशः_आकार));

	प्रत्यागम z_impl_k_msgq_put_front(msgq, दत्त);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य z_impl_k_msgq_get_attrs(संरचना k_msgq *msgq, संरचना k_msgq_attrs *attrs)
{
	attrs->सन्देशः_आकार = msgq->सन्देशः_आकार;
	attrs->max_msgs = msgq->max_msgs;
	attrs->प्रयुक्तम्_msgs = msgq->प्रयुक्तम्_msgs;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_msgq_get_attrs(संरचना k_msgq *msgq,
					   संरचना k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(attrs, परिमाण(संरचना k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_msgq_get(संरचना k_msgq *msgq, शून्य *दत्त, k_timeout_t timeout)
{
	__ASSERT(!arch_is_अन्तर्_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_कुञ्चिका_t कुञ्चिका;
	संरचना k_thread *pending_thread;
	पूर्णाङ्क परिणाम;
	bool resched = false;

	कुञ्चिका = k_spin_तालकम्(&msgq->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	यदि (likely(msgq->प्रयुक्तम्_msgs > 0U)) {
		size_t सन्देशः_आकार = msgq->सन्देशः_आकार;
		वर्ण *slot = msgq->read_ptr;
		वर्ण *अग्र = slot + सन्देशः_आकार;

		/* drop the message from the queue, then copy it out */
		यदि (अग्र == msgq->बफर_अन्तः) {
			अग्र = msgq->बफर_start;
		}
		msgq->read_ptr = अग्र;
		msgq->प्रयुक्तम्_msgs--;

		(शून्य)स्मृतिं_प्रतिलिख(दत्त, slot, सन्देशः_आकार);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->बफर_start) &&
				(msgq->write_ptr <= (msgq->बफर_अन्तः - 1)) &&
				((size_t)(uintptr_t)(msgq->बफर_अन्तः - msgq->write_ptr) >=
					msgq->सन्देशः_आकार));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		तालकम्_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			यदि (pending_thread != रिक्त) {
				SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				अग्र = slot + सन्देशः_आकार;
				यदि (अग्र == msgq->बफर_अन्तः) {
					अग्र = msgq->बफर_start;
				}
				msgq->write_ptr = अग्र;
				msgq->प्रयुक्तम्_msgs++;

				(शून्य)स्मृतिं_प्रतिलिख(slot, pending_thread->आधारः.swap_दत्त, सन्देशः_आकार);

				arch_thread_return_मानम्_समुच्चयः(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		परिणाम = 0;
	} अन्यथा यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		परिणाम = -ENOMSG;
	} अन्यथा {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->आधारः.swap_दत्त = दत्त;

		परिणाम = z_pend_curr(&msgq->तालकम्, कुञ्चिका, &msgq->wait_q, timeout);
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, परिणाम);
		प्रत्यागम परिणाम;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, परिणाम);

	यदि (resched) {
		z_reschedule(&msgq->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&msgq->तालकम्, कुञ्चिका);
	}

	प्रत्यागम परिणाम;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_msgq_get(संरचना k_msgq *msgq, शून्य *दत्त,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(दत्त, msgq->सन्देशः_आकार));

	प्रत्यागम z_impl_k_msgq_get(msgq, दत्त, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_msgq_peek(संरचना k_msgq *msgq, शून्य *दत्त)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क परिणाम;

	कुञ्चिका = k_spin_तालकम्(&msgq->तालकम्);

	यदि (msgq->प्रयुक्तम्_msgs > 0U) {
		/* take first available message from queue */
		(शून्य)स्मृतिं_प्रतिलिख(दत्त, msgq->read_ptr, msgq->सन्देशः_आकार);
		परिणाम = 0;
	} अन्यथा {
		/* don't wait for a message to become available */
		परिणाम = -ENOMSG;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_msgq, peek, msgq, परिणाम);

	k_spin_unlock(&msgq->तालकम्, कुञ्चिका);

	प्रत्यागम परिणाम;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_msgq_peek(संरचना k_msgq *msgq, शून्य *दत्त)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(दत्त, msgq->सन्देशः_आकार));

	प्रत्यागम z_impl_k_msgq_peek(msgq, दत्त);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_peek_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_msgq_peek_at(संरचना k_msgq *msgq, शून्य *दत्त, uint32_t idx)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क परिणाम;
	uint32_t अष्टकानि_गन्तव्यम्_अन्तः;
	uint32_t byte_विस्थापनम्;
	वर्ण *start_addr;

	कुञ्चिका = k_spin_तालकम्(&msgq->तालकम्);

	यदि (msgq->प्रयुक्तम्_msgs > idx) {
		अष्टकानि_गन्तव्यम्_अन्तः = (msgq->बफर_अन्तः - msgq->read_ptr);
		byte_विस्थापनम् = idx * msgq->सन्देशः_आकार;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		यदि (अष्टकानि_गन्तव्यम्_अन्तः <= byte_विस्थापनम्) {
			/* Tweak the values in case */
			byte_विस्थापनम् -= अष्टकानि_गन्तव्यम्_अन्तः;
			/* wrap-around is required */
			start_addr = msgq->बफर_start;
		}
		(शून्य)स्मृतिं_प्रतिलिख(दत्त, start_addr + byte_विस्थापनम्, msgq->सन्देशः_आकार);
		परिणाम = 0;
	} अन्यथा {
		/* don't wait for a message to become available */
		परिणाम = -ENOMSG;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_msgq, peek, msgq, परिणाम);

	k_spin_unlock(&msgq->तालकम्, कुञ्चिका);

	प्रत्यागम परिणाम;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_msgq_peek_at(संरचना k_msgq *msgq, शून्य *दत्त, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(दत्त, msgq->सन्देशः_आकार));

	प्रत्यागम z_impl_k_msgq_peek_at(msgq, दत्त, idx);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

शून्य z_impl_k_msgq_purge(संरचना k_msgq *msgq)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	bool resched = false;

	कुञ्चिका = k_spin_तालकम्(&msgq->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	यावत् (z_sched_wake(&msgq->wait_q, -ENOMSG, रिक्त)) {
		resched = true;
	}

	msgq->प्रयुक्तम्_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	यदि (resched) {
		z_reschedule(&msgq->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&msgq->तालकम्, कुञ्चिका);
	}
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_msgq_purge(संरचना k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_purge_mrsh.c>

स्थिर अन्तःस्थ uint32_t z_vrfy_k_msgq_अङ्कः_free_get(संरचना k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	प्रत्यागम z_impl_k_msgq_अङ्कः_free_get(msgq);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

स्थिर अन्तःस्थ uint32_t z_vrfy_k_msgq_अङ्कः_प्रयुक्तम्_get(संरचना k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	प्रत्यागम z_impl_k_msgq_अङ्कः_प्रयुक्तम्_get(msgq);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#यदि_अन्तः /* CONFIG_USERSPACE */
