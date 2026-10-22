/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/__assert.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/arch/cpu.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/arch/exception.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log_ctrl.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/fatal.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/debug/coredump.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak ଶୂନ୍ୟ arch_system_halt(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(ଶୂନ୍ୟ)arch_irq_lock();
	ପାଇଁ (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak ଶୂନ୍ୟ k_sys_fatal_error_handler(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା reason,
				      ଅଚଳ ସଂରଚନା arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#ଯଦି CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#ଅନ୍ୟଥା
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *thread_name_get(ସଂରଚନା k_thread *thread)
{
	ଅଚଳ ଅକ୍ଷର *thread_name = (thread != ଖାଲି) ? k_thread_name_get(thread) : ଖାଲି;

	ଯଦି ((thread_name == ଖାଲି) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	ଫେରନ୍ତୁ thread_name;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *reason_to_str(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା reason)
{
	ବାଛ (reason) {
	ସ୍ଥିତି K_ERR_CPU_EXCEPTION:
		ଫେରନ୍ତୁ "CPU exception";
	ସ୍ଥିତି K_ERR_SPURIOUS_IRQ:
		ଫେରନ୍ତୁ "Unhandled interrupt";
	ସ୍ଥିତି K_ERR_ଗଦା_CHK_FAIL:
		ଫେରନ୍ତୁ "Stack overflow";
	ସ୍ଥିତି K_ERR_KERNEL_OOPS:
		ଫେରନ୍ତୁ "Kernel oops";
	ସ୍ଥିତି K_ERR_KERNEL_PANIC:
		ଫେରନ୍ତୁ "Kernel panic";
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN ଶୂନ୍ୟ k_fatal_halt(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

ଶୂନ୍ୟ z_fatal_error(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା reason, ଅଚଳ ସଂରଚନା arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି = arch_irq_lock();
	ସଂରଚନା k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ଖାଲି;

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
#ଯଦି defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	ଯଦି ((esf != ଖାଲି) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	ଯଦି (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(ଚାବି, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	ଚାବି = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#ଅନ୍ୟଥା
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
	ଯଦି (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#ଯଦି defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ଯଦି ((esf != ଖାଲି) && arch_is_in_nested_exception(esf)) {
#ଯଦି defined(CONFIG_ଗଦା_SENTINEL)
			ଯଦି (reason != K_ERR_ଗଦା_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} ଅନ୍ୟଥା {
		/* Test mode */
#ଯଦି defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ଯଦି ((esf != ଖାଲି) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#ଯଦି defined(CONFIG_ଗଦା_SENTINEL)
			ଯଦି (reason != K_ERR_ଗଦା_CHK_FAIL) {
				arch_irq_unlock(ଚାବି);
				ଫେରନ୍ତୁ;
			}
#ଅନ୍ୟଥା
			arch_irq_unlock(ଚାବି);
			ଫେରନ୍ତୁ;
#endif /* CONFIG_STACK_SENTINEL */
		} ଅନ୍ୟଥା {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			ଯଦି (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(ଚାବି);
				ଫେରନ୍ତୁ;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(ଚାବି);

	ଯଦି (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
