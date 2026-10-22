/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/sys/sem.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>

#నిర్వచించు SYS_SEM_MINIMUM      0
#నిర్వచించు SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

స్థిర అంతఃవరుస atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_విలువ, new_విలువ;

	చేయి {
		old_విలువ = atomic_get(val);
		ఒకవేళ (old_విలువ < minimum) {
			ఆపు;
		}

		new_విలువ = old_విలువ - 1;
	} అయితే (atomic_cas(val, old_విలువ, new_విలువ) == 0);

	తిరిగి old_విలువ;
}

స్థిర అంతఃవరుస atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t గరిష్ఠ)
{
	atomic_t old_విలువ, new_విలువ;

	చేయి {
		old_విలువ = atomic_get(val);
		ఒకవేళ (old_విలువ >= గరిష్ఠ) {
			ఆపు;
		}

		new_విలువ = ((old_విలువ < minimum) ? minimum : old_విలువ) + 1;
	} అయితే (atomic_cas(val, old_విలువ, new_విలువ) == 0U);

	తిరిగి old_విలువ;
}

పూర్ణాంకం sys_sem_init(నిర్మాణం sys_sem *sem, గుర్తులేని పూర్ణాంకం initial_లెక్క,
		 గుర్తులేని పూర్ణాంకం హద్దు)
{
	ఒకవేళ ((sem == ఖాళీ) || (హద్దు == SYS_SEM_MINIMUM) ||
	    (initial_లెక్క > హద్దు) || (హద్దు > గరిష్ఠ_పూర్ణాంకం)) {
		తిరిగి -EINVAL;
	}

	(శూన్య)atomic_సమితి(&sem->futex.val, initial_లెక్క);
	sem->హద్దు = హద్దు;

	తిరిగి 0;
}

పూర్ణాంకం sys_sem_give(నిర్మాణం sys_sem *sem)
{
	పూర్ణాంకం ret = 0;
	atomic_t old_విలువ;

	old_విలువ = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->హద్దు);
	ఒకవేళ (old_విలువ < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ఒకవేళ (ret > 0) {
			తిరిగి 0;
		}
	} లేకపోతే ఒకవేళ (old_విలువ >= sem->హద్దు) {
		తిరిగి -EAGAIN;
	} లేకపోతే {
		;
	}
	తిరిగి ret;
}

పూర్ణాంకం sys_sem_take(నిర్మాణం sys_sem *sem, k_timeout_t timeout)
{
	పూర్ణాంకం ret = 0;
	atomic_t old_విలువ;

	చేయి {
		old_విలువ = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ఒకవేళ (old_విలువ > 0) {
			తిరిగి 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} అయితే (ret == 0 || ret == -EAGAIN);

	తిరిగి ret;
}

గుర్తులేని పూర్ణాంకం sys_sem_లెక్క_get(నిర్మాణం sys_sem *sem)
{
	పూర్ణాంకం విలువ = atomic_get(&sem->futex.val);

	తిరిగి (విలువ > SYS_SEM_MINIMUM) ? విలువ : SYS_SEM_MINIMUM;
}
