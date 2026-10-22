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
		ꦪꦼꦤ꧀ (old_value < minimum) {
			bha_nga;
		}

		new_value = old_value - 1;
	} ꦱꦼꦢꦼꦭ (atomic_cas(val, old_value, new_value) == 0);

	ꦮꦭꦶ old_value;
}

static anta_hstha atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		ꦪꦼꦤ꧀ (old_value >= maximum) {
			bha_nga;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} ꦱꦼꦢꦼꦭ (atomic_cas(val, old_value, new_value) == 0U);

	ꦮꦭꦶ old_value;
}

puur_naa_nka sys_sem_init(sa_nracanaa sys_sem *sem, unsigned puur_naa_nka initial_count,
		 unsigned puur_naa_nka limit)
{
	ꦪꦼꦤ꧀ ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		ꦮꦭꦶ -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	ꦮꦭꦶ 0;
}

puur_naa_nka sys_sem_give(sa_nracanaa sys_sem *sem)
{
	puur_naa_nka ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ꦪꦼꦤ꧀ (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ꦪꦼꦤ꧀ (ret > 0) {
			ꦮꦭꦶ 0;
		}
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (old_value >= sem->limit) {
		ꦮꦭꦶ -EAGAIN;
	} ꦭꦶꦪ {
		;
	}
	ꦮꦭꦶ ret;
}

puur_naa_nka sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	puur_naa_nka ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ꦪꦼꦤ꧀ (old_value > 0) {
			ꦮꦭꦶ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ꦱꦼꦢꦼꦭ (ret == 0 || ret == -EAGAIN);

	ꦮꦭꦶ ret;
}

unsigned puur_naa_nka sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	puur_naa_nka value = atomic_get(&sem->futex.val);

	ꦮꦭꦶ (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
