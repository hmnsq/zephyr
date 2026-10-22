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
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, ফাঁকা);
#endif /* CONFIG_OBJ_CORE_MSGQ */

স্থির অন্তঃরেখ bool msgq_handle_poll_events(কাঠামো k_msgq *msgq)
{
#ifdef CONFIG_POLL
	ফেরত z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_তথ্য_AVAILABLE);
#অন্যথা
	ARG_UNUSED(msgq);
	ফেরত false;
#endif /* CONFIG_POLL */
}

শূন্য k_msgq_init(কাঠামো k_msgq *msgq, অক্ষর *বাফার, size_t বার্তা_আকার,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!আকার_mul_overflow(max_msgs, বার্তা_আকার, &(size_t){0}));
	__ASSERT_NO_MSG(!আকার_add_overflow((size_t)(uintptr_t)বাফার, max_msgs * বার্তা_আকার,
					&(size_t){0}));

	msgq->বার্তা_আকার = বার্তা_আকার;
	msgq->max_msgs = max_msgs;
	msgq->বাফার_start = বাফার;
	msgq->বাফার_end = বাফার + (max_msgs * বার্তা_আকার);
	msgq->read_ptr = বাফার;
	msgq->write_ptr = বাফার;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (কাঠামো k_spinlock) {};
#ifdef CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#endif	/* CONFIG_POLL */

#ifdef CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#endif /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

পূর্ণসংখ্যা z_impl_k_msgq_alloc_init(কাঠামো k_msgq *msgq, size_t বার্তা_আকার,
			    uint32_t max_msgs)
{
	শূন্য *বাফার;
	পূর্ণসংখ্যা ret;
	size_t যোগ_আকার;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	যদি (আকার_mul_overflow(বার্তা_আকার, max_msgs, &যোগ_আকার)) {
		ret = -EINVAL;
	} অন্যথা {
		বাফার = z_thread_malloc(যোগ_আকার);
		যদি (বাফার != ফাঁকা) {
			k_msgq_init(msgq, বাফার, বার্তা_আকার, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} অন্যথা {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	ফেরত ret;
}

#ifdef CONFIG_USERSPACE
পূর্ণসংখ্যা z_vrfy_k_msgq_alloc_init(কাঠামো k_msgq *msgq, size_t বার্তা_আকার,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	ফেরত z_impl_k_msgq_alloc_init(msgq, বার্তা_আকার, max_msgs);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_msgq_cleanup(কাঠামো k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	পূর্ণসংখ্যা ret = 0;
	bool freed = false;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_শীর্ষ_locked(&msgq->wait_q) != ফাঁকা)) {
		ret = -EBUSY;
		যাও out;
	}

	CHECKIF(!locked && (z_waitq_শীর্ষ(&msgq->wait_q) != ফাঁকা)) {
		ret = -EBUSY;
		যাও out;
	}

	যদি ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->বাফার_start);
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
	ফেরত ret;
}

পূর্ণসংখ্যা k_msgq_cleanup(কাঠামো k_msgq *msgq)
{
	ফেরত z_msgq_cleanup(msgq, false);
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা put_বার্তা_in_queue(কাঠামো k_msgq *msgq, ধ্রুবক শূন্য *তথ্য,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	কাঠামো k_thread *pending_thread = ফাঁকা;
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ফলাফল;
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
			যদি (pending_thread != ফাঁকা) {
				/* copy into the receiver's buffer */
				(শূন্য)memcpy(pending_thread->base.swap_তথ্য, তথ্য,
					     msgq->বার্তা_আকার);
				arch_thread_return_মান_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		যদি (pending_thread == ফাঁকা) {
			size_t বার্তা_আকার = msgq->বার্তা_আকার;
			অক্ষর *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->বাফার_start) &&
					(msgq->write_ptr <= (msgq->বাফার_end - 1)) &&
					((size_t)(uintptr_t)(msgq->বাফার_end - msgq->write_ptr) >=
						msgq->বার্তা_আকার));
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
				অক্ষর *পরবর্তী;

				slot = msgq->write_ptr;
				পরবর্তী = slot + বার্তা_আকার;
				যদি (পরবর্তী == msgq->বাফার_end) {
					পরবর্তী = msgq->বাফার_start;
				}
				msgq->write_ptr = পরবর্তী;
			} অন্যথা {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				যদি (slot == msgq->বাফার_start) {
					slot = msgq->বাফার_end;
				}
				slot -= বার্তা_আকার;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(শূন্য)memcpy(slot, তথ্য, বার্তা_আকার);
			resched = msgq_handle_poll_events(msgq);
		}
		ফলাফল = 0;
	} অন্যথা যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		ফলাফল = -ENOMSG;
	} অন্যথা {
		যদি (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} অন্যথা {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_তথ্য = (শূন্য *) তথ্য;

		ফলাফল = z_pend_curr(&msgq->lock, চাবি, &msgq->wait_q, timeout);

		যদি (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ফলাফল);
		} অন্যথা {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ফলাফল);
		}

		ফেরত ফলাফল;
	}

	যদি (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ফলাফল);
	} অন্যথা {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ফলাফল);
	}

	যদি (resched) {
		z_reschedule(&msgq->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&msgq->lock, চাবি);
	}

	ফেরত ফলাফল;
}


পূর্ণসংখ্যা z_impl_k_msgq_put(কাঠামো k_msgq *msgq, ধ্রুবক শূন্য *তথ্য, k_timeout_t timeout)
{
	ফেরত put_বার্তা_in_queue(msgq, তথ্য, timeout, true);
}

পূর্ণসংখ্যা z_impl_k_msgq_put_front(কাঠামো k_msgq *msgq, ধ্রুবক শূন্য *তথ্য)
{
	ফেরত put_বার্তা_in_queue(msgq, তথ্য, K_NO_WAIT, false);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_msgq_put(কাঠামো k_msgq *msgq, ধ্রুবক শূন্য *তথ্য,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(তথ্য, msgq->বার্তা_আকার));

	ফেরত z_impl_k_msgq_put(msgq, তথ্য, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_put_mrsh.c>

স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_msgq_put_front(কাঠামো k_msgq *msgq, ধ্রুবক শূন্য *তথ্য)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(তথ্য, msgq->বার্তা_আকার));

	ফেরত z_impl_k_msgq_put_front(msgq, তথ্য);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_msgq_get_attrs(কাঠামো k_msgq *msgq, কাঠামো k_msgq_attrs *attrs)
{
	attrs->বার্তা_আকার = msgq->বার্তা_আকার;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ শূন্য z_vrfy_k_msgq_get_attrs(কাঠামো k_msgq *msgq,
					   কাঠামো k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, আকার(কাঠামো k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_msgq_get(কাঠামো k_msgq *msgq, শূন্য *তথ্য, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_চাবি_t চাবি;
	কাঠামো k_thread *pending_thread;
	পূর্ণসংখ্যা ফলাফল;
	bool resched = false;

	চাবি = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	যদি (likely(msgq->used_msgs > 0U)) {
		size_t বার্তা_আকার = msgq->বার্তা_আকার;
		অক্ষর *slot = msgq->read_ptr;
		অক্ষর *পরবর্তী = slot + বার্তা_আকার;

		/* drop the message from the queue, then copy it out */
		যদি (পরবর্তী == msgq->বাফার_end) {
			পরবর্তী = msgq->বাফার_start;
		}
		msgq->read_ptr = পরবর্তী;
		msgq->used_msgs--;

		(শূন্য)memcpy(তথ্য, slot, বার্তা_আকার);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->বাফার_start) &&
				(msgq->write_ptr <= (msgq->বাফার_end - 1)) &&
				((size_t)(uintptr_t)(msgq->বাফার_end - msgq->write_ptr) >=
					msgq->বার্তা_আকার));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			যদি (pending_thread != ফাঁকা) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				পরবর্তী = slot + বার্তা_আকার;
				যদি (পরবর্তী == msgq->বাফার_end) {
					পরবর্তী = msgq->বাফার_start;
				}
				msgq->write_ptr = পরবর্তী;
				msgq->used_msgs++;

				(শূন্য)memcpy(slot, pending_thread->base.swap_তথ্য, বার্তা_আকার);

				arch_thread_return_মান_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ফলাফল = 0;
	} অন্যথা যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		ফলাফল = -ENOMSG;
	} অন্যথা {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_তথ্য = তথ্য;

		ফলাফল = z_pend_curr(&msgq->lock, চাবি, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ফলাফল);
		ফেরত ফলাফল;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ফলাফল);

	যদি (resched) {
		z_reschedule(&msgq->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&msgq->lock, চাবি);
	}

	ফেরত ফলাফল;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_msgq_get(কাঠামো k_msgq *msgq, শূন্য *তথ্য,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(তথ্য, msgq->বার্তা_আকার));

	ফেরত z_impl_k_msgq_get(msgq, তথ্য, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_msgq_peek(কাঠামো k_msgq *msgq, শূন্য *তথ্য)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ফলাফল;

	চাবি = k_spin_lock(&msgq->lock);

	যদি (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(শূন্য)memcpy(তথ্য, msgq->read_ptr, msgq->বার্তা_আকার);
		ফলাফল = 0;
	} অন্যথা {
		/* don't wait for a message to become available */
		ফলাফল = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ফলাফল);

	k_spin_unlock(&msgq->lock, চাবি);

	ফেরত ফলাফল;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_msgq_peek(কাঠামো k_msgq *msgq, শূন্য *তথ্য)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(তথ্য, msgq->বার্তা_আকার));

	ফেরত z_impl_k_msgq_peek(msgq, তথ্য);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_peek_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_msgq_peek_at(কাঠামো k_msgq *msgq, শূন্য *তথ্য, uint32_t idx)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণসংখ্যা ফলাফল;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	অক্ষর *start_addr;

	চাবি = k_spin_lock(&msgq->lock);

	যদি (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->বাফার_end - msgq->read_ptr);
		byte_offset = idx * msgq->বার্তা_আকার;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		যদি (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->বাফার_start;
		}
		(শূন্য)memcpy(তথ্য, start_addr + byte_offset, msgq->বার্তা_আকার);
		ফলাফল = 0;
	} অন্যথা {
		/* don't wait for a message to become available */
		ফলাফল = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ফলাফল);

	k_spin_unlock(&msgq->lock, চাবি);

	ফেরত ফলাফল;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_msgq_peek_at(কাঠামো k_msgq *msgq, শূন্য *তথ্য, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(তথ্য, msgq->বার্তা_আকার));

	ফেরত z_impl_k_msgq_peek_at(msgq, তথ্য, idx);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_msgq_purge(কাঠামো k_msgq *msgq)
{
	k_spinlock_চাবি_t চাবি;
	bool resched = false;

	চাবি = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	যখন (z_sched_wake(&msgq->wait_q, -ENOMSG, ফাঁকা)) {
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
স্থির অন্তঃরেখ শূন্য z_vrfy_k_msgq_purge(কাঠামো k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_purge_mrsh.c>

স্থির অন্তঃরেখ uint32_t z_vrfy_k_msgq_অঙ্ক_free_get(কাঠামো k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ফেরত z_impl_k_msgq_অঙ্ক_free_get(msgq);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

স্থির অন্তঃরেখ uint32_t z_vrfy_k_msgq_অঙ্ক_used_get(কাঠামো k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ফেরত z_impl_k_msgq_অঙ্ক_used_get(msgq);
}
#অন্তর্ভাব <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
