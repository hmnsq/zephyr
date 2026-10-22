/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <wait_q.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <kernel_internal.h>
#ضمن <stdbool.h>

#ifdef CONFIG_OBJ_CORE_مكدس
K_OBJ_TYPE_DEFINE(obj_type_مكدس, k_مكدس, K_OBJ_TYPE_مكدس_ID, عدم);
#endif /* CONFIG_OBJ_CORE_STACK */

فراغ k_مكدس_init(بنية k_مكدس *مكدس, مكدس_data_t *buffer,
		  uint32_t رقم_entries)
{
	z_waitq_init(&مكدس->wait_q);
	مكدس->lock = (بنية k_spinlock) {};
	مكدس->next = buffer;
	مكدس->base = buffer;
	مكدس->قمة = مكدس->base + رقم_entries;
	مكدس->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_مكدس, مكدس);
	k_object_init(مكدس);

#ifdef CONFIG_OBJ_CORE_مكدس
	k_obj_core_init_and_link(K_OBJ_CORE(مكدس), &obj_type_مكدس);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_مكدس_alloc_init(بنية k_مكدس *مكدس, uint32_t رقم_entries)
{
	فراغ *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_مكدس, alloc_init, مكدس);

	/* Reject allocation sizes that cannot be represented. */
	إذا (size_mul_overflow(رقم_entries, حجم(مكدس_data_t), &total_size)) {
		ret = -ENOMEM;
	} وإلا {
		buffer = z_thread_malloc(total_size);
		إذا (buffer != عدم) {
			k_مكدس_init(مكدس, buffer, رقم_entries);
			مكدس->flags = K_مكدس_FLAG_ALLOC;
			ret = 0;
		} وإلا {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_مكدس, alloc_init, مكدس, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_مكدس_alloc_init(بنية k_مكدس *مكدس,
					      uint32_t رقم_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(مكدس, K_OBJ_مكدس));
	K_OOPS(K_SYSCALL_VERIFY(رقم_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(رقم_entries, حجم(مكدس_data_t),
					&total_size)));
	أرجع z_impl_k_مكدس_alloc_init(مكدس, رقم_entries);
}
#ضمن <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_مكدس_cleanup(بنية k_مكدس *مكدس, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_مكدس, cleanup, مكدس);

	صحيح ret = 0;
	bool freed = false;
	k_spinlock_مفتاح_t مفتاح = k_spin_lock(&مكدس->lock);

	إذا (locked && (z_waitq_head_locked(&مكدس->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا (!locked && (z_waitq_head(&مكدس->wait_q) != عدم)) {
		ret = -EAGAIN;
		اذهب out;
	}

	إذا ((مكدس->flags & K_مكدس_FLAG_ALLOC) != (uint8_t)0) {
		k_free(مكدس->base);
		مكدس->base = عدم;
		مكدس->flags &= ~K_مكدس_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&مكدس->lock, مفتاح);

#ifdef CONFIG_OBJ_CORE_مكدس
	إذا (freed) {
		k_obj_core_unlink(K_OBJ_CORE(مكدس));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_مكدس, cleanup, مكدس, ret);

	أرجع ret;
}

صحيح k_مكدس_cleanup(بنية k_مكدس *مكدس)
{
	أرجع z_مكدس_cleanup(مكدس, false);
}

صحيح z_impl_k_مكدس_ادفع(بنية k_مكدس *مكدس, مكدس_data_t data)
{
	صحيح ret = 0;
	k_spinlock_مفتاح_t مفتاح = k_spin_lock(&مكدس->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_مكدس, ادفع, مكدس);

	إذا (مكدس->next == مكدس->قمة) {
		ret = -ENOMEM;
		اذهب out;
	}

	إذا (z_sched_wake(&مكدس->wait_q, 0, (فراغ *)data)) {
		z_reschedule(&مكدس->lock, مفتاح);
		اذهب end;
	} وإلا {
		*(مكدس->next) = data;
		مكدس->next++;
		اذهب out;
	}

out:
	k_spin_unlock(&مكدس->lock, مفتاح);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_مكدس, ادفع, مكدس, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_مكدس_ادفع(بنية k_مكدس *مكدس, مكدس_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(مكدس, K_OBJ_مكدس));

	أرجع z_impl_k_مكدس_ادفع(مكدس, data);
}
#ضمن <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_مكدس_اسحب(بنية k_مكدس *مكدس, مكدس_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_مفتاح_t مفتاح;
	صحيح result;

	مفتاح = k_spin_lock(&مكدس->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_مكدس, اسحب, مكدس, timeout);

	إذا (likely(مكدس->next > مكدس->base)) {
		مكدس->next--;
		*data = *(مكدس->next);
		k_spin_unlock(&مكدس->lock, مفتاح);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_مكدس, اسحب, مكدس, timeout, 0);

		أرجع 0;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&مكدس->lock, مفتاح);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_مكدس, اسحب, مكدس, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_مكدس, اسحب, مكدس, timeout);

	result = z_pend_curr(&مكدس->lock, مفتاح, &مكدس->wait_q, timeout);
	إذا (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_مكدس, اسحب, مكدس, timeout, result);

		أرجع result;
	}

	*data = (مكدس_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_مكدس, اسحب, مكدس, timeout, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_مكدس_اسحب(بنية k_مكدس *مكدس,
				     مكدس_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(مكدس, K_OBJ_مكدس));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, حجم(مكدس_data_t)));
	أرجع z_impl_k_مكدس_اسحب(مكدس, data, timeout);
}
#ضمن <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
