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
FUNC_NORETURN __weak ಶೂನ್ಯ arch_system_halt(unsigned ಪೂರ್ಣಾಙ್ಕ reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(ಶೂನ್ಯ)arch_irq_lock();
	ಪ್ರತಿ (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak ಶೂನ್ಯ k_sys_fatal_error_handler(unsigned ಪೂರ್ಣಾಙ್ಕ reason,
				      ಧ್ರುವ ಸಂರಚನಾ arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#ಯದಿ CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#ಅನ್ಯಥಾ
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *thread_name_get(ಸಂರಚನಾ k_thread *thread)
{
	ಧ್ರುವ ವರ್ಣ *thread_name = (thread != ರಿಕ್ತ) ? k_thread_name_get(thread) : ರಿಕ್ತ;

	ಯದಿ ((thread_name == ರಿಕ್ತ) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	ಪ್ರತ್ಯಾಗಮ thread_name;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *reason_to_str(unsigned ಪೂರ್ಣಾಙ್ಕ reason)
{
	ವಿಕಲ್ಪ (reason) {
	ಪಕ್ಷ K_ERR_CPU_EXCEPTION:
		ಪ್ರತ್ಯಾಗಮ "CPU exception";
	ಪಕ್ಷ K_ERR_SPURIOUS_IRQ:
		ಪ್ರತ್ಯಾಗಮ "Unhandled interrupt";
	ಪಕ್ಷ K_ERR_ರಾಶಿ_CHK_FAIL:
		ಪ್ರತ್ಯಾಗಮ "Stack overflow";
	ಪಕ್ಷ K_ERR_KERNEL_OOPS:
		ಪ್ರತ್ಯಾಗಮ "Kernel oops";
	ಪಕ್ಷ K_ERR_KERNEL_PANIC:
		ಪ್ರತ್ಯಾಗಮ "Kernel panic";
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN ಶೂನ್ಯ k_fatal_halt(unsigned ಪೂರ್ಣಾಙ್ಕ reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

ಶೂನ್ಯ z_fatal_error(unsigned ಪೂರ್ಣಾಙ್ಕ reason, ಧ್ರುವ ಸಂರಚನಾ arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned ಪೂರ್ಣಾಙ್ಕ ಕೀಲಿ = arch_irq_lock();
	ಸಂರಚನಾ k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ರಿಕ್ತ;

	/* The printk lock is not trustworthy from here on: this context may
	 * have failed while holding it, or a CPU that is never coming back
	 * may own it. Give up on serialized output rather than on output.
	 */
	printk_panic();

	/* twister looks for the "ZEPHYR FATAL ERROR" string, don't
	 * change it without also updating twister
	 */
	EXCEPTION_DUMP(">>> ZEPHYR FATAL ERROR %d: %s on CPU %d", reason, reason_to_str(reason),
		       _current_cpu->id);

	/* FIXME: This doesn't seem to work as expected on all arches.
	 * Need a reliable way to determine whether the fault happened when
	 * an IRQ or exception was being handled, or thread context.
	 *
	 * See #17656
	 */
#ಯದಿ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	ಯದಿ ((esf != ರಿಕ್ತ) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	ಯದಿ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned ಪೂರ್ಣಾಙ್ಕ coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(ಕೀಲಿ, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	ಕೀಲಿ = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#ಅನ್ಯಥಾ
	coredump(reason, esf, thread);
#endif

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
	ಯದಿ (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#ಯದಿ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ಯದಿ ((esf != ರಿಕ್ತ) && arch_is_in_nested_exception(esf)) {
#ಯದಿ defined(CONFIG_ರಾಶಿ_SENTINEL)
			ಯದಿ (reason != K_ERR_ರಾಶಿ_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} ಅನ್ಯಥಾ {
		/* Test mode */
#ಯದಿ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ಯದಿ ((esf != ರಿಕ್ತ) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#ಯದಿ defined(CONFIG_ರಾಶಿ_SENTINEL)
			ಯದಿ (reason != K_ERR_ರಾಶಿ_CHK_FAIL) {
				arch_irq_unlock(ಕೀಲಿ);
				ಪ್ರತ್ಯಾಗಮ;
			}
#ಅನ್ಯಥಾ
			arch_irq_unlock(ಕೀಲಿ);
			ಪ್ರತ್ಯಾಗಮ;
#endif /* CONFIG_STACK_SENTINEL */
		} ಅನ್ಯಥಾ {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			ಯದಿ (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(ಕೀಲಿ);
				ಪ್ರತ್ಯಾಗಮ;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(ಕೀಲಿ);

	ಯದಿ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
