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

#که_تعریف_شوی CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_MSGQ */

ساكن مضمن bool msgq_handle_poll_events(بنية k_msgq *msgq)
{
#که_تعریف_شوی CONFIG_POLL
	أرجع z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_معلومات_شته);
#وإلا
	ARG_ناکارول_شوی(msgq);
	أرجع false;
#د_که_پای /* CONFIG_POLL */
}

فراغ k_msgq_init(بنية k_msgq *msgq, حرف *بفر, size_t پیغام_اندازه,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!اندازه_mul_ډکېدنه(max_msgs, پیغام_اندازه, &(size_t){0}));
	__ASSERT_NO_MSG(!اندازه_جمع_کړه_ډکېدنه((size_t)(uintptr_t)بفر, max_msgs * پیغام_اندازه,
					&(size_t){0}));

	msgq->پیغام_اندازه = پیغام_اندازه;
	msgq->max_msgs = max_msgs;
	msgq->بفر_پیل = بفر;
	msgq->بفر_پای_ټکی = بفر + (max_msgs * پیغام_اندازه);
	msgq->read_ptr = بفر;
	msgq->write_ptr = بفر;
	msgq->کارول_شوی_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->قلف = (بنية k_spinlock) {};
#که_تعریف_شوی CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#د_که_پای	/* CONFIG_POLL */

#که_تعریف_شوی CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#د_که_پای /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورټ_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

صحيح z_impl_k_msgq_alloc_init(بنية k_msgq *msgq, size_t پیغام_اندازه,
			    uint32_t max_msgs)
{
	فراغ *بفر;
	صحيح ret;
	size_t ټول_اندازه;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	إذا (اندازه_mul_ډکېدنه(پیغام_اندازه, max_msgs, &ټول_اندازه)) {
		ret = -EINVAL;
	} وإلا {
		بفر = z_thread_malloc(ټول_اندازه);
		إذا (بفر != عدم) {
			k_msgq_init(msgq, بفر, پیغام_اندازه, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	أرجع ret;
}

#که_تعریف_شوی CONFIG_USERSPACE
صحيح z_vrfy_k_msgq_alloc_init(بنية k_msgq *msgq, size_t پیغام_اندازه,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	أرجع z_impl_k_msgq_alloc_init(msgq, پیغام_اندازه, max_msgs);
}
#ضمن <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_msgq_cleanup(بنية k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلی_t کلی = k_spin_قلف(&msgq->قلف);

	CHECKIF(locked && (z_waitq_سر_locked(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب بهر;
	}

	CHECKIF(!locked && (z_waitq_سر(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب بهر;
	}

	إذا ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->بفر_پیل);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

بهر:
	k_spin_unlock(&msgq->قلف, کلی);

#که_تعریف_شوی CONFIG_OBJ_CORE_MSGQ
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#د_که_پای /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	أرجع ret;
}

صحيح k_msgq_cleanup(بنية k_msgq *msgq)
{
	أرجع z_msgq_cleanup(msgq, false);
}

ساكن مضمن صحيح put_پیغام_دننه_کتار(بنية k_msgq *msgq, ثابت فراغ *معلومات,
			k_timeout_t timeout, bool put_at_شا)
{
	__ASSERT(!arch_is_دننه_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	بنية k_thread *pending_thread = عدم;
	k_spinlock_کلی_t کلی;
	صحيح پایله;
	bool resched = false;

	کلی = k_spin_قلف(&msgq->قلف);

	إذا (put_at_شا) {
		SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} وإلا {
		SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	إذا (msgq->کارول_شوی_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		قلف_SCHED_SPINLOCK {
			pending_thread = z_unpend_لومړی_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				/* copy into the receiver's buffer */
				(فراغ)حافظه_نقل(pending_thread->بنسټ.بدلون_معلومات, معلومات,
					     msgq->پیغام_اندازه);
				arch_thread_return_ارزښت_سېټ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		إذا (pending_thread == عدم) {
			size_t پیغام_اندازه = msgq->پیغام_اندازه;
			حرف *خونه;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_پیل) &&
					(msgq->write_ptr <= (msgq->بفر_پای_ټکی - 1)) &&
					((size_t)(uintptr_t)(msgq->بفر_پای_ټکی - msgq->write_ptr) >=
						msgq->پیغام_اندازه));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			إذا (put_at_شا) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				حرف *بل;

				خونه = msgq->write_ptr;
				بل = خونه + پیغام_اندازه;
				إذا (بل == msgq->بفر_پای_ټکی) {
					بل = msgq->بفر_پیل;
				}
				msgq->write_ptr = بل;
			} وإلا {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				خونه = msgq->read_ptr;
				إذا (خونه == msgq->بفر_پیل) {
					خونه = msgq->بفر_پای_ټکی;
				}
				خونه -= پیغام_اندازه;
				msgq->read_ptr = خونه;
			}
			msgq->کارول_شوی_msgs++;
			(فراغ)حافظه_نقل(خونه, معلومات, پیغام_اندازه);
			resched = msgq_handle_poll_events(msgq);
		}
		پایله = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		پایله = -ENOMSG;
	} وإلا {
		إذا (put_at_شا) {
			SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} وإلا {
			SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->بنسټ.بدلون_معلومات = (فراغ *) معلومات;

		پایله = z_pend_curr(&msgq->قلف, کلی, &msgq->wait_q, timeout);

		إذا (put_at_شا) {
			SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, پایله);
		} وإلا {
			SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, پایله);
		}

		أرجع پایله;
	}

	إذا (put_at_شا) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, پایله);
	} وإلا {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, پایله);
	}

	إذا (resched) {
		z_reschedule(&msgq->قلف, کلی);
	} وإلا {
		k_spin_unlock(&msgq->قلف, کلی);
	}

	أرجع پایله;
}


صحيح z_impl_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *معلومات, k_timeout_t timeout)
{
	أرجع put_پیغام_دننه_کتار(msgq, معلومات, timeout, true);
}

صحيح z_impl_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *معلومات)
{
	أرجع put_پیغام_دننه_کتار(msgq, معلومات, K_NO_WAIT, false);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *معلومات,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_READ(معلومات, msgq->پیغام_اندازه));

	أرجع z_impl_k_msgq_put(msgq, معلومات, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_put_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_READ(معلومات, msgq->پیغام_اندازه));

	أرجع z_impl_k_msgq_put_front(msgq, معلومات);
}
#ضمن <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_get_attrs(بنية k_msgq *msgq, بنية k_msgq_attrs *attrs)
{
	attrs->پیغام_اندازه = msgq->پیغام_اندازه;
	attrs->max_msgs = msgq->max_msgs;
	attrs->کارول_شوی_msgs = msgq->کارول_شوی_msgs;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_get_attrs(بنية k_msgq *msgq,
					   بنية k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(attrs, حجم(بنية k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ضمن <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_get(بنية k_msgq *msgq, فراغ *معلومات, k_timeout_t timeout)
{
	__ASSERT(!arch_is_دننه_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_کلی_t کلی;
	بنية k_thread *pending_thread;
	صحيح پایله;
	bool resched = false;

	کلی = k_spin_قلف(&msgq->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	إذا (likely(msgq->کارول_شوی_msgs > 0U)) {
		size_t پیغام_اندازه = msgq->پیغام_اندازه;
		حرف *خونه = msgq->read_ptr;
		حرف *بل = خونه + پیغام_اندازه;

		/* drop the message from the queue, then copy it out */
		إذا (بل == msgq->بفر_پای_ټکی) {
			بل = msgq->بفر_پیل;
		}
		msgq->read_ptr = بل;
		msgq->کارول_شوی_msgs--;

		(فراغ)حافظه_نقل(معلومات, خونه, پیغام_اندازه);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_پیل) &&
				(msgq->write_ptr <= (msgq->بفر_پای_ټکی - 1)) &&
				((size_t)(uintptr_t)(msgq->بفر_پای_ټکی - msgq->write_ptr) >=
					msgq->پیغام_اندازه));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		قلف_SCHED_SPINLOCK {
			pending_thread = z_unpend_لومړی_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				خونه = msgq->write_ptr;
				بل = خونه + پیغام_اندازه;
				إذا (بل == msgq->بفر_پای_ټکی) {
					بل = msgq->بفر_پیل;
				}
				msgq->write_ptr = بل;
				msgq->کارول_شوی_msgs++;

				(فراغ)حافظه_نقل(خونه, pending_thread->بنسټ.بدلون_معلومات, پیغام_اندازه);

				arch_thread_return_ارزښت_سېټ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		پایله = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		پایله = -ENOMSG;
	} وإلا {
		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->بنسټ.بدلون_معلومات = معلومات;

		پایله = z_pend_curr(&msgq->قلف, کلی, &msgq->wait_q, timeout);
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, پایله);
		أرجع پایله;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, پایله);

	إذا (resched) {
		z_reschedule(&msgq->قلف, کلی);
	} وإلا {
		k_spin_unlock(&msgq->قلف, کلی);
	}

	أرجع پایله;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_get(بنية k_msgq *msgq, فراغ *معلومات,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(معلومات, msgq->پیغام_اندازه));

	أرجع z_impl_k_msgq_get(msgq, معلومات, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_get_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek(بنية k_msgq *msgq, فراغ *معلومات)
{
	k_spinlock_کلی_t کلی;
	صحيح پایله;

	کلی = k_spin_قلف(&msgq->قلف);

	إذا (msgq->کارول_شوی_msgs > 0U) {
		/* take first available message from queue */
		(فراغ)حافظه_نقل(معلومات, msgq->read_ptr, msgq->پیغام_اندازه);
		پایله = 0;
	} وإلا {
		/* don't wait for a message to become available */
		پایله = -ENOMSG;
	}

	SYS_پورټ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, پایله);

	k_spin_unlock(&msgq->قلف, کلی);

	أرجع پایله;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek(بنية k_msgq *msgq, فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(معلومات, msgq->پیغام_اندازه));

	أرجع z_impl_k_msgq_peek(msgq, معلومات);
}
#ضمن <zephyr/syscalls/k_msgq_peek_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *معلومات, uint32_t idx)
{
	k_spinlock_کلی_t کلی;
	صحيح پایله;
	uint32_t بایټونه_منزل_پای_ټکی;
	uint32_t byte_دننه_واټن;
	حرف *پیل_addr;

	کلی = k_spin_قلف(&msgq->قلف);

	إذا (msgq->کارول_شوی_msgs > idx) {
		بایټونه_منزل_پای_ټکی = (msgq->بفر_پای_ټکی - msgq->read_ptr);
		byte_دننه_واټن = idx * msgq->پیغام_اندازه;
		پیل_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		إذا (بایټونه_منزل_پای_ټکی <= byte_دننه_واټن) {
			/* Tweak the values in case */
			byte_دننه_واټن -= بایټونه_منزل_پای_ټکی;
			/* wrap-around is required */
			پیل_addr = msgq->بفر_پیل;
		}
		(فراغ)حافظه_نقل(معلومات, پیل_addr + byte_دننه_واټن, msgq->پیغام_اندازه);
		پایله = 0;
	} وإلا {
		/* don't wait for a message to become available */
		پایله = -ENOMSG;
	}

	SYS_پورټ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, پایله);

	k_spin_unlock(&msgq->قلف, کلی);

	أرجع پایله;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *معلومات, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(معلومات, msgq->پیغام_اندازه));

	أرجع z_impl_k_msgq_peek_at(msgq, معلومات, idx);
}
#ضمن <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_purge(بنية k_msgq *msgq)
{
	k_spinlock_کلی_t کلی;
	bool resched = false;

	کلی = k_spin_قلف(&msgq->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	طالما (z_sched_wake(&msgq->wait_q, -ENOMSG, عدم)) {
		resched = true;
	}

	msgq->کارول_شوی_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	إذا (resched) {
		z_reschedule(&msgq->قلف, کلی);
	} وإلا {
		k_spin_unlock(&msgq->قلف, کلی);
	}
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_purge(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_purge_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_عدد_free_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_عدد_free_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_عدد_کارول_شوی_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_عدد_کارول_شوی_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#د_که_پای /* CONFIG_USERSPACE */
