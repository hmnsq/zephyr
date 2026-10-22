/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/sem.h>

पूर्णाङ्क sys_sem_init(संरचना sys_sem *sem, अचिन्हित पूर्णाङ्क initial_गिनती,
		 अचिन्हित पूर्णाङ्क limit)
{
	k_sem_init(&sem->kernel_sem, initial_गिनती, limit);

	प्रत्यागम 0;
}

पूर्णाङ्क sys_sem_give(संरचना sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	प्रत्यागम 0;
}

पूर्णाङ्क sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णाङ्क ret_मूल्य = 0;

	ret_मूल्य = k_sem_take(&sem->kernel_sem, timeout);
	यदि ((ret_मूल्य == -EAGAIN) || (ret_मूल्य == -EBUSY)) {
		ret_मूल्य = -ETIMEDOUT;
	}

	प्रत्यागम ret_मूल्य;
}

अचिन्हित पूर्णाङ्क sys_sem_गिनती_get(संरचना sys_sem *sem)
{
	प्रत्यागम k_sem_गिनती_get(&sem->kernel_sem);
}
