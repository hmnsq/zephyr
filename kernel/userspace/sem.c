/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/sys/sem.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>

#परिभाषय SYS_SEM_MINIMUM      0
#परिभाषय SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

स्थिर अन्तःस्थ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_मानम्, new_मानम्;

	कुरु {
		old_मानम् = atomic_get(val);
		यदि (old_मानम् < minimum) {
			भङ्ग;
		}

		new_मानम् = old_मानम् - 1;
	} यावत् (atomic_cas(val, old_मानम्, new_मानम्) == 0);

	प्रत्यागम old_मानम्;
}

स्थिर अन्तःस्थ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t अधिकतमम्)
{
	atomic_t old_मानम्, new_मानम्;

	कुरु {
		old_मानम् = atomic_get(val);
		यदि (old_मानम् >= अधिकतमम्) {
			भङ्ग;
		}

		new_मानम् = ((old_मानम् < minimum) ? minimum : old_मानम्) + 1;
	} यावत् (atomic_cas(val, old_मानम्, new_मानम्) == 0U);

	प्रत्यागम old_मानम्;
}

पूर्णाङ्क sys_sem_init(संरचना sys_sem *sem, अचिह्नित पूर्णाङ्क initial_गणनम्,
		 अचिह्नित पूर्णाङ्क सीमा)
{
	यदि ((sem == रिक्त) || (सीमा == SYS_SEM_MINIMUM) ||
	    (initial_गणनम् > सीमा) || (सीमा > अधिकतम_पूर्णाङ्कः)) {
		प्रत्यागम -EINVAL;
	}

	(शून्य)atomic_समुच्चयः(&sem->futex.val, initial_गणनम्);
	sem->सीमा = सीमा;

	प्रत्यागम 0;
}

पूर्णाङ्क sys_sem_give(संरचना sys_sem *sem)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मानम्;

	old_मानम् = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->सीमा);
	यदि (old_मानम् < 0) {
		ret = k_futex_wake(&sem->futex, true);

		यदि (ret > 0) {
			प्रत्यागम 0;
		}
	} अन्यथा यदि (old_मानम् >= sem->सीमा) {
		प्रत्यागम -EAGAIN;
	} अन्यथा {
		;
	}
	प्रत्यागम ret;
}

पूर्णाङ्क sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मानम्;

	कुरु {
		old_मानम् = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		यदि (old_मानम् > 0) {
			प्रत्यागम 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} यावत् (ret == 0 || ret == -EAGAIN);

	प्रत्यागम ret;
}

अचिह्नित पूर्णाङ्क sys_sem_गणनम्_get(संरचना sys_sem *sem)
{
	पूर्णाङ्क मानम् = atomic_get(&sem->futex.val);

	प्रत्यागम (मानम् > SYS_SEM_MINIMUM) ? मानम् : SYS_SEM_MINIMUM;
}
