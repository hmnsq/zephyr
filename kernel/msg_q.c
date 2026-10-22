/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/linker/sections.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <scheduler.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/sys/check.h>

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_MSGQ */

સ્થિર અંતઃરેખ bool msgq_handle_poll_events(સંરચના k_msgq *msgq)
{
#ifdef CONFIG_POLL
	પાછા z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#નહીંતર
	ARG_UNUSED(msgq);
	પાછા false;
#endif /* CONFIG_POLL */
}

શૂન્ય k_msgq_init(સંરચના k_msgq *msgq, અક્ષર *buffer, size_t સંદેશ_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, સંદેશ_size, &(size_t){0}));
	__ASSERT_NO_MSG(!size_add_overflow((size_t)(uintptr_t)buffer, max_msgs * સંદેશ_size,
					&(size_t){0}));

	msgq->સંદેશ_size = સંદેશ_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * સંદેશ_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (સંરચના k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

પૂર્ણાંક z_impl_k_msgq_alloc_init(સંરચના k_msgq *msgq, size_t સંદેશ_size,
			    uint32_t max_msgs)
{
	શૂન્ય *buffer;
	પૂર્ણાંક ret;
	size_t total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	જો (size_mul_overflow(સંદેશ_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} નહીંતર {
		buffer = z_thread_malloc(total_size);
		જો (buffer != ખાલી) {
			k_msgq_init(msgq, buffer, સંદેશ_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} નહીંતર {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	પાછા ret;
}

#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_msgq_alloc_init(સંરચના k_msgq *msgq, size_t સંદેશ_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	પાછા z_impl_k_msgq_alloc_init(msgq, સંદેશ_size, max_msgs);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_msgq_cleanup(સંરચના k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	પૂર્ણાંક ret = 0;
	bool freed = false;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != ખાલી)) {
		ret = -EBUSY;
		જાઓ out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != ખાલી)) {
		ret = -EBUSY;
		જાઓ out;
	}

	જો ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, ચાવી);

#ifdef CONFIG_OBJ_CORE_MSGQ
	જો (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	પાછા ret;
}

પૂર્ણાંક k_msgq_cleanup(સંરચના k_msgq *msgq)
{
	પાછા z_msgq_cleanup(msgq, false);
}

સ્થિર અંતઃરેખ પૂર્ણાંક put_સંદેશ_in_queue(સંરચના k_msgq *msgq, અચલ શૂન્ય *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	સંરચના k_thread *pending_thread = ખાલી;
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક result;
	bool resched = false;

	ચાવી = k_spin_lock(&msgq->lock);

	જો (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} નહીંતર {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	જો (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			જો (pending_thread != ખાલી) {
				/* copy into the receiver's buffer */
				(શૂન્ય)memcpy(pending_thread->base.swap_data, data,
					     msgq->સંદેશ_size);
				arch_thread_return_કિંમત_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		જો (pending_thread == ખાલી) {
			size_t સંદેશ_size = msgq->સંદેશ_size;
			અક્ષર *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->સંદેશ_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			જો (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				અક્ષર *next;

				slot = msgq->write_ptr;
				next = slot + સંદેશ_size;
				જો (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} નહીંતર {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				જો (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= સંદેશ_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(શૂન્ય)memcpy(slot, data, સંદેશ_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} નહીંતર જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} નહીંતર {
		જો (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} નહીંતર {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (શૂન્ય *) data;

		result = z_pend_curr(&msgq->lock, ચાવી, &msgq->wait_q, timeout);

		જો (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} નહીંતર {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		પાછા result;
	}

	જો (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} નહીંતર {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	જો (resched) {
		z_reschedule(&msgq->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&msgq->lock, ચાવી);
	}

	પાછા result;
}


પૂર્ણાંક z_impl_k_msgq_put(સંરચના k_msgq *msgq, અચલ શૂન્ય *data, k_timeout_t timeout)
{
	પાછા put_સંદેશ_in_queue(msgq, data, timeout, true);
}

પૂર્ણાંક z_impl_k_msgq_put_front(સંરચના k_msgq *msgq, અચલ શૂન્ય *data)
{
	પાછા put_સંદેશ_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_msgq_put(સંરચના k_msgq *msgq, અચલ શૂન્ય *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->સંદેશ_size));

	પાછા z_impl_k_msgq_put(msgq, data, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_put_mrsh.c>

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_msgq_put_front(સંરચના k_msgq *msgq, અચલ શૂન્ય *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->સંદેશ_size));

	પાછા z_impl_k_msgq_put_front(msgq, data);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_impl_k_msgq_get_attrs(સંરચના k_msgq *msgq, સંરચના k_msgq_attrs *attrs)
{
	attrs->સંદેશ_size = msgq->સંદેશ_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_msgq_get_attrs(સંરચના k_msgq *msgq,
					   સંરચના k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, કદ(સંરચના k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_msgq_get(સંરચના k_msgq *msgq, શૂન્ય *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_ચાવી_t ચાવી;
	સંરચના k_thread *pending_thread;
	પૂર્ણાંક result;
	bool resched = false;

	ચાવી = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	જો (likely(msgq->used_msgs > 0U)) {
		size_t સંદેશ_size = msgq->સંદેશ_size;
		અક્ષર *slot = msgq->read_ptr;
		અક્ષર *next = slot + સંદેશ_size;

		/* drop the message from the queue, then copy it out */
		જો (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(શૂન્ય)memcpy(data, slot, સંદેશ_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->સંદેશ_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			જો (pending_thread != ખાલી) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + સંદેશ_size;
				જો (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(શૂન્ય)memcpy(slot, pending_thread->base.swap_data, સંદેશ_size);

				arch_thread_return_કિંમત_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} નહીંતર જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} નહીંતર {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, ચાવી, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		પાછા result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	જો (resched) {
		z_reschedule(&msgq->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&msgq->lock, ચાવી);
	}

	પાછા result;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_msgq_get(સંરચના k_msgq *msgq, શૂન્ય *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->સંદેશ_size));

	પાછા z_impl_k_msgq_get(msgq, data, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_msgq_peek(સંરચના k_msgq *msgq, શૂન્ય *data)
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક result;

	ચાવી = k_spin_lock(&msgq->lock);

	જો (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(શૂન્ય)memcpy(data, msgq->read_ptr, msgq->સંદેશ_size);
		result = 0;
	} નહીંતર {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, ચાવી);

	પાછા result;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_msgq_peek(સંરચના k_msgq *msgq, શૂન્ય *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->સંદેશ_size));

	પાછા z_impl_k_msgq_peek(msgq, data);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_msgq_peek_at(સંરચના k_msgq *msgq, શૂન્ય *data, uint32_t idx)
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	અક્ષર *start_addr;

	ચાવી = k_spin_lock(&msgq->lock);

	જો (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->સંદેશ_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		જો (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(શૂન્ય)memcpy(data, start_addr + byte_offset, msgq->સંદેશ_size);
		result = 0;
	} નહીંતર {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, ચાવી);

	પાછા result;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_msgq_peek_at(સંરચના k_msgq *msgq, શૂન્ય *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->સંદેશ_size));

	પાછા z_impl_k_msgq_peek_at(msgq, data, idx);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_impl_k_msgq_purge(સંરચના k_msgq *msgq)
{
	k_spinlock_ચાવી_t ચાવી;
	bool resched = false;

	ચાવી = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	જ્યારે (z_sched_wake(&msgq->wait_q, -ENOMSG, ખાલી)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	જો (resched) {
		z_reschedule(&msgq->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&msgq->lock, ચાવી);
	}
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_msgq_purge(સંરચના k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_purge_mrsh.c>

સ્થિર અંતઃરેખ uint32_t z_vrfy_k_msgq_અંક_free_get(સંરચના k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	પાછા z_impl_k_msgq_અંક_free_get(msgq);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

સ્થિર અંતઃરેખ uint32_t z_vrfy_k_msgq_અંક_used_get(સંરચના k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	પાછા z_impl_k_msgq_અંક_used_get(msgq);
}
#અન્તર્ભાવ <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
