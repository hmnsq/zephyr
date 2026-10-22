/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/sem.h>

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_ژماردن,
		 unsigned صحيح limit)
{
	k_sem_init(&sem->kernel_sem, initial_ژماردن, limit);

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	أرجع 0;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret_بەها = 0;

	ret_بەها = k_sem_take(&sem->kernel_sem, timeout);
	إذا ((ret_بەها == -EAGAIN) || (ret_بەها == -EBUSY)) {
		ret_بەها = -ETIMEDOUT;
	}

	أرجع ret_بەها;
}

unsigned صحيح sys_sem_ژماردن_get(بنية sys_sem *sem)
{
	أرجع k_sem_ژماردن_get(&sem->kernel_sem);
}
