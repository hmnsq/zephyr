/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/sys/sem.h>

முழு sys_sem_init(கட்டமைப்பு sys_sem *sem, unsigned முழு initial_மொத்தம்,
		 unsigned முழு limit)
{
	k_sem_init(&sem->kernel_sem, initial_மொத்தம், limit);

	திரும்பு 0;
}

முழு sys_sem_give(கட்டமைப்பு sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	திரும்பு 0;
}

முழு sys_sem_take(கட்டமைப்பு sys_sem *sem, k_timeout_t timeout)
{
	முழு ret_மதிப்பு = 0;

	ret_மதிப்பு = k_sem_take(&sem->kernel_sem, timeout);
	எனில் ((ret_மதிப்பு == -EAGAIN) || (ret_மதிப்பு == -EBUSY)) {
		ret_மதிப்பு = -ETIMEDOUT;
	}

	திரும்பு ret_மதிப்பு;
}

unsigned முழு sys_sem_மொத்தம்_get(கட்டமைப்பு sys_sem *sem)
{
	திரும்பு k_sem_மொத்தம்_get(&sem->kernel_sem);
}
