/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MSGQ */

ਸਥਿਰ ਸਤਰਅੰਦਰ bool msgq_handle_poll_events(ਬਣਤਰ k_msgq *msgq)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POLL
	ਵਾਪਸ z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_ਅੰਕੜੇ_ਉਪਲਬਧ);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(msgq);
	ਵਾਪਸ false;
#ਜੇ_ਅੰਤ /* CONFIG_POLL */
}

ਖਾਲੀ k_msgq_init(ਬਣਤਰ k_msgq *msgq, ਅੱਖਰ *ਬਫਰ, size_t ਸੁਨੇਹਾ_ਮਾਪ,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!ਮਾਪ_mul_ਉਛਾਲ(max_msgs, ਸੁਨੇਹਾ_ਮਾਪ, &(size_t){0}));
	__ASSERT_NO_MSG(!ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ((size_t)(uintptr_t)ਬਫਰ, max_msgs * ਸੁਨੇਹਾ_ਮਾਪ,
					&(size_t){0}));

	msgq->ਸੁਨੇਹਾ_ਮਾਪ = ਸੁਨੇਹਾ_ਮਾਪ;
	msgq->max_msgs = max_msgs;
	msgq->ਬਫਰ_ਸ਼ੁਰੂ = ਬਫਰ;
	msgq->ਬਫਰ_ਸਿਰਾ = ਬਫਰ + (max_msgs * ਸੁਨੇਹਾ_ਮਾਪ);
	msgq->read_ptr = ਬਫਰ;
	msgq->write_ptr = ਬਫਰ;
	msgq->ਵਰਤਿਆ_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->ਤਾਲਾ = (ਬਣਤਰ k_spinlock) {};
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#ਜੇ_ਅੰਤ	/* CONFIG_POLL */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MSGQ */

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

ਪੂਰਨ z_impl_k_msgq_alloc_init(ਬਣਤਰ k_msgq *msgq, size_t ਸੁਨੇਹਾ_ਮਾਪ,
			    uint32_t max_msgs)
{
	ਖਾਲੀ *ਬਫਰ;
	ਪੂਰਨ ret;
	size_t ਕੁੱਲ_ਮਾਪ;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	ਜੇ (ਮਾਪ_mul_ਉਛਾਲ(ਸੁਨੇਹਾ_ਮਾਪ, max_msgs, &ਕੁੱਲ_ਮਾਪ)) {
		ret = -EINVAL;
	} ਨਹੀਂਤਾਂ {
		ਬਫਰ = z_thread_malloc(ਕੁੱਲ_ਮਾਪ);
		ਜੇ (ਬਫਰ != ਨਿਰਾ) {
			k_msgq_init(msgq, ਬਫਰ, ਸੁਨੇਹਾ_ਮਾਪ, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} ਨਹੀਂਤਾਂ {
			ret = -ENOMEM;
		}
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_msgq_alloc_init(ਬਣਤਰ k_msgq *msgq, size_t ਸੁਨੇਹਾ_ਮਾਪ,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	ਵਾਪਸ z_impl_k_msgq_alloc_init(msgq, ਸੁਨੇਹਾ_ਮਾਪ, max_msgs);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_msgq_cleanup(ਬਣਤਰ k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	ਪੂਰਨ ret = 0;
	bool freed = false;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&msgq->ਤਾਲਾ);

	CHECKIF(locked && (z_waitq_ਸਿਰ_locked(&msgq->wait_q) != ਨਿਰਾ)) {
		ret = -EBUSY;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	CHECKIF(!locked && (z_waitq_ਸਿਰ(&msgq->wait_q) != ਨਿਰਾ)) {
		ret = -EBUSY;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->ਬਫਰ_ਸ਼ੁਰੂ);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

ਬਾਹਰਲਾ:
	k_spin_unlock(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MSGQ
	ਜੇ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MSGQ */

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	ਵਾਪਸ ret;
}

ਪੂਰਨ k_msgq_cleanup(ਬਣਤਰ k_msgq *msgq)
{
	ਵਾਪਸ z_msgq_cleanup(msgq, false);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ put_ਸੁਨੇਹਾ_ਵਿੱਚ_ਕਤਾਰ(ਬਣਤਰ k_msgq *msgq, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ,
			k_timeout_t timeout, bool put_at_ਪਿੱਠ)
{
	__ASSERT(!arch_is_ਵਿੱਚ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ਬਣਤਰ k_thread *pending_thread = ਨਿਰਾ;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ਨਤੀਜਾ;
	bool resched = false;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&msgq->ਤਾਲਾ);

	ਜੇ (put_at_ਪਿੱਠ) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} ਨਹੀਂਤਾਂ {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	ਜੇ (msgq->ਵਰਤਿਆ_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		ਤਾਲਾ_SCHED_SPINLOCK {
			pending_thread = z_unpend_ਪਹਿਲਾ_thread_locked(&msgq->wait_q);
			ਜੇ (pending_thread != ਨਿਰਾ) {
				/* copy into the receiver's buffer */
				(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(pending_thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ, ਅੰਕੜੇ,
					     msgq->ਸੁਨੇਹਾ_ਮਾਪ);
				arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ਜੇ (pending_thread == ਨਿਰਾ) {
			size_t ਸੁਨੇਹਾ_ਮਾਪ = msgq->ਸੁਨੇਹਾ_ਮਾਪ;
			ਅੱਖਰ *ਥਾਂ;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->ਬਫਰ_ਸ਼ੁਰੂ) &&
					(msgq->write_ptr <= (msgq->ਬਫਰ_ਸਿਰਾ - 1)) &&
					((size_t)(uintptr_t)(msgq->ਬਫਰ_ਸਿਰਾ - msgq->write_ptr) >=
						msgq->ਸੁਨੇਹਾ_ਮਾਪ));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			ਜੇ (put_at_ਪਿੱਠ) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				ਅੱਖਰ *ਅਗਲਾ;

				ਥਾਂ = msgq->write_ptr;
				ਅਗਲਾ = ਥਾਂ + ਸੁਨੇਹਾ_ਮਾਪ;
				ਜੇ (ਅਗਲਾ == msgq->ਬਫਰ_ਸਿਰਾ) {
					ਅਗਲਾ = msgq->ਬਫਰ_ਸ਼ੁਰੂ;
				}
				msgq->write_ptr = ਅਗਲਾ;
			} ਨਹੀਂਤਾਂ {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				ਥਾਂ = msgq->read_ptr;
				ਜੇ (ਥਾਂ == msgq->ਬਫਰ_ਸ਼ੁਰੂ) {
					ਥਾਂ = msgq->ਬਫਰ_ਸਿਰਾ;
				}
				ਥਾਂ -= ਸੁਨੇਹਾ_ਮਾਪ;
				msgq->read_ptr = ਥਾਂ;
			}
			msgq->ਵਰਤਿਆ_msgs++;
			(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਥਾਂ, ਅੰਕੜੇ, ਸੁਨੇਹਾ_ਮਾਪ);
			resched = msgq_handle_poll_events(msgq);
		}
		ਨਤੀਜਾ = 0;
	} ਨਹੀਂਤਾਂ ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		ਨਤੀਜਾ = -ENOMSG;
	} ਨਹੀਂਤਾਂ {
		ਜੇ (put_at_ਪਿੱਠ) {
			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} ਨਹੀਂਤਾਂ {
			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ = (ਖਾਲੀ *) ਅੰਕੜੇ;

		ਨਤੀਜਾ = z_pend_curr(&msgq->ਤਾਲਾ, ਕੁੰਜੀ, &msgq->wait_q, timeout);

		ਜੇ (put_at_ਪਿੱਠ) {
			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ਨਤੀਜਾ);
		} ਨਹੀਂਤਾਂ {
			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ਨਤੀਜਾ);
		}

		ਵਾਪਸ ਨਤੀਜਾ;
	}

	ਜੇ (put_at_ਪਿੱਠ) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ਨਤੀਜਾ);
	} ਨਹੀਂਤਾਂ {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ਨਤੀਜਾ);
	}

	ਜੇ (resched) {
		z_reschedule(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);
	}

	ਵਾਪਸ ਨਤੀਜਾ;
}


ਪੂਰਨ z_impl_k_msgq_put(ਬਣਤਰ k_msgq *msgq, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ, k_timeout_t timeout)
{
	ਵਾਪਸ put_ਸੁਨੇਹਾ_ਵਿੱਚ_ਕਤਾਰ(msgq, ਅੰਕੜੇ, timeout, true);
}

ਪੂਰਨ z_impl_k_msgq_put_front(ਬਣਤਰ k_msgq *msgq, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਵਾਪਸ put_ਸੁਨੇਹਾ_ਵਿੱਚ_ਕਤਾਰ(msgq, ਅੰਕੜੇ, K_NO_WAIT, false);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_msgq_put(ਬਣਤਰ k_msgq *msgq, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_READ(ਅੰਕੜੇ, msgq->ਸੁਨੇਹਾ_ਮਾਪ));

	ਵਾਪਸ z_impl_k_msgq_put(msgq, ਅੰਕੜੇ, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_put_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_msgq_put_front(ਬਣਤਰ k_msgq *msgq, ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_READ(ਅੰਕੜੇ, msgq->ਸੁਨੇਹਾ_ਮਾਪ));

	ਵਾਪਸ z_impl_k_msgq_put_front(msgq, ਅੰਕੜੇ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_impl_k_msgq_get_attrs(ਬਣਤਰ k_msgq *msgq, ਬਣਤਰ k_msgq_attrs *attrs)
{
	attrs->ਸੁਨੇਹਾ_ਮਾਪ = msgq->ਸੁਨੇਹਾ_ਮਾਪ;
	attrs->max_msgs = msgq->max_msgs;
	attrs->ਵਰਤਿਆ_msgs = msgq->ਵਰਤਿਆ_msgs;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_msgq_get_attrs(ਬਣਤਰ k_msgq *msgq,
					   ਬਣਤਰ k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(attrs, ਆਕਾਰ(ਬਣਤਰ k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_msgq_get(ਬਣਤਰ k_msgq *msgq, ਖਾਲੀ *ਅੰਕੜੇ, k_timeout_t timeout)
{
	__ASSERT(!arch_is_ਵਿੱਚ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਬਣਤਰ k_thread *pending_thread;
	ਪੂਰਨ ਨਤੀਜਾ;
	bool resched = false;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&msgq->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	ਜੇ (likely(msgq->ਵਰਤਿਆ_msgs > 0U)) {
		size_t ਸੁਨੇਹਾ_ਮਾਪ = msgq->ਸੁਨੇਹਾ_ਮਾਪ;
		ਅੱਖਰ *ਥਾਂ = msgq->read_ptr;
		ਅੱਖਰ *ਅਗਲਾ = ਥਾਂ + ਸੁਨੇਹਾ_ਮਾਪ;

		/* drop the message from the queue, then copy it out */
		ਜੇ (ਅਗਲਾ == msgq->ਬਫਰ_ਸਿਰਾ) {
			ਅਗਲਾ = msgq->ਬਫਰ_ਸ਼ੁਰੂ;
		}
		msgq->read_ptr = ਅਗਲਾ;
		msgq->ਵਰਤਿਆ_msgs--;

		(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, ਥਾਂ, ਸੁਨੇਹਾ_ਮਾਪ);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->ਬਫਰ_ਸ਼ੁਰੂ) &&
				(msgq->write_ptr <= (msgq->ਬਫਰ_ਸਿਰਾ - 1)) &&
				((size_t)(uintptr_t)(msgq->ਬਫਰ_ਸਿਰਾ - msgq->write_ptr) >=
					msgq->ਸੁਨੇਹਾ_ਮਾਪ));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		ਤਾਲਾ_SCHED_SPINLOCK {
			pending_thread = z_unpend_ਪਹਿਲਾ_thread_locked(&msgq->wait_q);
			ਜੇ (pending_thread != ਨਿਰਾ) {
				SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				ਥਾਂ = msgq->write_ptr;
				ਅਗਲਾ = ਥਾਂ + ਸੁਨੇਹਾ_ਮਾਪ;
				ਜੇ (ਅਗਲਾ == msgq->ਬਫਰ_ਸਿਰਾ) {
					ਅਗਲਾ = msgq->ਬਫਰ_ਸ਼ੁਰੂ;
				}
				msgq->write_ptr = ਅਗਲਾ;
				msgq->ਵਰਤਿਆ_msgs++;

				(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਥਾਂ, pending_thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ, ਸੁਨੇਹਾ_ਮਾਪ);

				arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ਨਤੀਜਾ = 0;
	} ਨਹੀਂਤਾਂ ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		ਨਤੀਜਾ = -ENOMSG;
	} ਨਹੀਂਤਾਂ {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ = ਅੰਕੜੇ;

		ਨਤੀਜਾ = z_pend_curr(&msgq->ਤਾਲਾ, ਕੁੰਜੀ, &msgq->wait_q, timeout);
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ਨਤੀਜਾ);
		ਵਾਪਸ ਨਤੀਜਾ;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ਨਤੀਜਾ);

	ਜੇ (resched) {
		z_reschedule(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);
	}

	ਵਾਪਸ ਨਤੀਜਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_msgq_get(ਬਣਤਰ k_msgq *msgq, ਖਾਲੀ *ਅੰਕੜੇ,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(ਅੰਕੜੇ, msgq->ਸੁਨੇਹਾ_ਮਾਪ));

	ਵਾਪਸ z_impl_k_msgq_get(msgq, ਅੰਕੜੇ, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_msgq_peek(ਬਣਤਰ k_msgq *msgq, ਖਾਲੀ *ਅੰਕੜੇ)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ਨਤੀਜਾ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&msgq->ਤਾਲਾ);

	ਜੇ (msgq->ਵਰਤਿਆ_msgs > 0U) {
		/* take first available message from queue */
		(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, msgq->read_ptr, msgq->ਸੁਨੇਹਾ_ਮਾਪ);
		ਨਤੀਜਾ = 0;
	} ਨਹੀਂਤਾਂ {
		/* don't wait for a message to become available */
		ਨਤੀਜਾ = -ENOMSG;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ਨਤੀਜਾ);

	k_spin_unlock(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ਨਤੀਜਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_msgq_peek(ਬਣਤਰ k_msgq *msgq, ਖਾਲੀ *ਅੰਕੜੇ)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(ਅੰਕੜੇ, msgq->ਸੁਨੇਹਾ_ਮਾਪ));

	ਵਾਪਸ z_impl_k_msgq_peek(msgq, ਅੰਕੜੇ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_peek_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_msgq_peek_at(ਬਣਤਰ k_msgq *msgq, ਖਾਲੀ *ਅੰਕੜੇ, uint32_t idx)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ਨਤੀਜਾ;
	uint32_t ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_ਸਿਰਾ;
	uint32_t byte_ਵਿੱਥ;
	ਅੱਖਰ *ਸ਼ੁਰੂ_addr;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&msgq->ਤਾਲਾ);

	ਜੇ (msgq->ਵਰਤਿਆ_msgs > idx) {
		ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_ਸਿਰਾ = (msgq->ਬਫਰ_ਸਿਰਾ - msgq->read_ptr);
		byte_ਵਿੱਥ = idx * msgq->ਸੁਨੇਹਾ_ਮਾਪ;
		ਸ਼ੁਰੂ_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		ਜੇ (ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_ਸਿਰਾ <= byte_ਵਿੱਥ) {
			/* Tweak the values in case */
			byte_ਵਿੱਥ -= ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_ਸਿਰਾ;
			/* wrap-around is required */
			ਸ਼ੁਰੂ_addr = msgq->ਬਫਰ_ਸ਼ੁਰੂ;
		}
		(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, ਸ਼ੁਰੂ_addr + byte_ਵਿੱਥ, msgq->ਸੁਨੇਹਾ_ਮਾਪ);
		ਨਤੀਜਾ = 0;
	} ਨਹੀਂਤਾਂ {
		/* don't wait for a message to become available */
		ਨਤੀਜਾ = -ENOMSG;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ਨਤੀਜਾ);

	k_spin_unlock(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ਨਤੀਜਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_msgq_peek_at(ਬਣਤਰ k_msgq *msgq, ਖਾਲੀ *ਅੰਕੜੇ, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(ਅੰਕੜੇ, msgq->ਸੁਨੇਹਾ_ਮਾਪ));

	ਵਾਪਸ z_impl_k_msgq_peek_at(msgq, ਅੰਕੜੇ, idx);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_impl_k_msgq_purge(ਬਣਤਰ k_msgq *msgq)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	bool resched = false;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&msgq->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	ਜਦੋਂ (z_sched_wake(&msgq->wait_q, -ENOMSG, ਨਿਰਾ)) {
		resched = true;
	}

	msgq->ਵਰਤਿਆ_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	ਜੇ (resched) {
		z_reschedule(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&msgq->ਤਾਲਾ, ਕੁੰਜੀ);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_msgq_purge(ਬਣਤਰ k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_purge_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t z_vrfy_k_msgq_ਅੰਕ_free_get(ਬਣਤਰ k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ਵਾਪਸ z_impl_k_msgq_ਅੰਕ_free_get(msgq);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t z_vrfy_k_msgq_ਅੰਕ_ਵਰਤਿਆ_get(ਬਣਤਰ k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ਵਾਪਸ z_impl_k_msgq_ਅੰਕ_ਵਰਤਿਆ_get(msgq);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
