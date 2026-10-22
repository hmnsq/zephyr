/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_ಅಂಕೆ_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cpu_mask_mod(k_tid_t thread, uint32_t enable_mask, uint32_t disable_mask)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		ಯದಿ (z_is_thread_prevented_from_running(thread)) {
			thread->base.cpu_mask |= enable_mask;
			thread->base.cpu_mask  &= ~disable_mask;

#ಯದಿ defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY)
			uint32_t m = thread->base.cpu_mask;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#endif /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} ಅನ್ಯಥಾ {
			ret = -EINVAL;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_thread_cpu_mask_clear(k_tid_t thread)
{
	ಪ್ರತ್ಯಾಗಮ cpu_mask_mod(thread, 0, 0xffffffff);
}

ಪೂರ್ಣಾಙ್ಕ k_thread_cpu_mask_enable_all(k_tid_t thread)
{
	ಪ್ರತ್ಯಾಗಮ cpu_mask_mod(thread, 0xffffffff, 0);
}

ಪೂರ್ಣಾಙ್ಕ k_thread_cpu_mask_enable(k_tid_t thread, ಪೂರ್ಣಾಙ್ಕ cpu)
{
	ಪ್ರತ್ಯಾಗಮ cpu_mask_mod(thread, BIT(cpu), 0);
}

ಪೂರ್ಣಾಙ್ಕ k_thread_cpu_mask_disable(k_tid_t thread, ಪೂರ್ಣಾಙ್ಕ cpu)
{
	ಪ್ರತ್ಯಾಗಮ cpu_mask_mod(thread, 0, BIT(cpu));
}

ಪೂರ್ಣಾಙ್ಕ k_thread_cpu_pin(k_tid_t thread, ಪೂರ್ಣಾಙ್ಕ cpu)
{
	uint32_t mask = BIT(cpu);

	ಪ್ರತ್ಯಾಗಮ cpu_mask_mod(thread, mask, ~mask);
}
