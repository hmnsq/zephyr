/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/sys/mutex.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>

స్థిర నిర్మాణం k_mutex *get_k_mutex(నిర్మాణం sys_mutex *mutex)
{
	నిర్మాణం k_object *obj;

	obj = k_object_కనుగొను(mutex);
	ఒకవేళ ((obj == ఖాళీ) || (obj->type != K_OBJ_SYS_MUTEX)) {
		తిరిగి ఖాళీ;
	}

	తిరిగి obj->దత్తాంశం.mutex;
}

స్థిర bool check_sys_mutex_addr(నిర్మాణం sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	తిరిగి K_SYSCALL_జ్ఞాపకం_WRITE(addr, పరిమాణం(నిర్మాణం sys_mutex));
}

పూర్ణాంకం z_impl_z_sys_mutex_kernel_తాళం(నిర్మాణం sys_mutex *mutex, k_timeout_t timeout)
{
	నిర్మాణం k_mutex *kernel_mutex = get_k_mutex(mutex);

	ఒకవేళ (kernel_mutex == ఖాళీ) {
		తిరిగి -EINVAL;
	}

	తిరిగి k_mutex_తాళం(kernel_mutex, timeout);
}

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_z_sys_mutex_kernel_తాళం(నిర్మాణం sys_mutex *mutex,
						 k_timeout_t timeout)
{
	ఒకవేళ (check_sys_mutex_addr(mutex)) {
		తిరిగి -EACCES;
	}

	తిరిగి z_impl_z_sys_mutex_kernel_తాళం(mutex, timeout);
}
#అన్తర్భావ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

పూర్ణాంకం z_impl_z_sys_mutex_kernel_unlock(నిర్మాణం sys_mutex *mutex)
{
	నిర్మాణం k_mutex *kernel_mutex = get_k_mutex(mutex);

	ఒకవేళ ((kernel_mutex == ఖాళీ) || (kernel_mutex->తాళం_లెక్క == 0)) {
		తిరిగి -EINVAL;
	}

	తిరిగి k_mutex_unlock(kernel_mutex);
}

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_z_sys_mutex_kernel_unlock(నిర్మాణం sys_mutex *mutex)
{
	ఒకవేళ (check_sys_mutex_addr(mutex)) {
		తిరిగి -EACCES;
	}

	తిరిగి z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#అన్తర్భావ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
