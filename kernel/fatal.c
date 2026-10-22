/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <zephyr/sys/__assert.h>
#അന്തര്ഭാവ <zephyr/arch/cpu.h>
#അന്തര്ഭാവ <zephyr/arch/exception.h>
#അന്തര്ഭാവ <zephyr/logging/log_ctrl.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>
#അന്തര്ഭാവ <zephyr/fatal.h>
#അന്തര്ഭാവ <zephyr/debug/coredump.h>
#അന്തര്ഭാവ <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak ശൂന്യം arch_system_halt(unsigned പൂർണ്ണസംഖ്യ reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(ശൂന്യം)arch_irq_lock();
	വേണ്ടി (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak ശൂന്യം k_sys_fatal_error_handler(unsigned പൂർണ്ണസംഖ്യ reason,
				      സ്ഥിരം ഘടന arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#എങ്കിൽ CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#അല്ലെങ്കിൽ
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

സ്ഥിതം സ്ഥിരം അക്ഷരം *thread_name_get(ഘടന k_thread *thread)
{
	സ്ഥിരം അക്ഷരം *thread_name = (thread != ശൂന്യ) ? k_thread_name_get(thread) : ശൂന്യ;

	എങ്കിൽ ((thread_name == ശൂന്യ) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	തിരികെ thread_name;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *reason_to_str(unsigned പൂർണ്ണസംഖ്യ reason)
{
	തിരഞ്ഞെടുപ്പ് (reason) {
	അവസ്ഥ K_ERR_CPU_EXCEPTION:
		തിരികെ "CPU exception";
	അവസ്ഥ K_ERR_SPURIOUS_IRQ:
		തിരികെ "Unhandled interrupt";
	അവസ്ഥ K_ERR_അട്ടി_CHK_FAIL:
		തിരികെ "Stack overflow";
	അവസ്ഥ K_ERR_KERNEL_OOPS:
		തിരികെ "Kernel oops";
	അവസ്ഥ K_ERR_KERNEL_PANIC:
		തിരികെ "Kernel panic";
	സ്ഥിരസ്ഥിതി:
		തിരികെ "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN ശൂന്യം k_fatal_halt(unsigned പൂർണ്ണസംഖ്യ reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

ശൂന്യം z_fatal_error(unsigned പൂർണ്ണസംഖ്യ reason, സ്ഥിരം ഘടന arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned പൂർണ്ണസംഖ്യ താക്കോൽ = arch_irq_lock();
	ഘടന k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ശൂന്യ;

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
#എങ്കിൽ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	എങ്കിൽ ((esf != ശൂന്യ) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	എങ്കിൽ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned പൂർണ്ണസംഖ്യ coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(താക്കോൽ, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	താക്കോൽ = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#അല്ലെങ്കിൽ
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
	എങ്കിൽ (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#എങ്കിൽ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		എങ്കിൽ ((esf != ശൂന്യ) && arch_is_in_nested_exception(esf)) {
#എങ്കിൽ defined(CONFIG_അട്ടി_SENTINEL)
			എങ്കിൽ (reason != K_ERR_അട്ടി_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} അല്ലെങ്കിൽ {
		/* Test mode */
#എങ്കിൽ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		എങ്കിൽ ((esf != ശൂന്യ) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#എങ്കിൽ defined(CONFIG_അട്ടി_SENTINEL)
			എങ്കിൽ (reason != K_ERR_അട്ടി_CHK_FAIL) {
				arch_irq_unlock(താക്കോൽ);
				തിരികെ;
			}
#അല്ലെങ്കിൽ
			arch_irq_unlock(താക്കോൽ);
			തിരികെ;
#endif /* CONFIG_STACK_SENTINEL */
		} അല്ലെങ്കിൽ {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			എങ്കിൽ (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(താക്കോൽ);
				തിരികെ;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(താക്കോൽ);

	എങ്കിൽ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
