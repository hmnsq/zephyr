/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/sem.h>

ਪੂਰਨ sys_sem_init(ਬਣਤਰ sys_sem *sem, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ initial_ਗਿਣਤੀ,
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਹੱਦਬੰਦੀ)
{
	k_sem_init(&sem->kernel_sem, initial_ਗਿਣਤੀ, ਹੱਦਬੰਦੀ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ sys_sem_give(ਬਣਤਰ sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ਵਾਪਸ 0;
}

ਪੂਰਨ sys_sem_take(ਬਣਤਰ sys_sem *sem, k_timeout_t timeout)
{
	ਪੂਰਨ ret_ਮੁੱਲ = 0;

	ret_ਮੁੱਲ = k_sem_take(&sem->kernel_sem, timeout);
	ਜੇ ((ret_ਮੁੱਲ == -EAGAIN) || (ret_ਮੁੱਲ == -EBUSY)) {
		ret_ਮੁੱਲ = -ETIMEDOUT;
	}

	ਵਾਪਸ ret_ਮੁੱਲ;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sys_sem_ਗਿਣਤੀ_get(ਬਣਤਰ sys_sem *sem)
{
	ਵਾਪਸ k_sem_ਗਿਣਤੀ_get(&sem->kernel_sem);
}
