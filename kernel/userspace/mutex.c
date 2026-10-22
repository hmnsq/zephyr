/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/mutex.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>

ਸਥਿਰ ਬਣਤਰ k_mutex *get_k_mutex(ਬਣਤਰ sys_mutex *mutex)
{
	ਬਣਤਰ k_object *obj;

	obj = k_object_ਲੱਭੋ(mutex);
	ਜੇ ((obj == ਨਿਰਾ) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ obj->ਅੰਕੜੇ.mutex;
}

ਸਥਿਰ bool check_sys_mutex_addr(ਬਣਤਰ sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ਵਾਪਸ K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(addr, ਆਕਾਰ(ਬਣਤਰ sys_mutex));
}

ਪੂਰਨ z_impl_z_sys_mutex_kernel_ਤਾਲਾ(ਬਣਤਰ sys_mutex *mutex, k_timeout_t timeout)
{
	ਬਣਤਰ k_mutex *kernel_mutex = get_k_mutex(mutex);

	ਜੇ (kernel_mutex == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ k_mutex_ਤਾਲਾ(kernel_mutex, timeout);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_z_sys_mutex_kernel_ਤਾਲਾ(ਬਣਤਰ sys_mutex *mutex,
						 k_timeout_t timeout)
{
	ਜੇ (check_sys_mutex_addr(mutex)) {
		ਵਾਪਸ -EACCES;
	}

	ਵਾਪਸ z_impl_z_sys_mutex_kernel_ਤਾਲਾ(mutex, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

ਪੂਰਨ z_impl_z_sys_mutex_kernel_unlock(ਬਣਤਰ sys_mutex *mutex)
{
	ਬਣਤਰ k_mutex *kernel_mutex = get_k_mutex(mutex);

	ਜੇ ((kernel_mutex == ਨਿਰਾ) || (kernel_mutex->ਤਾਲਾ_ਗਿਣਤੀ == 0)) {
		ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ k_mutex_unlock(kernel_mutex);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_z_sys_mutex_kernel_unlock(ਬਣਤਰ sys_mutex *mutex)
{
	ਜੇ (check_sys_mutex_addr(mutex)) {
		ਵਾਪਸ -EACCES;
	}

	ਵਾਪਸ z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
