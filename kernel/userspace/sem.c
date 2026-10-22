/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/sem.h>
#include <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

static anta_hstha atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		ถ้า (old_value < minimum) {
			bha_nga;
		}

		new_value = old_value - 1;
	} ขณะ (atomic_cas(val, old_value, new_value) == 0);

	คืน old_value;
}

static anta_hstha atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		ถ้า (old_value >= maximum) {
			bha_nga;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} ขณะ (atomic_cas(val, old_value, new_value) == 0U);

	คืน old_value;
}

จำนวนเต็ม sys_sem_init(sa_nracanaa sys_sem *sem, unsigned จำนวนเต็ม initial_count,
		 unsigned จำนวนเต็ม limit)
{
	ถ้า ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		คืน -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	คืน 0;
}

จำนวนเต็ม sys_sem_give(sa_nracanaa sys_sem *sem)
{
	จำนวนเต็ม ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ถ้า (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ถ้า (ret > 0) {
			คืน 0;
		}
	} มิฉะนั้น ถ้า (old_value >= sem->limit) {
		คืน -EAGAIN;
	} มิฉะนั้น {
		;
	}
	คืน ret;
}

จำนวนเต็ม sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	จำนวนเต็ม ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ถ้า (old_value > 0) {
			คืน 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ขณะ (ret == 0 || ret == -EAGAIN);

	คืน ret;
}

unsigned จำนวนเต็ม sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	จำนวนเต็ม value = atomic_get(&sem->futex.val);

	คืน (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
