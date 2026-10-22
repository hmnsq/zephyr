/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <zephyr/linker/sections.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <scheduler.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>
#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <zephyr/sys/check.h>

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_MSGQ */

நிலையான உள்வரி bool msgq_handle_poll_events(கட்டமைப்பு k_msgq *msgq)
{
#ifdef CONFIG_POLL
	திரும்பு z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#இல்லையெனில்
	ARG_UNUSED(msgq);
	திரும்பு false;
#endif /* CONFIG_POLL */
}

வெறுமை k_msgq_init(கட்டமைப்பு k_msgq *msgq, எழுத்து *buffer, size_t செய்தி_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, செய்தி_size, &(size_t){0}));
	__ASSERT_NO_MSG(!size_add_overflow((size_t)(uintptr_t)buffer, max_msgs * செய்தி_size,
					&(size_t){0}));

	msgq->செய்தி_size = செய்தி_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * செய்தி_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (கட்டமைப்பு k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

முழு z_impl_k_msgq_alloc_init(கட்டமைப்பு k_msgq *msgq, size_t செய்தி_size,
			    uint32_t max_msgs)
{
	வெறுமை *buffer;
	முழு ret;
	size_t total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	எனில் (size_mul_overflow(செய்தி_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} இல்லையெனில் {
		buffer = z_thread_malloc(total_size);
		எனில் (buffer != வெற்று) {
			k_msgq_init(msgq, buffer, செய்தி_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} இல்லையெனில் {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	திரும்பு ret;
}

#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_msgq_alloc_init(கட்டமைப்பு k_msgq *msgq, size_t செய்தி_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	திரும்பு z_impl_k_msgq_alloc_init(msgq, செய்தி_size, max_msgs);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_msgq_cleanup(கட்டமைப்பு k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	முழு ret = 0;
	bool freed = false;
	k_spinlock_சாவி_t சாவி = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != வெற்று)) {
		ret = -EBUSY;
		செல் out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != வெற்று)) {
		ret = -EBUSY;
		செல் out;
	}

	எனில் ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, சாவி);

#ifdef CONFIG_OBJ_CORE_MSGQ
	எனில் (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	திரும்பு ret;
}

முழு k_msgq_cleanup(கட்டமைப்பு k_msgq *msgq)
{
	திரும்பு z_msgq_cleanup(msgq, false);
}

நிலையான உள்வரி முழு put_செய்தி_in_queue(கட்டமைப்பு k_msgq *msgq, மாறிலி வெறுமை *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	கட்டமைப்பு k_thread *pending_thread = வெற்று;
	k_spinlock_சாவி_t சாவி;
	முழு result;
	bool resched = false;

	சாவி = k_spin_lock(&msgq->lock);

	எனில் (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} இல்லையெனில் {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	எனில் (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			எனில் (pending_thread != வெற்று) {
				/* copy into the receiver's buffer */
				(வெறுமை)memcpy(pending_thread->base.swap_data, data,
					     msgq->செய்தி_size);
				arch_thread_return_மதிப்பு_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		எனில் (pending_thread == வெற்று) {
			size_t செய்தி_size = msgq->செய்தி_size;
			எழுத்து *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->செய்தி_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			எனில் (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				எழுத்து *next;

				slot = msgq->write_ptr;
				next = slot + செய்தி_size;
				எனில் (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} இல்லையெனில் {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				எனில் (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= செய்தி_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(வெறுமை)memcpy(slot, data, செய்தி_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} இல்லையெனில் எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} இல்லையெனில் {
		எனில் (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} இல்லையெனில் {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (வெறுமை *) data;

		result = z_pend_curr(&msgq->lock, சாவி, &msgq->wait_q, timeout);

		எனில் (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} இல்லையெனில் {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		திரும்பு result;
	}

	எனில் (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} இல்லையெனில் {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	எனில் (resched) {
		z_reschedule(&msgq->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&msgq->lock, சாவி);
	}

	திரும்பு result;
}


முழு z_impl_k_msgq_put(கட்டமைப்பு k_msgq *msgq, மாறிலி வெறுமை *data, k_timeout_t timeout)
{
	திரும்பு put_செய்தி_in_queue(msgq, data, timeout, true);
}

முழு z_impl_k_msgq_put_front(கட்டமைப்பு k_msgq *msgq, மாறிலி வெறுமை *data)
{
	திரும்பு put_செய்தி_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_msgq_put(கட்டமைப்பு k_msgq *msgq, மாறிலி வெறுமை *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->செய்தி_size));

	திரும்பு z_impl_k_msgq_put(msgq, data, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_put_mrsh.c>

நிலையான உள்வரி முழு z_vrfy_k_msgq_put_front(கட்டமைப்பு k_msgq *msgq, மாறிலி வெறுமை *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->செய்தி_size));

	திரும்பு z_impl_k_msgq_put_front(msgq, data);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_impl_k_msgq_get_attrs(கட்டமைப்பு k_msgq *msgq, கட்டமைப்பு k_msgq_attrs *attrs)
{
	attrs->செய்தி_size = msgq->செய்தி_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_msgq_get_attrs(கட்டமைப்பு k_msgq *msgq,
					   கட்டமைப்பு k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, அளவு(கட்டமைப்பு k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_msgq_get(கட்டமைப்பு k_msgq *msgq, வெறுமை *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_சாவி_t சாவி;
	கட்டமைப்பு k_thread *pending_thread;
	முழு result;
	bool resched = false;

	சாவி = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	எனில் (likely(msgq->used_msgs > 0U)) {
		size_t செய்தி_size = msgq->செய்தி_size;
		எழுத்து *slot = msgq->read_ptr;
		எழுத்து *next = slot + செய்தி_size;

		/* drop the message from the queue, then copy it out */
		எனில் (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(வெறுமை)memcpy(data, slot, செய்தி_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->செய்தி_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			எனில் (pending_thread != வெற்று) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + செய்தி_size;
				எனில் (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(வெறுமை)memcpy(slot, pending_thread->base.swap_data, செய்தி_size);

				arch_thread_return_மதிப்பு_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} இல்லையெனில் எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} இல்லையெனில் {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, சாவி, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		திரும்பு result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	எனில் (resched) {
		z_reschedule(&msgq->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&msgq->lock, சாவி);
	}

	திரும்பு result;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_msgq_get(கட்டமைப்பு k_msgq *msgq, வெறுமை *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->செய்தி_size));

	திரும்பு z_impl_k_msgq_get(msgq, data, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_msgq_peek(கட்டமைப்பு k_msgq *msgq, வெறுமை *data)
{
	k_spinlock_சாவி_t சாவி;
	முழு result;

	சாவி = k_spin_lock(&msgq->lock);

	எனில் (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(வெறுமை)memcpy(data, msgq->read_ptr, msgq->செய்தி_size);
		result = 0;
	} இல்லையெனில் {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, சாவி);

	திரும்பு result;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_msgq_peek(கட்டமைப்பு k_msgq *msgq, வெறுமை *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->செய்தி_size));

	திரும்பு z_impl_k_msgq_peek(msgq, data);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_msgq_peek_at(கட்டமைப்பு k_msgq *msgq, வெறுமை *data, uint32_t idx)
{
	k_spinlock_சாவி_t சாவி;
	முழு result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	எழுத்து *start_addr;

	சாவி = k_spin_lock(&msgq->lock);

	எனில் (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->செய்தி_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		எனில் (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(வெறுமை)memcpy(data, start_addr + byte_offset, msgq->செய்தி_size);
		result = 0;
	} இல்லையெனில் {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, சாவி);

	திரும்பு result;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_msgq_peek_at(கட்டமைப்பு k_msgq *msgq, வெறுமை *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->செய்தி_size));

	திரும்பு z_impl_k_msgq_peek_at(msgq, data, idx);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_impl_k_msgq_purge(கட்டமைப்பு k_msgq *msgq)
{
	k_spinlock_சாவி_t சாவி;
	bool resched = false;

	சாவி = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	போது (z_sched_wake(&msgq->wait_q, -ENOMSG, வெற்று)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	எனில் (resched) {
		z_reschedule(&msgq->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&msgq->lock, சாவி);
	}
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_msgq_purge(கட்டமைப்பு k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_purge_mrsh.c>

நிலையான உள்வரி uint32_t z_vrfy_k_msgq_இலக்கம்_free_get(கட்டமைப்பு k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	திரும்பு z_impl_k_msgq_இலக்கம்_free_get(msgq);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

நிலையான உள்வரி uint32_t z_vrfy_k_msgq_இலக்கம்_used_get(கட்டமைப்பு k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	திரும்பு z_impl_k_msgq_இலக்கம்_used_get(msgq);
}
#அந்தர்பாவ <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
