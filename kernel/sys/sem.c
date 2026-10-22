/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

정수 sys_sem_init(struct sys_sem *sem, unsigned 정수 initial_count,
		 unsigned 정수 limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	반환 0;
}

정수 sys_sem_give(struct sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	반환 0;
}

정수 sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	정수 ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	만약 ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	반환 ret_value;
}

unsigned 정수 sys_sem_count_get(struct sys_sem *sem)
{
	반환 k_sem_count_get(&sem->kernel_sem);
}
