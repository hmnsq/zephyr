/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/sem.h>

ಪೂರ್ಣಾಙ್ಕ sys_sem_init(ಸಂರಚನಾ sys_sem *sem, unsigned ಪೂರ್ಣಾಙ್ಕ initial_ಎಣಿಕೆ,
		 unsigned ಪೂರ್ಣಾಙ್ಕ limit)
{
	k_sem_init(&sem->kernel_sem, initial_ಎಣಿಕೆ, limit);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ sys_sem_give(ಸಂರಚನಾ sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ sys_sem_take(ಸಂರಚನಾ sys_sem *sem, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಙ್ಕ ret_ಮೌಲ್ಯ = 0;

	ret_ಮೌಲ್ಯ = k_sem_take(&sem->kernel_sem, timeout);
	ಯದಿ ((ret_ಮೌಲ್ಯ == -EAGAIN) || (ret_ಮೌಲ್ಯ == -EBUSY)) {
		ret_ಮೌಲ್ಯ = -ETIMEDOUT;
	}

	ಪ್ರತ್ಯಾಗಮ ret_ಮೌಲ್ಯ;
}

unsigned ಪೂರ್ಣಾಙ್ಕ sys_sem_ಎಣಿಕೆ_get(ಸಂರಚನಾ sys_sem *sem)
{
	ಪ್ರತ್ಯಾಗಮ k_sem_ಎಣಿಕೆ_get(&sem->kernel_sem);
}
