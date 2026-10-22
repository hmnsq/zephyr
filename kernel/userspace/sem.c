/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/sys/sem.h>
#شامل <zephyr/internal/syscall_handler.h>

#تعریف_کرو SYS_SEM_MINIMUM      0
#تعریف_کرو SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ساکن اندرونی atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_قدر, new_قدر;

	کرو {
		old_قدر = atomic_get(val);
		اگر (old_قدر < minimum) {
			توڑو;
		}

		new_قدر = old_قدر - 1;
	} جب (atomic_cas(val, old_قدر, new_قدر) == 0);

	واپس old_قدر;
}

ساکن اندرونی atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t زیادہ_سے_زیادہ)
{
	atomic_t old_قدر, new_قدر;

	کرو {
		old_قدر = atomic_get(val);
		اگر (old_قدر >= زیادہ_سے_زیادہ) {
			توڑو;
		}

		new_قدر = ((old_قدر < minimum) ? minimum : old_قدر) + 1;
	} جب (atomic_cas(val, old_قدر, new_قدر) == 0U);

	واپس old_قدر;
}

صحیح sys_sem_init(ساخت sys_sem *sem, غیرعلامتی صحیح initial_گنتی,
		 غیرعلامتی صحیح حد_بندی)
{
	اگر ((sem == تہی) || (حد_بندی == SYS_SEM_MINIMUM) ||
	    (initial_گنتی > حد_بندی) || (حد_بندی > زیادہ_سے_زیادہ_عدد)) {
		واپس -EINVAL;
	}

	(خالی)atomic_سیٹ(&sem->futex.val, initial_گنتی);
	sem->حد_بندی = حد_بندی;

	واپس 0;
}

صحیح sys_sem_give(ساخت sys_sem *sem)
{
	صحیح ret = 0;
	atomic_t old_قدر;

	old_قدر = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->حد_بندی);
	اگر (old_قدر < 0) {
		ret = k_futex_wake(&sem->futex, true);

		اگر (ret > 0) {
			واپس 0;
		}
	} ورنہ اگر (old_قدر >= sem->حد_بندی) {
		واپس -EAGAIN;
	} ورنہ {
		;
	}
	واپس ret;
}

صحیح sys_sem_take(ساخت sys_sem *sem, k_timeout_t timeout)
{
	صحیح ret = 0;
	atomic_t old_قدر;

	کرو {
		old_قدر = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		اگر (old_قدر > 0) {
			واپس 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} جب (ret == 0 || ret == -EAGAIN);

	واپس ret;
}

غیرعلامتی صحیح sys_sem_گنتی_get(ساخت sys_sem *sem)
{
	صحیح قدر = atomic_get(&sem->futex.val);

	واپس (قدر > SYS_SEM_MINIMUM) ? قدر : SYS_SEM_MINIMUM;
}
