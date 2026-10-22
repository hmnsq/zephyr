/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/sem.h>

صحیح sys_sem_init(ساخت sys_sem *sem, غیرعلامتی صحیح initial_گنتی,
		 غیرعلامتی صحیح حد_بندی)
{
	k_sem_init(&sem->kernel_sem, initial_گنتی, حد_بندی);

	واپس 0;
}

صحیح sys_sem_give(ساخت sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	واپس 0;
}

صحیح sys_sem_take(ساخت sys_sem *sem, k_timeout_t timeout)
{
	صحیح ret_قدر = 0;

	ret_قدر = k_sem_take(&sem->kernel_sem, timeout);
	اگر ((ret_قدر == -EAGAIN) || (ret_قدر == -EBUSY)) {
		ret_قدر = -ETIMEDOUT;
	}

	واپس ret_قدر;
}

غیرعلامتی صحیح sys_sem_گنتی_get(ساخت sys_sem *sem)
{
	واپس k_sem_گنتی_get(&sem->kernel_sem);
}
