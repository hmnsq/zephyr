/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/sys/sem.h>

পূর্ণাঙ্ক sys_sem_init(সংরচনা sys_sem *sem, unsigned পূর্ণাঙ্ক initial_গুনতি,
		 unsigned পূর্ণাঙ্ক limit)
{
	k_sem_init(&sem->kernel_sem, initial_গুনতি, limit);

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক sys_sem_give(সংরচনা sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক sys_sem_take(সংরচনা sys_sem *sem, k_timeout_t timeout)
{
	পূর্ণাঙ্ক ret_মান = 0;

	ret_মান = k_sem_take(&sem->kernel_sem, timeout);
	যদি ((ret_মান == -EAGAIN) || (ret_মান == -EBUSY)) {
		ret_মান = -ETIMEDOUT;
	}

	প্রত্যাগম ret_মান;
}

unsigned পূর্ণাঙ্ক sys_sem_গুনতি_get(সংরচনা sys_sem *sem)
{
	প্রত্যাগম k_sem_গুনতি_get(&sem->kernel_sem);
}
