/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <zephyr/linker/sections.h>
#شامل <string.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <wait_q.h>
#شامل <zephyr/sys/dlist.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <kernel_internal.h>
#شامل <zephyr/sys/check.h>

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_MSGQ */

ایستا درونخطی bool msgq_handle_poll_events(ساختار k_msgq *msgq)
{
#ifdef CONFIG_POLL
	بازگشت z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#وگرنه
	ARG_UNUSED(msgq);
	بازگشت false;
#endif /* CONFIG_POLL */
}

تهی k_msgq_init(ساختار k_msgq *msgq, نویسه *buffer, size_t پیام_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, پیام_size, &(size_t){0}));
	__ASSERT_NO_MSG(!size_add_overflow((size_t)(uintptr_t)buffer, max_msgs * پیام_size,
					&(size_t){0}));

	msgq->پیام_size = پیام_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * پیام_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (ساختار k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

صحیح z_impl_k_msgq_alloc_init(ساختار k_msgq *msgq, size_t پیام_size,
			    uint32_t max_msgs)
{
	تهی *buffer;
	صحیح ret;
	size_t total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	اگر (size_mul_overflow(پیام_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} وگرنه {
		buffer = z_thread_malloc(total_size);
		اگر (buffer != پوچ) {
			k_msgq_init(msgq, buffer, پیام_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} وگرنه {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	بازگشت ret;
}

#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_msgq_alloc_init(ساختار k_msgq *msgq, size_t پیام_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	بازگشت z_impl_k_msgq_alloc_init(msgq, پیام_size, max_msgs);
}
#شامل <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_msgq_cleanup(ساختار k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	صحیح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != پوچ)) {
		ret = -EBUSY;
		برو out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != پوچ)) {
		ret = -EBUSY;
		برو out;
	}

	اگر ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, کلید);

#ifdef CONFIG_OBJ_CORE_MSGQ
	اگر (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	بازگشت ret;
}

صحیح k_msgq_cleanup(ساختار k_msgq *msgq)
{
	بازگشت z_msgq_cleanup(msgq, false);
}

ایستا درونخطی صحیح put_پیام_in_queue(ساختار k_msgq *msgq, ثابت تهی *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ساختار k_thread *pending_thread = پوچ;
	k_spinlock_کلید_t کلید;
	صحیح result;
	bool resched = false;

	کلید = k_spin_lock(&msgq->lock);

	اگر (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} وگرنه {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	اگر (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			اگر (pending_thread != پوچ) {
				/* copy into the receiver's buffer */
				(تهی)memcpy(pending_thread->base.swap_data, data,
					     msgq->پیام_size);
				arch_thread_return_مقدار_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		اگر (pending_thread == پوچ) {
			size_t پیام_size = msgq->پیام_size;
			نویسه *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->پیام_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			اگر (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				نویسه *next;

				slot = msgq->write_ptr;
				next = slot + پیام_size;
				اگر (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} وگرنه {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				اگر (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= پیام_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(تهی)memcpy(slot, data, پیام_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} وگرنه اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} وگرنه {
		اگر (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} وگرنه {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (تهی *) data;

		result = z_pend_curr(&msgq->lock, کلید, &msgq->wait_q, timeout);

		اگر (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} وگرنه {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		بازگشت result;
	}

	اگر (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} وگرنه {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	اگر (resched) {
		z_reschedule(&msgq->lock, کلید);
	} وگرنه {
		k_spin_unlock(&msgq->lock, کلید);
	}

	بازگشت result;
}


صحیح z_impl_k_msgq_put(ساختار k_msgq *msgq, ثابت تهی *data, k_timeout_t timeout)
{
	بازگشت put_پیام_in_queue(msgq, data, timeout, true);
}

صحیح z_impl_k_msgq_put_front(ساختار k_msgq *msgq, ثابت تهی *data)
{
	بازگشت put_پیام_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_msgq_put(ساختار k_msgq *msgq, ثابت تهی *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->پیام_size));

	بازگشت z_impl_k_msgq_put(msgq, data, timeout);
}
#شامل <zephyr/syscalls/k_msgq_put_mrsh.c>

ایستا درونخطی صحیح z_vrfy_k_msgq_put_front(ساختار k_msgq *msgq, ثابت تهی *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->پیام_size));

	بازگشت z_impl_k_msgq_put_front(msgq, data);
}
#شامل <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_impl_k_msgq_get_attrs(ساختار k_msgq *msgq, ساختار k_msgq_attrs *attrs)
{
	attrs->پیام_size = msgq->پیام_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_msgq_get_attrs(ساختار k_msgq *msgq,
					   ساختار k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, اندازه(ساختار k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#شامل <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_msgq_get(ساختار k_msgq *msgq, تهی *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_کلید_t کلید;
	ساختار k_thread *pending_thread;
	صحیح result;
	bool resched = false;

	کلید = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	اگر (likely(msgq->used_msgs > 0U)) {
		size_t پیام_size = msgq->پیام_size;
		نویسه *slot = msgq->read_ptr;
		نویسه *next = slot + پیام_size;

		/* drop the message from the queue, then copy it out */
		اگر (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(تهی)memcpy(data, slot, پیام_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->پیام_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			اگر (pending_thread != پوچ) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + پیام_size;
				اگر (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(تهی)memcpy(slot, pending_thread->base.swap_data, پیام_size);

				arch_thread_return_مقدار_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} وگرنه اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} وگرنه {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, کلید, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		بازگشت result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	اگر (resched) {
		z_reschedule(&msgq->lock, کلید);
	} وگرنه {
		k_spin_unlock(&msgq->lock, کلید);
	}

	بازگشت result;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_msgq_get(ساختار k_msgq *msgq, تهی *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->پیام_size));

	بازگشت z_impl_k_msgq_get(msgq, data, timeout);
}
#شامل <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_msgq_peek(ساختار k_msgq *msgq, تهی *data)
{
	k_spinlock_کلید_t کلید;
	صحیح result;

	کلید = k_spin_lock(&msgq->lock);

	اگر (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(تهی)memcpy(data, msgq->read_ptr, msgq->پیام_size);
		result = 0;
	} وگرنه {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, کلید);

	بازگشت result;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_msgq_peek(ساختار k_msgq *msgq, تهی *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->پیام_size));

	بازگشت z_impl_k_msgq_peek(msgq, data);
}
#شامل <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_msgq_peek_at(ساختار k_msgq *msgq, تهی *data, uint32_t idx)
{
	k_spinlock_کلید_t کلید;
	صحیح result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	نویسه *start_addr;

	کلید = k_spin_lock(&msgq->lock);

	اگر (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->پیام_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		اگر (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(تهی)memcpy(data, start_addr + byte_offset, msgq->پیام_size);
		result = 0;
	} وگرنه {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, کلید);

	بازگشت result;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_msgq_peek_at(ساختار k_msgq *msgq, تهی *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->پیام_size));

	بازگشت z_impl_k_msgq_peek_at(msgq, data, idx);
}
#شامل <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

تهی z_impl_k_msgq_purge(ساختار k_msgq *msgq)
{
	k_spinlock_کلید_t کلید;
	bool resched = false;

	کلید = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	وقتی (z_sched_wake(&msgq->wait_q, -ENOMSG, پوچ)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	اگر (resched) {
		z_reschedule(&msgq->lock, کلید);
	} وگرنه {
		k_spin_unlock(&msgq->lock, کلید);
	}
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_msgq_purge(ساختار k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#شامل <zephyr/syscalls/k_msgq_purge_mrsh.c>

ایستا درونخطی uint32_t z_vrfy_k_msgq_رقم_free_get(ساختار k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	بازگشت z_impl_k_msgq_رقم_free_get(msgq);
}
#شامل <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ایستا درونخطی uint32_t z_vrfy_k_msgq_رقم_used_get(ساختار k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	بازگشت z_impl_k_msgq_رقم_used_get(msgq);
}
#شامل <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
