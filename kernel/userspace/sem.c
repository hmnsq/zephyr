/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/sys/sem.h>
#ضمن <zephyr/internal/syscall_handler.h>

#تعریف_کړه SYS_SEM_MINIMUM      0
#تعریف_کړه SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ساكن مضمن atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_ارزښت, new_ارزښت;

	افعل {
		old_ارزښت = atomic_get(val);
		إذا (old_ارزښت < minimum) {
			اكسر;
		}

		new_ارزښت = old_ارزښت - 1;
	} طالما (atomic_cas(val, old_ارزښت, new_ارزښت) == 0);

	أرجع old_ارزښت;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t اعظمي)
{
	atomic_t old_ارزښت, new_ارزښت;

	افعل {
		old_ارزښت = atomic_get(val);
		إذا (old_ارزښت >= اعظمي) {
			اكسر;
		}

		new_ارزښت = ((old_ارزښت < minimum) ? minimum : old_ارزښت) + 1;
	} طالما (atomic_cas(val, old_ارزښت, new_ارزښت) == 0U);

	أرجع old_ارزښت;
}

صحيح sys_sem_init(بنية sys_sem *sem, بینښه صحيح initial_شمېر,
		 بینښه صحيح حد)
{
	إذا ((sem == عدم) || (حد == SYS_SEM_MINIMUM) ||
	    (initial_شمېر > حد) || (حد > لوی_عدد)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_سېټ(&sem->futex.val, initial_شمېر);
	sem->حد = حد;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_ارزښت;

	old_ارزښت = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->حد);
	إذا (old_ارزښت < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_ارزښت >= sem->حد) {
		أرجع -EAGAIN;
	} وإلا {
		;
	}
	أرجع ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_ارزښت;

	افعل {
		old_ارزښت = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		إذا (old_ارزښت > 0) {
			أرجع 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} طالما (ret == 0 || ret == -EAGAIN);

	أرجع ret;
}

بینښه صحيح sys_sem_شمېر_get(بنية sys_sem *sem)
{
	صحيح ارزښت = atomic_get(&sem->futex.val);

	أرجع (ارزښت > SYS_SEM_MINIMUM) ? ارزښت : SYS_SEM_MINIMUM;
}
