/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

ціле sys_sem_init(struct sys_sem *sem, unsigned ціле initial_count,
		 unsigned ціле limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	повернути 0;
}

ціле sys_sem_give(struct sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	повернути 0;
}

ціле sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	ціле ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	якщо ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	повернути ret_value;
}

unsigned ціле sys_sem_count_get(struct sys_sem *sem)
{
	повернути k_sem_count_get(&sem->kernel_sem);
}
