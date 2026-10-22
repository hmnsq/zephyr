/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>

ຈຳນວນເຕັມ sys_sem_init(sa_nracanaa sys_sem *sem, unsigned ຈຳນວນເຕັມ initial_count,
		 unsigned ຈຳນວນເຕັມ limit)
{
	k_sem_init(&sem->kernel_sem, initial_count, limit);

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ sys_sem_give(sa_nracanaa sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	ຈຳນວນເຕັມ ret_value = 0;

	ret_value = k_sem_take(&sem->kernel_sem, timeout);
	ຖ້າ ((ret_value == -EAGAIN) || (ret_value == -EBUSY)) {
		ret_value = -ETIMEDOUT;
	}

	ສົ່ງຄືນ ret_value;
}

unsigned ຈຳນວນເຕັມ sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	ສົ່ງຄືນ k_sem_count_get(&sem->kernel_sem);
}
