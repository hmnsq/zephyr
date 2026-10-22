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
	atomic_t old_मूल्य, new_मूल्य;

	कुरु {
		old_मूल्य = atomic_get(val);
		यदि (old_मूल्य < minimum) {
			भङ्ग;
		}

		new_मूल्य = old_मूल्य - 1;
	} यावत् (atomic_cas(val, old_मूल्य, new_मूल्य) == 0);

	प्रत्यागम old_मूल्य;
}

स्थिर अन्तःस्थ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_मूल्य, new_मूल्य;

	कुरु {
		old_मूल्य = atomic_get(val);
		यदि (old_मूल्य >= maximum) {
			भङ्ग;
		}

		new_मूल्य = ((old_मूल्य < minimum) ? minimum : old_मूल्य) + 1;
	} यावत् (atomic_cas(val, old_मूल्य, new_मूल्य) == 0U);

	प्रत्यागम old_मूल्य;
}

पूर्णाङ्क sys_sem_init(संरचना sys_sem *sem, unsigned पूर्णाङ्क initial_मोजणी,
		 unsigned पूर्णाङ्क limit)
{
	यदि ((sem == रिक्त) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_मोजणी > limit) || (limit > INT_MAX)) {
		प्रत्यागम -EINVAL;
	}

	(शून्य)atomic_set(&sem->futex.val, initial_मोजणी);
	sem->limit = limit;

	प्रत्यागम 0;
}

पूर्णाङ्क sys_sem_give(संरचना sys_sem *sem)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मूल्य;

	old_मूल्य = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	यदि (old_मूल्य < 0) {
		ret = k_futex_wake(&sem->futex, true);

		यदि (ret > 0) {
			प्रत्यागम 0;
		}
	} अन्यथा यदि (old_मूल्य >= sem->limit) {
		प्रत्यागम -EAGAIN;
	} अन्यथा {
		;
	}
	प्रत्यागम ret;
}

पूर्णाङ्क sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मूल्य;

	कुरु {
		old_मूल्य = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		यदि (old_मूल्य > 0) {
			प्रत्यागम 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} यावत् (ret == 0 || ret == -EAGAIN);

	प्रत्यागम ret;
}

unsigned पूर्णाङ्क sys_sem_मोजणी_get(संरचना sys_sem *sem)
{
	पूर्णाङ्क मूल्य = atomic_get(&sem->futex.val);

	प्रत्यागम (मूल्य > SYS_SEM_MINIMUM) ? मूल्य : SYS_SEM_MINIMUM;
}
