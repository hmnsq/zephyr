/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/sys/sem.h>

পূর্ণসংখ্যা sys_sem_init(কাঠামো sys_sem *sem, unsigned পূর্ণসংখ্যা initial_গুনতি,
		 unsigned পূর্ণসংখ্যা limit)
{
	k_sem_init(&sem->kernel_sem, initial_গুনতি, limit);

	ফেরত 0;
}

পূর্ণসংখ্যা sys_sem_give(কাঠামো sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ফেরত 0;
}

পূর্ণসংখ্যা sys_sem_take(কাঠামো sys_sem *sem, k_timeout_t timeout)
{
	পূর্ণসংখ্যা ret_মান = 0;

	ret_মান = k_sem_take(&sem->kernel_sem, timeout);
	যদি ((ret_মান == -EAGAIN) || (ret_মান == -EBUSY)) {
		ret_মান = -ETIMEDOUT;
	}

	ফেরত ret_মান;
}

unsigned পূর্ণসংখ্যা sys_sem_গুনতি_get(কাঠামো sys_sem *sem)
{
	ফেরত k_sem_গুনতি_get(&sem->kernel_sem);
}
