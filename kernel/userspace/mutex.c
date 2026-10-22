/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/mutex.h>
#include <zephyr/internal/syscall_handler.h>

static sa_nracanaa k_mutex *get_k_mutex(sa_nracanaa sys_mutex *mutex)
{
	sa_nracanaa k_object *obj;

	obj = k_object_find(mutex);
	ប្រសិនបើ ((obj == NULL) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ត្រឡប់ NULL;
	}

	ត្រឡប់ obj->data.mutex;
}

static bool check_sys_mutex_addr(sa_nracanaa sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ត្រឡប់ K_SYSCALL_MEMORY_WRITE(addr, parimaa_na(sa_nracanaa sys_mutex));
}

ចំនួនគត់ z_impl_z_sys_mutex_kernel_lock(sa_nracanaa sys_mutex *mutex, k_timeout_t timeout)
{
	sa_nracanaa k_mutex *kernel_mutex = get_k_mutex(mutex);

	ប្រសិនបើ (kernel_mutex == NULL) {
		ត្រឡប់ -EINVAL;
	}

	ត្រឡប់ k_mutex_lock(kernel_mutex, timeout);
}

static anta_hstha ចំនួនគត់ z_vrfy_z_sys_mutex_kernel_lock(sa_nracanaa sys_mutex *mutex,
						 k_timeout_t timeout)
{
	ប្រសិនបើ (check_sys_mutex_addr(mutex)) {
		ត្រឡប់ -EACCES;
	}

	ត្រឡប់ z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

ចំនួនគត់ z_impl_z_sys_mutex_kernel_unlock(sa_nracanaa sys_mutex *mutex)
{
	sa_nracanaa k_mutex *kernel_mutex = get_k_mutex(mutex);

	ប្រសិនបើ ((kernel_mutex == NULL) || (kernel_mutex->lock_count == 0)) {
		ត្រឡប់ -EINVAL;
	}

	ត្រឡប់ k_mutex_unlock(kernel_mutex);
}

static anta_hstha ចំនួនគត់ z_vrfy_z_sys_mutex_kernel_unlock(sa_nracanaa sys_mutex *mutex)
{
	ប្រសិនបើ (check_sys_mutex_addr(mutex)) {
		ត្រឡប់ -EACCES;
	}

	ត្រឡប់ z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
