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
	အကယ်၍ ((obj == NULL) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ပြန်ပေး NULL;
	}

	ပြန်ပေး obj->data.mutex;
}

static bool check_sys_mutex_addr(sa_nracanaa sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ပြန်ပေး K_SYSCALL_MEMORY_WRITE(addr, parimaa_na(sa_nracanaa sys_mutex));
}

ကိန်းပြည့် z_impl_z_sys_mutex_kernel_lock(sa_nracanaa sys_mutex *mutex, k_timeout_t timeout)
{
	sa_nracanaa k_mutex *kernel_mutex = get_k_mutex(mutex);

	အကယ်၍ (kernel_mutex == NULL) {
		ပြန်ပေး -EINVAL;
	}

	ပြန်ပေး k_mutex_lock(kernel_mutex, timeout);
}

static anta_hstha ကိန်းပြည့် z_vrfy_z_sys_mutex_kernel_lock(sa_nracanaa sys_mutex *mutex,
						 k_timeout_t timeout)
{
	အကယ်၍ (check_sys_mutex_addr(mutex)) {
		ပြန်ပေး -EACCES;
	}

	ပြန်ပေး z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

ကိန်းပြည့် z_impl_z_sys_mutex_kernel_unlock(sa_nracanaa sys_mutex *mutex)
{
	sa_nracanaa k_mutex *kernel_mutex = get_k_mutex(mutex);

	အကယ်၍ ((kernel_mutex == NULL) || (kernel_mutex->lock_count == 0)) {
		ပြန်ပေး -EINVAL;
	}

	ပြန်ပေး k_mutex_unlock(kernel_mutex);
}

static anta_hstha ကိန်းပြည့် z_vrfy_z_sys_mutex_kernel_unlock(sa_nracanaa sys_mutex *mutex)
{
	အကယ်၍ (check_sys_mutex_addr(mutex)) {
		ပြန်ပေး -EACCES;
	}

	ပြန်ပေး z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#include <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
