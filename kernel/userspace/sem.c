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
	atomic_t old_मान, new_मान;

	गर {
		old_मान = atomic_get(val);
		यदि (old_मान < minimum) {
			रोक;
		}

		new_मान = old_मान - 1;
	} जबसम्म (atomic_cas(val, old_मान, new_मान) == 0);

	फर्काऊ old_मान;
}

स्थिर अन्तःस्थ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_मान, new_मान;

	गर {
		old_मान = atomic_get(val);
		यदि (old_मान >= maximum) {
			रोक;
		}

		new_मान = ((old_मान < minimum) ? minimum : old_मान) + 1;
	} जबसम्म (atomic_cas(val, old_मान, new_मान) == 0U);

	फर्काऊ old_मान;
}

पूर्णाङ्क sys_sem_init(संरचना sys_sem *sem, अचिन्हित पूर्णाङ्क initial_गन्ती,
		 अचिन्हित पूर्णाङ्क limit)
{
	यदि ((sem == रिक्त) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_गन्ती > limit) || (limit > पूर्णांक_उच्च)) {
		फर्काऊ -EINVAL;
	}

	(शून्य)atomic_set(&sem->futex.val, initial_गन्ती);
	sem->limit = limit;

	फर्काऊ 0;
}

पूर्णाङ्क sys_sem_give(संरचना sys_sem *sem)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मान;

	old_मान = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	यदि (old_मान < 0) {
		ret = k_futex_wake(&sem->futex, true);

		यदि (ret > 0) {
			फर्काऊ 0;
		}
	} अन्यथा यदि (old_मान >= sem->limit) {
		फर्काऊ -EAGAIN;
	} अन्यथा {
		;
	}
	फर्काऊ ret;
}

पूर्णाङ्क sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णाङ्क ret = 0;
	atomic_t old_मान;

	गर {
		old_मान = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		यदि (old_मान > 0) {
			फर्काऊ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} जबसम्म (ret == 0 || ret == -EAGAIN);

	फर्काऊ ret;
}

अचिन्हित पूर्णाङ्क sys_sem_गन्ती_get(संरचना sys_sem *sem)
{
	पूर्णाङ्क मान = atomic_get(&sem->futex.val);

	फर्काऊ (मान > SYS_SEM_MINIMUM) ? मान : SYS_SEM_MINIMUM;
}
