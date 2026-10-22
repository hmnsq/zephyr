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
		nếu (old_value < minimum) {
			break;
		}

		new_value = old_value - 1;
	} trong_khi (atomic_cas(val, old_value, new_value) == 0);

	trả_về old_value;
}

static inline atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		nếu (old_value >= maximum) {
			break;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} trong_khi (atomic_cas(val, old_value, new_value) == 0U);

	trả_về old_value;
}

số_nguyên sys_sem_init(struct sys_sem *sem, unsigned số_nguyên initial_count,
		 unsigned số_nguyên limit)
{
	nếu ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		trả_về -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	trả_về 0;
}

số_nguyên sys_sem_give(struct sys_sem *sem)
{
	số_nguyên ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	nếu (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		nếu (ret > 0) {
			trả_về 0;
		}
	} không_thì nếu (old_value >= sem->limit) {
		trả_về -EAGAIN;
	} không_thì {
		;
	}
	trả_về ret;
}

số_nguyên sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	số_nguyên ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		nếu (old_value > 0) {
			trả_về 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} trong_khi (ret == 0 || ret == -EAGAIN);

	trả_về ret;
}

unsigned số_nguyên sys_sem_count_get(struct sys_sem *sem)
{
	số_nguyên value = atomic_get(&sem->futex.val);

	trả_về (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
