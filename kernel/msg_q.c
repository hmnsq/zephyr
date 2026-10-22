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

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_MSGQ */

स्थिर ओळीत bool msgq_handle_poll_events(संरचना k_msgq *msgq)
{
#ifdef CONFIG_POLL
	परत z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#अन्यथा
	ARG_UNUSED(msgq);
	परत false;
#endif /* CONFIG_POLL */
}

शून्य k_msgq_init(संरचना k_msgq *msgq, अक्षर *buffer, माप_प्रकार संदेश_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, संदेश_size, &(माप_प्रकार){0}));
	__ASSERT_NO_MSG(!size_add_overflow((माप_प्रकार)(uintptr_t)buffer, max_msgs * संदेश_size,
					&(माप_प्रकार){0}));

	msgq->संदेश_size = संदेश_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * संदेश_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (संरचना k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

पूर्णांक z_impl_k_msgq_alloc_init(संरचना k_msgq *msgq, माप_प्रकार संदेश_size,
			    uint32_t max_msgs)
{
	शून्य *buffer;
	पूर्णांक ret;
	माप_प्रकार total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	जर (size_mul_overflow(संदेश_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} अन्यथा {
		buffer = z_thread_malloc(total_size);
		जर (buffer != रिक्त) {
			k_msgq_init(msgq, buffer, संदेश_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	परत ret;
}

#ifdef CONFIG_USERSPACE
पूर्णांक z_vrfy_k_msgq_alloc_init(संरचना k_msgq *msgq, माप_प्रकार संदेश_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	परत z_impl_k_msgq_alloc_init(msgq, संदेश_size, max_msgs);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_msgq_cleanup(संरचना k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	पूर्णांक ret = 0;
	bool freed = false;
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != रिक्त)) {
		ret = -EBUSY;
		जा out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != रिक्त)) {
		ret = -EBUSY;
		जा out;
	}

	जर ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, किल्ली);

#ifdef CONFIG_OBJ_CORE_MSGQ
	जर (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	परत ret;
}

पूर्णांक k_msgq_cleanup(संरचना k_msgq *msgq)
{
	परत z_msgq_cleanup(msgq, false);
}

स्थिर ओळीत पूर्णांक put_संदेश_in_queue(संरचना k_msgq *msgq, स्थिरांक शून्य *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	संरचना k_thread *pending_thread = रिक्त;
	k_spinlock_किल्ली_t किल्ली;
	पूर्णांक result;
	bool resched = false;

	किल्ली = k_spin_lock(&msgq->lock);

	जर (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	जर (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			जर (pending_thread != रिक्त) {
				/* copy into the receiver's buffer */
				(शून्य)स_नकल(pending_thread->base.swap_data, data,
					     msgq->संदेश_size);
				arch_thread_return_मूल्य_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		जर (pending_thread == रिक्त) {
			माप_प्रकार संदेश_size = msgq->संदेश_size;
			अक्षर *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((माप_प्रकार)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->संदेश_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			जर (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				अक्षर *next;

				slot = msgq->write_ptr;
				next = slot + संदेश_size;
				जर (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} अन्यथा {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				जर (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= संदेश_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(शून्य)स_नकल(slot, data, संदेश_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} अन्यथा जर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} अन्यथा {
		जर (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} अन्यथा {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (शून्य *) data;

		result = z_pend_curr(&msgq->lock, किल्ली, &msgq->wait_q, timeout);

		जर (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} अन्यथा {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		परत result;
	}

	जर (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	जर (resched) {
		z_reschedule(&msgq->lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(&msgq->lock, किल्ली);
	}

	परत result;
}


पूर्णांक z_impl_k_msgq_put(संरचना k_msgq *msgq, स्थिरांक शून्य *data, k_timeout_t timeout)
{
	परत put_संदेश_in_queue(msgq, data, timeout, true);
}

पूर्णांक z_impl_k_msgq_put_front(संरचना k_msgq *msgq, स्थिरांक शून्य *data)
{
	परत put_संदेश_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_msgq_put(संरचना k_msgq *msgq, स्थिरांक शून्य *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->संदेश_size));

	परत z_impl_k_msgq_put(msgq, data, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_put_mrsh.c>

स्थिर ओळीत पूर्णांक z_vrfy_k_msgq_put_front(संरचना k_msgq *msgq, स्थिरांक शून्य *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->संदेश_size));

	परत z_impl_k_msgq_put_front(msgq, data);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_msgq_get_attrs(संरचना k_msgq *msgq, संरचना k_msgq_attrs *attrs)
{
	attrs->संदेश_size = msgq->संदेश_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_msgq_get_attrs(संरचना k_msgq *msgq,
					   संरचना k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, आकार(संरचना k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_msgq_get(संरचना k_msgq *msgq, शून्य *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_किल्ली_t किल्ली;
	संरचना k_thread *pending_thread;
	पूर्णांक result;
	bool resched = false;

	किल्ली = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	जर (likely(msgq->used_msgs > 0U)) {
		माप_प्रकार संदेश_size = msgq->संदेश_size;
		अक्षर *slot = msgq->read_ptr;
		अक्षर *next = slot + संदेश_size;

		/* drop the message from the queue, then copy it out */
		जर (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(शून्य)स_नकल(data, slot, संदेश_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((माप_प्रकार)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->संदेश_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			जर (pending_thread != रिक्त) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + संदेश_size;
				जर (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(शून्य)स_नकल(slot, pending_thread->base.swap_data, संदेश_size);

				arch_thread_return_मूल्य_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} अन्यथा जर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, किल्ली, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		परत result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	जर (resched) {
		z_reschedule(&msgq->lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(&msgq->lock, किल्ली);
	}

	परत result;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_msgq_get(संरचना k_msgq *msgq, शून्य *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->संदेश_size));

	परत z_impl_k_msgq_get(msgq, data, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_msgq_peek(संरचना k_msgq *msgq, शून्य *data)
{
	k_spinlock_किल्ली_t किल्ली;
	पूर्णांक result;

	किल्ली = k_spin_lock(&msgq->lock);

	जर (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(शून्य)स_नकल(data, msgq->read_ptr, msgq->संदेश_size);
		result = 0;
	} अन्यथा {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, किल्ली);

	परत result;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_msgq_peek(संरचना k_msgq *msgq, शून्य *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->संदेश_size));

	परत z_impl_k_msgq_peek(msgq, data);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_msgq_peek_at(संरचना k_msgq *msgq, शून्य *data, uint32_t idx)
{
	k_spinlock_किल्ली_t किल्ली;
	पूर्णांक result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	अक्षर *start_addr;

	किल्ली = k_spin_lock(&msgq->lock);

	जर (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->संदेश_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		जर (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(शून्य)स_नकल(data, start_addr + byte_offset, msgq->संदेश_size);
		result = 0;
	} अन्यथा {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, किल्ली);

	परत result;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_msgq_peek_at(संरचना k_msgq *msgq, शून्य *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->संदेश_size));

	परत z_impl_k_msgq_peek_at(msgq, data, idx);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

शून्य z_impl_k_msgq_purge(संरचना k_msgq *msgq)
{
	k_spinlock_किल्ली_t किल्ली;
	bool resched = false;

	किल्ली = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	पर्यंत (z_sched_wake(&msgq->wait_q, -ENOMSG, रिक्त)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	जर (resched) {
		z_reschedule(&msgq->lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(&msgq->lock, किल्ली);
	}
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_msgq_purge(संरचना k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_purge_mrsh.c>

स्थिर ओळीत uint32_t z_vrfy_k_msgq_अंक_free_get(संरचना k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	परत z_impl_k_msgq_अंक_free_get(msgq);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

स्थिर ओळीत uint32_t z_vrfy_k_msgq_अंक_used_get(संरचना k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	परत z_impl_k_msgq_अंक_used_get(msgq);
}
#अन्तर्भाव <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
