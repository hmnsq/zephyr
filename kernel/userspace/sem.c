/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/sys/sem.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>

#ವ್ಯಾಖ್ಯಾನಿಸು SYS_SEM_MINIMUM      0
#ವ್ಯಾಖ್ಯಾನಿಸು SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ;

	ಮಾಡು {
		old_ಮೌಲ್ಯ = atomic_get(val);
		ಆದರೆ (old_ಮೌಲ್ಯ < minimum) {
			ನಿಲ್ಲಿಸು;
		}

		new_ಮೌಲ್ಯ = old_ಮೌಲ್ಯ - 1;
	} ಆಗ (atomic_cas(val, old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ) == 0);

	ಹಿಂತಿರುಗಿ old_ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t ಗರಿಷ್ಠ)
{
	atomic_t old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ;

	ಮಾಡು {
		old_ಮೌಲ್ಯ = atomic_get(val);
		ಆದರೆ (old_ಮೌಲ್ಯ >= ಗರಿಷ್ಠ) {
			ನಿಲ್ಲಿಸು;
		}

		new_ಮೌಲ್ಯ = ((old_ಮೌಲ್ಯ < minimum) ? minimum : old_ಮೌಲ್ಯ) + 1;
	} ಆಗ (atomic_cas(val, old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ) == 0U);

	ಹಿಂತಿರುಗಿ old_ಮೌಲ್ಯ;
}

ಪೂರ್ಣಾಂಕ sys_sem_init(ರಚನೆ sys_sem *sem, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ initial_ಎಣಿಕೆ,
		 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ)
{
	ಆದರೆ ((sem == ಖಾಲಿ) || (ಎಲ್ಲೆ == SYS_SEM_MINIMUM) ||
	    (initial_ಎಣಿಕೆ > ಎಲ್ಲೆ) || (ಎಲ್ಲೆ > ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	(ಶೂನ್ಯ)atomic_ಗಣ(&sem->futex.val, initial_ಎಣಿಕೆ);
	sem->ಎಲ್ಲೆ = ಎಲ್ಲೆ;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ sys_sem_give(ರಚನೆ sys_sem *sem)
{
	ಪೂರ್ಣಾಂಕ ret = 0;
	atomic_t old_ಮೌಲ್ಯ;

	old_ಮೌಲ್ಯ = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->ಎಲ್ಲೆ);
	ಆದರೆ (old_ಮೌಲ್ಯ < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ಆದರೆ (ret > 0) {
			ಹಿಂತಿರುಗಿ 0;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (old_ಮೌಲ್ಯ >= sem->ಎಲ್ಲೆ) {
		ಹಿಂತಿರುಗಿ -EAGAIN;
	} ಇಲ್ಲದಿದ್ದರೆ {
		;
	}
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ sys_sem_take(ರಚನೆ sys_sem *sem, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಂಕ ret = 0;
	atomic_t old_ಮೌಲ್ಯ;

	ಮಾಡು {
		old_ಮೌಲ್ಯ = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ಆದರೆ (old_ಮೌಲ್ಯ > 0) {
			ಹಿಂತಿರುಗಿ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ಆಗ (ret == 0 || ret == -EAGAIN);

	ಹಿಂತಿರುಗಿ ret;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sys_sem_ಎಣಿಕೆ_get(ರಚನೆ sys_sem *sem)
{
	ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ = atomic_get(&sem->futex.val);

	ಹಿಂತಿರುಗಿ (ಮೌಲ್ಯ > SYS_SEM_MINIMUM) ? ಮೌಲ್ಯ : SYS_SEM_MINIMUM;
}
