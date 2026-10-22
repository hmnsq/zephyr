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
	atomic_t old_हालागा, new_हालागा;

	do {
		old_हालागा = atomic_get(val);
		if (old_हालागा < minimum) {
			break;
		}

		new_हालागा = old_हालागा - 1;
	} while (atomic_cas(val, old_हालागा, new_हालागा) == 0);

	return old_हालागा;
}

static inline atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_हालागा, new_हालागा;

	do {
		old_हालागा = atomic_get(val);
		if (old_हालागा >= maximum) {
			break;
		}

		new_हालागा = ((old_हालागा < minimum) ? minimum : old_हालागा) + 1;
	} while (atomic_cas(val, old_हालागा, new_हालागा) == 0U);

	return old_हालागा;
}

int sys_sem_init(struct sys_sem *sem, unsigned int initial_बिलांग,
		 unsigned int limit)
{
	if ((sem == NULL) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_बिलांग > limit) || (limit > INT_MAX)) {
		return -EINVAL;
	}

	(void)atomic_set(&sem->futex.val, initial_बिलांग);
	sem->limit = limit;

	return 0;
}

int sys_sem_give(struct sys_sem *sem)
{
	int ret = 0;
	atomic_t old_हालागा;

	old_हालागा = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	if (old_हालागा < 0) {
		ret = k_futex_wake(&sem->futex, true);

		if (ret > 0) {
			return 0;
		}
	} else if (old_हालागा >= sem->limit) {
		return -EAGAIN;
	} else {
		;
	}
	return ret;
}

int sys_sem_take(struct sys_sem *sem, k_timeout_t timeout)
{
	int ret = 0;
	atomic_t old_हालागा;

	do {
		old_हालागा = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		if (old_हालागा > 0) {
			return 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} while (ret == 0 || ret == -EAGAIN);

	return ret;
}

unsigned int sys_sem_बिलांग_get(struct sys_sem *sem)
{
	int हालागा = atomic_get(&sem->futex.val);

	return (हालागा > SYS_SEM_MINIMUM) ? हालागा : SYS_SEM_MINIMUM;
}
