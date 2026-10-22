/* Copyright (c) Zephy Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/sem.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_init(ସଂରଚନା sys_sem *sem, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା initial_ଗଣନା,
		 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit)
{
	k_sem_init(&sem->kernel_sem, initial_ଗଣନା, limit);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_give(ସଂରଚନା sys_sem *sem)
{
	k_sem_give(&sem->kernel_sem);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_take(ସଂରଚନା sys_sem *sem, k_timeout_t timeout)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret_ମୂଲ୍ୟ = 0;

	ret_ମୂଲ୍ୟ = k_sem_take(&sem->kernel_sem, timeout);
	ଯଦି ((ret_ମୂଲ୍ୟ == -EAGAIN) || (ret_ମୂଲ୍ୟ == -EBUSY)) {
		ret_ମୂଲ୍ୟ = -ETIMEDOUT;
	}

	ଫେରନ୍ତୁ ret_ମୂଲ୍ୟ;
}

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା sys_sem_ଗଣନା_get(ସଂରଚନା sys_sem *sem)
{
	ଫେରନ୍ତୁ k_sem_ଗଣନା_get(&sem->kernel_sem);
}
