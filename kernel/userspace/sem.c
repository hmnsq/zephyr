/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/sys/sem.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

स्थिर अन्तःस्थ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_मानम्, new_मानम्;

	करेहि {
		old_मानम् = atomic_get(val);
		जइ (old_मानम् < minimum) {
			भङ्ग;
		}

		new_मानम् = old_मानम् - 1;
	} जाव (atomic_cas(val, old_मानम्, new_मानम्) == 0);

	प्रत्यागम old_मानम्;
}

स्थिर अन्तःस्थ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_मानम्, new_मानम्;

	करेहि {
		old_मानम् = atomic_get(val);
		जइ (old_मानम् >= maximum) {
			भङ्ग;
		}

		new_मानम् = ((old_मानम् < minimum) ? minimum : old_मानम्) + 1;
	} जाव (atomic_cas(val, old_मानम्, new_मानम्) == 0U);

	प्रत्यागम old_मानम्;
}

पूर्णाङ्क sys_sem_init(संरचना sys_sem *sem, unsigned पूर्णाङ्क initial_गणनम्,
		 unsigned पूर्णाङ्क limit)
{
	जइ ((sem == रिक्त) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_गणनम् > limit) || (limit > INT_MAX)) {
		प्रत्यागम -EINVAL;
	}

	(शून्य)atomic_set(&sem->futex.val, initial_गणनम्);
	sem->limit = limit;

	प्रत्यागम 0;
}

पूर्णाङ्क sys_sem_give(संरचना sys_sem *sem)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मानम्;

	old_मानम् = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	जइ (old_मानम् < 0) {
		ret = k_futex_wake(&sem->futex, true);

		जइ (ret > 0) {
			प्रत्यागम 0;
		}
	} अण्णहा जइ (old_मानम् >= sem->limit) {
		प्रत्यागम -EAGAIN;
	} अण्णहा {
		;
	}
	प्रत्यागम ret;
}

पूर्णाङ्क sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मानम्;

	करेहि {
		old_मानम् = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		जइ (old_मानम् > 0) {
			प्रत्यागम 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} जाव (ret == 0 || ret == -EAGAIN);

	प्रत्यागम ret;
}

unsigned पूर्णाङ्क sys_sem_गणनम्_get(संरचना sys_sem *sem)
{
	पूर्णाङ्क मानम् = atomic_get(&sem->futex.val);

	प्रत्यागम (मानम् > SYS_SEM_MINIMUM) ? मानम् : SYS_SEM_MINIMUM;
}
