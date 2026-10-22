/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/sys/sem.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

സ്ഥിതം വരിയിൽ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_മൂല്യം, new_മൂല്യം;

	ചെയ്യുക {
		old_മൂല്യം = atomic_get(val);
		എങ്കിൽ (old_മൂല്യം < minimum) {
			നിർത്തുക;
		}

		new_മൂല്യം = old_മൂല്യം - 1;
	} ആകുമ്പോൾ (atomic_cas(val, old_മൂല്യം, new_മൂല്യം) == 0);

	തിരികെ old_മൂല്യം;
}

സ്ഥിതം വരിയിൽ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_മൂല്യം, new_മൂല്യം;

	ചെയ്യുക {
		old_മൂല്യം = atomic_get(val);
		എങ്കിൽ (old_മൂല്യം >= maximum) {
			നിർത്തുക;
		}

		new_മൂല്യം = ((old_മൂല്യം < minimum) ? minimum : old_മൂല്യം) + 1;
	} ആകുമ്പോൾ (atomic_cas(val, old_മൂല്യം, new_മൂല്യം) == 0U);

	തിരികെ old_മൂല്യം;
}

പൂർണ്ണസംഖ്യ sys_sem_init(ഘടന sys_sem *sem, unsigned പൂർണ്ണസംഖ്യ initial_എണ്ണം,
		 unsigned പൂർണ്ണസംഖ്യ limit)
{
	എങ്കിൽ ((sem == ശൂന്യ) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_എണ്ണം > limit) || (limit > INT_MAX)) {
		തിരികെ -EINVAL;
	}

	(ശൂന്യം)atomic_set(&sem->futex.val, initial_എണ്ണം);
	sem->limit = limit;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ sys_sem_give(ഘടന sys_sem *sem)
{
	പൂർണ്ണസംഖ്യ ret = 0;
	atomic_t old_മൂല്യം;

	old_മൂല്യം = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	എങ്കിൽ (old_മൂല്യം < 0) {
		ret = k_futex_wake(&sem->futex, true);

		എങ്കിൽ (ret > 0) {
			തിരികെ 0;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (old_മൂല്യം >= sem->limit) {
		തിരികെ -EAGAIN;
	} അല്ലെങ്കിൽ {
		;
	}
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ sys_sem_take(ഘടന sys_sem *sem, k_timeout_t timeout)
{
	പൂർണ്ണസംഖ്യ ret = 0;
	atomic_t old_മൂല്യം;

	ചെയ്യുക {
		old_മൂല്യം = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		എങ്കിൽ (old_മൂല്യം > 0) {
			തിരികെ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ആകുമ്പോൾ (ret == 0 || ret == -EAGAIN);

	തിരികെ ret;
}

unsigned പൂർണ്ണസംഖ്യ sys_sem_എണ്ണം_get(ഘടന sys_sem *sem)
{
	പൂർണ്ണസംഖ്യ മൂല്യം = atomic_get(&sem->futex.val);

	തിരികെ (മൂല്യം > SYS_SEM_MINIMUM) ? മൂല്യം : SYS_SEM_MINIMUM;
}
