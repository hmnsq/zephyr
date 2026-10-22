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
		якщо (old_value < minimum) {
			break;
		}

		new_value = old_value - 1;
	} поки (atomic_cas(val, old_value, new_value) == 0);

	повернути old_value;
}

static inline atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		якщо (old_value >= maximum) {
			break;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} поки (atomic_cas(val, old_value, new_value) == 0U);

	повернути old_value;
}

ціле sys_sem_init(struct sys_sem *sem, unsigned ціле initial_count,
		 unsigned ціле limit)
{
	якщо ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		повернути -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	повернути 0;
}

ціле sys_sem_give(struct sys_sem *sem)
{
	ціле ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	якщо (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		якщо (ret > 0) {
			повернути 0;
		}
	} інакше якщо (old_value >= sem->limit) {
		повернути -EAGAIN;
	} інакше {
		;
	}
	повернути ret;
}

ціле sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	ціле ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		якщо (old_value > 0) {
			повернути 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} поки (ret == 0 || ret == -EAGAIN);

	повернути ret;
}

unsigned ціле sys_sem_count_get(struct sys_sem *sem)
{
	ціле value = atomic_get(&sem->futex.val);

	повернути (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
