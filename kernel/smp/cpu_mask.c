/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ضمن <zephyr/kernel.h>
#ضمن <kspinlock.h>
#ضمن <ksched.h>
#ضمن <zephyr/spinlock.h>

# ifdef CONFIG_SMP
/* The mask width scales with CONFIG_MP_MAX_NUM_CPUS, up to 32 bits */
BUILD_ASSERT(CONFIG_MP_MAX_انگ_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


ساكن صحيح cpu_نقاب_mod(k_tid_t thread, uint32_t enable_نقاب, uint32_t disable_نقاب)
{
	صحيح ret = 0;

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		إذا (z_is_thread_prevented_from_running(thread)) {
			thread->بنياد.cpu_نقاب |= enable_نقاب;
			thread->بنياد.cpu_نقاب  &= ~disable_نقاب;

#إذا defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_نقاب_PIN_ONLY)
			uint32_t m = thread->بنياد.cpu_نقاب;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#جيڪڏهن_پورو /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} وإلا {
			ret = -EINVAL;
		}
	}

	أرجع ret;
}

صحيح k_thread_cpu_نقاب_clear(k_tid_t thread)
{
	أرجع cpu_نقاب_mod(thread, 0, 0xffffffff);
}

صحيح k_thread_cpu_نقاب_enable_all(k_tid_t thread)
{
	أرجع cpu_نقاب_mod(thread, 0xffffffff, 0);
}

صحيح k_thread_cpu_نقاب_enable(k_tid_t thread, صحيح cpu)
{
	أرجع cpu_نقاب_mod(thread, ٻٽو_انگ(cpu), 0);
}

صحيح k_thread_cpu_نقاب_disable(k_tid_t thread, صحيح cpu)
{
	أرجع cpu_نقاب_mod(thread, 0, ٻٽو_انگ(cpu));
}

صحيح k_thread_cpu_pin(k_tid_t thread, صحيح cpu)
{
	uint32_t نقاب = ٻٽو_انگ(cpu);

	أرجع cpu_نقاب_mod(thread, نقاب, ~نقاب);
}
