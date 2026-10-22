/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/sys/mutex.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>

நிலையான கட்டமைப்பு k_mutex *get_k_mutex(கட்டமைப்பு sys_mutex *mutex)
{
	கட்டமைப்பு k_object *obj;

	obj = k_object_find(mutex);
	எனில் ((obj == வெற்று) || (obj->type != K_OBJ_SYS_MUTEX)) {
		திரும்பு வெற்று;
	}

	திரும்பு obj->data.mutex;
}

நிலையான bool check_sys_mutex_addr(கட்டமைப்பு sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	திரும்பு K_SYSCALL_MEMORY_WRITE(addr, அளவு(கட்டமைப்பு sys_mutex));
}

முழு z_impl_z_sys_mutex_kernel_lock(கட்டமைப்பு sys_mutex *mutex, k_timeout_t timeout)
{
	கட்டமைப்பு k_mutex *kernel_mutex = get_k_mutex(mutex);

	எனில் (kernel_mutex == வெற்று) {
		திரும்பு -EINVAL;
	}

	திரும்பு k_mutex_lock(kernel_mutex, timeout);
}

நிலையான உள்வரி முழு z_vrfy_z_sys_mutex_kernel_lock(கட்டமைப்பு sys_mutex *mutex,
						 k_timeout_t timeout)
{
	எனில் (check_sys_mutex_addr(mutex)) {
		திரும்பு -EACCES;
	}

	திரும்பு z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

முழு z_impl_z_sys_mutex_kernel_unlock(கட்டமைப்பு sys_mutex *mutex)
{
	கட்டமைப்பு k_mutex *kernel_mutex = get_k_mutex(mutex);

	எனில் ((kernel_mutex == வெற்று) || (kernel_mutex->lock_மொத்தம் == 0)) {
		திரும்பு -EINVAL;
	}

	திரும்பு k_mutex_unlock(kernel_mutex);
}

நிலையான உள்வரி முழு z_vrfy_z_sys_mutex_kernel_unlock(கட்டமைப்பு sys_mutex *mutex)
{
	எனில் (check_sys_mutex_addr(mutex)) {
		திரும்பு -EACCES;
	}

	திரும்பு z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#அந்தர்பாவ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
