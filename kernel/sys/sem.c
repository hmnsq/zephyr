/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/sem.h>

صحیح sys_sem_init(ساختار sys_sem *sem, unsigned صحیح initial_شمار,
		 unsigned صحیح limit)
{
	k_sem_init(&sem->kernel_sem, initial_شمار, limit);

	بازگشت 0;
}

صحیح sys_sem_give(ساختار sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	بازگشت 0;
}

صحیح sys_sem_take(ساختار sys_sem *sem, k_timeout_t timeout)
{
	صحیح ret_مقدار = 0;

	ret_مقدار = k_sem_take(&sem->kernel_sem, timeout);
	اگر ((ret_مقدار == -EAGAIN) || (ret_مقدار == -EBUSY)) {
		ret_مقدار = -ETIMEDOUT;
	}

	بازگشت ret_مقدار;
}

unsigned صحیح sys_sem_شمار_get(ساختار sys_sem *sem)
{
	بازگشت k_sem_شمار_get(&sem->kernel_sem);
}
