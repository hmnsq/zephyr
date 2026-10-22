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


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cpu_ಮುಖವಾಡ_mod(k_tid_t thread, uint32_t enable_ಮುಖವಾಡ, uint32_t disable_ಮುಖವಾಡ)
{
	ಪೂರ್ಣಾಂಕ ret = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		ಆದರೆ (z_is_thread_prevented_from_running(thread)) {
			thread->ತಳ.cpu_ಮುಖವಾಡ |= enable_ಮುಖವಾಡ;
			thread->ತಳ.cpu_ಮುಖವಾಡ  &= ~disable_ಮುಖವಾಡ;

#ಆದರೆ defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY)
			uint32_t m = thread->ತಳ.cpu_ಮುಖವಾಡ;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#ಆದರೆ_ಅಂತ್ಯ /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} ಇಲ್ಲದಿದ್ದರೆ {
			ret = -EINVAL;
		}
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_thread_cpu_ಮುಖವಾಡ_clear(k_tid_t thread)
{
	ಹಿಂತಿರುಗಿ cpu_ಮುಖವಾಡ_mod(thread, 0, 0xffffffff);
}

ಪೂರ್ಣಾಂಕ k_thread_cpu_ಮುಖವಾಡ_enable_all(k_tid_t thread)
{
	ಹಿಂತಿರುಗಿ cpu_ಮುಖವಾಡ_mod(thread, 0xffffffff, 0);
}

ಪೂರ್ಣಾಂಕ k_thread_cpu_ಮುಖವಾಡ_enable(k_tid_t thread, ಪೂರ್ಣಾಂಕ cpu)
{
	ಹಿಂತಿರುಗಿ cpu_ಮುಖವಾಡ_mod(thread, ಬಿಟ್(cpu), 0);
}

ಪೂರ್ಣಾಂಕ k_thread_cpu_ಮುಖವಾಡ_disable(k_tid_t thread, ಪೂರ್ಣಾಂಕ cpu)
{
	ಹಿಂತಿರುಗಿ cpu_ಮುಖವಾಡ_mod(thread, 0, ಬಿಟ್(cpu));
}

ಪೂರ್ಣಾಂಕ k_thread_cpu_pin(k_tid_t thread, ಪೂರ್ಣಾಂಕ cpu)
{
	uint32_t ಮುಖವಾಡ = ಬಿಟ್(cpu);

	ಹಿಂತಿರುಗಿ cpu_ಮುಖವಾಡ_mod(thread, ಮುಖವಾಡ, ~ಮುಖವಾಡ);
}
