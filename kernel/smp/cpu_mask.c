/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#समावेश <zephyr/kernel.h>
#समावेश <kspinlock.h>
#समावेश <ksched.h>
#समावेश <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_अंक_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


अटल पूर्णांक cpu_mask_mod(k_tid_t thread, uint32_t enable_mask, uint32_t disable_mask)
{
	पूर्णांक ret = 0;

#अगर_घोषित CONFIG_SCHED_CPU_MASK_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		अगर (z_is_thread_prevented_from_running(thread)) {
			thread->base.cpu_mask |= enable_mask;
			thread->base.cpu_mask  &= ~disable_mask;

#अगर defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
			uint32_t m = thread->base.cpu_mask;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#पूर्ण_अगर /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} अन्यथा {
			ret = -EINVAL;
		}
	}

	वापस ret;
}

पूर्णांक k_thread_cpu_mask_clear(k_tid_t thread)
{
	वापस cpu_mask_mod(thread, 0, 0xffffffff);
}

पूर्णांक k_thread_cpu_mask_enable_all(k_tid_t thread)
{
	वापस cpu_mask_mod(thread, 0xffffffff, 0);
}

पूर्णांक k_thread_cpu_mask_enable(k_tid_t thread, पूर्णांक cpu)
{
	वापस cpu_mask_mod(thread, BIT(cpu), 0);
}

पूर्णांक k_thread_cpu_mask_disable(k_tid_t thread, पूर्णांक cpu)
{
	वापस cpu_mask_mod(thread, 0, BIT(cpu));
}

पूर्णांक k_thread_cpu_pin(k_tid_t thread, पूर्णांक cpu)
{
	uint32_t mask = BIT(cpu);

	वापस cpu_mask_mod(thread, mask, ~mask);
}
