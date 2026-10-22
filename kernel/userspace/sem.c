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
		만약 (old_value < minimum) {
			break;
		}

		new_value = old_value - 1;
	} 동안 (atomic_cas(val, old_value, new_value) == 0);

	반환 old_value;
}

static inline atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		만약 (old_value >= maximum) {
			break;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} 동안 (atomic_cas(val, old_value, new_value) == 0U);

	반환 old_value;
}

정수 sys_sem_init(struct sys_sem *sem, unsigned 정수 initial_count,
		 unsigned 정수 limit)
{
	만약 ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		반환 -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	반환 0;
}

정수 sys_sem_give(struct sys_sem *sem)
{
	정수 ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	만약 (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		만약 (ret > 0) {
			반환 0;
		}
	} 아니면 만약 (old_value >= sem->limit) {
		반환 -EAGAIN;
	} 아니면 {
		;
	}
	반환 ret;
}

정수 sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	정수 ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		만약 (old_value > 0) {
			반환 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} 동안 (ret == 0 || ret == -EAGAIN);

	반환 ret;
}

unsigned 정수 sys_sem_count_get(struct sys_sem *sem)
{
	정수 value = atomic_get(&sem->futex.val);

	반환 (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
