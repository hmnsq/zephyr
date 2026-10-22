/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/drivers/timer/system_timer.h>
#ಅನ್ತರ್ಭಾವ <zephyr/llext/symbol.h>
#ಅನ್ತರ್ಭಾವ <zephyr/pm/pm.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
/* private kernel APIs */
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ಶೂನ್ಯ idle(ಶೂನ್ಯ *unused1, ಶೂನ್ಯ *unused2, ಶೂನ್ಯ *unused3)
{
	ARG_ಬಳಸದ(unused1);
	ARG_ಬಳಸದ(unused2);
	ARG_ಬಳಸದ(unused3);

	__ASSERT_NO_MSG(_current->ತಳ.prio >= 0);

	ಆಗ (true) {
		/* SMP systems without a working IPI can't actual
		 * enter an idle state, because they can't be notified
		 * of scheduler changes (i.e. threads they should
		 * run).  They just spin instead, with a minimal
		 * relaxation loop to prevent hammering the scheduler
		 * lock and/or timer driver.  This is intended as a
		 * fallback configuration for new platform bringup.
		 */
		ಆದರೆ (IS_ENABLED(CONFIG_SMP) && !IS_ENABLED(CONFIG_SCHED_IPI_SUPPORTED)) {
			ಗಾಗಿ (ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i = 0; i < 100000; i++) {
				/* Empty loop */
			}
			z_ಅದಲುಬದಲು_unlocked();
		}

		/* Note weird API: k_cpu_idle() is called with local
		 * CPU interrupts masked, and returns with them
		 * unmasked.  It does not take a spinlock or other
		 * higher level construct.
		 */
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ = arch_irq_ಬೀಗ();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PM
		bool system_suspended = false;

		ಆದರೆ (!k_is_pre_kernel()) {
			/* Value preserving: the helper returns either at most
			 * SYS_CLOCK_MAX_WAIT (== INT32_MAX) or K_TICKS_FOREVER.
			 */
			_kernel.idle = (int32_t)z_get_ಮುಂದಿನ_timeout_expiry();

			/*
			 * Call the suspend hook function of the soc interface
			 * to allow entry into a low power state. The function
			 * returns false if low power state was not entered, in
			 * which case, kernel does normal idle processing.
			 */
			system_suspended = pm_system_suspend(_kernel.idle);
		}

		ಆದರೆ (system_suspended) {
			arch_irq_unlock(ಕೀಲಿ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			k_cpu_idle();
		}
#ಇಲ್ಲದಿದ್ದರೆ
		ARG_ಬಳಸದ(ಕೀಲಿ);
		k_cpu_idle();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_PM */

#ಆದರೆ !defined(CONFIG_PREEMPT_ENABLED)
# ಆದರೆ !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC)
		/* A legacy mess: the idle thread is by definition
		 * preemptible as far as the modern scheduler is
		 * concerned, but older platforms use
		 * CONFIG_PREEMPT_ENABLED=n as an optimization hint
		 * that interrupt exit always returns to the
		 * interrupted context.  So in that setup we need to
		 * explicitly yield in the idle thread otherwise
		 * nothing else will run once it starts.
		 */
		ಆದರೆ (_kernel.ready_q.cache != _current) {
			z_ಅದಲುಬದಲು_unlocked();
		}
# endif /* !defined(CONFIG_USE_SWITCH) || defined(CONFIG_SPARC) */
#ಆದರೆ_ಅಂತ್ಯ /* !defined(CONFIG_PREEMPT_ENABLED) */
	}
}

ಶೂನ್ಯ __weak arch_spin_relax(ಶೂನ್ಯ)
{
	__ASSERT(!arch_cpu_irqs_are_enabled(),
		 "this is meant to be called with IRQs disabled");

	arch_nop();
}
EXPORT_ಚಿಹ್ನೆ(arch_spin_relax);
