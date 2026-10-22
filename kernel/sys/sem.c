/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/sem.h>

ಪೂರ್ಣಾಂಕ sys_sem_init(ರಚನೆ sys_sem *sem, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ initial_ಎಣಿಕೆ,
		 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ)
{
	k_sem_init(&sem->kernel_sem, initial_ಎಣಿಕೆ, ಎಲ್ಲೆ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ sys_sem_give(ರಚನೆ sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ sys_sem_take(ರಚನೆ sys_sem *sem, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಂಕ ret_ಮೌಲ್ಯ = 0;

	ret_ಮೌಲ್ಯ = k_sem_take(&sem->kernel_sem, timeout);
	ಆದರೆ ((ret_ಮೌಲ್ಯ == -EAGAIN) || (ret_ಮೌಲ್ಯ == -EBUSY)) {
		ret_ಮೌಲ್ಯ = -ETIMEDOUT;
	}

	ಹಿಂತಿರುಗಿ ret_ಮೌಲ್ಯ;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sys_sem_ಎಣಿಕೆ_get(ರಚನೆ sys_sem *sem)
{
	ಹಿಂತಿರುಗಿ k_sem_ಎಣಿಕೆ_get(&sem->kernel_sem);
}
