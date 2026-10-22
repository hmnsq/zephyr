/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/sys/mutex.h>
#समावेश <zephyr/internal/syscall_handler.h>

अटल काष्ठा k_mutex *get_k_mutex(काष्ठा sys_mutex *mutex)
{
	काष्ठा k_object *obj;

	obj = k_object_find(mutex);
	अगर ((obj == शून्य) || (obj->type != K_OBJ_SYS_MUTEX)) {
		वापस शून्य;
	}

	वापस obj->तथ्य.mutex;
}

अटल bool check_sys_mutex_addr(काष्ठा sys_mutex *addr)
{
	/* sys_mutex memory is never touched, just used to lookup the
	 * underlying k_mutex, but we don't want threads using mutexes
	 * that are outside their memory domain
	 */
	वापस K_SYSCALL_MEMORY_WRITE(addr, माप(काष्ठा sys_mutex));
}

पूर्णांक z_impl_z_sys_mutex_kernel_lock(काष्ठा sys_mutex *mutex, k_timeout_t timeout)
{
	काष्ठा k_mutex *kernel_mutex = get_k_mutex(mutex);

	अगर (kernel_mutex == शून्य) {
		वापस -EINVAL;
	}

	वापस k_mutex_lock(kernel_mutex, timeout);
}

अटल अंतरभूत पूर्णांक z_vrfy_z_sys_mutex_kernel_lock(काष्ठा sys_mutex *mutex,
						 k_timeout_t timeout)
{
	अगर (check_sys_mutex_addr(mutex)) {
		वापस -EACCES;
	}

	वापस z_impl_z_sys_mutex_kernel_lock(mutex, timeout);
}
#समावेश <zephyr/syscalls/z_sys_mutex_kernel_lock_mrsh.c>

पूर्णांक z_impl_z_sys_mutex_kernel_unlock(काष्ठा sys_mutex *mutex)
{
	काष्ठा k_mutex *kernel_mutex = get_k_mutex(mutex);

	अगर ((kernel_mutex == शून्य) || (kernel_mutex->lock_गिनती == 0)) {
		वापस -EINVAL;
	}

	वापस k_mutex_unlock(kernel_mutex);
}

अटल अंतरभूत पूर्णांक z_vrfy_z_sys_mutex_kernel_unlock(काष्ठा sys_mutex *mutex)
{
	अगर (check_sys_mutex_addr(mutex)) {
		वापस -EACCES;
	}

	वापस z_impl_z_sys_mutex_kernel_unlock(mutex);
}
#समावेश <zephyr/syscalls/z_sys_mutex_kernel_unlock_mrsh.c>
