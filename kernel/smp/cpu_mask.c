/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/kernel.h>
#شامل <kspinlock.h>
#شامل <ksched.h>
#شامل <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_عدد_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


ساکن صحیح cpu_نقاب_mod(k_tid_t thread, uint32_t enable_نقاب, uint32_t disable_نقاب)
{
	صحیح ret = 0;

#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#اگر_ختم /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		اگر (z_is_thread_prevented_from_running(thread)) {
			thread->بنیاد.cpu_نقاب |= enable_نقاب;
			thread->بنیاد.cpu_نقاب  &= ~disable_نقاب;

#اگر defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
			uint32_t m = thread->بنیاد.cpu_نقاب;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#اگر_ختم /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} ورنہ {
			ret = -EINVAL;
		}
	}

	واپس ret;
}

صحیح k_thread_cpu_نقاب_clear(k_tid_t thread)
{
	واپس cpu_نقاب_mod(thread, 0, 0xffffffff);
}

صحیح k_thread_cpu_نقاب_enable_all(k_tid_t thread)
{
	واپس cpu_نقاب_mod(thread, 0xffffffff, 0);
}

صحیح k_thread_cpu_نقاب_enable(k_tid_t thread, صحیح cpu)
{
	واپس cpu_نقاب_mod(thread, ثنائی_ہندسہ(cpu), 0);
}

صحیح k_thread_cpu_نقاب_disable(k_tid_t thread, صحیح cpu)
{
	واپس cpu_نقاب_mod(thread, 0, ثنائی_ہندسہ(cpu));
}

صحیح k_thread_cpu_pin(k_tid_t thread, صحیح cpu)
{
	uint32_t نقاب = ثنائی_ہندسہ(cpu);

	واپس cpu_نقاب_mod(thread, نقاب, ~نقاب);
}
