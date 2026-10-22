/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/mutex.h>
#شامل <zephyr/internal/syscall_handler.h>

ایستا ساختار k_mutex *get_k_mutex(ساختار sys_mutex *mutex)
{
	ساختار k_object *obj;

	obj = k_object_find(mutex);
	اگر ((obj == پوچ) || (obj->type != K_OBJ_SYS_MUTEX)) {
		بازگشت پوچ;
	}

	بازگشت obj->data.mutex;
}

ایستا bool check_sys_mutex_addr(ساختار sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	بازگشت K_SYSCALL_MEMORY_WRITE(addr, اندازه(ساختار sys_mutex));
}

صحیح z_impl_z_sys_mutex_kernel_lock(ساختار sys_mutex *mutex, k_timeout_t timeout)
{
	ساختار k_mutex *kernel_mutex = get_k_mutex(mutex);

	اگر (kernel_mutex == پوچ) {
		بازگشت -EINVAL;
	}

	بازگشت k_mutex_lock(kernel_mutex, timeout);
}

ایستا درونخطی صحیح z_vrfy_z_sys_mutex_kernel_lock(ساختار sys_mutex *mutex,
						 k_timeout_t timeout)
{
	اگر (check_sys_mutex_addr(mutex)) {
		بازگشت -EACCES;
	}

	بازگشت z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#شامل <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

صحیح z_impl_z_sys_mutex_kernel_unlock(ساختار sys_mutex *mutex)
{
	ساختار k_mutex *kernel_mutex = get_k_mutex(mutex);

	اگر ((kernel_mutex == پوچ) || (kernel_mutex->lock_شمار == 0)) {
		بازگشت -EINVAL;
	}

	بازگشت k_mutex_unlock(kernel_mutex);
}

ایستا درونخطی صحیح z_vrfy_z_sys_mutex_kernel_unlock(ساختار sys_mutex *mutex)
{
	اگر (check_sys_mutex_addr(mutex)) {
		بازگشت -EACCES;
	}

	بازگشت z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#شامل <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
