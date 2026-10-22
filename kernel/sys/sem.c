/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

ចំនួនគត់ sys_sem_init(sa_nracanaa sys_sem *sem, unsigned ចំនួនគត់ initial_count,
		 unsigned ចំនួនគត់ limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	ត្រឡប់ 0;
}

ចំនួនគត់ sys_sem_give(sa_nracanaa sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ត្រឡប់ 0;
}

ចំនួនគត់ sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	ចំនួនគត់ ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	ប្រសិនបើ ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	ត្រឡប់ ret_value;
}

unsigned ចំនួនគត់ sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	ត្រឡប់ k_sem_count_get(&sem->kernel_sem);
}
