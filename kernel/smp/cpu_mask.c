/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_అంకె_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


స్థిర పూర్ణాంకం cpu_ముసుగు_mod(k_tid_t thread, uint32_t enable_ముసుగు, uint32_t disable_ముసుగు)
{
	పూర్ణాంకం ret = 0;

#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		ఒకవేళ (z_is_thread_prevented_from_running(thread)) {
			thread->పునాది.cpu_ముసుగు |= enable_ముసుగు;
			thread->పునాది.cpu_ముసుగు  &= ~disable_ముసుగు;

#ఒకవేళ defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY)
			uint32_t m = thread->పునాది.cpu_ముసుగు;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#అయితే_ముగింపు /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} లేకపోతే {
			ret = -EINVAL;
		}
	}

	తిరిగి ret;
}

పూర్ణాంకం k_thread_cpu_ముసుగు_clear(k_tid_t thread)
{
	తిరిగి cpu_ముసుగు_mod(thread, 0, 0xffffffff);
}

పూర్ణాంకం k_thread_cpu_ముసుగు_enable_all(k_tid_t thread)
{
	తిరిగి cpu_ముసుగు_mod(thread, 0xffffffff, 0);
}

పూర్ణాంకం k_thread_cpu_ముసుగు_enable(k_tid_t thread, పూర్ణాంకం cpu)
{
	తిరిగి cpu_ముసుగు_mod(thread, బిట్టు(cpu), 0);
}

పూర్ణాంకం k_thread_cpu_ముసుగు_disable(k_tid_t thread, పూర్ణాంకం cpu)
{
	తిరిగి cpu_ముసుగు_mod(thread, 0, బిట్టు(cpu));
}

పూర్ణాంకం k_thread_cpu_pin(k_tid_t thread, పూర్ణాంకం cpu)
{
	uint32_t ముసుగు = బిట్టు(cpu);

	తిరిగి cpu_ముసుగు_mod(thread, ముసుగు, ~ముసుగు);
}
