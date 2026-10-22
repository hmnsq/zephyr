/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/sys/sem.h>
#ضمن <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ساكن مضمن atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_value, new_value;

	ܥܒܕ {
		old_value = atomic_get(val);
		ܐܢ (old_value < minimum) {
			اكسر;
		}

		new_value = old_value - 1;
	} ܟܡܐ (atomic_cas(val, old_value, new_value) == 0);

	ܦܢܐ old_value;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	ܥܒܕ {
		old_value = atomic_get(val);
		ܐܢ (old_value >= maximum) {
			اكسر;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} ܟܡܐ (atomic_cas(val, old_value, new_value) == 0U);

	ܦܢܐ old_value;
}

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_count,
		 unsigned صحيح limit)
{
	ܐܢ ((sem == عدم) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		ܦܢܐ -EINVAL;
	}

	(فراغ)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	ܦܢܐ 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ܐܢ (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ܐܢ (ret > 0) {
			ܦܢܐ 0;
		}
	} ܐܚܪܢܐ ܐܢ (old_value >= sem->limit) {
		ܦܢܐ -EAGAIN;
	} ܐܚܪܢܐ {
		;
	}
	ܦܢܐ ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_value;

	ܥܒܕ {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ܐܢ (old_value > 0) {
			ܦܢܐ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ܟܡܐ (ret == 0 || ret == -EAGAIN);

	ܦܢܐ ret;
}

unsigned صحيح sys_sem_count_get(بنية sys_sem *sem)
{
	صحيح value = atomic_get(&sem->futex.val);

	ܦܢܐ (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
