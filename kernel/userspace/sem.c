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
	atomic_t old_قيمة, new_قيمة;

	افعل {
		old_قيمة = atomic_get(val);
		إذا (old_قيمة < minimum) {
			اكسر;
		}

		new_قيمة = old_قيمة - 1;
	} طالما (atomic_cas(val, old_قيمة, new_قيمة) == 0);

	أرجع old_قيمة;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_قيمة, new_قيمة;

	افعل {
		old_قيمة = atomic_get(val);
		إذا (old_قيمة >= maximum) {
			اكسر;
		}

		new_قيمة = ((old_قيمة < minimum) ? minimum : old_قيمة) + 1;
	} طالما (atomic_cas(val, old_قيمة, new_قيمة) == 0U);

	أرجع old_قيمة;
}

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_عدد,
		 unsigned صحيح limit)
{
	إذا ((sem == عدم) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_عدد > limit) || (limit > INT_MAX)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_set(&sem->futex.val, initial_عدد);
	sem->limit = limit;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_قيمة;

	old_قيمة = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	إذا (old_قيمة < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_قيمة >= sem->limit) {
		أرجع -EAGAIN;
	} وإلا {
		;
	}
	أرجع ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_قيمة;

	افعل {
		old_قيمة = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		إذا (old_قيمة > 0) {
			أرجع 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} طالما (ret == 0 || ret == -EAGAIN);

	أرجع ret;
}

unsigned صحيح sys_sem_عدد_get(بنية sys_sem *sem)
{
	صحيح قيمة = atomic_get(&sem->futex.val);

	أرجع (قيمة > SYS_SEM_MINIMUM) ? قيمة : SYS_SEM_MINIMUM;
}
