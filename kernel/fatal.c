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
FUNC_NORETURN __weak خالی arch_system_halt(غیرعلامتی صحیح reason)
{
	ARG_غیر_مستعمل(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(خالی)arch_irq_تالا();
	برائے (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak خالی k_sys_fatal_error_handler(غیرعلامتی صحیح reason,
				      ثابت ساخت arch_esf *esf)
{
	ARG_غیر_مستعمل(esf);
	LOG_PANIC();

#اگر CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#ورنہ
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#اگر_ختم /* CONFIG_RESET_ON_FATAL_ERROR */

	رمز_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

ساکن ثابت حرف *thread_name_get(ساخت k_thread *thread)
{
	ثابت حرف *thread_name = (thread != تہی) ? k_thread_name_get(thread) : تہی;

	اگر ((thread_name == تہی) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	واپس thread_name;
}

ساکن ثابت حرف *reason_مقصد_str(غیرعلامتی صحیح reason)
{
	انتخاب (reason) {
	حالت K_ERR_CPU_EXCEPTION:
		واپس "CPU exception";
	حالت K_ERR_SPURIOUS_IRQ:
		واپس "Unhandled interrupt";
	حالت K_ERR_ڈھیر_CHK_FAIL:
		واپس "Stack overflow";
	حالت K_ERR_KERNEL_OOPS:
		واپس "Kernel oops";
	حالت K_ERR_KERNEL_PANIC:
		واپس "Kernel panic";
	طےشدہ:
		واپس "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN خالی k_fatal_halt(غیرعلامتی صحیح reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

خالی z_fatal_error(غیرعلامتی صحیح reason, ثابت ساخت arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	غیرعلامتی صحیح کلید = arch_irq_تالا();
	ساخت k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : تہی;

	/* The printk lock is not trustworthy from here on: this context may
	 * have failed while holding it, or a CPU that is never coming back
	 * may own it. Give up on serialized output rather than on output.
	 */
	printk_panic();

	/* twister looks for the "ZEPHYR FATAL ERROR" string, don't
	 * change it without also updating twister
	 */
	EXCEPTION_DUMP(">>> ZEPHYR FATAL ERROR %d: %s on CPU %d", reason, reason_مقصد_str(reason),
		       _current_cpu->id);

	/* FIXME: This doesn't seem to work as expected on all arches.
	 * Need a reliable way to determine whether the fault happened when
	 * an IRQ or exception was being handled, or thread context.
	 *
	 * See #17656
	 */
#اگر defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	اگر ((esf != تہی) && arch_is_اندر_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#اگر_ختم /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	اگر (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#اگر_تعریف_شدہ CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	غیرعلامتی صحیح coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(کلید, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	کلید = arch_coredump_fatal_irq_تالا(&coredump_irq_cookie);
#ورنہ
	coredump(reason, esf, thread);
#اگر_ختم

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
		اگر ((esf != تہی) && arch_is_اندر_nested_exception(esf)) {
#اگر defined(CONFIG_ڈھیر_SENTINEL)
			اگر (reason != K_ERR_ڈھیر_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#اگر_ختم /* CONFIG_STACK_SENTINEL */
		}
#اگر_ختم /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} ورنہ {
		/* Test mode */
#اگر defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		اگر ((esf != تہی) && arch_is_اندر_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#اگر defined(CONFIG_ڈھیر_SENTINEL)
			اگر (reason != K_ERR_ڈھیر_CHK_FAIL) {
				arch_irq_unlock(کلید);
				واپس;
			}
#ورنہ
			arch_irq_unlock(کلید);
			واپس;
#اگر_ختم /* CONFIG_STACK_SENTINEL */
		} ورنہ {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			اگر (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(کلید);
				واپس;
			}
		}
#اگر_ختم /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(کلید);

	اگر (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
