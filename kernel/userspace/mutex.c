/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/mutex.h>
#ضمن <zephyr/internal/syscall_handler.h>

ساكن بنية k_mutex *get_k_mutex(بنية sys_mutex *mutex)
{
	بنية k_object *obj;

	obj = k_object_لبھو(mutex);
	إذا ((obj == عدم) || (obj->type != K_OBJ_SYS_MUTEX)) {
		أرجع عدم;
	}

	أرجع obj->انکڑے.mutex;
}

ساكن bool check_sys_mutex_addr(بنية sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	أرجع K_SYSCALL_حافظہ_WRITE(addr, حجم(بنية sys_mutex));
}

صحيح z_impl_z_sys_mutex_kernel_جندرا(بنية sys_mutex *mutex, k_timeout_t timeout)
{
	بنية k_mutex *kernel_mutex = get_k_mutex(mutex);

	إذا (kernel_mutex == عدم) {
		أرجع -EINVAL;
	}

	أرجع k_mutex_جندرا(kernel_mutex, timeout);
}

ساكن مضمن صحيح z_vrfy_z_sys_mutex_kernel_جندرا(بنية sys_mutex *mutex,
						 k_timeout_t timeout)
{
	إذا (check_sys_mutex_addr(mutex)) {
		أرجع -EACCES;
	}

	أرجع z_impl_z_sys_mutex_kernel_جندرا(mutex, timeout);
}
#ضمن <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

صحيح z_impl_z_sys_mutex_kernel_unlock(بنية sys_mutex *mutex)
{
	بنية k_mutex *kernel_mutex = get_k_mutex(mutex);

	إذا ((kernel_mutex == عدم) || (kernel_mutex->جندرا_گنتی == 0)) {
		أرجع -EINVAL;
	}

	أرجع k_mutex_unlock(kernel_mutex);
}

ساكن مضمن صحيح z_vrfy_z_sys_mutex_kernel_unlock(بنية sys_mutex *mutex)
{
	إذا (check_sys_mutex_addr(mutex)) {
		أرجع -EACCES;
	}

	أرجع z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#ضمن <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
