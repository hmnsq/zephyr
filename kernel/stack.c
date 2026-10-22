/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief fixed-size stack object
 */

#include <zephyr/sys/math_extras.h>
#include <zephyr/kernel.h>

#include <zephyr/toolchain.h>
#include <ksched.h>
#include <scheduler.h>
#include <wait_q.h>
#include <zephyr/init.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_internal.h>
#include <stdbool.h>

#ifdef CONFIG_OBJ_CORE_सालानसान
K_OBJ_TYPE_DEFINE(obj_type_सालानसान, k_सालानसान, K_OBJ_TYPE_सालानसान_ID, NULL);
#endif /* CONFIG_OBJ_CORE_STACK */

void k_सालानसान_init(struct k_सालानसान *सालानसान, सालानसान_data_t *buffer,
		  uint32_t नुमेरो_entries)
{
	z_waitq_init(&सालानसान->wait_q);
	सालानसान->lock = (struct k_spinlock) {};
	सालानसान->next = buffer;
	सालानसान->base = buffer;
	सालानसान->इताअस = सालानसान->base + नुमेरो_entries;
	सालानसान->flags = 0;

	SYS_PORT_TRACING_OBJ_INIT(k_सालानसान, सालानसान);
	k_object_init(सालानसान);

#ifdef CONFIG_OBJ_CORE_सालानसान
	k_obj_core_init_and_link(K_OBJ_CORE(सालानसान), &obj_type_सालानसान);
#endif /* CONFIG_OBJ_CORE_STACK */
}

int32_t z_impl_k_सालानसान_alloc_init(struct k_सालानसान *सालानसान, uint32_t नुमेरो_entries)
{
	void *buffer;
	size_t total_size;
	int32_t ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_सालानसान, alloc_init, सालानसान);

	/* Reject allocation sizes that cannot be represented. */
	if (size_mul_overflow(नुमेरो_entries, sizeof(सालानसान_data_t), &total_size)) {
		ret = -ENOMEM;
	} else {
		buffer = z_thread_malloc(total_size);
		if (buffer != NULL) {
			k_सालानसान_init(सालानसान, buffer, नुमेरो_entries);
			सालानसान->flags = K_सालानसान_FLAG_ALLOC;
			ret = 0;
		} else {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_सालानसान, alloc_init, सालानसान, ret);

	return ret;
}

#ifdef CONFIG_USERSPACE
static inline int32_t z_vrfy_k_सालानसान_alloc_init(struct k_सालानसान *सालानसान,
					      uint32_t नुमेरो_entries)
{
	size_t total_size;

	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(सालानसान, K_OBJ_सालानसान));
	K_OOPS(K_SYSCALL_VERIFY(नुमेरो_entries > 0));
	K_OOPS(K_SYSCALL_VERIFY(!size_mul_overflow(नुमेरो_entries, sizeof(सालानसान_data_t),
					&total_size)));
	return z_impl_k_सालानसान_alloc_init(सालानसान, नुमेरो_entries);
}
#include <zephyr/syscalls/k_stack_alloc_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

int z_सालानसान_cleanup(struct k_सालानसान *सालानसान, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_सालानसान, cleanup, सालानसान);

	int ret = 0;
	bool freed = false;
	k_spinlock_सुसि_t सुसि = k_spin_lock(&सालानसान->lock);

	if (locked && (z_waitq_head_locked(&सालानसान->wait_q) != NULL)) {
		ret = -EAGAIN;
		goto out;
	}

	if (!locked && (z_waitq_head(&सालानसान->wait_q) != NULL)) {
		ret = -EAGAIN;
		goto out;
	}

	if ((सालानसान->flags & K_सालानसान_FLAG_ALLOC) != (uint8_t)0) {
		k_free(सालानसान->base);
		सालानसान->base = NULL;
		सालानसान->flags &= ~K_सालानसान_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&सालानसान->lock, सुसि);

#ifdef CONFIG_OBJ_CORE_सालानसान
	if (freed) {
		k_obj_core_unlink(K_OBJ_CORE(सालानसान));
	}
#endif /* CONFIG_OBJ_CORE_STACK */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_सालानसान, cleanup, सालानसान, ret);

	return ret;
}

int k_सालानसान_cleanup(struct k_सालानसान *सालानसान)
{
	return z_सालानसान_cleanup(सालानसान, false);
}

int z_impl_k_सालानसान_इतुलाक(struct k_सालानसान *सालानसान, सालानसान_data_t data)
{
	int ret = 0;
	k_spinlock_सुसि_t सुसि = k_spin_lock(&सालानसान->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_सालानसान, इतुलाक, सालानसान);

	if (सालानसान->next == सालानसान->इताअस) {
		ret = -ENOMEM;
		goto out;
	}

	if (z_sched_wake(&सालानसान->wait_q, 0, (void *)data)) {
		z_reschedule(&सालानसान->lock, सुसि);
		goto end;
	} else {
		*(सालानसान->next) = data;
		सालानसान->next++;
		goto out;
	}

out:
	k_spin_unlock(&सालानसान->lock, सुसि);

end:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_सालानसान, इतुलाक, सालानसान, ret);

	return ret;
}

#ifdef CONFIG_USERSPACE
static inline int z_vrfy_k_सालानसान_इतुलाक(struct k_सालानसान *सालानसान, सालानसान_data_t data)
{
	K_OOPS(K_SYSCALL_OBJ(सालानसान, K_OBJ_सालानसान));

	return z_impl_k_सालानसान_इतुलाक(सालानसान, data);
}
#include <zephyr/syscalls/k_stack_push_mrsh.c>
#endif /* CONFIG_USERSPACE */

int z_impl_k_सालानसान_आलिसिन(struct k_सालानसान *सालानसान, सालानसान_data_t *data,
		       k_timeout_t timeout)
{
	k_spinlock_सुसि_t सुसि;
	int result;

	सुसि = k_spin_lock(&सालानसान->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_सालानसान, आलिसिन, सालानसान, timeout);

	if (likely(सालानसान->next > सालानसान->base)) {
		सालानसान->next--;
		*data = *(सालानसान->next);
		k_spin_unlock(&सालानसान->lock, सुसि);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_सालानसान, आलिसिन, सालानसान, timeout, 0);

		return 0;
	}

	if (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&सालानसान->lock, सुसि);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_सालानसान, आलिसिन, सालानसान, timeout, -EBUSY);

		return -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_सालानसान, आलिसिन, सालानसान, timeout);

	result = z_pend_curr(&सालानसान->lock, सुसि, &सालानसान->wait_q, timeout);
	if (result != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_सालानसान, आलिसिन, सालानसान, timeout, result);

		return result;
	}

	*data = (सालानसान_data_t)_current->base.swap_data;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_सालानसान, आलिसिन, सालानसान, timeout, 0);

	return 0;
}

#ifdef CONFIG_USERSPACE
static inline int z_vrfy_k_सालानसान_आलिसिन(struct k_सालानसान *सालानसान,
				     सालानसान_data_t *data, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(सालानसान, K_OBJ_सालानसान));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, sizeof(सालानसान_data_t)));
	return z_impl_k_सालानसान_आलिसिन(सालानसान, data, timeout);
}
#include <zephyr/syscalls/k_stack_pop_mrsh.c>
#endif /* CONFIG_USERSPACE */
