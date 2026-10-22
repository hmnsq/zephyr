/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>

#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <zephyr/sys/__assert.h>
#অন্তর্ভাব <zephyr/arch/cpu.h>
#অন্তর্ভাব <zephyr/arch/exception.h>
#অন্তর্ভাব <zephyr/logging/log_ctrl.h>
#অন্তর্ভাব <zephyr/logging/log.h>
#অন্তর্ভাব <zephyr/fatal.h>
#অন্তর্ভাব <zephyr/debug/coredump.h>
#অন্তর্ভাব <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak শূন্য arch_system_halt(unsigned পূর্ণসংখ্যা reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(শূন্য)arch_irq_lock();
	জন্য (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak শূন্য k_sys_fatal_error_handler(unsigned পূর্ণসংখ্যা reason,
				      ধ্রুবক কাঠামো arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#যদি CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#অন্যথা
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

স্থির ধ্রুবক অক্ষর *thread_name_get(কাঠামো k_thread *thread)
{
	ধ্রুবক অক্ষর *thread_name = (thread != ফাঁকা) ? k_thread_name_get(thread) : ফাঁকা;

	যদি ((thread_name == ফাঁকা) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	ফেরত thread_name;
}

স্থির ধ্রুবক অক্ষর *reason_to_str(unsigned পূর্ণসংখ্যা reason)
{
	নির্বাচন (reason) {
	ক্ষেত্র K_ERR_CPU_EXCEPTION:
		ফেরত "CPU exception";
	ক্ষেত্র K_ERR_SPURIOUS_IRQ:
		ফেরত "Unhandled interrupt";
	ক্ষেত্র K_ERR_স্তূপ_CHK_FAIL:
		ফেরত "Stack overflow";
	ক্ষেত্র K_ERR_KERNEL_OOPS:
		ফেরত "Kernel oops";
	ক্ষেত্র K_ERR_KERNEL_PANIC:
		ফেরত "Kernel panic";
	পূর্বনির্ধারিত:
		ফেরত "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN শূন্য k_fatal_halt(unsigned পূর্ণসংখ্যা reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

শূন্য z_fatal_error(unsigned পূর্ণসংখ্যা reason, ধ্রুবক কাঠামো arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned পূর্ণসংখ্যা চাবি = arch_irq_lock();
	কাঠামো k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ফাঁকা;

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
#যদি defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	যদি ((esf != ফাঁকা) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	যদি (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned পূর্ণসংখ্যা coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(চাবি, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	চাবি = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#অন্যথা
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
	যদি (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#যদি defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		যদি ((esf != ফাঁকা) && arch_is_in_nested_exception(esf)) {
#যদি defined(CONFIG_স্তূপ_SENTINEL)
			যদি (reason != K_ERR_স্তূপ_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} অন্যথা {
		/* Test mode */
#যদি defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		যদি ((esf != ফাঁকা) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#যদি defined(CONFIG_স্তূপ_SENTINEL)
			যদি (reason != K_ERR_স্তূপ_CHK_FAIL) {
				arch_irq_unlock(চাবি);
				ফেরত;
			}
#অন্যথা
			arch_irq_unlock(চাবি);
			ফেরত;
#endif /* CONFIG_STACK_SENTINEL */
		} অন্যথা {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			যদি (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(চাবি);
				ফেরত;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(চাবি);

	যদি (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
