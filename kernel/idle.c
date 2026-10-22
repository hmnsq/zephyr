/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/linker/sections.h>
#ضمن <zephyr/drivers/timer/system_timer.h>
#ضمن <zephyr/llext/symbol.h>
#ضمن <zephyr/pm/pm.h>
#ضمن <stdbool.h>
#ضمن <zephyr/logging/log.h>
/* private kernel APIs */
#ضمن <ksched.h>
#ضمن <kswap.h>
#ضمن <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

فراغ idle(فراغ *unused1, فراغ *unused2, فراغ *unused3)
{
	ARG_استفادهنشده(unused1);
	ARG_استفادهنشده(unused2);
	ARG_استفادهنشده(unused3);

	__ASSERT_NO_MSG(_current->پایه.prio >= 0);

	طالما (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		إذا (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			لكل (متطاير صحيح i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_جابجا_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		بیعلامت صحيح کلید = arch_irq_قفل();

#اگر_تعریف_شده CONFIG_PM
		bool system_suspended = false;

		إذا (!k_is_pre_kernel()) {
			/* Value preserving: the helper returns either at most
			 * SYS_CLOCK_MAX_WAIT (== INT32_MAX) or K_TICKS_FOREVER.
			 */
			_kernel.idle = (int32_t)z_get_بعدی_timeout_expiry();

			/*
			 * Call the suspend hook function of the soc interface
			 * to allow entry into a low power state. The function
			 * returns false if low power state was not entered, in
			 * which case, kernel does normal idle processing.
			 */
			system_suspended = pm_system_suspend(_kernel.idle);
		}

		إذا (system_suspended) {
			arch_irq_unlock(کلید);
		} وإلا {
			k_cpu_idle();
		}
#وإلا
		ARG_استفادهنشده(کلید);
		k_cpu_idle();
#پایان_اگر /* CONFIG_PM */

#إذا !defined(CONFIG_PREEMPT_ENABLED)
# إذا !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		إذا (_kernel.ready_q.cache != _current) {
			z_جابجا_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#پایان_اگر /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

فراغ __weak arch_spin_relax(فراغ)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_نماد(arch_spin_relax);
