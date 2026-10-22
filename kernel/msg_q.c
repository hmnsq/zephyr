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

#اگر_تعریف_شده CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, عدم);
#پایان_اگر /* CONFIG_OBJ_CORE_MSGQ */

ساكن مضمن bool msgq_handle_poll_events(بنية k_msgq *msgq)
{
#اگر_تعریف_شده CONFIG_POLL
	أرجع z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_داده_موجود);
#وإلا
	ARG_استفادهنشده(msgq);
	أرجع false;
#پایان_اگر /* CONFIG_POLL */
}

فراغ k_msgq_init(بنية k_msgq *msgq, حرف *میانگیر, size_t پیام_اندازه,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!اندازه_mul_سرریز(max_msgs, پیام_اندازه, &(size_t){0}));
	__ASSERT_NO_MSG(!اندازه_جمع_کن_سرریز((size_t)(uintptr_t)میانگیر, max_msgs * پیام_اندازه,
					&(size_t){0}));

	msgq->پیام_اندازه = پیام_اندازه;
	msgq->max_msgs = max_msgs;
	msgq->میانگیر_شروع = میانگیر;
	msgq->میانگیر_انتها = میانگیر + (max_msgs * پیام_اندازه);
	msgq->read_ptr = میانگیر;
	msgq->write_ptr = میانگیر;
	msgq->استفاده_شده_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->قفل = (بنية k_spinlock) {};
#اگر_تعریف_شده CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#پایان_اگر	/* CONFIG_POLL */

#اگر_تعریف_شده CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#پایان_اگر /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورت_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

صحيح z_impl_k_msgq_alloc_init(بنية k_msgq *msgq, size_t پیام_اندازه,
			    uint32_t max_msgs)
{
	فراغ *میانگیر;
	صحيح ret;
	size_t مجموع_اندازه;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	إذا (اندازه_mul_سرریز(پیام_اندازه, max_msgs, &مجموع_اندازه)) {
		ret = -EINVAL;
	} وإلا {
		میانگیر = z_thread_malloc(مجموع_اندازه);
		إذا (میانگیر != عدم) {
			k_msgq_init(msgq, میانگیر, پیام_اندازه, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USERSPACE
صحيح z_vrfy_k_msgq_alloc_init(بنية k_msgq *msgq, size_t پیام_اندازه,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	أرجع z_impl_k_msgq_alloc_init(msgq, پیام_اندازه, max_msgs);
}
#ضمن <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_msgq_cleanup(بنية k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_قفل(&msgq->قفل);

	CHECKIF(locked && (z_waitq_سر_locked(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب بیرون;
	}

	CHECKIF(!locked && (z_waitq_سر(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب بیرون;
	}

	إذا ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->میانگیر_شروع);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

بیرون:
	k_spin_unlock(&msgq->قفل, کلید);

#اگر_تعریف_شده CONFIG_OBJ_CORE_MSGQ
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#پایان_اگر /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	أرجع ret;
}

صحيح k_msgq_cleanup(بنية k_msgq *msgq)
{
	أرجع z_msgq_cleanup(msgq, false);
}

ساكن مضمن صحيح put_پیام_درون_صف(بنية k_msgq *msgq, ثابت فراغ *داده,
			k_timeout_t timeout, bool put_at_پشت)
{
	__ASSERT(!arch_is_درون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	بنية k_thread *pending_thread = عدم;
	k_spinlock_کلید_t کلید;
	صحيح نتیجه;
	bool resched = false;

	کلید = k_spin_قفل(&msgq->قفل);

	إذا (put_at_پشت) {
		SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} وإلا {
		SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	إذا (msgq->استفاده_شده_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		قفل_SCHED_SPINLOCK {
			pending_thread = z_unpend_اولی_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				/* copy into the receiver's buffer */
				(فراغ)کپی_حافظه(pending_thread->پایه.جابجا_داده, داده,
					     msgq->پیام_اندازه);
				arch_thread_return_مقدار_مجموعه(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		إذا (pending_thread == عدم) {
			size_t پیام_اندازه = msgq->پیام_اندازه;
			حرف *جایگاه;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->میانگیر_شروع) &&
					(msgq->write_ptr <= (msgq->میانگیر_انتها - 1)) &&
					((size_t)(uintptr_t)(msgq->میانگیر_انتها - msgq->write_ptr) >=
						msgq->پیام_اندازه));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			إذا (put_at_پشت) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				حرف *بعدی;

				جایگاه = msgq->write_ptr;
				بعدی = جایگاه + پیام_اندازه;
				إذا (بعدی == msgq->میانگیر_انتها) {
					بعدی = msgq->میانگیر_شروع;
				}
				msgq->write_ptr = بعدی;
			} وإلا {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				جایگاه = msgq->read_ptr;
				إذا (جایگاه == msgq->میانگیر_شروع) {
					جایگاه = msgq->میانگیر_انتها;
				}
				جایگاه -= پیام_اندازه;
				msgq->read_ptr = جایگاه;
			}
			msgq->استفاده_شده_msgs++;
			(فراغ)کپی_حافظه(جایگاه, داده, پیام_اندازه);
			resched = msgq_handle_poll_events(msgq);
		}
		نتیجه = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		نتیجه = -ENOMSG;
	} وإلا {
		إذا (put_at_پشت) {
			SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} وإلا {
			SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->پایه.جابجا_داده = (فراغ *) داده;

		نتیجه = z_pend_curr(&msgq->قفل, کلید, &msgq->wait_q, timeout);

		إذا (put_at_پشت) {
			SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتیجه);
		} وإلا {
			SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتیجه);
		}

		أرجع نتیجه;
	}

	إذا (put_at_پشت) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتیجه);
	} وإلا {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتیجه);
	}

	إذا (resched) {
		z_reschedule(&msgq->قفل, کلید);
	} وإلا {
		k_spin_unlock(&msgq->قفل, کلید);
	}

	أرجع نتیجه;
}


صحيح z_impl_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *داده, k_timeout_t timeout)
{
	أرجع put_پیام_درون_صف(msgq, داده, timeout, true);
}

صحيح z_impl_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *داده)
{
	أرجع put_پیام_درون_صف(msgq, داده, K_NO_WAIT, false);
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *داده,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_READ(داده, msgq->پیام_اندازه));

	أرجع z_impl_k_msgq_put(msgq, داده, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_put_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *داده)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_READ(داده, msgq->پیام_اندازه));

	أرجع z_impl_k_msgq_put_front(msgq, داده);
}
#ضمن <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_get_attrs(بنية k_msgq *msgq, بنية k_msgq_attrs *attrs)
{
	attrs->پیام_اندازه = msgq->پیام_اندازه;
	attrs->max_msgs = msgq->max_msgs;
	attrs->استفاده_شده_msgs = msgq->استفاده_شده_msgs;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_get_attrs(بنية k_msgq *msgq,
					   بنية k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(attrs, حجم(بنية k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ضمن <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_get(بنية k_msgq *msgq, فراغ *داده, k_timeout_t timeout)
{
	__ASSERT(!arch_is_درون_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_کلید_t کلید;
	بنية k_thread *pending_thread;
	صحيح نتیجه;
	bool resched = false;

	کلید = k_spin_قفل(&msgq->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	إذا (likely(msgq->استفاده_شده_msgs > 0U)) {
		size_t پیام_اندازه = msgq->پیام_اندازه;
		حرف *جایگاه = msgq->read_ptr;
		حرف *بعدی = جایگاه + پیام_اندازه;

		/* drop the message from the queue, then copy it out */
		إذا (بعدی == msgq->میانگیر_انتها) {
			بعدی = msgq->میانگیر_شروع;
		}
		msgq->read_ptr = بعدی;
		msgq->استفاده_شده_msgs--;

		(فراغ)کپی_حافظه(داده, جایگاه, پیام_اندازه);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->میانگیر_شروع) &&
				(msgq->write_ptr <= (msgq->میانگیر_انتها - 1)) &&
				((size_t)(uintptr_t)(msgq->میانگیر_انتها - msgq->write_ptr) >=
					msgq->پیام_اندازه));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		قفل_SCHED_SPINLOCK {
			pending_thread = z_unpend_اولی_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				جایگاه = msgq->write_ptr;
				بعدی = جایگاه + پیام_اندازه;
				إذا (بعدی == msgq->میانگیر_انتها) {
					بعدی = msgq->میانگیر_شروع;
				}
				msgq->write_ptr = بعدی;
				msgq->استفاده_شده_msgs++;

				(فراغ)کپی_حافظه(جایگاه, pending_thread->پایه.جابجا_داده, پیام_اندازه);

				arch_thread_return_مقدار_مجموعه(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		نتیجه = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		نتیجه = -ENOMSG;
	} وإلا {
		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->پایه.جابجا_داده = داده;

		نتیجه = z_pend_curr(&msgq->قفل, کلید, &msgq->wait_q, timeout);
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتیجه);
		أرجع نتیجه;
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتیجه);

	إذا (resched) {
		z_reschedule(&msgq->قفل, کلید);
	} وإلا {
		k_spin_unlock(&msgq->قفل, کلید);
	}

	أرجع نتیجه;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_get(بنية k_msgq *msgq, فراغ *داده,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(داده, msgq->پیام_اندازه));

	أرجع z_impl_k_msgq_get(msgq, داده, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek(بنية k_msgq *msgq, فراغ *داده)
{
	k_spinlock_کلید_t کلید;
	صحيح نتیجه;

	کلید = k_spin_قفل(&msgq->قفل);

	إذا (msgq->استفاده_شده_msgs > 0U) {
		/* take first available message from queue */
		(فراغ)کپی_حافظه(داده, msgq->read_ptr, msgq->پیام_اندازه);
		نتیجه = 0;
	} وإلا {
		/* don't wait for a message to become available */
		نتیجه = -ENOMSG;
	}

	SYS_پورت_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتیجه);

	k_spin_unlock(&msgq->قفل, کلید);

	أرجع نتیجه;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek(بنية k_msgq *msgq, فراغ *داده)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(داده, msgq->پیام_اندازه));

	أرجع z_impl_k_msgq_peek(msgq, داده);
}
#ضمن <zephyr/syscalls/k_msgq_peek_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *داده, uint32_t idx)
{
	k_spinlock_کلید_t کلید;
	صحيح نتیجه;
	uint32_t بایتها_مقصد_انتها;
	uint32_t byte_جابه_جایی;
	حرف *شروع_addr;

	کلید = k_spin_قفل(&msgq->قفل);

	إذا (msgq->استفاده_شده_msgs > idx) {
		بایتها_مقصد_انتها = (msgq->میانگیر_انتها - msgq->read_ptr);
		byte_جابه_جایی = idx * msgq->پیام_اندازه;
		شروع_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		إذا (بایتها_مقصد_انتها <= byte_جابه_جایی) {
			/* Tweak the values in case */
			byte_جابه_جایی -= بایتها_مقصد_انتها;
			/* wrap-around is required */
			شروع_addr = msgq->میانگیر_شروع;
		}
		(فراغ)کپی_حافظه(داده, شروع_addr + byte_جابه_جایی, msgq->پیام_اندازه);
		نتیجه = 0;
	} وإلا {
		/* don't wait for a message to become available */
		نتیجه = -ENOMSG;
	}

	SYS_پورت_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتیجه);

	k_spin_unlock(&msgq->قفل, کلید);

	أرجع نتیجه;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *داده, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظه_WRITE(داده, msgq->پیام_اندازه));

	أرجع z_impl_k_msgq_peek_at(msgq, داده, idx);
}
#ضمن <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_purge(بنية k_msgq *msgq)
{
	k_spinlock_کلید_t کلید;
	bool resched = false;

	کلید = k_spin_قفل(&msgq->قفل);

	SYS_پورت_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	طالما (z_sched_wake(&msgq->wait_q, -ENOMSG, عدم)) {
		resched = true;
	}

	msgq->استفاده_شده_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	إذا (resched) {
		z_reschedule(&msgq->قفل, کلید);
	} وإلا {
		k_spin_unlock(&msgq->قفل, کلید);
	}
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_purge(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_purge_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_رقم_free_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_رقم_free_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ساكن مضمن uint32_t z_vrfy_k_msgq_رقم_استفاده_شده_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_رقم_استفاده_شده_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#پایان_اگر /* CONFIG_USERSPACE */
