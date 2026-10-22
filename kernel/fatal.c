/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <zephyr/sys/__assert.h>
#अन्तर्भाव <zephyr/arch/cpu.h>
#अन्तर्भाव <zephyr/arch/exception.h>
#अन्तर्भाव <zephyr/logging/log_ctrl.h>
#अन्तर्भाव <zephyr/logging/log.h>
#अन्तर्भाव <zephyr/fatal.h>
#अन्तर्भाव <zephyr/debug/coredump.h>
#अन्तर्भाव <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak शून्य arch_system_halt(unsigned पूर्णाङ्क reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(शून्य)arch_irq_lock();
	अत्थाय (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak शून्य k_sys_fatal_error_handler(unsigned पूर्णाङ्क reason,
				      ध्रुव संरचना arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#सचे CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#अञ्ञथा
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

स्थिर ध्रुव वर्ण *thread_name_get(संरचना k_thread *thread)
{
	ध्रुव वर्ण *thread_name = (thread != रिक्त) ? k_thread_name_get(thread) : रिक्त;

	सचे ((thread_name == रिक्त) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	पटिनिवत्त thread_name;
}

स्थिर ध्रुव वर्ण *reason_to_str(unsigned पूर्णाङ्क reason)
{
	विकल्प (reason) {
	पक्ष K_ERR_CPU_EXCEPTION:
		पटिनिवत्त "CPU exception";
	पक्ष K_ERR_SPURIOUS_IRQ:
		पटिनिवत्त "Unhandled interrupt";
	पक्ष K_ERR_राशिः_CHK_FAIL:
		पटिनिवत्त "Stack overflow";
	पक्ष K_ERR_KERNEL_OOPS:
		पटिनिवत्त "Kernel oops";
	पक्ष K_ERR_KERNEL_PANIC:
		पटिनिवत्त "Kernel panic";
	स्वभाव:
		पटिनिवत्त "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN शून्य k_fatal_halt(unsigned पूर्णाङ्क reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

शून्य z_fatal_error(unsigned पूर्णाङ्क reason, ध्रुव संरचना arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned पूर्णाङ्क कुञ्चिका = arch_irq_lock();
	संरचना k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : रिक्त;

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
#सचे defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	सचे ((esf != रिक्त) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	सचे (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned पूर्णाङ्क coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(कुञ्चिका, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	कुञ्चिका = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#अञ्ञथा
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
	सचे (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#सचे defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		सचे ((esf != रिक्त) && arch_is_in_nested_exception(esf)) {
#सचे defined(CONFIG_राशिः_SENTINEL)
			सचे (reason != K_ERR_राशिः_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} अञ्ञथा {
		/* Test mode */
#सचे defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		सचे ((esf != रिक्त) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#सचे defined(CONFIG_राशिः_SENTINEL)
			सचे (reason != K_ERR_राशिः_CHK_FAIL) {
				arch_irq_unlock(कुञ्चिका);
				पटिनिवत्त;
			}
#अञ्ञथा
			arch_irq_unlock(कुञ्चिका);
			पटिनिवत्त;
#endif /* CONFIG_STACK_SENTINEL */
		} अञ्ञथा {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			सचे (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(कुञ्चिका);
				पटिनिवत्त;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(कुञ्चिका);

	सचे (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
