/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/mutex.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>

स्थिर संरचना k_mutex *get_k_mutex(संरचना sys_mutex *mutex)
{
	संरचना k_object *obj;

	obj = k_object_find(mutex);
	जर ((obj == रिक्त) || (obj->type != K_OBJ_SYS_MUTEX)) {
		परत रिक्त;
	}

	परत obj->data.mutex;
}

स्थिर bool check_sys_mutex_addr(संरचना sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	परत K_SYSCALL_MEMORY_WRITE(addr, आकार(संरचना sys_mutex));
}

पूर्णांक z_impl_z_sys_mutex_kernel_lock(संरचना sys_mutex *mutex, k_timeout_t timeout)
{
	संरचना k_mutex *kernel_mutex = get_k_mutex(mutex);

	जर (kernel_mutex == रिक्त) {
		परत -EINVAL;
	}

	परत k_mutex_lock(kernel_mutex, timeout);
}

स्थिर ओळीत पूर्णांक z_vrfy_z_sys_mutex_kernel_lock(संरचना sys_mutex *mutex,
						 k_timeout_t timeout)
{
	जर (check_sys_mutex_addr(mutex)) {
		परत -EACCES;
	}

	परत z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#अन्तर्भाव <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

पूर्णांक z_impl_z_sys_mutex_kernel_unlock(संरचना sys_mutex *mutex)
{
	संरचना k_mutex *kernel_mutex = get_k_mutex(mutex);

	जर ((kernel_mutex == रिक्त) || (kernel_mutex->lock_मोजणी == 0)) {
		परत -EINVAL;
	}

	परत k_mutex_unlock(kernel_mutex);
}

स्थिर ओळीत पूर्णांक z_vrfy_z_sys_mutex_kernel_unlock(संरचना sys_mutex *mutex)
{
	जर (check_sys_mutex_addr(mutex)) {
		परत -EACCES;
	}

	परत z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#अन्तर्भाव <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
