/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/sem.h>

पूर्णाङ्क sys_sem_init(संरचना sys_sem *sem, unsigned पूर्णाङ्क initial_गणनम्,
		 unsigned पूर्णाङ्क limit)
{
	k_sem_init(&sem->kernel_sem, initial_गणनम्, limit);

	प्रत्यागम 0;
}

पूर्णाङ्क sys_sem_give(संरचना sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	प्रत्यागम 0;
}

पूर्णाङ्क sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णाङ्क ret_मानम् = 0;

	ret_मानम् = k_sem_take(&sem->kernel_sem, timeout);
	जइ ((ret_मानम् == -EAGAIN) || (ret_मानम् == -EBUSY)) {
		ret_मानम् = -ETIMEDOUT;
	}

	प्रत्यागम ret_मानम्;
}

unsigned पूर्णाङ्क sys_sem_गणनम्_get(संरचना sys_sem *sem)
{
	प्रत्यागम k_sem_गणनम्_get(&sem->kernel_sem);
}
