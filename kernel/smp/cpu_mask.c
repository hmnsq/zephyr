/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_ਅੰਕ_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


ਸਥਿਰ ਪੂਰਨ cpu_ਨਕਾਬ_mod(k_tid_t thread, uint32_t enable_ਨਕਾਬ, uint32_t disable_ਨਕਾਬ)
{
	ਪੂਰਨ ret = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		ਜੇ (z_is_thread_prevented_from_running(thread)) {
			thread->ਅਧਾਰ.cpu_ਨਕਾਬ |= enable_ਨਕਾਬ;
			thread->ਅਧਾਰ.cpu_ਨਕਾਬ  &= ~disable_ਨਕਾਬ;

#ਜੇ defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY)
			uint32_t m = thread->ਅਧਾਰ.cpu_ਨਕਾਬ;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#ਜੇ_ਅੰਤ /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} ਨਹੀਂਤਾਂ {
			ret = -EINVAL;
		}
	}

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_thread_cpu_ਨਕਾਬ_clear(k_tid_t thread)
{
	ਵਾਪਸ cpu_ਨਕਾਬ_mod(thread, 0, 0xffffffff);
}

ਪੂਰਨ k_thread_cpu_ਨਕਾਬ_enable_all(k_tid_t thread)
{
	ਵਾਪਸ cpu_ਨਕਾਬ_mod(thread, 0xffffffff, 0);
}

ਪੂਰਨ k_thread_cpu_ਨਕਾਬ_enable(k_tid_t thread, ਪੂਰਨ cpu)
{
	ਵਾਪਸ cpu_ਨਕਾਬ_mod(thread, ਦੋਆਧਾਰੀ_ਅੰਕ(cpu), 0);
}

ਪੂਰਨ k_thread_cpu_ਨਕਾਬ_disable(k_tid_t thread, ਪੂਰਨ cpu)
{
	ਵਾਪਸ cpu_ਨਕਾਬ_mod(thread, 0, ਦੋਆਧਾਰੀ_ਅੰਕ(cpu));
}

ਪੂਰਨ k_thread_cpu_pin(k_tid_t thread, ਪੂਰਨ cpu)
{
	uint32_t ਨਕਾਬ = ਦੋਆਧਾਰੀ_ਅੰਕ(cpu);

	ਵਾਪਸ cpu_ਨਕਾਬ_mod(thread, ਨਕਾਬ, ~ਨਕਾਬ);
}
