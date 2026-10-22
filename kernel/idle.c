/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <zephyr/linker/sections.h>
#അന്തര്ഭാവ <zephyr/drivers/timer/system_timer.h>
#അന്തര്ഭാവ <zephyr/llext/symbol.h>
#അന്തര്ഭാവ <zephyr/pm/pm.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>
/* private kernel APIs */
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <kswap.h>
#അന്തര്ഭാവ <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ശൂന്യം idle(ശൂന്യം *unused1, ശൂന്യം *unused2, ശൂന്യം *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	__ASSERT_NO_MSG(_current->base.prio >= 0);

	ആകുമ്പോൾ (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		എങ്കിൽ (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			വേണ്ടി (അസ്ഥിര പൂർണ്ണസംഖ്യ i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_swap_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		unsigned പൂർണ്ണസംഖ്യ താക്കോൽ = arch_irq_lock();

#ifdef CONFIG_PM
		bool system_suspended = false;

		എങ്കിൽ (!k_is_pre_kernel()) {
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

		എങ്കിൽ (system_suspended) {
			arch_irq_unlock(താക്കോൽ);
		} അല്ലെങ്കിൽ {
			k_cpu_idle();
		}
#അല്ലെങ്കിൽ
		ARG_UNUSED(താക്കോൽ);
		k_cpu_idle();
#endif /* CONFIG_PM */

#എങ്കിൽ !defined(CONFIG_PREEMPT_ENABLED)
# എങ്കിൽ !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		എങ്കിൽ (_kernel.ready_q.cache != _current) {
			z_swap_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#endif /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

ശൂന്യം __weak arch_spin_relax(ശൂന്യം)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_ചിഹ്നം(arch_spin_relax);
