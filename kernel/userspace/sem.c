/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/sys/sem.h>
#ضمن <zephyr/internal/syscall_handler.h>

#تعریف_کن SYS_SEM_MINIMUM      0
#تعریف_کن SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ساكن مضمن atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_مقدار, new_مقدار;

	افعل {
		old_مقدار = atomic_get(val);
		إذا (old_مقدار < minimum) {
			اكسر;
		}

		new_مقدار = old_مقدار - 1;
	} طالما (atomic_cas(val, old_مقدار, new_مقدار) == 0);

	أرجع old_مقدار;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t بیشینه)
{
	atomic_t old_مقدار, new_مقدار;

	افعل {
		old_مقدار = atomic_get(val);
		إذا (old_مقدار >= بیشینه) {
			اكسر;
		}

		new_مقدار = ((old_مقدار < minimum) ? minimum : old_مقدار) + 1;
	} طالما (atomic_cas(val, old_مقدار, new_مقدار) == 0U);

	أرجع old_مقدار;
}

صحيح sys_sem_init(بنية sys_sem *sem, بیعلامت صحيح initial_شمار,
		 بیعلامت صحيح حد)
{
	إذا ((sem == عدم) || (حد == SYS_SEM_MINIMUM) ||
	    (initial_شمار > حد) || (حد > بیشترین_صحیح)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_مجموعه(&sem->futex.val, initial_شمار);
	sem->حد = حد;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_مقدار;

	old_مقدار = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->حد);
	إذا (old_مقدار < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_مقدار >= sem->حد) {
		أرجع -EAGAIN;
	} وإلا {
		;
	}
	أرجع ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_مقدار;

	افعل {
		old_مقدار = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		إذا (old_مقدار > 0) {
			أرجع 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} طالما (ret == 0 || ret == -EAGAIN);

	أرجع ret;
}

بیعلامت صحيح sys_sem_شمار_get(بنية sys_sem *sem)
{
	صحيح مقدار = atomic_get(&sem->futex.val);

	أرجع (مقدار > SYS_SEM_MINIMUM) ? مقدار : SYS_SEM_MINIMUM;
}
