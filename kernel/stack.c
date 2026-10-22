/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <wait_q.h>
#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <kernel_internal.h>
#شامل <stdbool.h>

#ifdef CONFIG_OBJ_CORE_پشته
K_OBJ_TYPE_DEFINE(obj_type_پشته, k_پشته, K_OBJ_TYPE_پشته_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_STACK */

تهی k_پشته_init(ساختار k_پشته *پشته, پشته_data_t *buffer,
		  uint32_t رقم_entries)
{
	z_waitq_init(&پشته->wait_q);
	پشته->lock = (ساختار k_spinlock) {};
	پشته->next = buffer;
	پشته->base = buffer;
	پشته->بالاترین = پشته->base + رقم_entries;
	پشته->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_پشته, پشته);
	k_object_init(پشته);

#ifdef CONFIG_OBJ_CORE_پشته
	k_obj_core_init_and_link(K_OBJ_CORE(پشته), &obj_type_پشته);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_پشته_alloc_init(ساختار k_پشته *پشته, uint32_t رقم_entries)
{
	تهی *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_پشته, alloc_init, پشته);

	/* Reject allocation sizes that cannot be represented. */
	اگر (size_mul_overflow(رقم_entries, اندازه(پشته_data_t), &total_size)) {
		ret = -ENOMEM;
	} وگرنه {
		buffer = z_thread_malloc(total_size);
		اگر (buffer != پوچ) {
			k_پشته_init(پشته, buffer, رقم_entries);
			پشته->flags = K_پشته_FLAG_ALLOC;
			ret = 0;
		} وگرنه {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_پشته, alloc_init, پشته, ret);

	بازگشت ret;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی int32_t z_vrfy_k_پشته_alloc_init(ساختار k_پشته *پشته,
					      uint32_t رقم_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(پشته, K_OBJ_پشته));
	K_OOPS(K_SYSCALL_VERIFY(رقم_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(رقم_entries, اندازه(پشته_data_t),
					&total_size)));
	بازگشت z_impl_k_پشته_alloc_init(پشته, رقم_entries);
}
#شامل <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_پشته_cleanup(ساختار k_پشته *پشته, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_پشته, cleanup, پشته);

	صحیح ret = 0;
	bool freed = false;
	k_spinlock_کلید_t کلید = k_spin_lock(&پشته->lock);

	اگر (locked && (z_waitq_head_locked(&پشته->wait_q) != پوچ)) {
		ret = -EAGAIN;
		برو out;
	}

	اگر (!locked && (z_waitq_head(&پشته->wait_q) != پوچ)) {
		ret = -EAGAIN;
		برو out;
	}

	اگر ((پشته->flags & K_پشته_FLAG_ALLOC) != (uint8_t)0) {
		k_free(پشته->base);
		پشته->base = پوچ;
		پشته->flags &= ~K_پشته_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&پشته->lock, کلید);

#ifdef CONFIG_OBJ_CORE_پشته
	اگر (freed) {
		k_obj_core_unlink(K_OBJ_CORE(پشته));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_پشته, cleanup, پشته, ret);

	بازگشت ret;
}

صحیح k_پشته_cleanup(ساختار k_پشته *پشته)
{
	بازگشت z_پشته_cleanup(پشته, false);
}

صحیح z_impl_k_پشته_افزودن(ساختار k_پشته *پشته, پشته_data_t data)
{
	صحیح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_lock(&پشته->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_پشته, افزودن, پشته);

	اگر (پشته->next == پشته->بالاترین) {
		ret = -ENOMEM;
		برو out;
	}

	اگر (z_sched_wake(&پشته->wait_q, 0, (تهی *)data)) {
		z_reschedule(&پشته->lock, کلید);
		برو end;
	} وگرنه {
		*(پشته->next) = data;
		پشته->next++;
		برو out;
	}

out:
	k_spin_unlock(&پشته->lock, کلید);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_پشته, افزودن, پشته, ret);

	بازگشت ret;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_پشته_افزودن(ساختار k_پشته *پشته, پشته_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(پشته, K_OBJ_پشته));

	بازگشت z_impl_k_پشته_افزودن(پشته, data);
}
#شامل <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_پشته_برداشت(ساختار k_پشته *پشته, پشته_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحیح result;

	کلید = k_spin_lock(&پشته->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_پشته, برداشت, پشته, timeout);

	اگر (likely(پشته->next > پشته->base)) {
		پشته->next--;
		*data = *(پشته->next);
		k_spin_unlock(&پشته->lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, 0);

		بازگشت 0;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&پشته->lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, -EBUSY);

		بازگشت -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_پشته, برداشت, پشته, timeout);

	result = z_pend_curr(&پشته->lock, کلید, &پشته->wait_q, timeout);
	اگر (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, result);

		بازگشت result;
	}

	*data = (پشته_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_پشته, برداشت, پشته, timeout, 0);

	بازگشت 0;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_پشته_برداشت(ساختار k_پشته *پشته,
				     پشته_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(پشته, K_OBJ_پشته));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, اندازه(پشته_data_t)));
	بازگشت z_impl_k_پشته_برداشت(پشته, data, timeout);
}
#شامل <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
