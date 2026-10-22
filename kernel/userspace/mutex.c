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
	यदि ((obj == रिक्त) || (obj->type != K_OBJ_SYS_MUTEX)) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम obj->data.mutex;
}

स्थिर bool check_sys_mutex_addr(संरचना sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	प्रत्यागम K_SYSCALL_MEMORY_WRITE(addr, परिमाण(संरचना sys_mutex));
}

पूर्णाङ्क z_impl_z_sys_mutex_kernel_lock(संरचना sys_mutex *mutex, k_timeout_t timeout)
{
	संरचना k_mutex *kernel_mutex = get_k_mutex(mutex);

	यदि (kernel_mutex == रिक्त) {
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम k_mutex_lock(kernel_mutex, timeout);
}

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_z_sys_mutex_kernel_lock(संरचना sys_mutex *mutex,
						 k_timeout_t timeout)
{
	यदि (check_sys_mutex_addr(mutex)) {
		प्रत्यागम -EACCES;
	}

	प्रत्यागम z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#अन्तर्भाव <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

पूर्णाङ्क z_impl_z_sys_mutex_kernel_unlock(संरचना sys_mutex *mutex)
{
	संरचना k_mutex *kernel_mutex = get_k_mutex(mutex);

	यदि ((kernel_mutex == रिक्त) || (kernel_mutex->lock_गिनती == 0)) {
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम k_mutex_unlock(kernel_mutex);
}

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_z_sys_mutex_kernel_unlock(संरचना sys_mutex *mutex)
{
	यदि (check_sys_mutex_addr(mutex)) {
		प्रत्यागम -EACCES;
	}

	प्रत्यागम z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#अन्तर्भाव <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
