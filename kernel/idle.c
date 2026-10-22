/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/linker/sections.h>
#అన్తర్భావ <zephyr/drivers/timer/system_timer.h>
#అన్తర్భావ <zephyr/llext/symbol.h>
#అన్తర్భావ <zephyr/pm/pm.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <zephyr/logging/log.h>
/* private kernel APIs */
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

శూన్య idle(శూన్య *unused1, శూన్య *unused2, శూన్య *unused3)
{
	ARG_వాడని(unused1);
	ARG_వాడని(unused2);
	ARG_వాడని(unused3);

	__ASSERT_NO_MSG(_current->పునాది.prio >= 0);

	అయితే (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		ఒకవేళ (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			కొరకు (అస్థిర పూర్ణాంకం i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_మార్పిడి_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		గుర్తులేని పూర్ణాంకం కీలకం = arch_irq_తాళం();

#నిర్వచితమైతే CONFIG_PM
		bool system_suspended = false;

		ఒకవేళ (!k_is_pre_kernel()) {
			/* Value preserving: the helper returns either at most
			 * SYS_CLOCK_MAX_WAIT (== INT32_MAX) or K_TICKS_FOREVER.
			 */
			_kernel.idle = (int32_t)z_get_తదుపరి_timeout_expiry();

			/*
			 * Call the suspend hook function of the soc interface
			 * to allow entry into a low power state. The function
			 * returns false if low power state was not entered, in
			 * which case, kernel does normal idle processing.
			 */
			system_suspended = pm_system_suspend(_kernel.idle);
		}

		ఒకవేళ (system_suspended) {
			arch_irq_unlock(కీలకం);
		} లేకపోతే {
			k_cpu_idle();
		}
#లేకపోతే
		ARG_వాడని(కీలకం);
		k_cpu_idle();
#అయితే_ముగింపు /* CONFIG_PM */

#ఒకవేళ !defined(CONFIG_PREEMPT_ENABLED)
# ఒకవేళ !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		ఒకవేళ (_kernel.ready_q.cache != _current) {
			z_మార్పిడి_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#అయితే_ముగింపు /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

శూన్య __weak arch_spin_relax(శూన్య)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_చిహ్నం(arch_spin_relax);
