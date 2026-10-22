/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/drivers/timer/system_timer.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/llext/symbol.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/pm/pm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
/* private kernel APIs */
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kswap.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ਖਾਲੀ idle(ਖਾਲੀ *unused1, ਖਾਲੀ *unused2, ਖਾਲੀ *unused3)
{
	ARG_ਅਣਵਰਤਿਆ(unused1);
	ARG_ਅਣਵਰਤਿਆ(unused2);
	ARG_ਅਣਵਰਤਿਆ(unused3);

	__ASSERT_NO_MSG(_current->ਅਧਾਰ.prio >= 0);

	ਜਦੋਂ (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		ਜੇ (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			ਲਈ (ਅਸਥਿਰ ਪੂਰਨ i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_ਅਦਲਾ_ਬਦਲੀ_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ = arch_irq_ਤਾਲਾ();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PM
		bool system_suspended = false;

		ਜੇ (!k_is_pre_kernel()) {
			/* Value preserving: the helper returns either at most
			 * SYS_CLOCK_MAX_WAIT (== INT32_MAX) or K_TICKS_FOREVER.
			 */
			_kernel.idle = (int32_t)z_get_ਅਗਲਾ_timeout_expiry();

			/*
			 * Call the suspend hook function of the soc interface
			 * to allow entry into a low power state. The function
			 * returns false if low power state was not entered, in
			 * which case, kernel does normal idle processing.
			 */
			system_suspended = pm_system_suspend(_kernel.idle);
		}

		ਜੇ (system_suspended) {
			arch_irq_unlock(ਕੁੰਜੀ);
		} ਨਹੀਂਤਾਂ {
			k_cpu_idle();
		}
#ਨਹੀਂਤਾਂ
		ARG_ਅਣਵਰਤਿਆ(ਕੁੰਜੀ);
		k_cpu_idle();
#ਜੇ_ਅੰਤ /* CONFIG_PM */

#ਜੇ !defined(CONFIG_PREEMPT_ENABLED)
# ਜੇ !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		ਜੇ (_kernel.ready_q.cache != _current) {
			z_ਅਦਲਾ_ਬਦਲੀ_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#ਜੇ_ਅੰਤ /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

ਖਾਲੀ __weak arch_spin_relax(ਖਾਲੀ)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_ਚਿੰਨ੍ਹ(arch_spin_relax);
