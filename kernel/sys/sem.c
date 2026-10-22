/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_سانى,
		 unsigned صحيح limit)
{
	k_sem_init(&sem->kernel_sem, initial_سانى, limit);

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	أرجع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_قىممەت = 0;

	ret_قىممەت = k_sem_take(&sem->kernel_sem, timeout);
	إذا ((ret_قىممەت == -EAGAIN) || (ret_قىممەت == -EBUSY)) {
		ret_قىممەت = -ETIMEDOUT;
	}

	أرجع ret_قىممەت;
}

unsigned صحيح sys_sem_سانى_get(بنية sys_sem *sem)
{
	أرجع k_sem_سانى_get(&sem->kernel_sem);
}
