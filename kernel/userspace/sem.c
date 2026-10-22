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
	atomic_t old_درج, new_درج;

	افعل {
		old_درج = atomic_get(val);
		إذا (old_درج < minimum) {
			اكسر;
		}

		new_درج = old_درج - 1;
	} طالما (atomic_cas(val, old_درج, new_درج) == 0);

	أرجع old_درج;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_درج, new_درج;

	افعل {
		old_درج = atomic_get(val);
		إذا (old_درج >= maximum) {
			اكسر;
		}

		new_درج = ((old_درج < minimum) ? minimum : old_درج) + 1;
	} طالما (atomic_cas(val, old_درج, new_درج) == 0U);

	أرجع old_درج;
}

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_كدي,
		 unsigned صحيح limit)
{
	إذا ((sem == عدم) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_كدي > limit) || (limit > INT_MAX)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_set(&sem->futex.val, initial_كدي);
	sem->limit = limit;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_درج;

	old_درج = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	إذا (old_درج < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_درج >= sem->limit) {
		أرجع -EAGAIN;
	} وإلا {
		;
	}
	أرجع ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_درج;

	افعل {
		old_درج = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		إذا (old_درج > 0) {
			أرجع 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} طالما (ret == 0 || ret == -EAGAIN);

	أرجع ret;
}

unsigned صحيح sys_sem_كدي_get(بنية sys_sem *sem)
{
	صحيح درج = atomic_get(&sem->futex.val);

	أرجع (درج > SYS_SEM_MINIMUM) ? درج : SYS_SEM_MINIMUM;
}
