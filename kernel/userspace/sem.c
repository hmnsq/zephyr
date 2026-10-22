/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/sys/sem.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

স্থির অন্তঃস্থ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_মান, new_মান;

	কৰা {
		old_মান = atomic_get(val);
		যদি (old_মান < minimum) {
			ভঙ্গ;
		}

		new_মান = old_মান - 1;
	} যেতিয়ালৈ (atomic_cas(val, old_মান, new_মান) == 0);

	ঘূৰাই old_মান;
}

স্থির অন্তঃস্থ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_মান, new_মান;

	কৰা {
		old_মান = atomic_get(val);
		যদি (old_মান >= maximum) {
			ভঙ্গ;
		}

		new_মান = ((old_মান < minimum) ? minimum : old_মান) + 1;
	} যেতিয়ালৈ (atomic_cas(val, old_মান, new_মান) == 0U);

	ঘূৰাই old_মান;
}

পূর্ণাঙ্ক sys_sem_init(সংরচনা sys_sem *sem, unsigned পূর্ণাঙ্ক initial_গুনতি,
		 unsigned পূর্ণাঙ্ক limit)
{
	যদি ((sem == রিক্ত) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_গুনতি > limit) || (limit > INT_MAX)) {
		ঘূৰাই -EINVAL;
	}

	(শূন্য)atomic_set(&sem->futex.val, initial_গুনতি);
	sem->limit = limit;

	ঘূৰাই 0;
}

পূর্ণাঙ্ক sys_sem_give(সংরচনা sys_sem *sem)
{
	পূর্ণাঙ্ক ret = 0;
	atomic_t old_মান;

	old_মান = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	যদি (old_মান < 0) {
		ret = k_futex_wake(&sem->futex, true);

		যদি (ret > 0) {
			ঘূৰাই 0;
		}
	} অন্যথা যদি (old_মান >= sem->limit) {
		ঘূৰাই -EAGAIN;
	} অন্যথা {
		;
	}
	ঘূৰাই ret;
}

পূর্ণাঙ্ক sys_sem_take(সংরচনা sys_sem *sem, k_timeout_t timeout)
{
	পূর্ণাঙ্ক ret = 0;
	atomic_t old_মান;

	কৰা {
		old_মান = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		যদি (old_মান > 0) {
			ঘূৰাই 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} যেতিয়ালৈ (ret == 0 || ret == -EAGAIN);

	ঘূৰাই ret;
}

unsigned পূর্ণাঙ্ক sys_sem_গুনতি_get(সংরচনা sys_sem *sem)
{
	পূর্ণাঙ্ক মান = atomic_get(&sem->futex.val);

	ঘূৰাই (মান > SYS_SEM_MINIMUM) ? মান : SYS_SEM_MINIMUM;
}
