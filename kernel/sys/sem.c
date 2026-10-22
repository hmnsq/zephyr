/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/sys/sem.h>

പൂർണ്ണസംഖ്യ sys_sem_init(ഘടന sys_sem *sem, unsigned പൂർണ്ണസംഖ്യ initial_എണ്ണം,
		 unsigned പൂർണ്ണസംഖ്യ limit)
{
	k_sem_init(&sem->kernel_sem, initial_എണ്ണം, limit);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ sys_sem_give(ഘടന sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ sys_sem_take(ഘടന sys_sem *sem, k_timeout_t timeout)
{
	പൂർണ്ണസംഖ്യ ret_മൂല്യം = 0;

	ret_മൂല്യം = k_sem_take(&sem->kernel_sem, timeout);
	എങ്കിൽ ((ret_മൂല്യം == -EAGAIN) || (ret_മൂല്യം == -EBUSY)) {
		ret_മൂല്യം = -ETIMEDOUT;
	}

	തിരികെ ret_മൂല്യം;
}

unsigned പൂർണ്ണസംഖ്യ sys_sem_എണ്ണം_get(ഘടന sys_sem *sem)
{
	തിരികെ k_sem_എണ്ണം_get(&sem->kernel_sem);
}
