/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/linker/sections.h>
#समावेश <zephyr/drivers/timer/system_timer.h>
#समावेश <zephyr/llext/symbol.h>
#समावेश <zephyr/pm/pm.h>
#समावेश <stdbool.h>
#समावेश <zephyr/logging/log.h>
/* private kernel APIs */
#समावेश <ksched.h>
#समावेश <kswap.h>
#समावेश <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

व्योम idle(व्योम *unused1, व्योम *unused2, व्योम *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	__ASSERT_NO_MSG(_current->base.prio >= 0);

	जबतक (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		अगर (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			क्रम (अस्थिर पूर्णांक i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_swap_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		अचिन्हित पूर्णांक कुंजी = arch_irq_lock();

#अगर_घोषित CONFIG_PM
		bool system_suspended = false;

		अगर (!k_is_pre_kernel()) {
			/* Value preserving: the helper returns either at most
			 * SYS_CLOCK_MAX_WAIT (== INT32_MAX) or K_TICKS_FOREVER.
			 */
			_kernel.idle = (int32_t)z_get_अग्र_timeout_expiry();

			/*
			 * Call the suspend hook function of the soc interface
			 * to allow entry into a low power state. The function
			 * returns false if low power state was not entered, in
			 * which case, kernel does normal idle processing.
			 */
			system_suspended = pm_system_suspend(_kernel.idle);
		}

		अगर (system_suspended) {
			arch_irq_unlock(कुंजी);
		} अन्यथा {
			k_cpu_idle();
		}
#अन्यथा
		ARG_UNUSED(कुंजी);
		k_cpu_idle();
#पूर्ण_अगर /* CONFIG_PM */

#अगर !defined(CONFIG_PREEMPT_ENABLED)
# अगर !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		अगर (_kernel.ready_q.cache != _current) {
			z_swap_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#पूर्ण_अगर /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

व्योम __weak arch_spin_relax(व्योम)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_प्रतीक(arch_spin_relax);
