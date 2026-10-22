/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

số_nguyên sys_sem_init(struct sys_sem *sem, unsigned số_nguyên initial_count,
		 unsigned số_nguyên limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	trả_về 0;
}

số_nguyên sys_sem_give(struct sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	trả_về 0;
}

số_nguyên sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	số_nguyên ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	nếu ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	trả_về ret_value;
}

unsigned số_nguyên sys_sem_count_get(struct sys_sem *sem)
{
	trả_về k_sem_count_get(&sem->kernel_sem);
}
