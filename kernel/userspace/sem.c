/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/sys/sem.h>
#समावेश <zephyr/internal/syscall_handler.h>

#घोषणा SYS_SEM_MINIMUM      0
#घोषणा SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

अटल अंतरभूत atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_मूल्य, new_मूल्य;

	करो {
		old_मूल्य = atomic_get(val);
		अगर (old_मूल्य < minimum) {
			अवरोध;
		}

		new_मूल्य = old_मूल्य - 1;
	} जबतक (atomic_cas(val, old_मूल्य, new_मूल्य) == 0);

	वापस old_मूल्य;
}

अटल अंतरभूत atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_मूल्य, new_मूल्य;

	करो {
		old_मूल्य = atomic_get(val);
		अगर (old_मूल्य >= maximum) {
			अवरोध;
		}

		new_मूल्य = ((old_मूल्य < minimum) ? minimum : old_मूल्य) + 1;
	} जबतक (atomic_cas(val, old_मूल्य, new_मूल्य) == 0U);

	वापस old_मूल्य;
}

पूर्णांक sys_sem_init(काष्ठा sys_sem *sem, अचिन्हित पूर्णांक initial_गिनती,
		 अचिन्हित पूर्णांक limit)
{
	अगर ((sem == शून्य) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_गिनती > limit) || (limit > पूर्णांक_उच्च)) {
		वापस -EINVAL;
	}

	(व्योम)atomic_set(&sem->futex.val, initial_गिनती);
	sem->limit = limit;

	वापस 0;
}

पूर्णांक sys_sem_give(काष्ठा sys_sem *sem)
{
	पूर्णांक ret = 0;
	atomic_t old_मूल्य;

	old_मूल्य = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	अगर (old_मूल्य < 0) {
		ret = k_futex_wake(&sem->futex, true);

		अगर (ret > 0) {
			वापस 0;
		}
	} अन्यथा अगर (old_मूल्य >= sem->limit) {
		वापस -EAGAIN;
	} अन्यथा {
		;
	}
	वापस ret;
}

पूर्णांक sys_sem_take(काष्ठा sys_sem *sem, k_timeout_t timeout)
{
	पूर्णांक ret = 0;
	atomic_t old_मूल्य;

	करो {
		old_मूल्य = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		अगर (old_मूल्य > 0) {
			वापस 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} जबतक (ret == 0 || ret == -EAGAIN);

	वापस ret;
}

अचिन्हित पूर्णांक sys_sem_गिनती_get(काष्ठा sys_sem *sem)
{
	पूर्णांक मूल्य = atomic_get(&sem->futex.val);

	वापस (मूल्य > SYS_SEM_MINIMUM) ? मूल्य : SYS_SEM_MINIMUM;
}
