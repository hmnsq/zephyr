/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_count,
		 unsigned صحيح limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	أرجع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	𐡄𐡍 ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	أرجع ret_value;
}

unsigned صحيح sys_sem_count_get(بنية sys_sem *sem)
{
	أرجع k_sem_count_get(&sem->kernel_sem);
}
