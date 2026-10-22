/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/mutex.h>
#شامل <zephyr/internal/syscall_handler.h>

ساکن ساخت k_mutex *get_k_mutex(ساخت sys_mutex *mutex)
{
	ساخت k_object *obj;

	obj = k_object_ڈھونڈو(mutex);
	اگر ((obj == تہی) || (obj->type != K_OBJ_SYS_MUTEX)) {
		واپس تہی;
	}

	واپس obj->ڈیٹا.mutex;
}

ساکن bool check_sys_mutex_addr(ساخت sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	واپس K_SYSCALL_حافظہ_WRITE(addr, حجم(ساخت sys_mutex));
}

صحیح z_impl_z_sys_mutex_kernel_تالا(ساخت sys_mutex *mutex, k_timeout_t timeout)
{
	ساخت k_mutex *kernel_mutex = get_k_mutex(mutex);

	اگر (kernel_mutex == تہی) {
		واپس -EINVAL;
	}

	واپس k_mutex_تالا(kernel_mutex, timeout);
}

ساکن اندرونی صحیح z_vrfy_z_sys_mutex_kernel_تالا(ساخت sys_mutex *mutex,
						 k_timeout_t timeout)
{
	اگر (check_sys_mutex_addr(mutex)) {
		واپس -EACCES;
	}

	واپس z_impl_z_sys_mutex_kernel_تالا(mutex, timeout);
}
#شامل <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

صحیح z_impl_z_sys_mutex_kernel_unlock(ساخت sys_mutex *mutex)
{
	ساخت k_mutex *kernel_mutex = get_k_mutex(mutex);

	اگر ((kernel_mutex == تہی) || (kernel_mutex->تالا_گنتی == 0)) {
		واپس -EINVAL;
	}

	واپس k_mutex_unlock(kernel_mutex);
}

ساکن اندرونی صحیح z_vrfy_z_sys_mutex_kernel_unlock(ساخت sys_mutex *mutex)
{
	اگر (check_sys_mutex_addr(mutex)) {
		واپس -EACCES;
	}

	واپس z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#شامل <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
