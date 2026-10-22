/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/sys/sem.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>

#define SYS_SEM_MINIMUM      0
#define SYS_SEM_CONTENDED    (SYS_SEM_MINIMUM - 1)

ସ୍ଥିର ଧାଡ଼ିଭିତରେ atomic_t bounded_dec(atomic_t *val, atomic_t minimum)
{
	atomic_t old_ମୂଲ୍ୟ, new_ମୂଲ୍ୟ;

	କର {
		old_ମୂଲ୍ୟ = atomic_get(val);
		ଯଦି (old_ମୂଲ୍ୟ < minimum) {
			ଭାଙ୍ଗ;
		}

		new_ମୂଲ୍ୟ = old_ମୂଲ୍ୟ - 1;
	} ଯେତେବେଳେ (atomic_cas(val, old_ମୂଲ୍ୟ, new_ମୂଲ୍ୟ) == 0);

	ଫେରନ୍ତୁ old_ମୂଲ୍ୟ;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ atomic_t bounded_inc(atomic_t *val, atomic_t minimum,
				   atomic_t maximum)
{
	atomic_t old_ମୂଲ୍ୟ, new_ମୂଲ୍ୟ;

	କର {
		old_ମୂଲ୍ୟ = atomic_get(val);
		ଯଦି (old_ମୂଲ୍ୟ >= maximum) {
			ଭାଙ୍ଗ;
		}

		new_ମୂଲ୍ୟ = ((old_ମୂଲ୍ୟ < minimum) ? minimum : old_ମୂଲ୍ୟ) + 1;
	} ଯେତେବେଳେ (atomic_cas(val, old_ମୂଲ୍ୟ, new_ମୂଲ୍ୟ) == 0U);

	ଫେରନ୍ତୁ old_ମୂଲ୍ୟ;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_init(ସଂରଚନା sys_sem *sem, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା initial_ଗଣନା,
		 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit)
{
	ଯଦି ((sem == ଖାଲି) || (limit == SYS_SEM_MINIMUM) ||
	    (initial_ଗଣନା > limit) || (limit > INT_MAX)) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	(ଶୂନ୍ୟ)atomic_set(&sem->futex.val, initial_ଗଣନା);
	sem->limit = limit;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_give(ସଂରଚନା sys_sem *sem)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	atomic_t old_ମୂଲ୍ୟ;

	old_ମୂଲ୍ୟ = bounded_inc(&sem->futex.val,
				SYS_SEM_MINIMUM, sem->limit);
	ଯଦି (old_ମୂଲ୍ୟ < 0) {
		ret = k_futex_wake(&sem->futex, true);

		ଯଦି (ret > 0) {
			ଫେରନ୍ତୁ 0;
		}
	} ଅନ୍ୟଥା ଯଦି (old_ମୂଲ୍ୟ >= sem->limit) {
		ଫେରନ୍ତୁ -EAGAIN;
	} ଅନ୍ୟଥା {
		;
	}
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_take(ସଂରଚନା sys_sem *sem, k_timeout_t timeout)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	atomic_t old_ମୂଲ୍ୟ;

	କର {
		old_ମୂଲ୍ୟ = bounded_dec(&sem->futex.val,
					SYS_SEM_MINIMUM);
		ଯଦି (old_ମୂଲ୍ୟ > 0) {
			ଫେରନ୍ତୁ 0;
		}

		ret = k_futex_wait(&sem->futex,
				   SYS_SEM_CONTENDED, timeout);
	} ଯେତେବେଳେ (ret == 0 || ret == -EAGAIN);

	ଫେରନ୍ତୁ ret;
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_ଗଣନା_get(ସଂରଚନା sys_sem *sem)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ = atomic_get(&sem->futex.val);

	ଫେରନ୍ତୁ (ମୂଲ୍ୟ > SYS_SEM_MINIMUM) ? ମୂଲ୍ୟ : SYS_SEM_MINIMUM;
}
