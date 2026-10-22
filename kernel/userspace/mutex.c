/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/sys/mutex.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>

સ્થિર સંરચના k_mutex *get_k_mutex(સંરચના sys_mutex *mutex)
{
	સંરચના k_object *obj;

	obj = k_object_find(mutex);
	જો ((obj == ખાલી) || (obj->type != K_OBJ_SYS_MUTEX)) {
		પાછા ખાલી;
	}

	પાછા obj->data.mutex;
}

સ્થિર bool check_sys_mutex_addr(સંરચના sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	પાછા K_SYSCALL_MEMORY_WRITE(addr, કદ(સંરચના sys_mutex));
}

પૂર્ણાંક z_impl_z_sys_mutex_kernel_lock(સંરચના sys_mutex *mutex, k_timeout_t timeout)
{
	સંરચના k_mutex *kernel_mutex = get_k_mutex(mutex);

	જો (kernel_mutex == ખાલી) {
		પાછા -EINVAL;
	}

	પાછા k_mutex_lock(kernel_mutex, timeout);
}

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_z_sys_mutex_kernel_lock(સંરચના sys_mutex *mutex,
						 k_timeout_t timeout)
{
	જો (check_sys_mutex_addr(mutex)) {
		પાછા -EACCES;
	}

	પાછા z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

પૂર્ણાંક z_impl_z_sys_mutex_kernel_unlock(સંરચના sys_mutex *mutex)
{
	સંરચના k_mutex *kernel_mutex = get_k_mutex(mutex);

	જો ((kernel_mutex == ખાલી) || (kernel_mutex->lock_ગણતરી == 0)) {
		પાછા -EINVAL;
	}

	પાછા k_mutex_unlock(kernel_mutex);
}

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_z_sys_mutex_kernel_unlock(સંરચના sys_mutex *mutex)
{
	જો (check_sys_mutex_addr(mutex)) {
		પાછા -EACCES;
	}

	પાછા z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#અન્તર્ભાવ <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
