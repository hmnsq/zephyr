/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/sem.h>
#include <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

static inline atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		если (old_value < minimum) {
			break;
		}

		new_value = old_value - 1;
	} пока (atomic_cas(val, old_value, new_value) == 0);

	вернуть old_value;
}

static inline atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		если (old_value >= maximum) {
			break;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} пока (atomic_cas(val, old_value, new_value) == 0U);

	вернуть old_value;
}

целое sys_sem_init(struct sys_sem *sem, unsigned целое initial_count,
		 unsigned целое limit)
{
	если ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		вернуть -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	вернуть 0;
}

целое sys_sem_give(struct sys_sem *sem)
{
	целое ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	если (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		если (ret > 0) {
			вернуть 0;
		}
	} иначе если (old_value >= sem->limit) {
		вернуть -EAGAIN;
	} иначе {
		;
	}
	вернуть ret;
}

целое sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	целое ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		если (old_value > 0) {
			вернуть 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} пока (ret == 0 || ret == -EAGAIN);

	вернуть ret;
}

unsigned целое sys_sem_count_get(struct sys_sem *sem)
{
	целое value = atomic_get(&sem->futex.val);

	вернуть (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
