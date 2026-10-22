/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/sys/sem.h>
#ضمن <zephyr/internal/syscall_handler.h>

#وصف_ڪريو SYS_SEM_MINIMUM      0
#وصف_ڪريو SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

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
				   atomic_t وڌ_کان_وڌ)
{
	atomic_t old_قدر, new_قدر;

	افعل {
		old_قدر = atomic_get(val);
		إذا (old_قدر >= وڌ_کان_وڌ) {
			اكسر;
		}

		new_قدر = ((old_قدر < minimum) ? minimum : old_قدر) + 1;
	} طالما (atomic_cas(val, old_قدر, new_قدر) == 0U);

	أرجع old_قدر;
}

صحيح sys_sem_init(بنية sys_sem *sem, بنانشان صحيح initial_ڳڻپ,
		 بنانشان صحيح حد_بندي)
{
	إذا ((sem == عدم) || (حد_بندي == SYS_SEM_MINIMUM) ||
	    (initial_ڳڻپ > حد_بندي) || (حد_بندي > وڌ_کان_وڌ_عدد)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_سيٽ(&sem->futex.val, initial_ڳڻپ);
	sem->حد_بندي = حد_بندي;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_قدر;

	old_قدر = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->حد_بندي);
	إذا (old_قدر < 0) {
		ret = k_futex_wake(&sem->futex, true);

		إذا (ret > 0) {
			أرجع 0;
		}
	} وإلا إذا (old_قدر >= sem->حد_بندي) {
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

بنانشان صحيح sys_sem_ڳڻپ_get(بنية sys_sem *sem)
{
	صحيح قدر = atomic_get(&sem->futex.val);

	أرجع (قدر > SYS_SEM_MINIMUM) ? قدر : SYS_SEM_MINIMUM;
}
