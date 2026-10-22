/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/sys/mutex.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>

স্থির সংরচনা k_mutex *get_k_mutex(সংরচনা sys_mutex *mutex)
{
	সংরচনা k_object *obj;

	obj = k_object_find(mutex);
	যদি ((obj == রিক্ত) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই obj->data.mutex;
}

স্থির bool check_sys_mutex_addr(সংরচনা sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ঘূৰাই K_SYSCALL_MEMORY_WRITE(addr, পরিমাণ(সংরচনা sys_mutex));
}

পূর্ণাঙ্ক z_impl_z_sys_mutex_kernel_lock(সংরচনা sys_mutex *mutex, k_timeout_t timeout)
{
	সংরচনা k_mutex *kernel_mutex = get_k_mutex(mutex);

	যদি (kernel_mutex == রিক্ত) {
		ঘূৰাই -EINVAL;
	}

	ঘূৰাই k_mutex_lock(kernel_mutex, timeout);
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_z_sys_mutex_kernel_lock(সংরচনা sys_mutex *mutex,
						 k_timeout_t timeout)
{
	যদি (check_sys_mutex_addr(mutex)) {
		ঘূৰাই -EACCES;
	}

	ঘূৰাই z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#অন্তর্ভাৱ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

পূর্ণাঙ্ক z_impl_z_sys_mutex_kernel_unlock(সংরচনা sys_mutex *mutex)
{
	সংরচনা k_mutex *kernel_mutex = get_k_mutex(mutex);

	যদি ((kernel_mutex == রিক্ত) || (kernel_mutex->lock_গুনতি == 0)) {
		ঘূৰাই -EINVAL;
	}

	ঘূৰাই k_mutex_unlock(kernel_mutex);
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_z_sys_mutex_kernel_unlock(সংরচনা sys_mutex *mutex)
{
	যদি (check_sys_mutex_addr(mutex)) {
		ঘূৰাই -EACCES;
	}

	ঘূৰাই z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#অন্তর্ভাৱ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
