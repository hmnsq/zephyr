/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kspinlock.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_അക്കം_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


സ്ഥിതം പൂർണ്ണസംഖ്യ cpu_mask_mod(k_tid_t thread, uint32_t enable_mask, uint32_t disable_mask)
{
	പൂർണ്ണസംഖ്യ ret = 0;

#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		എങ്കിൽ (z_is_thread_prevented_from_running(thread)) {
			thread->base.cpu_mask |= enable_mask;
			thread->base.cpu_mask  &= ~disable_mask;

#എങ്കിൽ defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
			uint32_t m = thread->base.cpu_mask;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#endif /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} അല്ലെങ്കിൽ {
			ret = -EINVAL;
		}
	}

	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_thread_cpu_mask_clear(k_tid_t thread)
{
	തിരികെ cpu_mask_mod(thread, 0, 0xffffffff);
}

പൂർണ്ണസംഖ്യ k_thread_cpu_mask_enable_all(k_tid_t thread)
{
	തിരികെ cpu_mask_mod(thread, 0xffffffff, 0);
}

പൂർണ്ണസംഖ്യ k_thread_cpu_mask_enable(k_tid_t thread, പൂർണ്ണസംഖ്യ cpu)
{
	തിരികെ cpu_mask_mod(thread, BIT(cpu), 0);
}

പൂർണ്ണസംഖ്യ k_thread_cpu_mask_disable(k_tid_t thread, പൂർണ്ണസംഖ്യ cpu)
{
	തിരികെ cpu_mask_mod(thread, 0, BIT(cpu));
}

പൂർണ്ണസംഖ്യ k_thread_cpu_pin(k_tid_t thread, പൂർണ്ണസംഖ്യ cpu)
{
	uint32_t mask = BIT(cpu);

	തിരികെ cpu_mask_mod(thread, mask, ~mask);
}
