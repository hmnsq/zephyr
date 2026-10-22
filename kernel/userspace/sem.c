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
		ако (old_value < minimum) {
			break;
		}

		new_value = old_value - 1;
	} докато (atomic_cas(val, old_value, new_value) == 0);

	върни old_value;
}

static inline atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		ако (old_value >= maximum) {
			break;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} докато (atomic_cas(val, old_value, new_value) == 0U);

	върни old_value;
}

цяло sys_sem_init(struct sys_sem *sem, unsigned цяло initial_count,
		 unsigned цяло limit)
{
	ако ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		върни -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	върни 0;
}

цяло sys_sem_give(struct sys_sem *sem)
{
	цяло ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ако (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ако (ret > 0) {
			върни 0;
		}
	} иначе ако (old_value >= sem->limit) {
		върни -EAGAIN;
	} иначе {
		;
	}
	върни ret;
}

цяло sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	цяло ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ако (old_value > 0) {
			върни 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} докато (ret == 0 || ret == -EAGAIN);

	върни ret;
}

unsigned цяло sys_sem_count_get(struct sys_sem *sem)
{
	цяло value = atomic_get(&sem->futex.val);

	върни (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
