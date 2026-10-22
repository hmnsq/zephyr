/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/linker/sections.h>
#અન્તર્ભાવ <zephyr/drivers/timer/system_timer.h>
#અન્તર્ભાવ <zephyr/llext/symbol.h>
#અન્તર્ભાવ <zephyr/pm/pm.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
/* private kernel APIs */
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <kswap.h>
#અન્તર્ભાવ <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

શૂન્ય idle(શૂન્ય *unused1, શૂન્ય *unused2, શૂન્ય *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	__ASSERT_NO_MSG(_current->base.prio >= 0);

	જ્યારે (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		જો (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			માટે (અસ્થિર પૂર્ણાંક i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_swap_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		unsigned પૂર્ણાંક ચાવી = arch_irq_lock();

#ifdef CONFIG_PM
		bool system_suspended = false;

		જો (!k_is_pre_kernel()) {
			/* Value preserving: the helper returns either at most
			 * SYS_CLOCK_MAX_WAIT (== INT32_MAX) or K_TICKS_FOREVER.
			 */
			_kernel.idle = (int32_t)z_get_next_timeout_expiry();

			/*
			 * Call the suspend hook function of the soc interface
			 * to allow entry into a low power state. The function
			 * returns false if low power state was not entered, in
			 * which case, kernel does normal idle processing.
			 */
			system_suspended = pm_system_suspend(_kernel.idle);
		}

		જો (system_suspended) {
			arch_irq_unlock(ચાવી);
		} નહીંતર {
			k_cpu_idle();
		}
#નહીંતર
		ARG_UNUSED(ચાવી);
		k_cpu_idle();
#endif /* CONFIG_PM */

#જો !defined(CONFIG_PREEMPT_ENABLED)
# જો !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		જો (_kernel.ready_q.cache != _current) {
			z_swap_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#endif /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

શૂન્ય __weak arch_spin_relax(શૂન્ય)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_પ્રતીક(arch_spin_relax);
