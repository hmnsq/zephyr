/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/linker/sections.h>
#ضمن <string.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <wait_q.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/sys/check.h>

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, عدم);
#endif /* CONFIG_OBJ_CORE_MSGQ */

ساكن مضمن bool msgq_handle_poll_events(بنية k_msgq *msgq)
{
#ifdef CONFIG_POLL
	أرجع z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#وإلا
	ARG_UNUSED(msgq);
	أرجع false;
#endif /* CONFIG_POLL */
}

فراغ k_msgq_init(بنية k_msgq *msgq, حرف *buffer, size_t msg_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, msg_size, &(size_t){0}));
	__ASSERT_NO_MSG(!size_add_overflow((size_t)(uintptr_t)buffer, max_msgs * msg_size,
					&(size_t){0}));

	msgq->msg_size = msg_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * msg_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (بنية k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

صحيح z_impl_k_msgq_alloc_init(بنية k_msgq *msgq, size_t msg_size,
			    uint32_t max_msgs)
{
	فراغ *buffer;
	صحيح ret;
	size_t total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	𐡄𐡍 (size_mul_overflow(msg_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} وإلا {
		buffer = z_thread_malloc(total_size);
		𐡄𐡍 (buffer != عدم) {
			k_msgq_init(msgq, buffer, msg_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	أرجع ret;
}

#ifdef CONFIG_USERSPACE
صحيح z_vrfy_k_msgq_alloc_init(بنية k_msgq *msgq, size_t msg_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	أرجع z_impl_k_msgq_alloc_init(msgq, msg_size, max_msgs);
}
#ضمن <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_msgq_cleanup(بنية k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_key_t key = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب out;
	}

	𐡄𐡍 ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, key);

#ifdef CONFIG_OBJ_CORE_MSGQ
	𐡄𐡍 (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	أرجع ret;
}

صحيح k_msgq_cleanup(بنية k_msgq *msgq)
{
	أرجع z_msgq_cleanup(msgq, false);
}

ساكن مضمن صحيح put_msg_in_queue(بنية k_msgq *msgq, ثابت فراغ *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	بنية k_thread *pending_thread = عدم;
	k_spinlock_key_t key;
	صحيح result;
	bool resched = false;

	key = k_spin_lock(&msgq->lock);

	𐡄𐡍 (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} وإلا {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	𐡄𐡍 (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			𐡄𐡍 (pending_thread != عدم) {
				/* copy into the receiver's buffer */
				(فراغ)memcpy(pending_thread->base.swap_data, data,
					     msgq->msg_size);
				arch_thread_return_value_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		𐡄𐡍 (pending_thread == عدم) {
			size_t msg_size = msgq->msg_size;
			حرف *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->msg_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			𐡄𐡍 (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				حرف *next;

				slot = msgq->write_ptr;
				next = slot + msg_size;
				𐡄𐡍 (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} وإلا {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				𐡄𐡍 (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= msg_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(فراغ)memcpy(slot, data, msg_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} وإلا 𐡄𐡍 (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} وإلا {
		𐡄𐡍 (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} وإلا {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (فراغ *) data;

		result = z_pend_curr(&msgq->lock, key, &msgq->wait_q, timeout);

		𐡄𐡍 (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} وإلا {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		أرجع result;
	}

	𐡄𐡍 (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} وإلا {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	𐡄𐡍 (resched) {
		z_reschedule(&msgq->lock, key);
	} وإلا {
		k_spin_unlock(&msgq->lock, key);
	}

	أرجع result;
}


صحيح z_impl_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *data, k_timeout_t timeout)
{
	أرجع put_msg_in_queue(msgq, data, timeout, true);
}

صحيح z_impl_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *data)
{
	أرجع put_msg_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->msg_size));

	أرجع z_impl_k_msgq_put(msgq, data, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_put_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->msg_size));

	أرجع z_impl_k_msgq_put_front(msgq, data);
}
#ضمن <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_get_attrs(بنية k_msgq *msgq, بنية k_msgq_attrs *attrs)
{
	attrs->msg_size = msgq->msg_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_get_attrs(بنية k_msgq *msgq,
					   بنية k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, حجم(بنية k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ضمن <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_get(بنية k_msgq *msgq, فراغ *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_key_t key;
	بنية k_thread *pending_thread;
	صحيح result;
	bool resched = false;

	key = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	𐡄𐡍 (likely(msgq->used_msgs > 0U)) {
		size_t msg_size = msgq->msg_size;
		حرف *slot = msgq->read_ptr;
		حرف *next = slot + msg_size;

		/* drop the message from the queue, then copy it out */
		𐡄𐡍 (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(فراغ)memcpy(data, slot, msg_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->msg_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			𐡄𐡍 (pending_thread != عدم) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + msg_size;
				𐡄𐡍 (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(فراغ)memcpy(slot, pending_thread->base.swap_data, msg_size);

				arch_thread_return_value_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} وإلا 𐡄𐡍 (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} وإلا {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, key, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		أرجع result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	𐡄𐡍 (resched) {
		z_reschedule(&msgq->lock, key);
	} وإلا {
		k_spin_unlock(&msgq->lock, key);
	}

	أرجع result;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_get(بنية k_msgq *msgq, فراغ *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->msg_size));

	أرجع z_impl_k_msgq_get(msgq, data, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek(بنية k_msgq *msgq, فراغ *data)
{
	k_spinlock_key_t key;
	صحيح result;

	key = k_spin_lock(&msgq->lock);

	𐡄𐡍 (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(فراغ)memcpy(data, msgq->read_ptr, msgq->msg_size);
		result = 0;
	} وإلا {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, key);

	أرجع result;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek(بنية k_msgq *msgq, فراغ *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->msg_size));

	أرجع z_impl_k_msgq_peek(msgq, data);
}
#ضمن <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *data, uint32_t idx)
{
	k_spinlock_key_t key;
	صحيح result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	حرف *start_addr;

	key = k_spin_lock(&msgq->lock);

	𐡄𐡍 (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->msg_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		𐡄𐡍 (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(فراغ)memcpy(data, start_addr + byte_offset, msgq->msg_size);
		result = 0;
	} وإلا {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, key);

	أرجع result;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->msg_size));

	أرجع z_impl_k_msgq_peek_at(msgq, data, idx);
}
#ضمن <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_purge(بنية k_msgq *msgq)
{
	k_spinlock_key_t key;
	bool resched = false;

	key = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	طالما (z_sched_wake(&msgq->wait_q, -ENOMSG, عدم)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	𐡄𐡍 (resched) {
		z_reschedule(&msgq->lock, key);
	} وإلا {
		k_spin_unlock(&msgq->lock, key);
	}
}

#ifdef CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_purge(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_purge_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_num_free_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_num_free_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_num_used_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_num_used_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
