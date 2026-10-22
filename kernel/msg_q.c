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

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_MSGQ */

ساکن اندرونی bool msgq_handle_poll_events(ساخت k_msgq *msgq)
{
#اگر_تعریف_شدہ CONFIG_POLL
	واپس z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_ڈیٹا_دستیاب);
#ورنہ
	ARG_غیر_مستعمل(msgq);
	واپس false;
#اگر_ختم /* CONFIG_POLL */
}

خالی k_msgq_init(ساخت k_msgq *msgq, حرف *بفر, size_t پیغام_سائز,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!سائز_mul_حد_سے_تجاوز(max_msgs, پیغام_سائز, &(size_t){0}));
	__ASSERT_NO_MSG(!سائز_جوڑو_حد_سے_تجاوز((size_t)(uintptr_t)بفر, max_msgs * پیغام_سائز,
					&(size_t){0}));

	msgq->پیغام_سائز = پیغام_سائز;
	msgq->max_msgs = max_msgs;
	msgq->بفر_آغاز = بفر;
	msgq->بفر_انتہا = بفر + (max_msgs * پیغام_سائز);
	msgq->read_ptr = بفر;
	msgq->write_ptr = بفر;
	msgq->مستعمل_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->تالا = (ساخت k_spinlock) {};
#اگر_تعریف_شدہ CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#اگر_ختم	/* CONFIG_POLL */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#اگر_ختم /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورٹ_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

صحیح z_impl_k_msgq_alloc_init(ساخت k_msgq *msgq, size_t پیغام_سائز,
			    uint32_t max_msgs)
{
	خالی *بفر;
	صحیح ret;
	size_t کل_سائز;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	اگر (سائز_mul_حد_سے_تجاوز(پیغام_سائز, max_msgs, &کل_سائز)) {
		ret = -EINVAL;
	} ورنہ {
		بفر = z_thread_malloc(کل_سائز);
		اگر (بفر != تہی) {
			k_msgq_init(msgq, بفر, پیغام_سائز, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} ورنہ {
			ret = -ENOMEM;
		}
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_msgq_alloc_init(ساخت k_msgq *msgq, size_t پیغام_سائز,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	واپس z_impl_k_msgq_alloc_init(msgq, پیغام_سائز, max_msgs);
}
#شامل <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_msgq_cleanup(ساخت k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	صحیح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_تالا(&msgq->تالا);

	CHECKIF(locked && (z_waitq_سر_locked(&msgq->wait_q) != تہی)) {
		ret = -EBUSY;
		جاؤ برآمد;
	}

	CHECKIF(!locked && (z_waitq_سر(&msgq->wait_q) != تہی)) {
		ret = -EBUSY;
		جاؤ برآمد;
	}

	اگر ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->بفر_آغاز);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

برآمد:
	k_spin_unlock(&msgq->تالا, کلید);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_MSGQ
	اگر (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#اگر_ختم /* CONFIG_OBJ_CORE_MSGQ */

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	واپس ret;
}

صحیح k_msgq_cleanup(ساخت k_msgq *msgq)
{
	واپس z_msgq_cleanup(msgq, false);
}

ساکن اندرونی صحیح put_پیغام_اندر_قطار(ساخت k_msgq *msgq, ثابت خالی *ڈیٹا,
			k_timeout_t timeout, bool put_at_عقب)
{
	__ASSERT(!arch_is_اندر_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	ساخت k_thread *pending_thread = تہی;
	k_spinlock_کلید_t کلید;
	صحیح نتیجہ;
	bool resched = false;

	کلید = k_spin_تالا(&msgq->تالا);

	اگر (put_at_عقب) {
		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} ورنہ {
		SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	اگر (msgq->مستعمل_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		تالا_SCHED_SPINLOCK {
			pending_thread = z_unpend_پہلا_thread_locked(&msgq->wait_q);
			اگر (pending_thread != تہی) {
				/* copy into the receiver's buffer */
				(خالی)حافظہ_نقل(pending_thread->بنیاد.تبادلہ_ڈیٹا, ڈیٹا,
					     msgq->پیغام_سائز);
				arch_thread_return_قدر_سیٹ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		اگر (pending_thread == تہی) {
			size_t پیغام_سائز = msgq->پیغام_سائز;
			حرف *خانہ;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_آغاز) &&
					(msgq->write_ptr <= (msgq->بفر_انتہا - 1)) &&
					((size_t)(uintptr_t)(msgq->بفر_انتہا - msgq->write_ptr) >=
						msgq->پیغام_سائز));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			اگر (put_at_عقب) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				حرف *اگلا;

				خانہ = msgq->write_ptr;
				اگلا = خانہ + پیغام_سائز;
				اگر (اگلا == msgq->بفر_انتہا) {
					اگلا = msgq->بفر_آغاز;
				}
				msgq->write_ptr = اگلا;
			} ورنہ {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				خانہ = msgq->read_ptr;
				اگر (خانہ == msgq->بفر_آغاز) {
					خانہ = msgq->بفر_انتہا;
				}
				خانہ -= پیغام_سائز;
				msgq->read_ptr = خانہ;
			}
			msgq->مستعمل_msgs++;
			(خالی)حافظہ_نقل(خانہ, ڈیٹا, پیغام_سائز);
			resched = msgq_handle_poll_events(msgq);
		}
		نتیجہ = 0;
	} ورنہ اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		نتیجہ = -ENOMSG;
	} ورنہ {
		اگر (put_at_عقب) {
			SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} ورنہ {
			SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->بنیاد.تبادلہ_ڈیٹا = (خالی *) ڈیٹا;

		نتیجہ = z_pend_curr(&msgq->تالا, کلید, &msgq->wait_q, timeout);

		اگر (put_at_عقب) {
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتیجہ);
		} ورنہ {
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتیجہ);
		}

		واپس نتیجہ;
	}

	اگر (put_at_عقب) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, نتیجہ);
	} ورنہ {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, نتیجہ);
	}

	اگر (resched) {
		z_reschedule(&msgq->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&msgq->تالا, کلید);
	}

	واپس نتیجہ;
}


صحیح z_impl_k_msgq_put(ساخت k_msgq *msgq, ثابت خالی *ڈیٹا, k_timeout_t timeout)
{
	واپس put_پیغام_اندر_قطار(msgq, ڈیٹا, timeout, true);
}

صحیح z_impl_k_msgq_put_front(ساخت k_msgq *msgq, ثابت خالی *ڈیٹا)
{
	واپس put_پیغام_اندر_قطار(msgq, ڈیٹا, K_NO_WAIT, false);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_msgq_put(ساخت k_msgq *msgq, ثابت خالی *ڈیٹا,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_READ(ڈیٹا, msgq->پیغام_سائز));

	واپس z_impl_k_msgq_put(msgq, ڈیٹا, timeout);
}
#شامل <zephyr/syscalls/k_msgq_put_mrsh.c>

ساکن اندرونی صحیح z_vrfy_k_msgq_put_front(ساخت k_msgq *msgq, ثابت خالی *ڈیٹا)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_READ(ڈیٹا, msgq->پیغام_سائز));

	واپس z_impl_k_msgq_put_front(msgq, ڈیٹا);
}
#شامل <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_impl_k_msgq_get_attrs(ساخت k_msgq *msgq, ساخت k_msgq_attrs *attrs)
{
	attrs->پیغام_سائز = msgq->پیغام_سائز;
	attrs->max_msgs = msgq->max_msgs;
	attrs->مستعمل_msgs = msgq->مستعمل_msgs;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_msgq_get_attrs(ساخت k_msgq *msgq,
					   ساخت k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(attrs, حجم(ساخت k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#شامل <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_msgq_get(ساخت k_msgq *msgq, خالی *ڈیٹا, k_timeout_t timeout)
{
	__ASSERT(!arch_is_اندر_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_کلید_t کلید;
	ساخت k_thread *pending_thread;
	صحیح نتیجہ;
	bool resched = false;

	کلید = k_spin_تالا(&msgq->تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	اگر (likely(msgq->مستعمل_msgs > 0U)) {
		size_t پیغام_سائز = msgq->پیغام_سائز;
		حرف *خانہ = msgq->read_ptr;
		حرف *اگلا = خانہ + پیغام_سائز;

		/* drop the message from the queue, then copy it out */
		اگر (اگلا == msgq->بفر_انتہا) {
			اگلا = msgq->بفر_آغاز;
		}
		msgq->read_ptr = اگلا;
		msgq->مستعمل_msgs--;

		(خالی)حافظہ_نقل(ڈیٹا, خانہ, پیغام_سائز);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->بفر_آغاز) &&
				(msgq->write_ptr <= (msgq->بفر_انتہا - 1)) &&
				((size_t)(uintptr_t)(msgq->بفر_انتہا - msgq->write_ptr) >=
					msgq->پیغام_سائز));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		تالا_SCHED_SPINLOCK {
			pending_thread = z_unpend_پہلا_thread_locked(&msgq->wait_q);
			اگر (pending_thread != تہی) {
				SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				خانہ = msgq->write_ptr;
				اگلا = خانہ + پیغام_سائز;
				اگر (اگلا == msgq->بفر_انتہا) {
					اگلا = msgq->بفر_آغاز;
				}
				msgq->write_ptr = اگلا;
				msgq->مستعمل_msgs++;

				(خالی)حافظہ_نقل(خانہ, pending_thread->بنیاد.تبادلہ_ڈیٹا, پیغام_سائز);

				arch_thread_return_قدر_سیٹ(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		نتیجہ = 0;
	} ورنہ اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		نتیجہ = -ENOMSG;
	} ورنہ {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->بنیاد.تبادلہ_ڈیٹا = ڈیٹا;

		نتیجہ = z_pend_curr(&msgq->تالا, کلید, &msgq->wait_q, timeout);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتیجہ);
		واپس نتیجہ;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, نتیجہ);

	اگر (resched) {
		z_reschedule(&msgq->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&msgq->تالا, کلید);
	}

	واپس نتیجہ;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_msgq_get(ساخت k_msgq *msgq, خالی *ڈیٹا,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(ڈیٹا, msgq->پیغام_سائز));

	واپس z_impl_k_msgq_get(msgq, ڈیٹا, timeout);
}
#شامل <zephyr/syscalls/k_msgq_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_msgq_peek(ساخت k_msgq *msgq, خالی *ڈیٹا)
{
	k_spinlock_کلید_t کلید;
	صحیح نتیجہ;

	کلید = k_spin_تالا(&msgq->تالا);

	اگر (msgq->مستعمل_msgs > 0U) {
		/* take first available message from queue */
		(خالی)حافظہ_نقل(ڈیٹا, msgq->read_ptr, msgq->پیغام_سائز);
		نتیجہ = 0;
	} ورنہ {
		/* don't wait for a message to become available */
		نتیجہ = -ENOMSG;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتیجہ);

	k_spin_unlock(&msgq->تالا, کلید);

	واپس نتیجہ;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_msgq_peek(ساخت k_msgq *msgq, خالی *ڈیٹا)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(ڈیٹا, msgq->پیغام_سائز));

	واپس z_impl_k_msgq_peek(msgq, ڈیٹا);
}
#شامل <zephyr/syscalls/k_msgq_peek_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_msgq_peek_at(ساخت k_msgq *msgq, خالی *ڈیٹا, uint32_t idx)
{
	k_spinlock_کلید_t کلید;
	صحیح نتیجہ;
	uint32_t بائٹیں_مقصد_انتہا;
	uint32_t byte_انحراف;
	حرف *آغاز_addr;

	کلید = k_spin_تالا(&msgq->تالا);

	اگر (msgq->مستعمل_msgs > idx) {
		بائٹیں_مقصد_انتہا = (msgq->بفر_انتہا - msgq->read_ptr);
		byte_انحراف = idx * msgq->پیغام_سائز;
		آغاز_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		اگر (بائٹیں_مقصد_انتہا <= byte_انحراف) {
			/* Tweak the values in case */
			byte_انحراف -= بائٹیں_مقصد_انتہا;
			/* wrap-around is required */
			آغاز_addr = msgq->بفر_آغاز;
		}
		(خالی)حافظہ_نقل(ڈیٹا, آغاز_addr + byte_انحراف, msgq->پیغام_سائز);
		نتیجہ = 0;
	} ورنہ {
		/* don't wait for a message to become available */
		نتیجہ = -ENOMSG;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC(k_msgq, peek, msgq, نتیجہ);

	k_spin_unlock(&msgq->تالا, کلید);

	واپس نتیجہ;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_msgq_peek_at(ساخت k_msgq *msgq, خالی *ڈیٹا, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(ڈیٹا, msgq->پیغام_سائز));

	واپس z_impl_k_msgq_peek_at(msgq, ڈیٹا, idx);
}
#شامل <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

خالی z_impl_k_msgq_purge(ساخت k_msgq *msgq)
{
	k_spinlock_کلید_t کلید;
	bool resched = false;

	کلید = k_spin_تالا(&msgq->تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	جب (z_sched_wake(&msgq->wait_q, -ENOMSG, تہی)) {
		resched = true;
	}

	msgq->مستعمل_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	اگر (resched) {
		z_reschedule(&msgq->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&msgq->تالا, کلید);
	}
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_msgq_purge(ساخت k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#شامل <zephyr/syscalls/k_msgq_purge_mrsh.c>

ساکن اندرونی uint32_t z_vrfy_k_msgq_عدد_free_get(ساخت k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	واپس z_impl_k_msgq_عدد_free_get(msgq);
}
#شامل <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

ساکن اندرونی uint32_t z_vrfy_k_msgq_عدد_مستعمل_get(ساخت k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	واپس z_impl_k_msgq_عدد_مستعمل_get(msgq);
}
#شامل <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#اگر_ختم /* CONFIG_USERSPACE */
