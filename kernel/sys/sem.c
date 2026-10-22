/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, بیعلامت صحيح initial_شمار,
		 بیعلامت صحيح حد)
{
	k_sem_init(&sem->kernel_sem, initial_شمار, حد);

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	أرجع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_مقدار = 0;

	ret_مقدار = k_sem_take(&sem->kernel_sem, timeout);
	إذا ((ret_مقدار == -EAGAIN) || (ret_مقدار == -EBUSY)) {
		ret_مقدار = -ETIMEDOUT;
	}

	أرجع ret_مقدار;
}

بیعلامت صحيح sys_sem_شمار_get(بنية sys_sem *sem)
{
	أرجع k_sem_شمار_get(&sem->kernel_sem);
}
