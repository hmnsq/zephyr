/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/__assert.h>
#ಅನ್ತರ್ಭಾವ <zephyr/arch/cpu.h>
#ಅನ್ತರ್ಭಾವ <zephyr/arch/exception.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log_ctrl.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
#ಅನ್ತರ್ಭಾವ <zephyr/fatal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/debug/coredump.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak ಶೂನ್ಯ arch_system_halt(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ reason)
{
	ARG_ಬಳಸದ(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(ಶೂನ್ಯ)arch_irq_ಬೀಗ();
	ಗಾಗಿ (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak ಶೂನ್ಯ k_sys_fatal_error_handler(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ reason,
				      ಸ್ಥಿರಾಂಕ ರಚನೆ arch_esf *esf)
{
	ARG_ಬಳಸದ(esf);
	LOG_PANIC();

#ಆದರೆ CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#ಇಲ್ಲದಿದ್ದರೆ
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_RESET_ON_FATAL_ERROR */

	ಸಂಕೇತ_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *thread_name_get(ರಚನೆ k_thread *thread)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *thread_name = (thread != ಖಾಲಿ) ? k_thread_name_get(thread) : ಖಾಲಿ;

	ಆದರೆ ((thread_name == ಖಾಲಿ) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	ಹಿಂತಿರುಗಿ thread_name;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *reason_ಗಮ್ಯ_str(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ reason)
{
	ಆಯ್ಕೆ (reason) {
	ಸ್ಥಿತಿ K_ERR_CPU_EXCEPTION:
		ಹಿಂತಿರುಗಿ "CPU exception";
	ಸ್ಥಿತಿ K_ERR_SPURIOUS_IRQ:
		ಹಿಂತಿರುಗಿ "Unhandled interrupt";
	ಸ್ಥಿತಿ K_ERR_ರಾಶಿ_CHK_FAIL:
		ಹಿಂತಿರುಗಿ "Stack overflow";
	ಸ್ಥಿತಿ K_ERR_KERNEL_OOPS:
		ಹಿಂತಿರುಗಿ "Kernel oops";
	ಸ್ಥಿತಿ K_ERR_KERNEL_PANIC:
		ಹಿಂತಿರುಗಿ "Kernel panic";
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN ಶೂನ್ಯ k_fatal_halt(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

ಶೂನ್ಯ z_fatal_error(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ reason, ಸ್ಥಿರಾಂಕ ರಚನೆ arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ = arch_irq_ಬೀಗ();
	ರಚನೆ k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ಖಾಲಿ;

	/* The printk lock is not trustworthy from here on: this context may
	 * have failed while holding it, or a CPU that is never coming back
	 * may own it. Give up on serialized output rather than on output.
	 */
	printk_panic();

	/* twister looks for the "ZEPHYR FATAL ERROR" string, don't
	 * change it without also updating twister
	 */
	EXCEPTION_DUMP(">>> ZEPHYR FATAL ERROR %d: %s on CPU %d", reason, reason_ಗಮ್ಯ_str(reason),
		       _current_cpu->id);

	/* FIXME: This doesn't seem to work as expected on all arches.
	 * Need a reliable way to determine whether the fault happened when
	 * an IRQ or exception was being handled, or thread context.
	 *
	 * See #17656
	 */
#ಆದರೆ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	ಆದರೆ ((esf != ಖಾಲಿ) && arch_is_ಒಳ_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	ಆದರೆ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(ಕೀಲಿ, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	ಕೀಲಿ = arch_coredump_fatal_irq_ಬೀಗ(&coredump_irq_cookie);
#ಇಲ್ಲದಿದ್ದರೆ
	coredump(reason, esf, thread);
#ಆದರೆ_ಅಂತ್ಯ

	k_sys_fatal_error_handler(reason, esf);

	/* If the system fatal error handler returns, then kill the faulting
	 * thread; a policy decision was made not to hang the system.
	 *
	 * Policy for fatal errors in ISRs: unconditionally panic.
	 *
	 * There is one exception to this policy: a stack sentinel
	 * check may be performed (on behalf of the current thread)
	 * during ISR exit, but in this case the thread should be
	 * aborted.
	 *
	 * Note that k_thread_abort() returns on some architectures but
	 * not others; e.g. on ARC, x86_64, Xtensa with ASM2, ARM
	 */
	ಆದರೆ (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#ಆದರೆ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ಆದರೆ ((esf != ಖಾಲಿ) && arch_is_ಒಳ_nested_exception(esf)) {
#ಆದರೆ defined(CONFIG_ರಾಶಿ_SENTINEL)
			ಆದರೆ (reason != K_ERR_ರಾಶಿ_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_SENTINEL */
		}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Test mode */
#ಆದರೆ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ಆದರೆ ((esf != ಖಾಲಿ) && arch_is_ಒಳ_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#ಆದರೆ defined(CONFIG_ರಾಶಿ_SENTINEL)
			ಆದರೆ (reason != K_ERR_ರಾಶಿ_CHK_FAIL) {
				arch_irq_unlock(ಕೀಲಿ);
				ಹಿಂತಿರುಗಿ;
			}
#ಇಲ್ಲದಿದ್ದರೆ
			arch_irq_unlock(ಕೀಲಿ);
			ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_SENTINEL */
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			ಆದರೆ (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(ಕೀಲಿ);
				ಹಿಂತಿರುಗಿ;
			}
		}
#ಆದರೆ_ಅಂತ್ಯ /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(ಕೀಲಿ);

	ಆದರೆ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
