/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/mutex.h>
#include <zephyr/internal/syscall_handler.h>

static struct k_mutex *get_k_mutex(struct sys_mutex *mutex)
{
	struct k_object *obj;

	obj = k_object_find(mutex);
	如果 ((obj == NULL) || (obj->type != K_OBJ_SYS_MUTEX)) {
		返回 NULL;
	}

	返回 obj->data.mutex;
}

static bool check_sys_mutex_addr(struct sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	返回 K_SYSCALL_MEMORY_WRITE(addr, sizeof(struct sys_mutex));
}

整数 z_impl_z_sys_mutex_kernel_lock(struct sys_mutex *mutex, k_timeout_t timeout)
{
	struct k_mutex *kernel_mutex = get_k_mutex(mutex);

	如果 (kernel_mutex == NULL) {
		返回 -EINVAL;
	}

	返回 k_mutex_lock(kernel_mutex, timeout);
}

static inline 整数 z_vrfy_z_sys_mutex_kernel_lock(struct sys_mutex *mutex,
						 k_timeout_t timeout)
{
	如果 (check_sys_mutex_addr(mutex)) {
		返回 -EACCES;
	}

	返回 z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

整数 z_impl_z_sys_mutex_kernel_unlock(struct sys_mutex *mutex)
{
	struct k_mutex *kernel_mutex = get_k_mutex(mutex);

	如果 ((kernel_mutex == NULL) || (kernel_mutex->lock_count == 0)) {
		返回 -EINVAL;
	}

	返回 k_mutex_unlock(kernel_mutex);
}

static inline 整数 z_vrfy_z_sys_mutex_kernel_unlock(struct sys_mutex *mutex)
{
	如果 (check_sys_mutex_addr(mutex)) {
		返回 -EACCES;
	}

	返回 z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
