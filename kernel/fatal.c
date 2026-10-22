/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>

#ضمن <kernel_internal.h>
#ضمن <zephyr/sys/__assert.h>
#ضمن <zephyr/arch/cpu.h>
#ضمن <zephyr/arch/exception.h>
#ضمن <zephyr/logging/log_ctrl.h>
#ضمن <zephyr/logging/log.h>
#ضمن <zephyr/fatal.h>
#ضمن <zephyr/debug/coredump.h>
#ضمن <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak فراغ arch_system_halt(unsigned صحيح reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(فراغ)arch_irq_lock();
	لكل (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak فراغ k_sys_fatal_error_handler(unsigned صحيح reason,
				      ثابت بنية arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#إذا CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#وإلا
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

ساكن ثابت حرف *thread_name_get(بنية k_thread *thread)
{
	ثابت حرف *thread_name = (thread != عدم) ? k_thread_name_get(thread) : عدم;

	إذا ((thread_name == عدم) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	أرجع thread_name;
}

ساكن ثابت حرف *reason_to_str(unsigned صحيح reason)
{
	بدل (reason) {
	حالة K_ERR_CPU_EXCEPTION:
		أرجع "CPU exception";
	حالة K_ERR_SPURIOUS_IRQ:
		أرجع "Unhandled interrupt";
	حالة K_ERR_کەڵەکە_CHK_FAIL:
		أرجع "Stack overflow";
	حالة K_ERR_KERNEL_OOPS:
		أرجع "Kernel oops";
	حالة K_ERR_KERNEL_PANIC:
		أرجع "Kernel panic";
	افتراضي:
		أرجع "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN فراغ k_fatal_halt(unsigned صحيح reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

فراغ z_fatal_error(unsigned صحيح reason, ثابت بنية arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned صحيح کلیل = arch_irq_lock();
	بنية k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : عدم;

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
#إذا defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	إذا ((esf != عدم) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	إذا (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned صحيح coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(کلیل, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	کلیل = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#وإلا
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
	إذا (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#إذا defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		إذا ((esf != عدم) && arch_is_in_nested_exception(esf)) {
#إذا defined(CONFIG_کەڵەکە_SENTINEL)
			إذا (reason != K_ERR_کەڵەکە_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} وإلا {
		/* Test mode */
#إذا defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		إذا ((esf != عدم) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#إذا defined(CONFIG_کەڵەکە_SENTINEL)
			إذا (reason != K_ERR_کەڵەکە_CHK_FAIL) {
				arch_irq_unlock(کلیل);
				أرجع;
			}
#وإلا
			arch_irq_unlock(کلیل);
			أرجع;
#endif /* CONFIG_STACK_SENTINEL */
		} وإلا {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			إذا (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(کلیل);
				أرجع;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(کلیل);

	إذا (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
