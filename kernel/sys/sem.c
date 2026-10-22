/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_کيراءن,
		 unsigned صحيح limit)
{
	k_sem_init(&sem->kernel_sem, initial_کيراءن, limit);

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	أرجع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_نيلاي = 0;

	ret_نيلاي = k_sem_take(&sem->kernel_sem, timeout);
	إذا ((ret_نيلاي == -EAGAIN) || (ret_نيلاي == -EBUSY)) {
		ret_نيلاي = -ETIMEDOUT;
	}

	أرجع ret_نيلاي;
}

unsigned صحيح sys_sem_کيراءن_get(بنية sys_sem *sem)
{
	أرجع k_sem_کيراءن_get(&sem->kernel_sem);
}
