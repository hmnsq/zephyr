/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kspinlock.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_અંક_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


સ્થિર પૂર્ણાંક cpu_mask_mod(k_tid_t thread, uint32_t enable_mask, uint32_t disable_mask)
{
	પૂર્ણાંક ret = 0;

#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		જો (z_is_thread_prevented_from_running(thread)) {
			thread->base.cpu_mask |= enable_mask;
			thread->base.cpu_mask  &= ~disable_mask;

#જો defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
			uint32_t m = thread->base.cpu_mask;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#endif /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} નહીંતર {
			ret = -EINVAL;
		}
	}

	પાછા ret;
}

પૂર્ણાંક k_thread_cpu_mask_clear(k_tid_t thread)
{
	પાછા cpu_mask_mod(thread, 0, 0xffffffff);
}

પૂર્ણાંક k_thread_cpu_mask_enable_all(k_tid_t thread)
{
	પાછા cpu_mask_mod(thread, 0xffffffff, 0);
}

પૂર્ણાંક k_thread_cpu_mask_enable(k_tid_t thread, પૂર્ણાંક cpu)
{
	પાછા cpu_mask_mod(thread, BIT(cpu), 0);
}

પૂર્ણાંક k_thread_cpu_mask_disable(k_tid_t thread, પૂર્ણાંક cpu)
{
	પાછા cpu_mask_mod(thread, 0, BIT(cpu));
}

પૂર્ણાંક k_thread_cpu_pin(k_tid_t thread, પૂર્ણાંક cpu)
{
	uint32_t mask = BIT(cpu);

	પાછા cpu_mask_mod(thread, mask, ~mask);
}
