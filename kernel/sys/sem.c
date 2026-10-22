/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/sem.h>

पूर्णाङ्क sys_sem_init(संरचना sys_sem *sem, अचिन्हित पूर्णाङ्क initial_गन्ती,
		 अचिन्हित पूर्णाङ्क limit)
{
	k_sem_init(&sem->kernel_sem, initial_गन्ती, limit);

	फर्काऊ 0;
}

पूर्णाङ्क sys_sem_give(संरचना sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	फर्काऊ 0;
}

पूर्णाङ्क sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णाङ्क ret_मान = 0;

	ret_मान = k_sem_take(&sem->kernel_sem, timeout);
	यदि ((ret_मान == -EAGAIN) || (ret_मान == -EBUSY)) {
		ret_मान = -ETIMEDOUT;
	}

	फर्काऊ ret_मान;
}

अचिन्हित पूर्णाङ्क sys_sem_गन्ती_get(संरचना sys_sem *sem)
{
	फर्काऊ k_sem_गन्ती_get(&sem->kernel_sem);
}
