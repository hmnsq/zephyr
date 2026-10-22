/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/sys/mutex.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>

স্থির কাঠামো k_mutex *get_k_mutex(কাঠামো sys_mutex *mutex)
{
	কাঠামো k_object *obj;

	obj = k_object_find(mutex);
	যদি ((obj == ফাঁকা) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ফেরত ফাঁকা;
	}

	ফেরত obj->তথ্য.mutex;
}

স্থির bool check_sys_mutex_addr(কাঠামো sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ফেরত K_SYSCALL_MEMORY_WRITE(addr, আকার(কাঠামো sys_mutex));
}

পূর্ণসংখ্যা z_impl_z_sys_mutex_kernel_lock(কাঠামো sys_mutex *mutex, k_timeout_t timeout)
{
	কাঠামো k_mutex *kernel_mutex = get_k_mutex(mutex);

	যদি (kernel_mutex == ফাঁকা) {
		ফেরত -EINVAL;
	}

	ফেরত k_mutex_lock(kernel_mutex, timeout);
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_z_sys_mutex_kernel_lock(কাঠামো sys_mutex *mutex,
						 k_timeout_t timeout)
{
	যদি (check_sys_mutex_addr(mutex)) {
		ফেরত -EACCES;
	}

	ফেরত z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

পূর্ণসংখ্যা z_impl_z_sys_mutex_kernel_unlock(কাঠামো sys_mutex *mutex)
{
	কাঠামো k_mutex *kernel_mutex = get_k_mutex(mutex);

	যদি ((kernel_mutex == ফাঁকা) || (kernel_mutex->lock_গুনতি == 0)) {
		ফেরত -EINVAL;
	}

	ফেরত k_mutex_unlock(kernel_mutex);
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_z_sys_mutex_kernel_unlock(কাঠামো sys_mutex *mutex)
{
	যদি (check_sys_mutex_addr(mutex)) {
		ফেরত -EACCES;
	}

	ফেরত z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#অন্তর্ভাব <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
