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

#جے_تعریف_شدہ CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, عدم);
#جے_انت /* CONFIG_OBJ_CORE_MSGQ */

ساكن مضمن bool msgq_handle_poll_events(بنية k_msgq *msgq)
{
#جے_تعریف_شدہ CONFIG_POLL
	أرجع z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_انکڑے_دستیاب);
#وإلا
	ARG_بن_ورتیا(msgq);
	أرجع false;
#جے_انت /* CONFIG_POLL */
}

فراغ k_msgq_init(بنية k_msgq *msgq, حرف *بفر, size_t پیغام_ناپ,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!ناپ_mul_حد_توں_ودھ(max_msgs, پیغام_ناپ, &(size_t){0}));
	__ASSERT_NO_MSG(!ناپ_جوڑو_حد_توں_ودھ((size_t)(uintptr_t)بفر, max_msgs * پیغام_ناپ,
					&(size_t){0}));

	msgq->پیغام_ناپ = پیغام_ناپ;
	msgq->max_msgs = max_msgs;
	msgq->بفر_شروع = بفر;
	msgq->بفر_سرا = بفر + (max_msgs * پیغام_ناپ);
	msgq->read_ptr = بفر;
	msgq->write_ptr = بفر;
	msgq->ورتیا_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->جندرا = (بنية k_spinlock) {};
#جے_تعریف_شدہ CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#جے_انت	/* CONFIG_POLL */

#جے_تعریف_شدہ CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#جے_انت /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورٹ_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

صحيح z_impl_k_msgq_alloc_init(بنية k_msgq *msgq, size_t پیغام_ناپ,
			    uint32_t max_msgs)
{
	فراغ *بفر;
	صحيح ret;
	size_t کل_ناپ;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	إذا (ناپ_mul_حد_توں_ودھ(پیغام_ناپ, max_msgs, &کل_ناپ)) {
		ret = -EINVAL;
	} وإلا {
		بفر = z_thread_malloc(کل_ناپ);
		إذا (بفر != عدم) {
			k_msgq_init(msgq, بفر, پیغام_ناپ, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
صحيح z_vrfy_k_msgq_alloc_init(بنية k_msgq *msgq, size_t پیغام_ناپ,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	أرجع z_impl_k_msgq_alloc_init(msgq, پیغام_ناپ, max_msgs);
}
#ضمن <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_msgq_cleanup(بنية k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_جندرا(&msgq->جندرا);

	CHECKIF(locked && (z_waitq_سر_locked(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب باہرلا;
	}

	CHECKIF(!locked && (z_waitq_سر(&msgq->wait_q) != عدم)) {
		ret = -EBUSY;
		اذهب باہرلا;
	}

	إذا ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->بفر_شروع);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

باہرلا:
	k_spin_unlock(&msgq->جندرا, کلید);

#جے_تعریف_شدہ CONFIG_OBJ_CORE_MSGQ
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#جے_انت /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	أرجع ret;
}

صحيح k_msgq_cleanup(بنية k_msgq *msgq)
{
	أرجع z_msgq_cleanup(msgq, false);
}

ساكن مضمن صحيح put_پیغام_وچ_قطار(بنية k_msgq *msgq, ثابت فراغ *انکڑے,
			k_timeout_t timeout, bool put_at_پٹھ)
{
	__ASSERT(!arch_is_وچ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	بنية k_thread *pending_thread = عدم;
	k_spinlock_کلید_t کلید;
	صحيح نتیجہ;
	bool resched = false;

	کلید = k_spin_جندرا(&msgq->جندرا);

	إذا (put_at_پٹھ) {
		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} وإلا {
		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	إذا (msgq->ورتیا_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		جندرا_SCHED_SPINLOCK {
			pending_thread = z_unpend_پہلا_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				/* copy into the receiver's buffer */
				(فراغ)حافظہ_نقل(pending_thread->ادھار.وٹاندرا_انکڑے, انکڑے,
					     msgq->پیغام_ناپ);
				arch_thread_return_قدر_سیٹ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		إذا (pending_thread == عدم) {
			size_t پیغام_ناپ = msgq->پیغام_ناپ;
			حرف *تھاں;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_شروع) &&
					(msgq->write_ptr <= (msgq->بفر_سرا - 1)) &&
					((size_t)(uintptr_t)(msgq->بفر_سرا - msgq->write_ptr) >=
						msgq->پیغام_ناپ));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			إذا (put_at_پٹھ) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				حرف *اگلا;

				تھاں = msgq->write_ptr;
				اگلا = تھاں + پیغام_ناپ;
				إذا (اگلا == msgq->بفر_سرا) {
					اگلا = msgq->بفر_شروع;
				}
				msgq->write_ptr = اگلا;
			} وإلا {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				تھاں = msgq->read_ptr;
				إذا (تھاں == msgq->بفر_شروع) {
					تھاں = msgq->بفر_سرا;
				}
				تھاں -= پیغام_ناپ;
				msgq->read_ptr = تھاں;
			}
			msgq->ورتیا_msgs++;
			(فراغ)حافظہ_نقل(تھاں, انکڑے, پیغام_ناپ);
			resched = msgq_handle_poll_events(msgq);
		}
		نتیجہ = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		نتیجہ = -ENOMSG;
	} وإلا {
		إذا (put_at_پٹھ) {
			SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} وإلا {
			SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->ادھار.وٹاندرا_انکڑے = (فراغ *) انکڑے;

		نتیجہ = z_pend_curr(&msgq->جندرا, کلید, &msgq->wait_q, timeout);

		إذا (put_at_پٹھ) {
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتیجہ);
		} وإلا {
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتیجہ);
		}

		أرجع نتیجہ;
	}

	إذا (put_at_پٹھ) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتیجہ);
	} وإلا {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتیجہ);
	}

	إذا (resched) {
		z_reschedule(&msgq->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&msgq->جندرا, کلید);
	}

	أرجع نتیجہ;
}


صحيح z_impl_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *انکڑے, k_timeout_t timeout)
{
	أرجع put_پیغام_وچ_قطار(msgq, انکڑے, timeout, true);
}

صحيح z_impl_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *انکڑے)
{
	أرجع put_پیغام_وچ_قطار(msgq, انکڑے, K_NO_WAIT, false);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_put(بنية k_msgq *msgq, ثابت فراغ *انکڑے,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_READ(انکڑے, msgq->پیغام_ناپ));

	أرجع z_impl_k_msgq_put(msgq, انکڑے, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_put_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_msgq_put_front(بنية k_msgq *msgq, ثابت فراغ *انکڑے)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_READ(انکڑے, msgq->پیغام_ناپ));

	أرجع z_impl_k_msgq_put_front(msgq, انکڑے);
}
#ضمن <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_get_attrs(بنية k_msgq *msgq, بنية k_msgq_attrs *attrs)
{
	attrs->پیغام_ناپ = msgq->پیغام_ناپ;
	attrs->max_msgs = msgq->max_msgs;
	attrs->ورتیا_msgs = msgq->ورتیا_msgs;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_msgq_get_attrs(بنية k_msgq *msgq,
					   بنية k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(attrs, حجم(بنية k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#ضمن <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_get(بنية k_msgq *msgq, فراغ *انکڑے, k_timeout_t timeout)
{
	__ASSERT(!arch_is_وچ_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_کلید_t کلید;
	بنية k_thread *pending_thread;
	صحيح نتیجہ;
	bool resched = false;

	کلید = k_spin_جندرا(&msgq->جندرا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	إذا (likely(msgq->ورتیا_msgs > 0U)) {
		size_t پیغام_ناپ = msgq->پیغام_ناپ;
		حرف *تھاں = msgq->read_ptr;
		حرف *اگلا = تھاں + پیغام_ناپ;

		/* drop the message from the queue, then copy it out */
		إذا (اگلا == msgq->بفر_سرا) {
			اگلا = msgq->بفر_شروع;
		}
		msgq->read_ptr = اگلا;
		msgq->ورتیا_msgs--;

		(فراغ)حافظہ_نقل(انکڑے, تھاں, پیغام_ناپ);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_شروع) &&
				(msgq->write_ptr <= (msgq->بفر_سرا - 1)) &&
				((size_t)(uintptr_t)(msgq->بفر_سرا - msgq->write_ptr) >=
					msgq->پیغام_ناپ));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		جندرا_SCHED_SPINLOCK {
			pending_thread = z_unpend_پہلا_thread_locked(&msgq->wait_q);
			إذا (pending_thread != عدم) {
				SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				تھاں = msgq->write_ptr;
				اگلا = تھاں + پیغام_ناپ;
				إذا (اگلا == msgq->بفر_سرا) {
					اگلا = msgq->بفر_شروع;
				}
				msgq->write_ptr = اگلا;
				msgq->ورتیا_msgs++;

				(فراغ)حافظہ_نقل(تھاں, pending_thread->ادھار.وٹاندرا_انکڑے, پیغام_ناپ);

				arch_thread_return_قدر_سیٹ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		نتیجہ = 0;
	} وإلا إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		نتیجہ = -ENOMSG;
	} وإلا {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->ادھار.وٹاندرا_انکڑے = انکڑے;

		نتیجہ = z_pend_curr(&msgq->جندرا, کلید, &msgq->wait_q, timeout);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتیجہ);
		أرجع نتیجہ;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتیجہ);

	إذا (resched) {
		z_reschedule(&msgq->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&msgq->جندرا, کلید);
	}

	أرجع نتیجہ;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_get(بنية k_msgq *msgq, فراغ *انکڑے,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(انکڑے, msgq->پیغام_ناپ));

	أرجع z_impl_k_msgq_get(msgq, انکڑے, timeout);
}
#ضمن <zephyr/syscalls/k_msgq_get_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek(بنية k_msgq *msgq, فراغ *انکڑے)
{
	k_spinlock_کلید_t کلید;
	صحيح نتیجہ;

	کلید = k_spin_جندرا(&msgq->جندرا);

	إذا (msgq->ورتیا_msgs > 0U) {
		/* take first available message from queue */
		(فراغ)حافظہ_نقل(انکڑے, msgq->read_ptr, msgq->پیغام_ناپ);
		نتیجہ = 0;
	} وإلا {
		/* don't wait for a message to become available */
		نتیجہ = -ENOMSG;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتیجہ);

	k_spin_unlock(&msgq->جندرا, کلید);

	أرجع نتیجہ;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek(بنية k_msgq *msgq, فراغ *انکڑے)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(انکڑے, msgq->پیغام_ناپ));

	أرجع z_impl_k_msgq_peek(msgq, انکڑے);
}
#ضمن <zephyr/syscalls/k_msgq_peek_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_impl_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *انکڑے, uint32_t idx)
{
	k_spinlock_کلید_t کلید;
	صحيح نتیجہ;
	uint32_t بائٹاں_منزل_سرا;
	uint32_t byte_وتھ;
	حرف *شروع_addr;

	کلید = k_spin_جندرا(&msgq->جندرا);

	إذا (msgq->ورتیا_msgs > idx) {
		بائٹاں_منزل_سرا = (msgq->بفر_سرا - msgq->read_ptr);
		byte_وتھ = idx * msgq->پیغام_ناپ;
		شروع_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		إذا (بائٹاں_منزل_سرا <= byte_وتھ) {
			/* Tweak the values in case */
			byte_وتھ -= بائٹاں_منزل_سرا;
			/* wrap-around is required */
			شروع_addr = msgq->بفر_شروع;
		}
		(فراغ)حافظہ_نقل(انکڑے, شروع_addr + byte_وتھ, msgq->پیغام_ناپ);
		نتیجہ = 0;
	} وإلا {
		/* don't wait for a message to become available */
		نتیجہ = -ENOMSG;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتیجہ);

	k_spin_unlock(&msgq->جندرا, کلید);

	أرجع نتیجہ;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_msgq_peek_at(بنية k_msgq *msgq, فراغ *انکڑے, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(انکڑے, msgq->پیغام_ناپ));

	أرجع z_impl_k_msgq_peek_at(msgq, انکڑے, idx);
}
#ضمن <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

فراغ z_impl_k_msgq_purge(بنية k_msgq *msgq)
{
	k_spinlock_کلید_t کلید;
	bool resched = false;

	کلید = k_spin_جندرا(&msgq->جندرا);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	طالما (z_sched_wake(&msgq->wait_q, -ENOMSG, عدم)) {
		resched = true;
	}

	msgq->ورتیا_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	إذا (resched) {
		z_reschedule(&msgq->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&msgq->جندرا, کلید);
	}
}

#جے_تعریف_شدہ CONFIG_USERSPACE
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

ساكن مضمن uint32_t z_vrfy_k_msgq_عدد_ورتیا_get(بنية k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	أرجع z_impl_k_msgq_عدد_ورتیا_get(msgq);
}
#ضمن <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#جے_انت /* CONFIG_USERSPACE */
