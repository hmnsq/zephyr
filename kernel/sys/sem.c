/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

ཧྲིལ sys_sem_init(sa_nracanaa sys_sem *sem, unsigned ཧྲིལ initial_count,
		 unsigned ཧྲིལ limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	ལོག 0;
}

ཧྲིལ sys_sem_give(sa_nracanaa sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ལོག 0;
}

ཧྲིལ sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	ཧྲིལ ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	གལ་སྲིད ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	ལོག ret_value;
}

unsigned ཧྲིལ sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	ལོག k_sem_count_get(&sem->kernel_sem);
}
