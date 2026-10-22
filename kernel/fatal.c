/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>

#شامل <kernel_internal.h>
#شامل <zephyr/sys/__assert.h>
#شامل <zephyr/arch/cpu.h>
#شامل <zephyr/arch/exception.h>
#شامل <zephyr/logging/log_ctrl.h>
#شامل <zephyr/logging/log.h>
#شامل <zephyr/fatal.h>
#شامل <zephyr/debug/coredump.h>
#شامل <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak تهی arch_system_halt(unsigned صحیح reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(تهی)arch_irq_lock();
	برای (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak تهی k_sys_fatal_error_handler(unsigned صحیح reason,
				      ثابت ساختار arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#اگر CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#وگرنه
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

ایستا ثابت نویسه *thread_name_get(ساختار k_thread *thread)
{
	ثابت نویسه *thread_name = (thread != پوچ) ? k_thread_name_get(thread) : پوچ;

	اگر ((thread_name == پوچ) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	بازگشت thread_name;
}

ایستا ثابت نویسه *reason_to_str(unsigned صحیح reason)
{
	انتخاب (reason) {
	حالت K_ERR_CPU_EXCEPTION:
		بازگشت "CPU exception";
	حالت K_ERR_SPURIOUS_IRQ:
		بازگشت "Unhandled interrupt";
	حالت K_ERR_پشته_CHK_FAIL:
		بازگشت "Stack overflow";
	حالت K_ERR_KERNEL_OOPS:
		بازگشت "Kernel oops";
	حالت K_ERR_KERNEL_PANIC:
		بازگشت "Kernel panic";
	پیشفرض:
		بازگشت "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN تهی k_fatal_halt(unsigned صحیح reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

تهی z_fatal_error(unsigned صحیح reason, ثابت ساختار arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned صحیح کلید = arch_irq_lock();
	ساختار k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : پوچ;

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
#اگر defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	اگر ((esf != پوچ) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	اگر (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned صحیح coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(کلید, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	کلید = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#وگرنه
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
	اگر (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#اگر defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		اگر ((esf != پوچ) && arch_is_in_nested_exception(esf)) {
#اگر defined(CONFIG_پشته_SENTINEL)
			اگر (reason != K_ERR_پشته_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} وگرنه {
		/* Test mode */
#اگر defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		اگر ((esf != پوچ) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#اگر defined(CONFIG_پشته_SENTINEL)
			اگر (reason != K_ERR_پشته_CHK_FAIL) {
				arch_irq_unlock(کلید);
				بازگشت;
			}
#وگرنه
			arch_irq_unlock(کلید);
			بازگشت;
#endif /* CONFIG_STACK_SENTINEL */
		} وگرنه {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			اگر (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(کلید);
				بازگشت;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(کلید);

	اگر (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
