/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/sys/sem.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

স্থির অন্তঃরেখ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_মান, new_মান;

	করো {
		old_মান = atomic_get(val);
		যদি (old_মান < minimum) {
			বিরতি;
		}

		new_মান = old_মান - 1;
	} যখন (atomic_cas(val, old_মান, new_মান) == 0);

	ফেরত old_মান;
}

স্থির অন্তঃরেখ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_মান, new_মান;

	করো {
		old_মান = atomic_get(val);
		যদি (old_মান >= maximum) {
			বিরতি;
		}

		new_মান = ((old_মান < minimum) ? minimum : old_মান) + 1;
	} যখন (atomic_cas(val, old_মান, new_মান) == 0U);

	ফেরত old_মান;
}

পূর্ণসংখ্যা sys_sem_init(কাঠামো sys_sem *sem, unsigned পূর্ণসংখ্যা initial_গুনতি,
		 unsigned পূর্ণসংখ্যা limit)
{
	যদি ((sem == ফাঁকা) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_গুনতি > limit) || (limit > INT_MAX)) {
		ফেরত -EINVAL;
	}

	(শূন্য)atomic_set(&sem->futex.val, initial_গুনতি);
	sem->limit = limit;

	ফেরত 0;
}

পূর্ণসংখ্যা sys_sem_give(কাঠামো sys_sem *sem)
{
	পূর্ণসংখ্যা ret = 0;
	atomic_t old_মান;

	old_মান = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	যদি (old_মান < 0) {
		ret = k_futex_wake(&sem->futex, true);

		যদি (ret > 0) {
			ফেরত 0;
		}
	} অন্যথা যদি (old_মান >= sem->limit) {
		ফেরত -EAGAIN;
	} অন্যথা {
		;
	}
	ফেরত ret;
}

পূর্ণসংখ্যা sys_sem_take(কাঠামো sys_sem *sem, k_timeout_t timeout)
{
	পূর্ণসংখ্যা ret = 0;
	atomic_t old_মান;

	করো {
		old_মান = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		যদি (old_মান > 0) {
			ফেরত 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} যখন (ret == 0 || ret == -EAGAIN);

	ফেরত ret;
}

unsigned পূর্ণসংখ্যা sys_sem_গুনতি_get(কাঠামো sys_sem *sem)
{
	পূর্ণসংখ্যা মান = atomic_get(&sem->futex.val);

	ফেরত (মান > SYS_SEM_MINIMUM) ? মান : SYS_SEM_MINIMUM;
}
