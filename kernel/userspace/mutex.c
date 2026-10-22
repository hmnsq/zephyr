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
	ᬬᬾᬦ᭄ ((obj == NULL) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ᬯᬮᬶ NULL;
	}

	ᬯᬮᬶ obj->data.mutex;
}

static bool check_sys_mutex_addr(sa_nracanaa sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ᬯᬮᬶ K_SYSCALL_MEMORY_WRITE(addr, parimaa_na(sa_nracanaa sys_mutex));
}

puur_naa_nka z_impl_z_sys_mutex_kernel_lock(sa_nracanaa sys_mutex *mutex, k_timeout_t timeout)
{
	sa_nracanaa k_mutex *kernel_mutex = get_k_mutex(mutex);

	ᬬᬾᬦ᭄ (kernel_mutex == NULL) {
		ᬯᬮᬶ -EINVAL;
	}

	ᬯᬮᬶ k_mutex_lock(kernel_mutex, timeout);
}

static anta_hstha puur_naa_nka z_vrfy_z_sys_mutex_kernel_lock(sa_nracanaa sys_mutex *mutex,
						 k_timeout_t timeout)
{
	ᬬᬾᬦ᭄ (check_sys_mutex_addr(mutex)) {
		ᬯᬮᬶ -EACCES;
	}

	ᬯᬮᬶ z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

puur_naa_nka z_impl_z_sys_mutex_kernel_unlock(sa_nracanaa sys_mutex *mutex)
{
	sa_nracanaa k_mutex *kernel_mutex = get_k_mutex(mutex);

	ᬬᬾᬦ᭄ ((kernel_mutex == NULL) || (kernel_mutex->lock_count == 0)) {
		ᬯᬮᬶ -EINVAL;
	}

	ᬯᬮᬶ k_mutex_unlock(kernel_mutex);
}

static anta_hstha puur_naa_nka z_vrfy_z_sys_mutex_kernel_unlock(sa_nracanaa sys_mutex *mutex)
{
	ᬬᬾᬦ᭄ (check_sys_mutex_addr(mutex)) {
		ᬯᬮᬶ -EACCES;
	}

	ᬯᬮᬶ z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
