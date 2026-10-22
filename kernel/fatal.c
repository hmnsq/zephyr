/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/__assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/arch/cpu.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/arch/exception.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log_ctrl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/fatal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/debug/coredump.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak ਖਾਲੀ arch_system_halt(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reason)
{
	ARG_ਅਣਵਰਤਿਆ(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(ਖਾਲੀ)arch_irq_ਤਾਲਾ();
	ਲਈ (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak ਖਾਲੀ k_sys_fatal_error_handler(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reason,
				      ਅਚਲ ਬਣਤਰ arch_esf *esf)
{
	ARG_ਅਣਵਰਤਿਆ(esf);
	LOG_PANIC();

#ਜੇ CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#ਨਹੀਂਤਾਂ
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#ਜੇ_ਅੰਤ /* CONFIG_RESET_ON_FATAL_ERROR */

	ਕੋਡ_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *thread_name_get(ਬਣਤਰ k_thread *thread)
{
	ਅਚਲ ਅੱਖਰ *thread_name = (thread != ਨਿਰਾ) ? k_thread_name_get(thread) : ਨਿਰਾ;

	ਜੇ ((thread_name == ਨਿਰਾ) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	ਵਾਪਸ thread_name;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *reason_ਮੰਜ਼ਿਲ_str(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reason)
{
	ਚੋਣ (reason) {
	ਹਾਲਤ K_ERR_CPU_EXCEPTION:
		ਵਾਪਸ "CPU exception";
	ਹਾਲਤ K_ERR_SPURIOUS_IRQ:
		ਵਾਪਸ "Unhandled interrupt";
	ਹਾਲਤ K_ERR_ਢੇਰ_CHK_FAIL:
		ਵਾਪਸ "Stack overflow";
	ਹਾਲਤ K_ERR_KERNEL_OOPS:
		ਵਾਪਸ "Kernel oops";
	ਹਾਲਤ K_ERR_KERNEL_PANIC:
		ਵਾਪਸ "Kernel panic";
	ਮੂਲ:
		ਵਾਪਸ "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN ਖਾਲੀ k_fatal_halt(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

ਖਾਲੀ z_fatal_error(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ reason, ਅਚਲ ਬਣਤਰ arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ = arch_irq_ਤਾਲਾ();
	ਬਣਤਰ k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ਨਿਰਾ;

	/* The printk lock is not trustworthy from here on: this context may
	 * have failed while holding it, or a CPU that is never coming back
	 * may own it. Give up on serialized output rather than on output.
	 */
	printk_panic();

	/* twister looks for the "ZEPHYR FATAL ERROR" string, don't
	 * change it without also updating twister
	 */
	EXCEPTION_DUMP(">>> ZEPHYR FATAL ERROR %d: %s on CPU %d", reason, reason_ਮੰਜ਼ਿਲ_str(reason),
		       _current_cpu->id);

	/* FIXME: This doesn't seem to work as expected on all arches.
	 * Need a reliable way to determine whether the fault happened when
	 * an IRQ or exception was being handled, or thread context.
	 *
	 * See #17656
	 */
#ਜੇ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	ਜੇ ((esf != ਨਿਰਾ) && arch_is_ਵਿੱਚ_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	ਜੇ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(ਕੁੰਜੀ, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	ਕੁੰਜੀ = arch_coredump_fatal_irq_ਤਾਲਾ(&coredump_irq_cookie);
#ਨਹੀਂਤਾਂ
	coredump(reason, esf, thread);
#ਜੇ_ਅੰਤ

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
	ਜੇ (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#ਜੇ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ਜੇ ((esf != ਨਿਰਾ) && arch_is_ਵਿੱਚ_nested_exception(esf)) {
#ਜੇ defined(CONFIG_ਢੇਰ_SENTINEL)
			ਜੇ (reason != K_ERR_ਢੇਰ_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#ਜੇ_ਅੰਤ /* CONFIG_STACK_SENTINEL */
		}
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} ਨਹੀਂਤਾਂ {
		/* Test mode */
#ਜੇ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ਜੇ ((esf != ਨਿਰਾ) && arch_is_ਵਿੱਚ_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#ਜੇ defined(CONFIG_ਢੇਰ_SENTINEL)
			ਜੇ (reason != K_ERR_ਢੇਰ_CHK_FAIL) {
				arch_irq_unlock(ਕੁੰਜੀ);
				ਵਾਪਸ;
			}
#ਨਹੀਂਤਾਂ
			arch_irq_unlock(ਕੁੰਜੀ);
			ਵਾਪਸ;
#ਜੇ_ਅੰਤ /* CONFIG_STACK_SENTINEL */
		} ਨਹੀਂਤਾਂ {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			ਜੇ (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(ਕੁੰਜੀ);
				ਵਾਪਸ;
			}
		}
#ਜੇ_ਅੰਤ /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(ਕੁੰਜੀ);

	ਜੇ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
