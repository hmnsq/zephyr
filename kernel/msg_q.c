/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <zephyr/linker/sections.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <scheduler.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_MSGQ */

സ്ഥിതം വരിയിൽ bool msgq_handle_poll_events(ഘടന k_msgq *msgq)
{
#ifdef CONFIG_POLL
	തിരികെ z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#അല്ലെങ്കിൽ
	ARG_UNUSED(msgq);
	തിരികെ false;
#endif /* CONFIG_POLL */
}

ശൂന്യം k_msgq_init(ഘടന k_msgq *msgq, അക്ഷരം *buffer, size_t സന്ദേശം_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, സന്ദേശം_size, &(size_t){0}));
	__ASSERT_NO_MSG(!size_add_overflow((size_t)(uintptr_t)buffer, max_msgs * സന്ദേശം_size,
					&(size_t){0}));

	msgq->സന്ദേശം_size = സന്ദേശം_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * സന്ദേശം_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (ഘടന k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

പൂർണ്ണസംഖ്യ z_impl_k_msgq_alloc_init(ഘടന k_msgq *msgq, size_t സന്ദേശം_size,
			    uint32_t max_msgs)
{
	ശൂന്യം *buffer;
	പൂർണ്ണസംഖ്യ ret;
	size_t total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	എങ്കിൽ (size_mul_overflow(സന്ദേശം_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} അല്ലെങ്കിൽ {
		buffer = z_thread_malloc(total_size);
		എങ്കിൽ (buffer != ശൂന്യ) {
			k_msgq_init(msgq, buffer, സന്ദേശം_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} അല്ലെങ്കിൽ {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	തിരികെ ret;
}

#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_msgq_alloc_init(ഘടന k_msgq *msgq, size_t സന്ദേശം_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	തിരികെ z_impl_k_msgq_alloc_init(msgq, സന്ദേശം_size, max_msgs);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_msgq_cleanup(ഘടന k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	പൂർണ്ണസംഖ്യ ret = 0;
	bool freed = false;
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != ശൂന്യ)) {
		ret = -EBUSY;
		പോകുക out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != ശൂന്യ)) {
		ret = -EBUSY;
		പോകുക out;
	}

	എങ്കിൽ ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, താക്കോൽ);

#ifdef CONFIG_OBJ_CORE_MSGQ
	എങ്കിൽ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_msgq_cleanup(ഘടന k_msgq *msgq)
{
	തിരികെ z_msgq_cleanup(msgq, false);
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ put_സന്ദേശം_in_queue(ഘടന k_msgq *msgq, സ്ഥിരം ശൂന്യം *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ഘടന k_thread *pending_thread = ശൂന്യ;
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ result;
	bool resched = false;

	താക്കോൽ = k_spin_lock(&msgq->lock);

	എങ്കിൽ (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} അല്ലെങ്കിൽ {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	എങ്കിൽ (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			എങ്കിൽ (pending_thread != ശൂന്യ) {
				/* copy into the receiver's buffer */
				(ശൂന്യം)memcpy(pending_thread->base.swap_data, data,
					     msgq->സന്ദേശം_size);
				arch_thread_return_മൂല്യം_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		എങ്കിൽ (pending_thread == ശൂന്യ) {
			size_t സന്ദേശം_size = msgq->സന്ദേശം_size;
			അക്ഷരം *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->സന്ദേശം_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			എങ്കിൽ (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				അക്ഷരം *next;

				slot = msgq->write_ptr;
				next = slot + സന്ദേശം_size;
				എങ്കിൽ (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} അല്ലെങ്കിൽ {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				എങ്കിൽ (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= സന്ദേശം_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(ശൂന്യം)memcpy(slot, data, സന്ദേശം_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} അല്ലെങ്കിൽ {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (ശൂന്യം *) data;

		result = z_pend_curr(&msgq->lock, താക്കോൽ, &msgq->wait_q, timeout);

		എങ്കിൽ (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} അല്ലെങ്കിൽ {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		തിരികെ result;
	}

	എങ്കിൽ (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} അല്ലെങ്കിൽ {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	എങ്കിൽ (resched) {
		z_reschedule(&msgq->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&msgq->lock, താക്കോൽ);
	}

	തിരികെ result;
}


പൂർണ്ണസംഖ്യ z_impl_k_msgq_put(ഘടന k_msgq *msgq, സ്ഥിരം ശൂന്യം *data, k_timeout_t timeout)
{
	തിരികെ put_സന്ദേശം_in_queue(msgq, data, timeout, true);
}

പൂർണ്ണസംഖ്യ z_impl_k_msgq_put_front(ഘടന k_msgq *msgq, സ്ഥിരം ശൂന്യം *data)
{
	തിരികെ put_സന്ദേശം_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_msgq_put(ഘടന k_msgq *msgq, സ്ഥിരം ശൂന്യം *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->സന്ദേശം_size));

	തിരികെ z_impl_k_msgq_put(msgq, data, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_put_mrsh.c>

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_msgq_put_front(ഘടന k_msgq *msgq, സ്ഥിരം ശൂന്യം *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->സന്ദേശം_size));

	തിരികെ z_impl_k_msgq_put_front(msgq, data);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_impl_k_msgq_get_attrs(ഘടന k_msgq *msgq, ഘടന k_msgq_attrs *attrs)
{
	attrs->സന്ദേശം_size = msgq->സന്ദേശം_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_msgq_get_attrs(ഘടന k_msgq *msgq,
					   ഘടന k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, വലുപ്പം(ഘടന k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_msgq_get(ഘടന k_msgq *msgq, ശൂന്യം *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_താക്കോൽ_t താക്കോൽ;
	ഘടന k_thread *pending_thread;
	പൂർണ്ണസംഖ്യ result;
	bool resched = false;

	താക്കോൽ = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	എങ്കിൽ (likely(msgq->used_msgs > 0U)) {
		size_t സന്ദേശം_size = msgq->സന്ദേശം_size;
		അക്ഷരം *slot = msgq->read_ptr;
		അക്ഷരം *next = slot + സന്ദേശം_size;

		/* drop the message from the queue, then copy it out */
		എങ്കിൽ (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(ശൂന്യം)memcpy(data, slot, സന്ദേശം_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->സന്ദേശം_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			എങ്കിൽ (pending_thread != ശൂന്യ) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + സന്ദേശം_size;
				എങ്കിൽ (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(ശൂന്യം)memcpy(slot, pending_thread->base.swap_data, സന്ദേശം_size);

				arch_thread_return_മൂല്യം_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} അല്ലെങ്കിൽ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, താക്കോൽ, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		തിരികെ result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	എങ്കിൽ (resched) {
		z_reschedule(&msgq->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&msgq->lock, താക്കോൽ);
	}

	തിരികെ result;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_msgq_get(ഘടന k_msgq *msgq, ശൂന്യം *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->സന്ദേശം_size));

	തിരികെ z_impl_k_msgq_get(msgq, data, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_msgq_peek(ഘടന k_msgq *msgq, ശൂന്യം *data)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ result;

	താക്കോൽ = k_spin_lock(&msgq->lock);

	എങ്കിൽ (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(ശൂന്യം)memcpy(data, msgq->read_ptr, msgq->സന്ദേശം_size);
		result = 0;
	} അല്ലെങ്കിൽ {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, താക്കോൽ);

	തിരികെ result;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_msgq_peek(ഘടന k_msgq *msgq, ശൂന്യം *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->സന്ദേശം_size));

	തിരികെ z_impl_k_msgq_peek(msgq, data);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_msgq_peek_at(ഘടന k_msgq *msgq, ശൂന്യം *data, uint32_t idx)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	അക്ഷരം *start_addr;

	താക്കോൽ = k_spin_lock(&msgq->lock);

	എങ്കിൽ (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->സന്ദേശം_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		എങ്കിൽ (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(ശൂന്യം)memcpy(data, start_addr + byte_offset, msgq->സന്ദേശം_size);
		result = 0;
	} അല്ലെങ്കിൽ {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, താക്കോൽ);

	തിരികെ result;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_msgq_peek_at(ഘടന k_msgq *msgq, ശൂന്യം *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->സന്ദേശം_size));

	തിരികെ z_impl_k_msgq_peek_at(msgq, data, idx);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_impl_k_msgq_purge(ഘടന k_msgq *msgq)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	bool resched = false;

	താക്കോൽ = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	ആകുമ്പോൾ (z_sched_wake(&msgq->wait_q, -ENOMSG, ശൂന്യ)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	എങ്കിൽ (resched) {
		z_reschedule(&msgq->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&msgq->lock, താക്കോൽ);
	}
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_msgq_purge(ഘടന k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_purge_mrsh.c>

സ്ഥിതം വരിയിൽ uint32_t z_vrfy_k_msgq_അക്കം_free_get(ഘടന k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	തിരികെ z_impl_k_msgq_അക്കം_free_get(msgq);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

സ്ഥിതം വരിയിൽ uint32_t z_vrfy_k_msgq_അക്കം_used_get(ഘടന k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	തിരികെ z_impl_k_msgq_അക്കം_used_get(msgq);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
