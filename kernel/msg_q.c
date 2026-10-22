/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#অন্তর্ভাব <zephyr/kernel.h>

#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <zephyr/linker/sections.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <scheduler.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>
#অন্তর্ভাব <zephyr/sys/math_extras.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <zephyr/sys/check.h>

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, রিক্ত);
#endif /* CONFIG_OBJ_CORE_MSGQ */

স্থির অন্তঃস্থ bool msgq_handle_poll_events(সংরচনা k_msgq *msgq)
{
#ifdef CONFIG_POLL
	প্রত্যাগম z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#অন্যথা
	ARG_UNUSED(msgq);
	প্রত্যাগম false;
#endif /* CONFIG_POLL */
}

শূন্য k_msgq_init(সংরচনা k_msgq *msgq, বর্ণ *buffer, size_t বার্তা_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, বার্তা_size, &(size_t){0}));
	__ASSERT_NO_MSG(!size_add_overflow((size_t)(uintptr_t)buffer, max_msgs * বার্তা_size,
					&(size_t){0}));

	msgq->বার্তা_size = বার্তা_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * বার্তা_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (সংরচনা k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

পূর্ণাঙ্ক z_impl_k_msgq_alloc_init(সংরচনা k_msgq *msgq, size_t বার্তা_size,
			    uint32_t max_msgs)
{
	শূন্য *buffer;
	পূর্ণাঙ্ক ret;
	size_t total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	যদি (size_mul_overflow(বার্তা_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} অন্যথা {
		buffer = z_thread_malloc(total_size);
		যদি (buffer != রিক্ত) {
			k_msgq_init(msgq, buffer, বার্তা_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} অন্যথা {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	প্রত্যাগম ret;
}

#ifdef CONFIG_USERSPACE
পূর্ণাঙ্ক z_vrfy_k_msgq_alloc_init(সংরচনা k_msgq *msgq, size_t বার্তা_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	প্রত্যাগম z_impl_k_msgq_alloc_init(msgq, বার্তা_size, max_msgs);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_msgq_cleanup(সংরচনা k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	পূর্ণাঙ্ক ret = 0;
	bool freed = false;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != রিক্ত)) {
		ret = -EBUSY;
		গচ্ছ out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != রিক্ত)) {
		ret = -EBUSY;
		গচ্ছ out;
	}

	যদি ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, চাবি);

#ifdef CONFIG_OBJ_CORE_MSGQ
	যদি (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	প্রত্যাগম ret;
}

পূর্ণাঙ্ক k_msgq_cleanup(সংরচনা k_msgq *msgq)
{
	প্রত্যাগম z_msgq_cleanup(msgq, false);
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক put_বার্তা_in_queue(সংরচনা k_msgq *msgq, ধ্রুব শূন্য *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	সংরচনা k_thread *pending_thread = রিক্ত;
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক result;
	bool resched = false;

	চাবি = k_spin_lock(&msgq->lock);

	যদি (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} অন্যথা {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	যদি (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			যদি (pending_thread != রিক্ত) {
				/* copy into the receiver's buffer */
				(শূন্য)memcpy(pending_thread->base.swap_data, data,
					     msgq->বার্তা_size);
				arch_thread_return_মান_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		যদি (pending_thread == রিক্ত) {
			size_t বার্তা_size = msgq->বার্তা_size;
			বর্ণ *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->বার্তা_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			যদি (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				বর্ণ *next;

				slot = msgq->write_ptr;
				next = slot + বার্তা_size;
				যদি (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} অন্যথা {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				যদি (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= বার্তা_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(শূন্য)memcpy(slot, data, বার্তা_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} অন্যথা যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} অন্যথা {
		যদি (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} অন্যথা {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (শূন্য *) data;

		result = z_pend_curr(&msgq->lock, চাবি, &msgq->wait_q, timeout);

		যদি (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} অন্যথা {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		প্রত্যাগম result;
	}

	যদি (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} অন্যথা {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	যদি (resched) {
		z_reschedule(&msgq->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&msgq->lock, চাবি);
	}

	প্রত্যাগম result;
}


পূর্ণাঙ্ক z_impl_k_msgq_put(সংরচনা k_msgq *msgq, ধ্রুব শূন্য *data, k_timeout_t timeout)
{
	প্রত্যাগম put_বার্তা_in_queue(msgq, data, timeout, true);
}

পূর্ণাঙ্ক z_impl_k_msgq_put_front(সংরচনা k_msgq *msgq, ধ্রুব শূন্য *data)
{
	প্রত্যাগম put_বার্তা_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_msgq_put(সংরচনা k_msgq *msgq, ধ্রুব শূন্য *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->বার্তা_size));

	প্রত্যাগম z_impl_k_msgq_put(msgq, data, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_put_mrsh.c>

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_msgq_put_front(সংরচনা k_msgq *msgq, ধ্রুব শূন্য *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->বার্তা_size));

	প্রত্যাগম z_impl_k_msgq_put_front(msgq, data);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_msgq_get_attrs(সংরচনা k_msgq *msgq, সংরচনা k_msgq_attrs *attrs)
{
	attrs->বার্তা_size = msgq->বার্তা_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_msgq_get_attrs(সংরচনা k_msgq *msgq,
					   সংরচনা k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, পরিমাণ(সংরচনা k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_msgq_get(সংরচনা k_msgq *msgq, শূন্য *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_চাবি_t চাবি;
	সংরচনা k_thread *pending_thread;
	পূর্ণাঙ্ক result;
	bool resched = false;

	চাবি = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	যদি (likely(msgq->used_msgs > 0U)) {
		size_t বার্তা_size = msgq->বার্তা_size;
		বর্ণ *slot = msgq->read_ptr;
		বর্ণ *next = slot + বার্তা_size;

		/* drop the message from the queue, then copy it out */
		যদি (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(শূন্য)memcpy(data, slot, বার্তা_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->বার্তা_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			যদি (pending_thread != রিক্ত) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + বার্তা_size;
				যদি (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(শূন্য)memcpy(slot, pending_thread->base.swap_data, বার্তা_size);

				arch_thread_return_মান_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} অন্যথা যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} অন্যথা {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, চাবি, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		প্রত্যাগম result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	যদি (resched) {
		z_reschedule(&msgq->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&msgq->lock, চাবি);
	}

	প্রত্যাগম result;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_msgq_get(সংরচনা k_msgq *msgq, শূন্য *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->বার্তা_size));

	প্রত্যাগম z_impl_k_msgq_get(msgq, data, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_msgq_peek(সংরচনা k_msgq *msgq, শূন্য *data)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক result;

	চাবি = k_spin_lock(&msgq->lock);

	যদি (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(শূন্য)memcpy(data, msgq->read_ptr, msgq->বার্তা_size);
		result = 0;
	} অন্যথা {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, চাবি);

	প্রত্যাগম result;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_msgq_peek(সংরচনা k_msgq *msgq, শূন্য *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->বার্তা_size));

	প্রত্যাগম z_impl_k_msgq_peek(msgq, data);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_msgq_peek_at(সংরচনা k_msgq *msgq, শূন্য *data, uint32_t idx)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	বর্ণ *start_addr;

	চাবি = k_spin_lock(&msgq->lock);

	যদি (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->বার্তা_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		যদি (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(শূন্য)memcpy(data, start_addr + byte_offset, msgq->বার্তা_size);
		result = 0;
	} অন্যথা {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, চাবি);

	প্রত্যাগম result;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_msgq_peek_at(সংরচনা k_msgq *msgq, শূন্য *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->বার্তা_size));

	প্রত্যাগম z_impl_k_msgq_peek_at(msgq, data, idx);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_msgq_purge(সংরচনা k_msgq *msgq)
{
	k_spinlock_চাবি_t চাবি;
	bool resched = false;

	চাবি = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	যাবত্ (z_sched_wake(&msgq->wait_q, -ENOMSG, রিক্ত)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	যদি (resched) {
		z_reschedule(&msgq->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&msgq->lock, চাবি);
	}
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_msgq_purge(সংরচনা k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_purge_mrsh.c>

স্থির অন্তঃস্থ uint32_t z_vrfy_k_msgq_অঙ্ক_free_get(সংরচনা k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	প্রত্যাগম z_impl_k_msgq_অঙ্ক_free_get(msgq);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

স্থির অন্তঃস্থ uint32_t z_vrfy_k_msgq_অঙ্ক_used_get(সংরচনা k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	প্রত্যাগম z_impl_k_msgq_অঙ্ক_used_get(msgq);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
