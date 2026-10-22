/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/sys/sem.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

நிலையான உள்வரி atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_மதிப்பு, new_மதிப்பு;

	செய் {
		old_மதிப்பு = atomic_get(val);
		எனில் (old_மதிப்பு < minimum) {
			நிறுத்து;
		}

		new_மதிப்பு = old_மதிப்பு - 1;
	} போது (atomic_cas(val, old_மதிப்பு, new_மதிப்பு) == 0);

	திரும்பு old_மதிப்பு;
}

நிலையான உள்வரி atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_மதிப்பு, new_மதிப்பு;

	செய் {
		old_மதிப்பு = atomic_get(val);
		எனில் (old_மதிப்பு >= maximum) {
			நிறுத்து;
		}

		new_மதிப்பு = ((old_மதிப்பு < minimum) ? minimum : old_மதிப்பு) + 1;
	} போது (atomic_cas(val, old_மதிப்பு, new_மதிப்பு) == 0U);

	திரும்பு old_மதிப்பு;
}

முழு sys_sem_init(கட்டமைப்பு sys_sem *sem, unsigned முழு initial_மொத்தம்,
		 unsigned முழு limit)
{
	எனில் ((sem == வெற்று) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_மொத்தம் > limit) || (limit > INT_MAX)) {
		திரும்பு -EINVAL;
	}

	(வெறுமை)atomic_set(&sem->futex.val, initial_மொத்தம்);
	sem->limit = limit;

	திரும்பு 0;
}

முழு sys_sem_give(கட்டமைப்பு sys_sem *sem)
{
	முழு ret = 0;
	atomic_t old_மதிப்பு;

	old_மதிப்பு = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	எனில் (old_மதிப்பு < 0) {
		ret = k_futex_wake(&sem->futex, true);

		எனில் (ret > 0) {
			திரும்பு 0;
		}
	} இல்லையெனில் எனில் (old_மதிப்பு >= sem->limit) {
		திரும்பு -EAGAIN;
	} இல்லையெனில் {
		;
	}
	திரும்பு ret;
}

முழு sys_sem_take(கட்டமைப்பு sys_sem *sem, k_timeout_t timeout)
{
	முழு ret = 0;
	atomic_t old_மதிப்பு;

	செய் {
		old_மதிப்பு = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		எனில் (old_மதிப்பு > 0) {
			திரும்பு 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} போது (ret == 0 || ret == -EAGAIN);

	திரும்பு ret;
}

unsigned முழு sys_sem_மொத்தம்_get(கட்டமைப்பு sys_sem *sem)
{
	முழு மதிப்பு = atomic_get(&sem->futex.val);

	திரும்பு (மதிப்பு > SYS_SEM_MINIMUM) ? மதிப்பு : SYS_SEM_MINIMUM;
}
