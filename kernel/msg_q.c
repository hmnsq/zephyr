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

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_MSGQ */

ساكن مضمن bool msgq_handle_poll_events(بنية k_msgq *msgq)
{
#جيڪڏهن_وصف_ٿيل CONFIG_POLL
	أرجع z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_معلومات_موجود);
#وإلا
	ARG_اڻ_استعمال_ٿيل(msgq);
	أرجع false;
#جيڪڏهن_پورو /* CONFIG_POLL */
}

فراغ k_msgq_init(بنية k_msgq *msgq, حرف *بفر, size_t نياپو_سائيز,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!سائيز_mul_حد_کان_مٿي(max_msgs, نياپو_سائيز, &(size_t){0}));
	__ASSERT_NO_MSG(!سائيز_جوڙيو_حد_کان_مٿي((size_t)(uintptr_t)بفر, max_msgs * نياپو_سائيز,
					&(size_t){0}));

	msgq->نياپو_سائيز = نياپو_سائيز;
	msgq->max_msgs = max_msgs;
	msgq->بفر_شروعات = بفر;
	msgq->بفر_پڇاڙي = بفر + (max_msgs * نياپو_سائيز);
	msgq->read_ptr = بفر;
	msgq->write_ptr = بفر;
	msgq->استعمال_ٿيل_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->تالو = (بنية k_spinlock) {};
#جيڪڏهن_وصف_ٿيل CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#جيڪڏهن_پورو	/* CONFIG_POLL */

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورٽ_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

صحيح z_impl_k_msgq_alloc_init(بنية k_msgq *msgq, size_t نياپو_سائيز,
			    uint32_t max_msgs)
{
	فراغ *بفر;
	صحيح ret;
	size_t ڪل_سائيز;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	إذا (سائيز_mul_حد_کان_مٿي(نياپو_سائيز, max_msgs, &ڪل_سائيز)) {
		ret = -EINVAL;
	} وإلا {
		بفر = z_thread_malloc(ڪل_سائيز);
		إذا (بفر != عدم) {
			k_msgq_init(msgq, بفر, نياپو_سائيز, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
صحيح z_vrfy_k_msgq_alloc_init(بنية k_msgq *msgq, size_t نياپو_سائيز,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	أرجع z_impl_k_msgq_alloc_init(msgq, نياپو_سائيز, max_msgs);
}
#ضمن <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_msgq_cleanup(بنية k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&msgq->تالو);

	CHECKIF(locked && (z_waitq_سر_locked(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب ٻاهريون;
	}

	CHECKIF(!locked && (z_waitq_سر(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب ٻاهريون;
	}

	إذا ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->بفر_شروعات);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

ٻاهريون:
	k_spin_unlock(&msgq->تالو, ڪنجي);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_MSGQ
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	أرجع ret;
}

صحيح k_msgq_cleanup(بنية k_msgq *msgq)
{
	أرجع z_msgq_cleanup(msgq, false);
}

ساكن مضمن صحيح put_نياپو_اندريون_قطار(بنية k_msgq *msgq, ثابت فراغ *معلومات,
			k_timeout_t timeout, bool put_at_پٺ)
{
	__ASSERT(!arch_is_اندريون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	بنية k_thread *pending_thread = عدم;
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح نتيجو;
	bool resched = false;

	ڪنجي = k_spin_تالو(&msgq->تالو);

	إذا (put_at_پٺ) {
		SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} وإلا {
		SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	إذا (msgq->استعمال_ٿيل_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		تالو_SCHED_SPINLOCK {
			pending_thread = z_unpend_پهريون_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				/* copy into the receiver's buffer */
				(فراغ)يادگيري_نقل(pending_thread->بنياد.مٽا_سٽا_معلومات, معلومات,
					     msgq->نياپو_سائيز);
				arch_thread_return_قدر_سيٽ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		إذا (pending_thread == عدم) {
			size_t نياپو_سائيز = msgq->نياپو_سائيز;
			حرف *جڳهه;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_شروعات) &&
					(msgq->write_ptr <= (msgq->بفر_پڇاڙي - 1)) &&
					((size_t)(uintptr_t)(msgq->بفر_پڇاڙي - msgq->write_ptr) >=
						msgq->نياپو_سائيز));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			إذا (put_at_پٺ) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				حرف *ايندڙ;

				جڳهه = msgq->write_ptr;
				ايندڙ = جڳهه + نياپو_سائيز;
				إذا (ايندڙ == msgq->بفر_پڇاڙي) {
					ايندڙ = msgq->بفر_شروعات;
				}
				msgq->write_ptr = ايندڙ;
			} وإلا {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				جڳهه = msgq->read_ptr;
				إذا (جڳهه == msgq->بفر_شروعات) {
					جڳهه = msgq->بفر_پڇاڙي;
				}
				جڳهه -= نياپو_سائيز;
				msgq->read_ptr = جڳهه;
			}
			msgq->استعمال_ٿيل_msgs++;
			(فراغ)يادگيري_نقل(جڳهه, معلومات, نياپو_سائيز);
			resched = msgq_handle_poll_events(msgq);
		}
		نتيجو = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		نتيجو = -ENOMSG;
	} وإلا {
		إذا (put_at_پٺ) {
			SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} وإلا {
			SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->بنياد.مٽا_سٽا_معلومات = (فراغ *) معلومات;

		نتيجو = z_pend_curr(&msgq->تالو, ڪنجي, &msgq->wait_q, timeout);

		إذا (put_at_پٺ) {
			SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتيجو);
		} وإلا {
			SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتيجو);
		}

		أرجع نتيجو;
	}

	إذا (put_at_پٺ) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتيجو);
	} وإلا {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتيجو);
	}

	إذا (resched) {
		z_reschedule(&msgq->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&msgq->تالو, ڪنجي);
	}

	أرجع نتيجو;
}


صحيح z_impl_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *معلومات, k_timeout_t timeout)
{
	أرجع put_نياپو_اندريون_قطار(msgq, معلومات, timeout, true);
}

صحيح z_impl_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *معلومات)
{
	أرجع put_نياپو_اندريون_قطار(msgq, معلومات, K_NO_WAIT, false);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *معلومات,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_يادگيري_READ(معلومات, msgq->نياپو_سائيز));

	أرجع z_impl_k_msgq_put(msgq, معلومات, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_put_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_يادگيري_READ(معلومات, msgq->نياپو_سائيز));

	أرجع z_impl_k_msgq_put_front(msgq, معلومات);
}
#ضمن <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_get_attrs(بنية k_msgq *msgq, بنية k_msgq_attrs *attrs)
{
	attrs->نياپو_سائيز = msgq->نياپو_سائيز;
	attrs->max_msgs = msgq->max_msgs;
	attrs->استعمال_ٿيل_msgs = msgq->استعمال_ٿيل_msgs;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_get_attrs(بنية k_msgq *msgq,
					   بنية k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(attrs, حجم(بنية k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ضمن <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_get(بنية k_msgq *msgq, فراغ *معلومات, k_timeout_t timeout)
{
	__ASSERT(!arch_is_اندريون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_ڪنجي_t ڪنجي;
	بنية k_thread *pending_thread;
	صحيح نتيجو;
	bool resched = false;

	ڪنجي = k_spin_تالو(&msgq->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	إذا (likely(msgq->استعمال_ٿيل_msgs > 0U)) {
		size_t نياپو_سائيز = msgq->نياپو_سائيز;
		حرف *جڳهه = msgq->read_ptr;
		حرف *ايندڙ = جڳهه + نياپو_سائيز;

		/* drop the message from the queue, then copy it out */
		إذا (ايندڙ == msgq->بفر_پڇاڙي) {
			ايندڙ = msgq->بفر_شروعات;
		}
		msgq->read_ptr = ايندڙ;
		msgq->استعمال_ٿيل_msgs--;

		(فراغ)يادگيري_نقل(معلومات, جڳهه, نياپو_سائيز);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_شروعات) &&
				(msgq->write_ptr <= (msgq->بفر_پڇاڙي - 1)) &&
				((size_t)(uintptr_t)(msgq->بفر_پڇاڙي - msgq->write_ptr) >=
					msgq->نياپو_سائيز));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		تالو_SCHED_SPINLOCK {
			pending_thread = z_unpend_پهريون_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				جڳهه = msgq->write_ptr;
				ايندڙ = جڳهه + نياپو_سائيز;
				إذا (ايندڙ == msgq->بفر_پڇاڙي) {
					ايندڙ = msgq->بفر_شروعات;
				}
				msgq->write_ptr = ايندڙ;
				msgq->استعمال_ٿيل_msgs++;

				(فراغ)يادگيري_نقل(جڳهه, pending_thread->بنياد.مٽا_سٽا_معلومات, نياپو_سائيز);

				arch_thread_return_قدر_سيٽ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		نتيجو = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		نتيجو = -ENOMSG;
	} وإلا {
		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->بنياد.مٽا_سٽا_معلومات = معلومات;

		نتيجو = z_pend_curr(&msgq->تالو, ڪنجي, &msgq->wait_q, timeout);
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتيجو);
		أرجع نتيجو;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتيجو);

	إذا (resched) {
		z_reschedule(&msgq->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&msgq->تالو, ڪنجي);
	}

	أرجع نتيجو;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_get(بنية k_msgq *msgq, فراغ *معلومات,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(معلومات, msgq->نياپو_سائيز));

	أرجع z_impl_k_msgq_get(msgq, معلومات, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_get_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek(بنية k_msgq *msgq, فراغ *معلومات)
{
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح نتيجو;

	ڪنجي = k_spin_تالو(&msgq->تالو);

	إذا (msgq->استعمال_ٿيل_msgs > 0U) {
		/* take first available message from queue */
		(فراغ)يادگيري_نقل(معلومات, msgq->read_ptr, msgq->نياپو_سائيز);
		نتيجو = 0;
	} وإلا {
		/* don't wait for a message to become available */
		نتيجو = -ENOMSG;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتيجو);

	k_spin_unlock(&msgq->تالو, ڪنجي);

	أرجع نتيجو;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek(بنية k_msgq *msgq, فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(معلومات, msgq->نياپو_سائيز));

	أرجع z_impl_k_msgq_peek(msgq, معلومات);
}
#ضمن <zephyr/syscalls/k_msgq_peek_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *معلومات, uint32_t idx)
{
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح نتيجو;
	uint32_t بائيٽون_منزل_پڇاڙي;
	uint32_t byte_وٿي;
	حرف *شروعات_addr;

	ڪنجي = k_spin_تالو(&msgq->تالو);

	إذا (msgq->استعمال_ٿيل_msgs > idx) {
		بائيٽون_منزل_پڇاڙي = (msgq->بفر_پڇاڙي - msgq->read_ptr);
		byte_وٿي = idx * msgq->نياپو_سائيز;
		شروعات_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		إذا (بائيٽون_منزل_پڇاڙي <= byte_وٿي) {
			/* Tweak the values in case */
			byte_وٿي -= بائيٽون_منزل_پڇاڙي;
			/* wrap-around is required */
			شروعات_addr = msgq->بفر_شروعات;
		}
		(فراغ)يادگيري_نقل(معلومات, شروعات_addr + byte_وٿي, msgq->نياپو_سائيز);
		نتيجو = 0;
	} وإلا {
		/* don't wait for a message to become available */
		نتيجو = -ENOMSG;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتيجو);

	k_spin_unlock(&msgq->تالو, ڪنجي);

	أرجع نتيجو;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *معلومات, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(معلومات, msgq->نياپو_سائيز));

	أرجع z_impl_k_msgq_peek_at(msgq, معلومات, idx);
}
#ضمن <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_purge(بنية k_msgq *msgq)
{
	k_spinlock_ڪنجي_t ڪنجي;
	bool resched = false;

	ڪنجي = k_spin_تالو(&msgq->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	طالما (z_sched_wake(&msgq->wait_q, -ENOMSG, عدم)) {
		resched = true;
	}

	msgq->استعمال_ٿيل_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	إذا (resched) {
		z_reschedule(&msgq->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&msgq->تالو, ڪنجي);
	}
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_purge(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_purge_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_انگ_free_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_انگ_free_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_انگ_استعمال_ٿيل_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_انگ_استعمال_ٿيل_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#جيڪڏهن_پورو /* CONFIG_USERSPACE */
