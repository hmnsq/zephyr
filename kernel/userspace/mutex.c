/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/sys/mutex.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>

സ്ഥിതം ഘടന k_mutex *get_k_mutex(ഘടന sys_mutex *mutex)
{
	ഘടന k_object *obj;

	obj = k_object_find(mutex);
	എങ്കിൽ ((obj == ശൂന്യ) || (obj->type != K_OBJ_SYS_MUTEX)) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ obj->data.mutex;
}

സ്ഥിതം bool check_sys_mutex_addr(ഘടന sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	തിരികെ K_SYSCALL_MEMORY_WRITE(addr, വലുപ്പം(ഘടന sys_mutex));
}

പൂർണ്ണസംഖ്യ z_impl_z_sys_mutex_kernel_lock(ഘടന sys_mutex *mutex, k_timeout_t timeout)
{
	ഘടന k_mutex *kernel_mutex = get_k_mutex(mutex);

	എങ്കിൽ (kernel_mutex == ശൂന്യ) {
		തിരികെ -EINVAL;
	}

	തിരികെ k_mutex_lock(kernel_mutex, timeout);
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_z_sys_mutex_kernel_lock(ഘടന sys_mutex *mutex,
						 k_timeout_t timeout)
{
	എങ്കിൽ (check_sys_mutex_addr(mutex)) {
		തിരികെ -EACCES;
	}

	തിരികെ z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

പൂർണ്ണസംഖ്യ z_impl_z_sys_mutex_kernel_unlock(ഘടന sys_mutex *mutex)
{
	ഘടന k_mutex *kernel_mutex = get_k_mutex(mutex);

	എങ്കിൽ ((kernel_mutex == ശൂന്യ) || (kernel_mutex->lock_എണ്ണം == 0)) {
		തിരികെ -EINVAL;
	}

	തിരികെ k_mutex_unlock(kernel_mutex);
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_z_sys_mutex_kernel_unlock(ഘടന sys_mutex *mutex)
{
	എങ്കിൽ (check_sys_mutex_addr(mutex)) {
		തിരികെ -EACCES;
	}

	തിരികെ z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#അന്തര്ഭാവ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
