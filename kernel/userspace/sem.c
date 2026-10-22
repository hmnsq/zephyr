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
		གལ་སྲིད (old_value < minimum) {
			bha_nga;
		}

		new_value = old_value - 1;
	} སྐབས (atomic_cas(val, old_value, new_value) == 0);

	ལོག old_value;
}

static anta_hstha atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		གལ་སྲིད (old_value >= maximum) {
			bha_nga;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} སྐབས (atomic_cas(val, old_value, new_value) == 0U);

	ལོག old_value;
}

ཧྲིལ sys_sem_init(sa_nracanaa sys_sem *sem, unsigned ཧྲིལ initial_count,
		 unsigned ཧྲིལ limit)
{
	གལ་སྲིད ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		ལོག -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	ལོག 0;
}

ཧྲིལ sys_sem_give(sa_nracanaa sys_sem *sem)
{
	ཧྲིལ ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	གལ་སྲིད (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		གལ་སྲིད (ret > 0) {
			ལོག 0;
		}
	} དེ་མིན གལ་སྲིད (old_value >= sem->limit) {
		ལོག -EAGAIN;
	} དེ་མིན {
		;
	}
	ལོག ret;
}

ཧྲིལ sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	ཧྲིལ ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		གལ་སྲིད (old_value > 0) {
			ལོག 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} སྐབས (ret == 0 || ret == -EAGAIN);

	ལོག ret;
}

unsigned ཧྲིལ sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	ཧྲིལ value = atomic_get(&sem->futex.val);

	ལོག (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
