/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

整数 sys_sem_init(struct sys_sem *sem, unsigned 整数 initial_count,
		 unsigned 整数 limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	戻る 0;
}

整数 sys_sem_give(struct sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	戻る 0;
}

整数 sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	整数 ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	もし ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	戻る ret_value;
}

unsigned 整数 sys_sem_count_get(struct sys_sem *sem)
{
	戻る k_sem_count_get(&sem->kernel_sem);
}
