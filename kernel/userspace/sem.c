/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/sys/sem.h>
#شامل <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ایستا درونخطی atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_مقدار, new_مقدار;

	انجامبده {
		old_مقدار = atomic_get(val);
		اگر (old_مقدار < minimum) {
			بشکن;
		}

		new_مقدار = old_مقدار - 1;
	} وقتی (atomic_cas(val, old_مقدار, new_مقدار) == 0);

	بازگشت old_مقدار;
}

ایستا درونخطی atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_مقدار, new_مقدار;

	انجامبده {
		old_مقدار = atomic_get(val);
		اگر (old_مقدار >= maximum) {
			بشکن;
		}

		new_مقدار = ((old_مقدار < minimum) ? minimum : old_مقدار) + 1;
	} وقتی (atomic_cas(val, old_مقدار, new_مقدار) == 0U);

	بازگشت old_مقدار;
}

صحیح sys_sem_init(ساختار sys_sem *sem, unsigned صحیح initial_شمار,
		 unsigned صحیح limit)
{
	اگر ((sem == پوچ) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_شمار > limit) || (limit > INT_MAX)) {
		بازگشت -EINVAL;
	}

	(تهی)atomic_set(&sem->futex.val, initial_شمار);
	sem->limit = limit;

	بازگشت 0;
}

صحیح sys_sem_give(ساختار sys_sem *sem)
{
	صحیح ret = 0;
	atomic_t old_مقدار;

	old_مقدار = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	اگر (old_مقدار < 0) {
		ret = k_futex_wake(&sem->futex, true);

		اگر (ret > 0) {
			بازگشت 0;
		}
	} وگرنه اگر (old_مقدار >= sem->limit) {
		بازگشت -EAGAIN;
	} وگرنه {
		;
	}
	بازگشت ret;
}

صحیح sys_sem_take(ساختار sys_sem *sem, k_timeout_t timeout)
{
	صحیح ret = 0;
	atomic_t old_مقدار;

	انجامبده {
		old_مقدار = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		اگر (old_مقدار > 0) {
			بازگشت 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} وقتی (ret == 0 || ret == -EAGAIN);

	بازگشت ret;
}

unsigned صحیح sys_sem_شمار_get(ساختار sys_sem *sem)
{
	صحیح مقدار = atomic_get(&sem->futex.val);

	بازگشت (مقدار > SYS_SEM_MINIMUM) ? مقدار : SYS_SEM_MINIMUM;
}
