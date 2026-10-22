/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/sys/sem.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

સ્થિર અંતઃરેખ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_કિંમત, new_કિંમત;

	કરો {
		old_કિંમત = atomic_get(val);
		જો (old_કિંમત < minimum) {
			તોડો;
		}

		new_કિંમત = old_કિંમત - 1;
	} જ્યારે (atomic_cas(val, old_કિંમત, new_કિંમત) == 0);

	પાછા old_કિંમત;
}

સ્થિર અંતઃરેખ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_કિંમત, new_કિંમત;

	કરો {
		old_કિંમત = atomic_get(val);
		જો (old_કિંમત >= maximum) {
			તોડો;
		}

		new_કિંમત = ((old_કિંમત < minimum) ? minimum : old_કિંમત) + 1;
	} જ્યારે (atomic_cas(val, old_કિંમત, new_કિંમત) == 0U);

	પાછા old_કિંમત;
}

પૂર્ણાંક sys_sem_init(સંરચના sys_sem *sem, unsigned પૂર્ણાંક initial_ગણતરી,
		 unsigned પૂર્ણાંક limit)
{
	જો ((sem == ખાલી) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_ગણતરી > limit) || (limit > INT_MAX)) {
		પાછા -EINVAL;
	}

	(શૂન્ય)atomic_set(&sem->futex.val, initial_ગણતરી);
	sem->limit = limit;

	પાછા 0;
}

પૂર્ણાંક sys_sem_give(સંરચના sys_sem *sem)
{
	પૂર્ણાંક ret = 0;
	atomic_t old_કિંમત;

	old_કિંમત = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	જો (old_કિંમત < 0) {
		ret = k_futex_wake(&sem->futex, true);

		જો (ret > 0) {
			પાછા 0;
		}
	} નહીંતર જો (old_કિંમત >= sem->limit) {
		પાછા -EAGAIN;
	} નહીંતર {
		;
	}
	પાછા ret;
}

પૂર્ણાંક sys_sem_take(સંરચના sys_sem *sem, k_timeout_t timeout)
{
	પૂર્ણાંક ret = 0;
	atomic_t old_કિંમત;

	કરો {
		old_કિંમત = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		જો (old_કિંમત > 0) {
			પાછા 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} જ્યારે (ret == 0 || ret == -EAGAIN);

	પાછા ret;
}

unsigned પૂર્ણાંક sys_sem_ગણતરી_get(સંરચના sys_sem *sem)
{
	પૂર્ણાંક કિંમત = atomic_get(&sem->futex.val);

	પાછા (કિંમત > SYS_SEM_MINIMUM) ? કિંમત : SYS_SEM_MINIMUM;
}
