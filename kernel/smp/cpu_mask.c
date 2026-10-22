/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <kspinlock.h>
#include <ksched.h>
#include <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_NUM_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


static số_nguyên cpu_mask_mod(k_tid_t thread, uint32_t enable_mask, uint32_t disable_mask)
{
	số_nguyên ret = 0;

#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		nếu (z_is_thread_prevented_from_running(thread)) {
			thread->base.cpu_mask |= enable_mask;
			thread->base.cpu_mask  &= ~disable_mask;

#nếu defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
			uint32_t m = thread->base.cpu_mask;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#endif /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} không_thì {
			ret = -EINVAL;
		}
	}

	trả_về ret;
}

số_nguyên k_thread_cpu_mask_clear(k_tid_t thread)
{
	trả_về cpu_mask_mod(thread, 0, 0xffffffff);
}

số_nguyên k_thread_cpu_mask_enable_all(k_tid_t thread)
{
	trả_về cpu_mask_mod(thread, 0xffffffff, 0);
}

số_nguyên k_thread_cpu_mask_enable(k_tid_t thread, số_nguyên cpu)
{
	trả_về cpu_mask_mod(thread, BIT(cpu), 0);
}

số_nguyên k_thread_cpu_mask_disable(k_tid_t thread, số_nguyên cpu)
{
	trả_về cpu_mask_mod(thread, 0, BIT(cpu));
}

số_nguyên k_thread_cpu_pin(k_tid_t thread, số_nguyên cpu)
{
	uint32_t mask = BIT(cpu);

	trả_về cpu_mask_mod(thread, mask, ~mask);
}
