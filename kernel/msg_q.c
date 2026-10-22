/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/sections.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MSGQ */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool msgq_handle_poll_events(ರಚನೆ k_msgq *msgq)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POLL
	ಹಿಂತಿರುಗಿ z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_ದತ್ತಾಂಶ_ಲಭ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(msgq);
	ಹಿಂತಿರುಗಿ false;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POLL */
}

ಶೂನ್ಯ k_msgq_init(ರಚನೆ k_msgq *msgq, ಅಕ್ಷರ *ಬಫರ್, size_t ಸಂದೇಶ_ಗಾತ್ರ,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!ಗಾತ್ರ_mul_ಅತಿಪ್ರವಾಹ(max_msgs, ಸಂದೇಶ_ಗಾತ್ರ, &(size_t){0}));
	__ASSERT_NO_MSG(!ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ((size_t)(uintptr_t)ಬಫರ್, max_msgs * ಸಂದೇಶ_ಗಾತ್ರ,
					&(size_t){0}));

	msgq->ಸಂದೇಶ_ಗಾತ್ರ = ಸಂದೇಶ_ಗಾತ್ರ;
	msgq->max_msgs = max_msgs;
	msgq->ಬಫರ್_ಆರಂಭ = ಬಫರ್;
	msgq->ಬಫರ್_ಕೊನೆ = ಬಫರ್ + (max_msgs * ಸಂದೇಶ_ಗಾತ್ರ);
	msgq->read_ptr = ಬಫರ್;
	msgq->write_ptr = ಬಫರ್;
	msgq->ಬಳಸಿದ_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->ಬೀಗ = (ರಚನೆ k_spinlock) {};
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#ಆದರೆ_ಅಂತ್ಯ	/* CONFIG_POLL */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MSGQ */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

ಪೂರ್ಣಾಂಕ z_impl_k_msgq_alloc_init(ರಚನೆ k_msgq *msgq, size_t ಸಂದೇಶ_ಗಾತ್ರ,
			    uint32_t max_msgs)
{
	ಶೂನ್ಯ *ಬಫರ್;
	ಪೂರ್ಣಾಂಕ ret;
	size_t ಒಟ್ಟು_ಗಾತ್ರ;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	ಆದರೆ (ಗಾತ್ರ_mul_ಅತಿಪ್ರವಾಹ(ಸಂದೇಶ_ಗಾತ್ರ, max_msgs, &ಒಟ್ಟು_ಗಾತ್ರ)) {
		ret = -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಬಫರ್ = z_thread_malloc(ಒಟ್ಟು_ಗಾತ್ರ);
		ಆದರೆ (ಬಫರ್ != ಖಾಲಿ) {
			k_msgq_init(msgq, ಬಫರ್, ಸಂದೇಶ_ಗಾತ್ರ, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ret = -ENOMEM;
		}
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_msgq_alloc_init(ರಚನೆ k_msgq *msgq, size_t ಸಂದೇಶ_ಗಾತ್ರ,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	ಹಿಂತಿರುಗಿ z_impl_k_msgq_alloc_init(msgq, ಸಂದೇಶ_ಗಾತ್ರ, max_msgs);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_msgq_cleanup(ರಚನೆ k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	ಪೂರ್ಣಾಂಕ ret = 0;
	bool freed = false;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&msgq->ಬೀಗ);

	CHECKIF(locked && (z_waitq_ತಲೆ_locked(&msgq->wait_q) != ಖಾಲಿ)) {
		ret = -EBUSY;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	CHECKIF(!locked && (z_waitq_ತಲೆ(&msgq->wait_q) != ಖಾಲಿ)) {
		ret = -EBUSY;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->ಬಫರ್_ಆರಂಭ);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

ಹೊರಹೋಗು:
	k_spin_unlock(&msgq->ಬೀಗ, ಕೀಲಿ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MSGQ
	ಆದರೆ (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MSGQ */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_msgq_cleanup(ರಚನೆ k_msgq *msgq)
{
	ಹಿಂತಿರುಗಿ z_msgq_cleanup(msgq, false);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ put_ಸಂದೇಶ_ಒಳ_ಸರತಿ(ರಚನೆ k_msgq *msgq, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ,
			k_timeout_t timeout, bool put_at_ಹಿಂಭಾಗ)
{
	__ASSERT(!arch_is_ಒಳ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ರಚನೆ k_thread *pending_thread = ಖಾಲಿ;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	bool resched = false;

	ಕೀಲಿ = k_spin_ಬೀಗ(&msgq->ಬೀಗ);

	ಆದರೆ (put_at_ಹಿಂಭಾಗ) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} ಇಲ್ಲದಿದ್ದರೆ {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	ಆದರೆ (msgq->ಬಳಸಿದ_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		ಬೀಗ_SCHED_SPINLOCK {
			pending_thread = z_unpend_ಮೊದಲ_thread_locked(&msgq->wait_q);
			ಆದರೆ (pending_thread != ಖಾಲಿ) {
				/* copy into the receiver's buffer */
				(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(pending_thread->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ, ದತ್ತಾಂಶ,
					     msgq->ಸಂದೇಶ_ಗಾತ್ರ);
				arch_thread_return_ಮೌಲ್ಯ_ಗಣ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ಆದರೆ (pending_thread == ಖಾಲಿ) {
			size_t ಸಂದೇಶ_ಗಾತ್ರ = msgq->ಸಂದೇಶ_ಗಾತ್ರ;
			ಅಕ್ಷರ *ಸ್ಥಾನ;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->ಬಫರ್_ಆರಂಭ) &&
					(msgq->write_ptr <= (msgq->ಬಫರ್_ಕೊನೆ - 1)) &&
					((size_t)(uintptr_t)(msgq->ಬಫರ್_ಕೊನೆ - msgq->write_ptr) >=
						msgq->ಸಂದೇಶ_ಗಾತ್ರ));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			ಆದರೆ (put_at_ಹಿಂಭಾಗ) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				ಅಕ್ಷರ *ಮುಂದಿನ;

				ಸ್ಥಾನ = msgq->write_ptr;
				ಮುಂದಿನ = ಸ್ಥಾನ + ಸಂದೇಶ_ಗಾತ್ರ;
				ಆದರೆ (ಮುಂದಿನ == msgq->ಬಫರ್_ಕೊನೆ) {
					ಮುಂದಿನ = msgq->ಬಫರ್_ಆರಂಭ;
				}
				msgq->write_ptr = ಮುಂದಿನ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				ಸ್ಥಾನ = msgq->read_ptr;
				ಆದರೆ (ಸ್ಥಾನ == msgq->ಬಫರ್_ಆರಂಭ) {
					ಸ್ಥಾನ = msgq->ಬಫರ್_ಕೊನೆ;
				}
				ಸ್ಥಾನ -= ಸಂದೇಶ_ಗಾತ್ರ;
				msgq->read_ptr = ಸ್ಥಾನ;
			}
			msgq->ಬಳಸಿದ_msgs++;
			(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(ಸ್ಥಾನ, ದತ್ತಾಂಶ, ಸಂದೇಶ_ಗಾತ್ರ);
			resched = msgq_handle_poll_events(msgq);
		}
		ಫಲಿತಾಂಶ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		ಫಲಿತಾಂಶ = -ENOMSG;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (put_at_ಹಿಂಭಾಗ) {
			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} ಇಲ್ಲದಿದ್ದರೆ {
			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ = (ಶೂನ್ಯ *) ದತ್ತಾಂಶ;

		ಫಲಿತಾಂಶ = z_pend_curr(&msgq->ಬೀಗ, ಕೀಲಿ, &msgq->wait_q, timeout);

		ಆದರೆ (put_at_ಹಿಂಭಾಗ) {
			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ಫಲಿತಾಂಶ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ಫಲಿತಾಂಶ);
		}

		ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
	}

	ಆದರೆ (put_at_ಹಿಂಭಾಗ) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, ಫಲಿತಾಂಶ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, ಫಲಿತಾಂಶ);
	}

	ಆದರೆ (resched) {
		z_reschedule(&msgq->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&msgq->ಬೀಗ, ಕೀಲಿ);
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}


ಪೂರ್ಣಾಂಕ z_impl_k_msgq_put(ರಚನೆ k_msgq *msgq, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ, k_timeout_t timeout)
{
	ಹಿಂತಿರುಗಿ put_ಸಂದೇಶ_ಒಳ_ಸರತಿ(msgq, ದತ್ತಾಂಶ, timeout, true);
}

ಪೂರ್ಣಾಂಕ z_impl_k_msgq_put_front(ರಚನೆ k_msgq *msgq, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಹಿಂತಿರುಗಿ put_ಸಂದೇಶ_ಒಳ_ಸರತಿ(msgq, ದತ್ತಾಂಶ, K_NO_WAIT, false);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_msgq_put(ರಚನೆ k_msgq *msgq, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_READ(ದತ್ತಾಂಶ, msgq->ಸಂದೇಶ_ಗಾತ್ರ));

	ಹಿಂತಿರುಗಿ z_impl_k_msgq_put(msgq, ದತ್ತಾಂಶ, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_put_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_msgq_put_front(ರಚನೆ k_msgq *msgq, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_READ(ದತ್ತಾಂಶ, msgq->ಸಂದೇಶ_ಗಾತ್ರ));

	ಹಿಂತಿರುಗಿ z_impl_k_msgq_put_front(msgq, ದತ್ತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_msgq_get_attrs(ರಚನೆ k_msgq *msgq, ರಚನೆ k_msgq_attrs *attrs)
{
	attrs->ಸಂದೇಶ_ಗಾತ್ರ = msgq->ಸಂದೇಶ_ಗಾತ್ರ;
	attrs->max_msgs = msgq->max_msgs;
	attrs->ಬಳಸಿದ_msgs = msgq->ಬಳಸಿದ_msgs;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_msgq_get_attrs(ರಚನೆ k_msgq *msgq,
					   ರಚನೆ k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(attrs, ಗಾತ್ರ(ರಚನೆ k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_msgq_get(ರಚನೆ k_msgq *msgq, ಶೂನ್ಯ *ದತ್ತಾಂಶ, k_timeout_t timeout)
{
	__ASSERT(!arch_is_ಒಳ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ರಚನೆ k_thread *pending_thread;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	bool resched = false;

	ಕೀಲಿ = k_spin_ಬೀಗ(&msgq->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	ಆದರೆ (likely(msgq->ಬಳಸಿದ_msgs > 0U)) {
		size_t ಸಂದೇಶ_ಗಾತ್ರ = msgq->ಸಂದೇಶ_ಗಾತ್ರ;
		ಅಕ್ಷರ *ಸ್ಥಾನ = msgq->read_ptr;
		ಅಕ್ಷರ *ಮುಂದಿನ = ಸ್ಥಾನ + ಸಂದೇಶ_ಗಾತ್ರ;

		/* drop the message from the queue, then copy it out */
		ಆದರೆ (ಮುಂದಿನ == msgq->ಬಫರ್_ಕೊನೆ) {
			ಮುಂದಿನ = msgq->ಬಫರ್_ಆರಂಭ;
		}
		msgq->read_ptr = ಮುಂದಿನ;
		msgq->ಬಳಸಿದ_msgs--;

		(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, ಸ್ಥಾನ, ಸಂದೇಶ_ಗಾತ್ರ);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->ಬಫರ್_ಆರಂಭ) &&
				(msgq->write_ptr <= (msgq->ಬಫರ್_ಕೊನೆ - 1)) &&
				((size_t)(uintptr_t)(msgq->ಬಫರ್_ಕೊನೆ - msgq->write_ptr) >=
					msgq->ಸಂದೇಶ_ಗಾತ್ರ));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		ಬೀಗ_SCHED_SPINLOCK {
			pending_thread = z_unpend_ಮೊದಲ_thread_locked(&msgq->wait_q);
			ಆದರೆ (pending_thread != ಖಾಲಿ) {
				SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				ಸ್ಥಾನ = msgq->write_ptr;
				ಮುಂದಿನ = ಸ್ಥಾನ + ಸಂದೇಶ_ಗಾತ್ರ;
				ಆದರೆ (ಮುಂದಿನ == msgq->ಬಫರ್_ಕೊನೆ) {
					ಮುಂದಿನ = msgq->ಬಫರ್_ಆರಂಭ;
				}
				msgq->write_ptr = ಮುಂದಿನ;
				msgq->ಬಳಸಿದ_msgs++;

				(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(ಸ್ಥಾನ, pending_thread->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ, ಸಂದೇಶ_ಗಾತ್ರ);

				arch_thread_return_ಮೌಲ್ಯ_ಗಣ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		ಫಲಿತಾಂಶ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		ಫಲಿತಾಂಶ = -ENOMSG;
	} ಇಲ್ಲದಿದ್ದರೆ {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;

		ಫಲಿತಾಂಶ = z_pend_curr(&msgq->ಬೀಗ, ಕೀಲಿ, &msgq->wait_q, timeout);
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ಫಲಿತಾಂಶ);
		ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, ಫಲಿತಾಂಶ);

	ಆದರೆ (resched) {
		z_reschedule(&msgq->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&msgq->ಬೀಗ, ಕೀಲಿ);
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_msgq_get(ರಚನೆ k_msgq *msgq, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(ದತ್ತಾಂಶ, msgq->ಸಂದೇಶ_ಗಾತ್ರ));

	ಹಿಂತಿರುಗಿ z_impl_k_msgq_get(msgq, ದತ್ತಾಂಶ, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_msgq_peek(ರಚನೆ k_msgq *msgq, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&msgq->ಬೀಗ);

	ಆದರೆ (msgq->ಬಳಸಿದ_msgs > 0U) {
		/* take first available message from queue */
		(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, msgq->read_ptr, msgq->ಸಂದೇಶ_ಗಾತ್ರ);
		ಫಲಿತಾಂಶ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* don't wait for a message to become available */
		ಫಲಿತಾಂಶ = -ENOMSG;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ಫಲಿತಾಂಶ);

	k_spin_unlock(&msgq->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_msgq_peek(ರಚನೆ k_msgq *msgq, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(ದತ್ತಾಂಶ, msgq->ಸಂದೇಶ_ಗಾತ್ರ));

	ಹಿಂತಿರುಗಿ z_impl_k_msgq_peek(msgq, ದತ್ತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_peek_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_msgq_peek_at(ರಚನೆ k_msgq *msgq, ಶೂನ್ಯ *ದತ್ತಾಂಶ, uint32_t idx)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;
	uint32_t ಬೈಟುಗಳು_ಗಮ್ಯ_ಕೊನೆ;
	uint32_t byte_ಸ್ಥಾನಾಂತರ;
	ಅಕ್ಷರ *ಆರಂಭ_addr;

	ಕೀಲಿ = k_spin_ಬೀಗ(&msgq->ಬೀಗ);

	ಆದರೆ (msgq->ಬಳಸಿದ_msgs > idx) {
		ಬೈಟುಗಳು_ಗಮ್ಯ_ಕೊನೆ = (msgq->ಬಫರ್_ಕೊನೆ - msgq->read_ptr);
		byte_ಸ್ಥಾನಾಂತರ = idx * msgq->ಸಂದೇಶ_ಗಾತ್ರ;
		ಆರಂಭ_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		ಆದರೆ (ಬೈಟುಗಳು_ಗಮ್ಯ_ಕೊನೆ <= byte_ಸ್ಥಾನಾಂತರ) {
			/* Tweak the values in case */
			byte_ಸ್ಥಾನಾಂತರ -= ಬೈಟುಗಳು_ಗಮ್ಯ_ಕೊನೆ;
			/* wrap-around is required */
			ಆರಂಭ_addr = msgq->ಬಫರ್_ಆರಂಭ;
		}
		(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, ಆರಂಭ_addr + byte_ಸ್ಥಾನಾಂತರ, msgq->ಸಂದೇಶ_ಗಾತ್ರ);
		ಫಲಿತಾಂಶ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* don't wait for a message to become available */
		ಫಲಿತಾಂಶ = -ENOMSG;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_msgq, peek, msgq, ಫಲಿತಾಂಶ);

	k_spin_unlock(&msgq->ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_msgq_peek_at(ರಚನೆ k_msgq *msgq, ಶೂನ್ಯ *ದತ್ತಾಂಶ, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(ದತ್ತಾಂಶ, msgq->ಸಂದೇಶ_ಗಾತ್ರ));

	ಹಿಂತಿರುಗಿ z_impl_k_msgq_peek_at(msgq, ದತ್ತಾಂಶ, idx);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_msgq_purge(ರಚನೆ k_msgq *msgq)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	bool resched = false;

	ಕೀಲಿ = k_spin_ಬೀಗ(&msgq->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	ಆಗ (z_sched_wake(&msgq->wait_q, -ENOMSG, ಖಾಲಿ)) {
		resched = true;
	}

	msgq->ಬಳಸಿದ_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	ಆದರೆ (resched) {
		z_reschedule(&msgq->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&msgq->ಬೀಗ, ಕೀಲಿ);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_msgq_purge(ರಚನೆ k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_purge_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t z_vrfy_k_msgq_ಅಂಕೆ_free_get(ರಚನೆ k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ಹಿಂತಿರುಗಿ z_impl_k_msgq_ಅಂಕೆ_free_get(msgq);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t z_vrfy_k_msgq_ಅಂಕೆ_ಬಳಸಿದ_get(ರಚನೆ k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	ಹಿಂತಿರುಗಿ z_impl_k_msgq_ಅಂಕೆ_ಬಳಸಿದ_get(msgq);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
