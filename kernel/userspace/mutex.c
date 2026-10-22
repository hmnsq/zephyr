/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/mutex.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>

ಸ್ಥಿರ ಸಂರಚನಾ k_mutex *get_k_mutex(ಸಂರಚನಾ sys_mutex *mutex)
{
	ಸಂರಚನಾ k_object *obj;

	obj = k_object_find(mutex);
	ಯದಿ ((obj == ರಿಕ್ತ) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ obj->data.mutex;
}

ಸ್ಥಿರ bool check_sys_mutex_addr(ಸಂರಚನಾ sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ಪ್ರತ್ಯಾಗಮ K_SYSCALL_MEMORY_WRITE(addr, ಪರಿಮಾಣ(ಸಂರಚನಾ sys_mutex));
}

ಪೂರ್ಣಾಙ್ಕ z_impl_z_sys_mutex_kernel_lock(ಸಂರಚನಾ sys_mutex *mutex, k_timeout_t timeout)
{
	ಸಂರಚನಾ k_mutex *kernel_mutex = get_k_mutex(mutex);

	ಯದಿ (kernel_mutex == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಪ್ರತ್ಯಾಗಮ k_mutex_lock(kernel_mutex, timeout);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_z_sys_mutex_kernel_lock(ಸಂರಚನಾ sys_mutex *mutex,
						 k_timeout_t timeout)
{
	ಯದಿ (check_sys_mutex_addr(mutex)) {
		ಪ್ರತ್ಯಾಗಮ -EACCES;
	}

	ಪ್ರತ್ಯಾಗಮ z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

ಪೂರ್ಣಾಙ್ಕ z_impl_z_sys_mutex_kernel_unlock(ಸಂರಚನಾ sys_mutex *mutex)
{
	ಸಂರಚನಾ k_mutex *kernel_mutex = get_k_mutex(mutex);

	ಯದಿ ((kernel_mutex == ರಿಕ್ತ) || (kernel_mutex->lock_ಎಣಿಕೆ == 0)) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಪ್ರತ್ಯಾಗಮ k_mutex_unlock(kernel_mutex);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_z_sys_mutex_kernel_unlock(ಸಂರಚನಾ sys_mutex *mutex)
{
	ಯದಿ (check_sys_mutex_addr(mutex)) {
		ಪ್ರತ್ಯಾಗಮ -EACCES;
	}

	ಪ್ರತ್ಯಾಗಮ z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
