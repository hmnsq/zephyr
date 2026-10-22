/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/toolchain.h>
#شامل <zephyr/linker/sections.h>
#شامل <zephyr/drivers/timer/system_timer.h>
#شامل <zephyr/llext/symbol.h>
#شامل <zephyr/pm/pm.h>
#شامل <stdbool.h>
#شامل <zephyr/logging/log.h>
/* private kernel APIs */
#شامل <ksched.h>
#شامل <kswap.h>
#شامل <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

خالی idle(خالی *unused1, خالی *unused2, خالی *unused3)
{
	ARG_غیر_مستعمل(unused1);
	ARG_غیر_مستعمل(unused2);
	ARG_غیر_مستعمل(unused3);

	__ASSERT_NO_MSG(_current->بنیاد.prio >= 0);

	جب (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		اگر (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			برائے (متلون صحیح i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_تبادلہ_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		غیرعلامتی صحیح کلید = arch_irq_تالا();

#اگر_تعریف_شدہ CONFIG_PM
		bool system_suspended = false;

		اگر (!k_is_pre_kernel()) {
			/* Value preserving: the helper returns either at most
			 * SYS_CLOCK_MAX_WAIT (== INT32_MAX) or K_TICKS_FOREVER.
			 */
			_kernel.idle = (int32_t)z_get_اگلا_timeout_expiry();

			/*
			 * Call the suspend hook function of the soc interface
			 * to allow entry into a low power state. The function
			 * returns false if low power state was not entered, in
			 * which case, kernel does normal idle processing.
			 */
			system_suspended = pm_system_suspend(_kernel.idle);
		}

		اگر (system_suspended) {
			arch_irq_unlock(کلید);
		} ورنہ {
			k_cpu_idle();
		}
#ورنہ
		ARG_غیر_مستعمل(کلید);
		k_cpu_idle();
#اگر_ختم /* CONFIG_PM */

#اگر !defined(CONFIG_PREEMPT_ENABLED)
# اگر !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		اگر (_kernel.ready_q.cache != _current) {
			z_تبادلہ_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#اگر_ختم /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

خالی __weak arch_spin_relax(خالی)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_علامت(arch_spin_relax);
