/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/sys/sem.h>

પૂર્ણાંક sys_sem_init(સંરચના sys_sem *sem, unsigned પૂર્ણાંક initial_ગણતરી,
		 unsigned પૂર્ણાંક limit)
{
	k_sem_init(&sem->kernel_sem, initial_ગણતરી, limit);

	પાછા 0;
}

પૂર્ણાંક sys_sem_give(સંરચના sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	પાછા 0;
}

પૂર્ણાંક sys_sem_take(સંરચના sys_sem *sem, k_timeout_t timeout)
{
	પૂર્ણાંક ret_કિંમત = 0;

	ret_કિંમત = k_sem_take(&sem->kernel_sem, timeout);
	જો ((ret_કિંમત == -EAGAIN) || (ret_કિંમત == -EBUSY)) {
		ret_કિંમત = -ETIMEDOUT;
	}

	પાછા ret_કિંમત;
}

unsigned પૂર્ણાંક sys_sem_ગણતરી_get(સંરચના sys_sem *sem)
{
	પાછા k_sem_ગણતરી_get(&sem->kernel_sem);
}
