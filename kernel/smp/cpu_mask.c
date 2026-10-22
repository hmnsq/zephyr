/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kspinlock.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_ଅଙ୍କ_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_mask_mod(k_tid_t thread, uint32_t enable_mask, uint32_t disable_mask)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		ଯଦି (z_is_thread_prevented_from_running(thread)) {
			thread->base.cpu_mask |= enable_mask;
			thread->base.cpu_mask  &= ~disable_mask;

#ଯଦି defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
			uint32_t m = thread->base.cpu_mask;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#endif /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} ଅନ୍ୟଥା {
			ret = -EINVAL;
		}
	}

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_cpu_mask_clear(k_tid_t thread)
{
	ଫେରନ୍ତୁ cpu_mask_mod(thread, 0, 0xffffffff);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_cpu_mask_enable_all(k_tid_t thread)
{
	ଫେରନ୍ତୁ cpu_mask_mod(thread, 0xffffffff, 0);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_cpu_mask_enable(k_tid_t thread, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu)
{
	ଫେରନ୍ତୁ cpu_mask_mod(thread, BIT(cpu), 0);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_cpu_mask_disable(k_tid_t thread, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu)
{
	ଫେରନ୍ତୁ cpu_mask_mod(thread, 0, BIT(cpu));
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_cpu_pin(k_tid_t thread, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu)
{
	uint32_t mask = BIT(cpu);

	ଫେରନ୍ତୁ cpu_mask_mod(thread, mask, ~mask);
}
