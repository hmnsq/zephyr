/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/sem.h>

पूर्णांक sys_sem_init(संरचना sys_sem *sem, अचिन्हित पूर्णांक initial_मोजणी,
		 अचिन्हित पूर्णांक limit)
{
	k_sem_init(&sem->kernel_sem, initial_मोजणी, limit);

	परत 0;
}

पूर्णांक sys_sem_give(संरचना sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	परत 0;
}

पूर्णांक sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णांक ret_मूल्य = 0;

	ret_मूल्य = k_sem_take(&sem->kernel_sem, timeout);
	जर ((ret_मूल्य == -EAGAIN) || (ret_मूल्य == -EBUSY)) {
		ret_मूल्य = -ETIMEDOUT;
	}

	परत ret_मूल्य;
}

अचिन्हित पूर्णांक sys_sem_मोजणी_get(संरचना sys_sem *sem)
{
	परत k_sem_मोजणी_get(&sem->kernel_sem);
}
