/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/mutex.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>

ಸ್ಥಿರ ರಚನೆ k_mutex *get_k_mutex(ರಚನೆ sys_mutex *mutex)
{
	ರಚನೆ k_object *obj;

	obj = k_object_ಕಂಡುಹಿಡಿ(mutex);
	ಆದರೆ ((obj == ಖಾಲಿ) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ obj->ದತ್ತಾಂಶ.mutex;
}

ಸ್ಥಿರ bool check_sys_mutex_addr(ರಚನೆ sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ಹಿಂತಿರುಗಿ K_SYSCALL_ಸ್ಮರಣೆ_WRITE(addr, ಗಾತ್ರ(ರಚನೆ sys_mutex));
}

ಪೂರ್ಣಾಂಕ z_impl_z_sys_mutex_kernel_ಬೀಗ(ರಚನೆ sys_mutex *mutex, k_timeout_t timeout)
{
	ರಚನೆ k_mutex *kernel_mutex = get_k_mutex(mutex);

	ಆದರೆ (kernel_mutex == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ k_mutex_ಬೀಗ(kernel_mutex, timeout);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_z_sys_mutex_kernel_ಬೀಗ(ರಚನೆ sys_mutex *mutex,
						 k_timeout_t timeout)
{
	ಆದರೆ (check_sys_mutex_addr(mutex)) {
		ಹಿಂತಿರುಗಿ -EACCES;
	}

	ಹಿಂತಿರುಗಿ z_impl_z_sys_mutex_kernel_ಬೀಗ(mutex, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

ಪೂರ್ಣಾಂಕ z_impl_z_sys_mutex_kernel_unlock(ರಚನೆ sys_mutex *mutex)
{
	ರಚನೆ k_mutex *kernel_mutex = get_k_mutex(mutex);

	ಆದರೆ ((kernel_mutex == ಖಾಲಿ) || (kernel_mutex->ಬೀಗ_ಎಣಿಕೆ == 0)) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ k_mutex_unlock(kernel_mutex);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_z_sys_mutex_kernel_unlock(ರಚನೆ sys_mutex *mutex)
{
	ಆದರೆ (check_sys_mutex_addr(mutex)) {
		ಹಿಂತಿರುಗಿ -EACCES;
	}

	ಹಿಂತಿರುಗಿ z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
