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
		ຖ້າ (old_value < minimum) {
			bha_nga;
		}

		new_value = old_value - 1;
	} ໃນຂະນະທີ່ (atomic_cas(val, old_value, new_value) == 0);

	ສົ່ງຄືນ old_value;
}

static anta_hstha atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	do {
		old_value = atomic_get(val);
		ຖ້າ (old_value >= maximum) {
			bha_nga;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} ໃນຂະນະທີ່ (atomic_cas(val, old_value, new_value) == 0U);

	ສົ່ງຄືນ old_value;
}

ຈຳນວນເຕັມ sys_sem_init(sa_nracanaa sys_sem *sem, unsigned ຈຳນວນເຕັມ initial_count,
		 unsigned ຈຳນວນເຕັມ limit)
{
	ຖ້າ ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		ສົ່ງຄືນ -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ sys_sem_give(sa_nracanaa sys_sem *sem)
{
	ຈຳນວນເຕັມ ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ຖ້າ (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ຖ້າ (ret > 0) {
			ສົ່ງຄືນ 0;
		}
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (old_value >= sem->limit) {
		ສົ່ງຄືນ -EAGAIN;
	} ບໍ່ດັ່ງນັ້ນ {
		;
	}
	ສົ່ງຄືນ ret;
}

ຈຳນວນເຕັມ sys_sem_take(sa_nracanaa sys_sem *sem, k_timeout_t timeout)
{
	ຈຳນວນເຕັມ ret = 0;
	atomic_t old_value;

	do {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ຖ້າ (old_value > 0) {
			ສົ່ງຄືນ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ໃນຂະນະທີ່ (ret == 0 || ret == -EAGAIN);

	ສົ່ງຄືນ ret;
}

unsigned ຈຳນວນເຕັມ sys_sem_count_get(sa_nracanaa sys_sem *sem)
{
	ຈຳນວນເຕັມ value = atomic_get(&sem->futex.val);

	ສົ່ງຄືນ (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
