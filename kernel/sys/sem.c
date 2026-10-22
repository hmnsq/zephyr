/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, بینښه صحيح initial_شمېر,
		 بینښه صحيح حد)
{
	k_sem_init(&sem->kernel_sem, initial_شمېر, حد);

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	أرجع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_ارزښت = 0;

	ret_ارزښت = k_sem_take(&sem->kernel_sem, timeout);
	إذا ((ret_ارزښت == -EAGAIN) || (ret_ارزښت == -EBUSY)) {
		ret_ارزښت = -ETIMEDOUT;
	}

	أرجع ret_ارزښت;
}

بینښه صحيح sys_sem_شمېر_get(بنية sys_sem *sem)
{
	أرجع k_sem_شمېر_get(&sem->kernel_sem);
}
