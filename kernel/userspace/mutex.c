/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/mutex.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>

ସ୍ଥିର ସଂରଚନା k_mutex *get_k_mutex(ସଂରଚନା sys_mutex *mutex)
{
	ସଂରଚନା k_object *obj;

	obj = k_object_find(mutex);
	ଯଦି ((obj == ଖାଲି) || (obj->type != K_OBJ_SYS_MUTEX)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ obj->data.mutex;
}

ସ୍ଥିର bool check_sys_mutex_addr(ସଂରଚନା sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	ଫେରନ୍ତୁ K_SYSCALL_MEMORY_WRITE(addr, ଆକାର(ସଂରଚନା sys_mutex));
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_z_sys_mutex_kernel_lock(ସଂରଚନା sys_mutex *mutex, k_timeout_t timeout)
{
	ସଂରଚନା k_mutex *kernel_mutex = get_k_mutex(mutex);

	ଯଦି (kernel_mutex == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଫେରନ୍ତୁ k_mutex_lock(kernel_mutex, timeout);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_z_sys_mutex_kernel_lock(ସଂରଚନା sys_mutex *mutex,
						 k_timeout_t timeout)
{
	ଯଦି (check_sys_mutex_addr(mutex)) {
		ଫେରନ୍ତୁ -EACCES;
	}

	ଫେରନ୍ତୁ z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_z_sys_mutex_kernel_unlock(ସଂରଚନା sys_mutex *mutex)
{
	ସଂରଚନା k_mutex *kernel_mutex = get_k_mutex(mutex);

	ଯଦି ((kernel_mutex == ଖାଲି) || (kernel_mutex->lock_ଗଣନା == 0)) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଫେରନ୍ତୁ k_mutex_unlock(kernel_mutex);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_z_sys_mutex_kernel_unlock(ସଂରଚନା sys_mutex *mutex)
{
	ଯଦି (check_sys_mutex_addr(mutex)) {
		ଫେରନ୍ତୁ -EACCES;
	}

	ଫେରନ୍ତୁ z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
