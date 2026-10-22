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
	atomic_t old_value, new_value;

	𐤐𐤏𐤋 {
		old_value = atomic_get(val);
		𐤀𐤌 (old_value < minimum) {
			اكسر;
		}

		new_value = old_value - 1;
	} طالما (atomic_cas(val, old_value, new_value) == 0);

	أرجع old_value;
}

ساكن مضمن atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_value, new_value;

	𐤐𐤏𐤋 {
		old_value = atomic_get(val);
		𐤀𐤌 (old_value >= maximum) {
			اكسر;
		}

		new_value = ((old_value < minimum) ? minimum : old_value) + 1;
	} طالما (atomic_cas(val, old_value, new_value) == 0U);

	أرجع old_value;
}

صحيح sys_sem_init(بنية sys_sem *sem, unsigned صحيح initial_count,
		 unsigned صحيح limit)
{
	𐤀𐤌 ((sem == عدم) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_count > limit) || (limit > INT_MAX)) {
		أرجع -EINVAL;
	}

	(فراغ)atomic_set(&sem->futex.val, initial_count);
	sem->limit = limit;

	أرجع 0;
}

صحيح sys_sem_give(بنية sys_sem *sem)
{
	صحيح ret = 0;
	atomic_t old_value;

	old_value = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	𐤀𐤌 (old_value < 0) {
		ret = k_futex_wake(&sem->futex, true);

		𐤀𐤌 (ret > 0) {
			أرجع 0;
		}
	} وإلا 𐤀𐤌 (old_value >= sem->limit) {
		أرجع -EAGAIN;
	} وإلا {
		;
	}
	أرجع ret;
}

صحيح sys_sem_take(بنية sys_sem *sem, k_timeout_t timeout)
{
	صحيح ret = 0;
	atomic_t old_value;

	𐤐𐤏𐤋 {
		old_value = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		𐤀𐤌 (old_value > 0) {
			أرجع 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} طالما (ret == 0 || ret == -EAGAIN);

	أرجع ret;
}

unsigned صحيح sys_sem_count_get(بنية sys_sem *sem)
{
	صحيح value = atomic_get(&sem->futex.val);

	أرجع (value > SYS_SEM_MINIMUM) ? value : SYS_SEM_MINIMUM;
}
