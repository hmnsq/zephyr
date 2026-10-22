/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/sys/sem.h>

पूर्णांक sys_sem_init(काष्ठा sys_sem *sem, अचिन्हित पूर्णांक initial_गिनती,
		 अचिन्हित पूर्णांक limit)
{
	k_sem_init(&sem->kernel_sem, initial_गिनती, limit);

	वापस 0;
}

पूर्णांक sys_sem_give(काष्ठा sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	वापस 0;
}

पूर्णांक sys_sem_take(काष्ठा sys_sem *sem, k_timeout_t timeout)
{
	पूर्णांक ret_मूल्य = 0;

	ret_मूल्य = k_sem_take(&sem->kernel_sem, timeout);
	अगर ((ret_मूल्य == -EAGAIN) || (ret_मूल्य == -EBUSY)) {
		ret_मूल्य = -ETIMEDOUT;
	}

	वापस ret_मूल्य;
}

अचिन्हित पूर्णांक sys_sem_गिनती_get(काष्ठा sys_sem *sem)
{
	वापस k_sem_गिनती_get(&sem->kernel_sem);
}
