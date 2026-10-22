/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

целое sys_sem_init(struct sys_sem *sem, unsigned целое initial_count,
		 unsigned целое limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	вернуть 0;
}

целое sys_sem_give(struct sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	вернуть 0;
}

целое sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	целое ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	если ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	вернуть ret_value;
}

unsigned целое sys_sem_count_get(struct sys_sem *sem)
{
	вернуть k_sem_count_get(&sem->kernel_sem);
}
