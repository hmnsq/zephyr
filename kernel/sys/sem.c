/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_تعداد,
		 unsigned صحيح limit)
{
	k_sem_init(&sem->kernel_sem, initial_تعداد, limit);

	إرجاع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	إرجاع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_قيمة = 0;

	ret_قيمة = k_sem_take(&sem->kernel_sem, timeout);
	إذا ((ret_قيمة == -EAGAIN) || (ret_قيمة == -EBUSY)) {
		ret_قيمة = -ETIMEDOUT;
	}

	إرجاع ret_قيمة;
}

unsigned صحيح sys_sem_تعداد_get(بنية sys_sem *sem)
{
	إرجاع k_sem_تعداد_get(&sem->kernel_sem);
}
