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
	} док (atomic_cas(val, old_value, new_value) == 0);

	врати old_value;
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
	} док (atomic_cas(val, old_value, new_value) == 0U);

	врати old_value;
}

цео sys_sem_init(struct sys_sem *sem, unsigned цео initial_count,
		 unsigned цео limit)
{
	ако ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		врати -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	врати 0;
}

цео sys_sem_give(struct sys_sem *sem)
{
	цео ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ако (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ако (ret > 0) {
			врати 0;
		}
	} инако ако (old_value >= sem->limit) {
		врати -EAGAIN;
	} инако {
		;
	}
	врати ret;
}

цео sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	цео ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ако (old_value > 0) {
			врати 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} док (ret == 0 || ret == -EAGAIN);

	врати ret;
}

unsigned цео sys_sem_count_get(struct sys_sem *sem)
{
	цео value = atomic_get(&sem->futex.val);

	врати (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
