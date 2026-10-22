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
		အကယ်၍ (old_value < minimum) {
			bha_nga;
		}

		new_value = old_value - 1;
	} နေစဉ် (atomic_cas(val, old_value, new_value) == 0);

	ပြန်ပေး old_value;
}

static anta_hstha atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		အကယ်၍ (old_value >= maximum) {
			bha_nga;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} နေစဉ် (atomic_cas(val, old_value, new_value) == 0U);

	ပြန်ပေး old_value;
}

ကိန်းပြည့် sys_sem_init(sa_nracanaa sys_sem *sem, unsigned ကိန်းပြည့် initial_count,
		 unsigned ကိန်းပြည့် limit)
{
	အကယ်၍ ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		ပြန်ပေး -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	ပြန်ပေး 0;
}

ကိန်းပြည့် sys_sem_give(sa_nracanaa sys_sem *sem)
{
	ကိန်းပြည့် ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	အကယ်၍ (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		အကယ်၍ (ret > 0) {
			ပြန်ပေး 0;
		}
	} မဟုတ်ပါက အကယ်၍ (old_value >= sem->limit) {
		ပြန်ပေး -EAGAIN;
	} မဟုတ်ပါက {
		;
	}
	ပြန်ပေး ret;
}

ကိန်းပြည့် sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	ကိန်းပြည့် ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		အကယ်၍ (old_value > 0) {
			ပြန်ပေး 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} နေစဉ် (ret == 0 || ret == -EAGAIN);

	ပြန်ပေး ret;
}

unsigned ကိန်းပြည့် sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	ကိန်းပြည့် value = atomic_get(&sem->futex.val);

	ပြန်ပေး (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
