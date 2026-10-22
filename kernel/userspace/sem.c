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
		もし (old_value < minimum) {
			break;
		}

		new_value = old_value - 1;
	} ながら (atomic_cas(val, old_value, new_value) == 0);

	戻る old_value;
}

static inline atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		もし (old_value >= maximum) {
			break;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} ながら (atomic_cas(val, old_value, new_value) == 0U);

	戻る old_value;
}

整数 sys_sem_init(struct sys_sem *sem, unsigned 整数 initial_count,
		 unsigned 整数 limit)
{
	もし ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		戻る -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	戻る 0;
}

整数 sys_sem_give(struct sys_sem *sem)
{
	整数 ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	もし (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		もし (ret > 0) {
			戻る 0;
		}
	} 否则 もし (old_value >= sem->limit) {
		戻る -EAGAIN;
	} 否则 {
		;
	}
	戻る ret;
}

整数 sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	整数 ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		もし (old_value > 0) {
			戻る 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ながら (ret == 0 || ret == -EAGAIN);

	戻る ret;
}

unsigned 整数 sys_sem_count_get(struct sys_sem *sem)
{
	整数 value = atomic_get(&sem->futex.val);

	戻る (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
