/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

පූර්ණ_සංඛ්‍යා sys_sem_init(sa_nracanaa sys_sem *sem, unsigned පූර්ණ_සංඛ්‍යා initial_count,
		 unsigned පූර්ණ_සංඛ්‍යා limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා sys_sem_give(sa_nracanaa sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	පූර්ණ_සංඛ්‍යා ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	නම් ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	ආපසු ret_value;
}

unsigned පූර්ණ_සංඛ්‍යා sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	ආපසු k_sem_count_get(&sem->kernel_sem);
}
