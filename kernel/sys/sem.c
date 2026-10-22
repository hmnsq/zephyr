/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/sys/sem.h>

పూర్ణాంకం sys_sem_init(నిర్మాణం sys_sem *sem, గుర్తులేని పూర్ణాంకం initial_లెక్క,
		 గుర్తులేని పూర్ణాంకం హద్దు)
{
	k_sem_init(&sem->kernel_sem, initial_లెక్క, హద్దు);

	తిరిగి 0;
}

పూర్ణాంకం sys_sem_give(నిర్మాణం sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	తిరిగి 0;
}

పూర్ణాంకం sys_sem_take(నిర్మాణం sys_sem *sem, k_timeout_t timeout)
{
	పూర్ణాంకం ret_విలువ = 0;

	ret_విలువ = k_sem_take(&sem->kernel_sem, timeout);
	ఒకవేళ ((ret_విలువ == -EAGAIN) || (ret_విలువ == -EBUSY)) {
		ret_విలువ = -ETIMEDOUT;
	}

	తిరిగి ret_విలువ;
}

గుర్తులేని పూర్ణాంకం sys_sem_లెక్క_get(నిర్మాణం sys_sem *sem)
{
	తిరిగి k_sem_లెక్క_get(&sem->kernel_sem);
}
