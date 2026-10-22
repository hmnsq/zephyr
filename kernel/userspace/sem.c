/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/sys/sem.h>
#ضمن <zephyr/internal/syscall_handler.h>

#تعریف_کرو SYS_SEM_MINIMUM      0
#تعریف_کرو SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

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
				   atomic_t ودھ_توں_ودھ)
{
	atomic_t old_قدر, new_قدر;

	افعل {
		old_قدر = atomic_get(val);
		إذا (old_قدر >= ودھ_توں_ودھ) {
			اكسر;
		}

		new_قدر = ((old_قدر < minimum) ? minimum : old_قدر) + 1;
	} طالما (atomic_cas(val, old_قدر, new_قدر) == 0U);

	أرجع old_قدر;
}

صحيح sys_sem_init(بنية sys_sem *sem, بنانشانی صحيح initial_گنتی,
		 بنانشانی صحيح حد_بندی)
{
	إذا ((sem == عدم) || (حد_بندی == SYS_SEM_MINIMUM) ||
	    (initial_گنتی > حد_بندی) || (حد_بندی > ودھ_توں_ودھ_عدد)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_سیٹ(&sem->futex.val, initial_گنتی);
	sem->حد_بندی = حد_بندی;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_قدر;

	old_قدر = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->حد_بندی);
	إذا (old_قدر < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_قدر >= sem->حد_بندی) {
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

بنانشانی صحيح sys_sem_گنتی_get(بنية sys_sem *sem)
{
	صحيح قدر = atomic_get(&sem->futex.val);

	أرجع (قدر > SYS_SEM_MINIMUM) ? قدر : SYS_SEM_MINIMUM;
}
