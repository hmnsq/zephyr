/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/sys/sem.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ;

	ಕುರು {
		old_ಮೌಲ್ಯ = atomic_get(val);
		ಯದಿ (old_ಮೌಲ್ಯ < minimum) {
			ಭಙ್ಗ;
		}

		new_ಮೌಲ್ಯ = old_ಮೌಲ್ಯ - 1;
	} ಯಾವತ್ (atomic_cas(val, old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ) == 0);

	ಪ್ರತ್ಯಾಗಮ old_ಮೌಲ್ಯ;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ;

	ಕುರು {
		old_ಮೌಲ್ಯ = atomic_get(val);
		ಯದಿ (old_ಮೌಲ್ಯ >= maximum) {
			ಭಙ್ಗ;
		}

		new_ಮೌಲ್ಯ = ((old_ಮೌಲ್ಯ < minimum) ? minimum : old_ಮೌಲ್ಯ) + 1;
	} ಯಾವತ್ (atomic_cas(val, old_ಮೌಲ್ಯ, new_ಮೌಲ್ಯ) == 0U);

	ಪ್ರತ್ಯಾಗಮ old_ಮೌಲ್ಯ;
}

ಪೂರ್ಣಾಙ್ಕ sys_sem_init(ಸಂರಚನಾ sys_sem *sem, unsigned ಪೂರ್ಣಾಙ್ಕ initial_ಎಣಿಕೆ,
		 unsigned ಪೂರ್ಣಾಙ್ಕ limit)
{
	ಯದಿ ((sem == ರಿಕ್ತ) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_ಎಣಿಕೆ > limit) || (limit > INT_MAX)) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	(ಶೂನ್ಯ)atomic_set(&sem->futex.val, initial_ಎಣಿಕೆ);
	sem->limit = limit;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ sys_sem_give(ಸಂರಚನಾ sys_sem *sem)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	atomic_t old_ಮೌಲ್ಯ;

	old_ಮೌಲ್ಯ = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ಯದಿ (old_ಮೌಲ್ಯ < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ಯದಿ (ret > 0) {
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	} ಅನ್ಯಥಾ ಯದಿ (old_ಮೌಲ್ಯ >= sem->limit) {
		ಪ್ರತ್ಯಾಗಮ -EAGAIN;
	} ಅನ್ಯಥಾ {
		;
	}
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ sys_sem_take(ಸಂರಚನಾ sys_sem *sem, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	atomic_t old_ಮೌಲ್ಯ;

	ಕುರು {
		old_ಮೌಲ್ಯ = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ಯದಿ (old_ಮೌಲ್ಯ > 0) {
			ಪ್ರತ್ಯಾಗಮ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ಯಾವತ್ (ret == 0 || ret == -EAGAIN);

	ಪ್ರತ್ಯಾಗಮ ret;
}

unsigned ಪೂರ್ಣಾಙ್ಕ sys_sem_ಎಣಿಕೆ_get(ಸಂರಚನಾ sys_sem *sem)
{
	ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ = atomic_get(&sem->futex.val);

	ಪ್ರತ್ಯಾಗಮ (ಮೌಲ್ಯ > SYS_SEM_MINIMUM) ? ಮೌಲ್ಯ : SYS_SEM_MINIMUM;
}
