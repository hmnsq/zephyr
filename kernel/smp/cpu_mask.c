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
BUILD_ASSERT(CONFIG_MP_MAX_رقم_CPUS <= 32, "Too many CPUs for mask word");
# endif /* CONFIG_SMP */


ساكن صحيح cpu_ماسک_mod(k_tid_t thread, uint32_t enable_ماسک, uint32_t disable_ماسک)
{
	صحيح ret = 0;

#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک_PIN_ONLY
	__ASSERT(z_is_thread_prevented_from_running(thread),
		 "Running threads cannot change CPU pin");
#پایان_اگر /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

	Z_SCHED_SPINLOCK {
		إذا (z_is_thread_prevented_from_running(thread)) {
			thread->پایه.cpu_ماسک |= enable_ماسک;
			thread->پایه.cpu_ماسک  &= ~disable_ماسک;

#إذا defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_ماسک_PIN_ONLY)
			uint32_t m = thread->پایه.cpu_ماسک;

			__ASSERT(m != 0 && (m & (m - 1)) == 0,
				 "PIN_ONLY requires exactly one CPU in mask");
#پایان_اگر /* defined(CONFIG_ASSERT) && defined(CONFIG_SCHED_CPU_MASK_PIN_ONLY) */

		} وإلا {
			ret = -EINVAL;
		}
	}

	أرجع ret;
}

صحيح k_thread_cpu_ماسک_clear(k_tid_t thread)
{
	أرجع cpu_ماسک_mod(thread, 0, 0xffffffff);
}

صحيح k_thread_cpu_ماسک_enable_all(k_tid_t thread)
{
	أرجع cpu_ماسک_mod(thread, 0xffffffff, 0);
}

صحيح k_thread_cpu_ماسک_enable(k_tid_t thread, صحيح cpu)
{
	أرجع cpu_ماسک_mod(thread, بیت(cpu), 0);
}

صحيح k_thread_cpu_ماسک_disable(k_tid_t thread, صحيح cpu)
{
	أرجع cpu_ماسک_mod(thread, 0, بیت(cpu));
}

صحيح k_thread_cpu_pin(k_tid_t thread, صحيح cpu)
{
	uint32_t ماسک = بیت(cpu);

	أرجع cpu_ماسک_mod(thread, ماسک, ~ماسک);
}
