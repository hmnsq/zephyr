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
		නම් (old_value < minimum) {
			bha_nga;
		}

		new_value = old_value - 1;
	} අතරතුර (atomic_cas(val, old_value, new_value) == 0);

	ආපසු old_value;
}

static anta_hstha atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		නම් (old_value >= maximum) {
			bha_nga;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} අතරතුර (atomic_cas(val, old_value, new_value) == 0U);

	ආපසු old_value;
}

පූර්ණ_සංඛ්‍යා sys_sem_init(sa_nracanaa sys_sem *sem, unsigned පූර්ණ_සංඛ්‍යා initial_count,
		 unsigned පූර්ණ_සංඛ්‍යා limit)
{
	නම් ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		ආපසු -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා sys_sem_give(sa_nracanaa sys_sem *sem)
{
	පූර්ණ_සංඛ්‍යා ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	නම් (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		නම් (ret > 0) {
			ආපසු 0;
		}
	} නැතහොත් නම් (old_value >= sem->limit) {
		ආපසු -EAGAIN;
	} නැතහොත් {
		;
	}
	ආපසු ret;
}

පූර්ණ_සංඛ්‍යා sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	පූර්ණ_සංඛ්‍යා ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		නම් (old_value > 0) {
			ආපසු 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} අතරතුර (ret == 0 || ret == -EAGAIN);

	ආපසු ret;
}

unsigned පූර්ණ_සංඛ්‍යා sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	පූර්ණ_සංඛ්‍යා value = atomic_get(&sem->futex.val);

	ආපසු (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
