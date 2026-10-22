/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/linker/sections.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/dlist.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/check.h>

#ifdef CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, ଖାଲି);
#endif /* CONFIG_OBJ_CORE_MSGQ */

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool msgq_handle_poll_events(ସଂରଚନା k_msgq *msgq)
{
#ifdef CONFIG_POLL
	ଫେରନ୍ତୁ z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_DATA_AVAILABLE);
#ଅନ୍ୟଥା
	ARG_UNUSED(msgq);
	ଫେରନ୍ତୁ false;
#endif /* CONFIG_POLL */
}

ଶୂନ୍ୟ k_msgq_init(ସଂରଚନା k_msgq *msgq, ଅକ୍ଷର *buffer, size_t ସନ୍ଦେଶ_size,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!size_mul_overflow(max_msgs, ସନ୍ଦେଶ_size, &(size_t){0}));
	__ASSERT_NO_MSG(!size_add_overflow((size_t)(uintptr_t)buffer, max_msgs * ସନ୍ଦେଶ_size,
					&(size_t){0}));

	msgq->ସନ୍ଦେଶ_size = ସନ୍ଦେଶ_size;
	msgq->max_msgs = max_msgs;
	msgq->buffer_start = buffer;
	msgq->buffer_end = buffer + (max_msgs * ସନ୍ଦେଶ_size);
	msgq->read_ptr = buffer;
	msgq->write_ptr = buffer;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (ସଂରଚନା k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_msgq_alloc_init(ସଂରଚନା k_msgq *msgq, size_t ସନ୍ଦେଶ_size,
			    uint32_t max_msgs)
{
	ଶୂନ୍ୟ *buffer;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	size_t total_size;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	ଯଦି (size_mul_overflow(ସନ୍ଦେଶ_size, max_msgs, &total_size)) {
		ret = -EINVAL;
	} ଅନ୍ୟଥା {
		buffer = z_thread_malloc(total_size);
		ଯଦି (buffer != ଖାଲି) {
			k_msgq_init(msgq, buffer, ସନ୍ଦେଶ_size, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} ଅନ୍ୟଥା {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_msgq_alloc_init(ସଂରଚନା k_msgq *msgq, size_t ସନ୍ଦେଶ_size,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	ଫେରନ୍ତୁ z_impl_k_msgq_alloc_init(msgq, ସନ୍ଦେଶ_size, max_msgs);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_msgq_cleanup(ସଂରଚନା k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	bool freed = false;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_head_locked(&msgq->wait_q) != ଖାଲି)) {
		ret = -EBUSY;
		ଯାଅ out;
	}

	CHECKIF(!locked && (z_waitq_head(&msgq->wait_q) != ଖାଲି)) {
		ret = -EBUSY;
		ଯାଅ out;
	}

	ଯଦି ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->buffer_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, ଚାବି);

#ifdef CONFIG_OBJ_CORE_MSGQ
	ଯଦି (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_msgq_cleanup(ସଂରଚନା k_msgq *msgq)
{
	ଫେରନ୍ତୁ z_msgq_cleanup(msgq, false);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା put_ସନ୍ଦେଶ_in_queue(ସଂରଚନା k_msgq *msgq, ଅଚଳ ଶୂନ୍ୟ *data,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ସଂରଚନା k_thread *pending_thread = ଖାଲି;
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	bool resched = false;

	ଚାବି = k_spin_lock(&msgq->lock);

	ଯଦି (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} ଅନ୍ୟଥା {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	ଯଦି (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			ଯଦି (pending_thread != ଖାଲି) {
				/* copy into the receiver's buffer */
				(ଶୂନ୍ୟ)memcpy(pending_thread->base.swap_data, data,
					     msgq->ସନ୍ଦେଶ_size);
				arch_thread_return_ମୂଲ୍ୟ_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ଯଦି (pending_thread == ଖାଲି) {
			size_t ସନ୍ଦେଶ_size = msgq->ସନ୍ଦେଶ_size;
			ଅକ୍ଷର *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
					(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
					((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
						msgq->ସନ୍ଦେଶ_size));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			ଯଦି (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				ଅକ୍ଷର *next;

				slot = msgq->write_ptr;
				next = slot + ସନ୍ଦେଶ_size;
				ଯଦି (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
			} ଅନ୍ୟଥା {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				ଯଦି (slot == msgq->buffer_start) {
					slot = msgq->buffer_end;
				}
				slot -= ସନ୍ଦେଶ_size;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(ଶୂନ୍ୟ)memcpy(slot, data, ସନ୍ଦେଶ_size);
			resched = msgq_handle_poll_events(msgq);
		}
		result = 0;
	} ଅନ୍ୟଥା ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		result = -ENOMSG;
	} ଅନ୍ୟଥା {
		ଯଦି (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} ଅନ୍ୟଥା {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_data = (ଶୂନ୍ୟ *) data;

		result = z_pend_curr(&msgq->lock, ଚାବି, &msgq->wait_q, timeout);

		ଯଦି (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
		} ଅନ୍ୟଥା {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
		}

		ଫେରନ୍ତୁ result;
	}

	ଯଦି (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, result);
	} ଅନ୍ୟଥା {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, result);
	}

	ଯଦି (resched) {
		z_reschedule(&msgq->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&msgq->lock, ଚାବି);
	}

	ଫେରନ୍ତୁ result;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_msgq_put(ସଂରଚନା k_msgq *msgq, ଅଚଳ ଶୂନ୍ୟ *data, k_timeout_t timeout)
{
	ଫେରନ୍ତୁ put_ସନ୍ଦେଶ_in_queue(msgq, data, timeout, true);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_msgq_put_front(ସଂରଚନା k_msgq *msgq, ଅଚଳ ଶୂନ୍ୟ *data)
{
	ଫେରନ୍ତୁ put_ସନ୍ଦେଶ_in_queue(msgq, data, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_msgq_put(ସଂରଚନା k_msgq *msgq, ଅଚଳ ଶୂନ୍ୟ *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->ସନ୍ଦେଶ_size));

	ଫେରନ୍ତୁ z_impl_k_msgq_put(msgq, data, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_put_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_msgq_put_front(ସଂରଚନା k_msgq *msgq, ଅଚଳ ଶୂନ୍ୟ *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, msgq->ସନ୍ଦେଶ_size));

	ଫେରନ୍ତୁ z_impl_k_msgq_put_front(msgq, data);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ z_impl_k_msgq_get_attrs(ସଂରଚନା k_msgq *msgq, ସଂରଚନା k_msgq_attrs *attrs)
{
	attrs->ସନ୍ଦେଶ_size = msgq->ସନ୍ଦେଶ_size;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_msgq_get_attrs(ସଂରଚନା k_msgq *msgq,
					   ସଂରଚନା k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, ଆକାର(ସଂରଚନା k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_msgq_get(ସଂରଚନା k_msgq *msgq, ଶୂନ୍ୟ *data, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_ଚାବି_t ଚାବି;
	ସଂରଚନା k_thread *pending_thread;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	bool resched = false;

	ଚାବି = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	ଯଦି (likely(msgq->used_msgs > 0U)) {
		size_t ସନ୍ଦେଶ_size = msgq->ସନ୍ଦେଶ_size;
		ଅକ୍ଷର *slot = msgq->read_ptr;
		ଅକ୍ଷର *next = slot + ସନ୍ଦେଶ_size;

		/* drop the message from the queue, then copy it out */
		ଯଦି (next == msgq->buffer_end) {
			next = msgq->buffer_start;
		}
		msgq->read_ptr = next;
		msgq->used_msgs--;

		(ଶୂନ୍ୟ)memcpy(data, slot, ସନ୍ଦେଶ_size);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->buffer_start) &&
				(msgq->write_ptr <= (msgq->buffer_end - 1)) &&
				((size_t)(uintptr_t)(msgq->buffer_end - msgq->write_ptr) >=
					msgq->ସନ୍ଦେଶ_size));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			ଯଦି (pending_thread != ଖାଲି) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				next = slot + ସନ୍ଦେଶ_size;
				ଯଦି (next == msgq->buffer_end) {
					next = msgq->buffer_start;
				}
				msgq->write_ptr = next;
				msgq->used_msgs++;

				(ଶୂନ୍ୟ)memcpy(slot, pending_thread->base.swap_data, ସନ୍ଦେଶ_size);

				arch_thread_return_ମୂଲ୍ୟ_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		result = 0;
	} ଅନ୍ୟଥା ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	} ଅନ୍ୟଥା {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_data = data;

		result = z_pend_curr(&msgq->lock, ଚାବି, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);
		ଫେରନ୍ତୁ result;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, result);

	ଯଦି (resched) {
		z_reschedule(&msgq->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&msgq->lock, ଚାବି);
	}

	ଫେରନ୍ତୁ result;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_msgq_get(ସଂରଚନା k_msgq *msgq, ଶୂନ୍ୟ *data,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->ସନ୍ଦେଶ_size));

	ଫେରନ୍ତୁ z_impl_k_msgq_get(msgq, data, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_msgq_peek(ସଂରଚନା k_msgq *msgq, ଶୂନ୍ୟ *data)
{
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	ଚାବି = k_spin_lock(&msgq->lock);

	ଯଦି (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(ଶୂନ୍ୟ)memcpy(data, msgq->read_ptr, msgq->ସନ୍ଦେଶ_size);
		result = 0;
	} ଅନ୍ୟଥା {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, ଚାବି);

	ଫେରନ୍ତୁ result;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_msgq_peek(ସଂରଚନା k_msgq *msgq, ଶୂନ୍ୟ *data)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->ସନ୍ଦେଶ_size));

	ଫେରନ୍ତୁ z_impl_k_msgq_peek(msgq, data);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_msgq_peek_at(ସଂରଚନା k_msgq *msgq, ଶୂନ୍ୟ *data, uint32_t idx)
{
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	ଅକ୍ଷର *start_addr;

	ଚାବି = k_spin_lock(&msgq->lock);

	ଯଦି (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->buffer_end - msgq->read_ptr);
		byte_offset = idx * msgq->ସନ୍ଦେଶ_size;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		ଯଦି (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->buffer_start;
		}
		(ଶୂନ୍ୟ)memcpy(data, start_addr + byte_offset, msgq->ସନ୍ଦେଶ_size);
		result = 0;
	} ଅନ୍ୟଥା {
		/* don't wait for a message to become available */
		result = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, result);

	k_spin_unlock(&msgq->lock, ଚାବି);

	ଫେରନ୍ତୁ result;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_msgq_peek_at(ସଂରଚନା k_msgq *msgq, ଶୂନ୍ୟ *data, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, msgq->ସନ୍ଦେଶ_size));

	ଫେରନ୍ତୁ z_impl_k_msgq_peek_at(msgq, data, idx);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ z_impl_k_msgq_purge(ସଂରଚନା k_msgq *msgq)
{
	k_spinlock_ଚାବି_t ଚାବି;
	bool resched = false;

	ଚାବି = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	ଯେତେବେଳେ (z_sched_wake(&msgq->wait_q, -ENOMSG, ଖାଲି)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	ଯଦି (resched) {
		z_reschedule(&msgq->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&msgq->lock, ଚାବି);
	}
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_msgq_purge(ସଂରଚନା k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_purge_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint32_t z_vrfy_k_msgq_ଅଙ୍କ_free_get(ସଂରଚନା k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ଫେରନ୍ତୁ z_impl_k_msgq_ଅଙ୍କ_free_get(msgq);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint32_t z_vrfy_k_msgq_ଅଙ୍କ_used_get(ସଂରଚନା k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ଫେରନ୍ତୁ z_impl_k_msgq_ଅଙ୍କ_used_get(msgq);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
