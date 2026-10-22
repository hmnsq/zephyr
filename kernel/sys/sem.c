/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_كدي,
		 unsigned صحيح limit)
{
	k_sem_init(&sem->kernel_sem, initial_كدي, limit);

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	أرجع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_درج = 0;

	ret_درج = k_sem_take(&sem->kernel_sem, timeout);
	إذا ((ret_درج == -EAGAIN) || (ret_درج == -EBUSY)) {
		ret_درج = -ETIMEDOUT;
	}

	أرجع ret_درج;
}

unsigned صحيح sys_sem_كدي_get(بنية sys_sem *sem)
{
	أرجع k_sem_كدي_get(&sem->kernel_sem);
}
