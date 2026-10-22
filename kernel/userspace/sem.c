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
	atomic_t old_قىممەت, new_قىممەت;

	افعل {
		old_قىممەت = atomic_get(val);
		إذا (old_قىممەت < minimum) {
			اكسر;
		}

		new_قىممەت = old_قىممەت - 1;
	} طالما (atomic_cas(val, old_قىممەت, new_قىممەت) == 0);

	أرجع old_قىممەت;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_قىممەت, new_قىممەت;

	افعل {
		old_قىممەت = atomic_get(val);
		إذا (old_قىممەت >= maximum) {
			اكسر;
		}

		new_قىممەت = ((old_قىممەت < minimum) ? minimum : old_قىممەت) + 1;
	} طالما (atomic_cas(val, old_قىممەت, new_قىممەت) == 0U);

	أرجع old_قىممەت;
}

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_سانى,
		 unsigned صحيح limit)
{
	إذا ((sem == عدم) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_سانى > limit) || (limit > INT_MAX)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_set(&sem->futex.val, initial_سانى);
	sem->limit = limit;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_قىممەت;

	old_قىممەت = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	إذا (old_قىممەت < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_قىممەت >= sem->limit) {
		أرجع -EAGAIN;
	} وإلا {
		;
	}
	أرجع ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_قىممەت;

	افعل {
		old_قىممەت = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		إذا (old_قىممەت > 0) {
			أرجع 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} طالما (ret == 0 || ret == -EAGAIN);

	أرجع ret;
}

unsigned صحيح sys_sem_سانى_get(بنية sys_sem *sem)
{
	صحيح قىممەت = atomic_get(&sem->futex.val);

	أرجع (قىممەت > SYS_SEM_MINIMUM) ? قىممەت : SYS_SEM_MINIMUM;
}
