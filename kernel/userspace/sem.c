/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/sys/sem.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

स्थिर ओळीत atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_मूल्य, new_मूल्य;

	करा {
		old_मूल्य = atomic_get(val);
		जर (old_मूल्य < minimum) {
			थांबा;
		}

		new_मूल्य = old_मूल्य - 1;
	} पर्यंत (atomic_cas(val, old_मूल्य, new_मूल्य) == 0);

	परत old_मूल्य;
}

स्थिर ओळीत atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_मूल्य, new_मूल्य;

	करा {
		old_मूल्य = atomic_get(val);
		जर (old_मूल्य >= maximum) {
			थांबा;
		}

		new_मूल्य = ((old_मूल्य < minimum) ? minimum : old_मूल्य) + 1;
	} पर्यंत (atomic_cas(val, old_मूल्य, new_मूल्य) == 0U);

	परत old_मूल्य;
}

पूर्णांक sys_sem_init(संरचना sys_sem *sem, अचिन्हित पूर्णांक initial_मोजणी,
		 अचिन्हित पूर्णांक limit)
{
	जर ((sem == रिक्त) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_मोजणी > limit) || (limit > पूर्णांक_उच्च)) {
		परत -EINVAL;
	}

	(शून्य)atomic_set(&sem->futex.val, initial_मोजणी);
	sem->limit = limit;

	परत 0;
}

पूर्णांक sys_sem_give(संरचना sys_sem *sem)
{
	पूर्णांक ret = 0;
	atomic_t old_मूल्य;

	old_मूल्य = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	जर (old_मूल्य < 0) {
		ret = k_futex_wake(&sem->futex, true);

		जर (ret > 0) {
			परत 0;
		}
	} अन्यथा जर (old_मूल्य >= sem->limit) {
		परत -EAGAIN;
	} अन्यथा {
		;
	}
	परत ret;
}

पूर्णांक sys_sem_take(संरचना sys_sem *sem, k_timeout_t timeout)
{
	पूर्णांक ret = 0;
	atomic_t old_मूल्य;

	करा {
		old_मूल्य = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		जर (old_मूल्य > 0) {
			परत 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} पर्यंत (ret == 0 || ret == -EAGAIN);

	परत ret;
}

अचिन्हित पूर्णांक sys_sem_मोजणी_get(संरचना sys_sem *sem)
{
	पूर्णांक मूल्य = atomic_get(&sem->futex.val);

	परत (मूल्य > SYS_SEM_MINIMUM) ? मूल्य : SYS_SEM_MINIMUM;
}
