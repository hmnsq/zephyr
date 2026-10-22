/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kspinlock.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_अङ्कः_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


स्थिर पूर्णाङ्क cpu_आवरणम्_mod(k_tid_t thread, uint32_t enable_आवरणम्, uint32_t disable_आवरणम्)
{
	पूर्णाङ्क ret = 0;

#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		यदि (z_is_thread_prevented_from_running(thread)) {
			thread->आधारः.cpu_आवरणम् |= enable_आवरणम्;
			thread->आधारः.cpu_आवरणम्  &= ~disable_आवरणम्;

#यदि defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY)
			uint32_t m = thread->आधारः.cpu_आवरणम्;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#यदि_अन्तः /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} अन्यथा {
			ret = -EINVAL;
		}
	}

	प्रत्यागम ret;
}

पूर्णाङ्क k_thread_cpu_आवरणम्_clear(k_tid_t thread)
{
	प्रत्यागम cpu_आवरणम्_mod(thread, 0, 0xffffffff);
}

पूर्णाङ्क k_thread_cpu_आवरणम्_enable_all(k_tid_t thread)
{
	प्रत्यागम cpu_आवरणम्_mod(thread, 0xffffffff, 0);
}

पूर्णाङ्क k_thread_cpu_आवरणम्_enable(k_tid_t thread, पूर्णाङ्क cpu)
{
	प्रत्यागम cpu_आवरणम्_mod(thread, BIT(cpu), 0);
}

पूर्णाङ्क k_thread_cpu_आवरणम्_disable(k_tid_t thread, पूर्णाङ्क cpu)
{
	प्रत्यागम cpu_आवरणम्_mod(thread, 0, BIT(cpu));
}

पूर्णाङ्क k_thread_cpu_pin(k_tid_t thread, पूर्णाङ्क cpu)
{
	uint32_t आवरणम् = BIT(cpu);

	प्रत्यागम cpu_आवरणम्_mod(thread, आवरणम्, ~आवरणम्);
}
