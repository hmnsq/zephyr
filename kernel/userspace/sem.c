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
	atomic_t old_قدر, new_قدر;

	افعل {
		old_قدر = atomic_get(val);
		إذا (old_قدر < minimum) {
			اكسر;
		}

		new_قدر = old_قدر - 1;
	} طالما (atomic_cas(val, old_قدر, new_قدر) == 0);

	أرجع old_قدر;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_قدر, new_قدر;

	افعل {
		old_قدر = atomic_get(val);
		إذا (old_قدر >= maximum) {
			اكسر;
		}

		new_قدر = ((old_قدر < minimum) ? minimum : old_قدر) + 1;
	} طالما (atomic_cas(val, old_قدر, new_قدر) == 0U);

	أرجع old_قدر;
}

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_گنتی,
		 unsigned صحيح limit)
{
	إذا ((sem == عدم) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_گنتی > limit) || (limit > INT_MAX)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_set(&sem->futex.val, initial_گنتی);
	sem->limit = limit;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_قدر;

	old_قدر = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	إذا (old_قدر < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_قدر >= sem->limit) {
		أرجع -EAGAIN;
	} وإلا {
		;
	}
	أرجع ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_قدر;

	افعل {
		old_قدر = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		إذا (old_قدر > 0) {
			أرجع 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} طالما (ret == 0 || ret == -EAGAIN);

	أرجع ret;
}

unsigned صحيح sys_sem_گنتی_get(بنية sys_sem *sem)
{
	صحيح قدر = atomic_get(&sem->futex.val);

	أرجع (قدر > SYS_SEM_MINIMUM) ? قدر : SYS_SEM_MINIMUM;
}
