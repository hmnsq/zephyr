/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <zephyr/sys/__assert.h>
#அந்தர்பாவ <zephyr/arch/cpu.h>
#அந்தர்பாவ <zephyr/arch/exception.h>
#அந்தர்பாவ <zephyr/logging/log_ctrl.h>
#அந்தர்பாவ <zephyr/logging/log.h>
#அந்தர்பாவ <zephyr/fatal.h>
#அந்தர்பாவ <zephyr/debug/coredump.h>
#அந்தர்பாவ <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak வெறுமை arch_system_halt(unsigned முழு reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(வெறுமை)arch_irq_lock();
	க்கு (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak வெறுமை k_sys_fatal_error_handler(unsigned முழு reason,
				      மாறிலி கட்டமைப்பு arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#எனில் CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#இல்லையெனில்
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

நிலையான மாறிலி எழுத்து *thread_name_get(கட்டமைப்பு k_thread *thread)
{
	மாறிலி எழுத்து *thread_name = (thread != வெற்று) ? k_thread_name_get(thread) : வெற்று;

	எனில் ((thread_name == வெற்று) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	திரும்பு thread_name;
}

நிலையான மாறிலி எழுத்து *reason_to_str(unsigned முழு reason)
{
	தேர்வு (reason) {
	நிலை K_ERR_CPU_EXCEPTION:
		திரும்பு "CPU exception";
	நிலை K_ERR_SPURIOUS_IRQ:
		திரும்பு "Unhandled interrupt";
	நிலை K_ERR_அடுக்கு_CHK_FAIL:
		திரும்பு "Stack overflow";
	நிலை K_ERR_KERNEL_OOPS:
		திரும்பு "Kernel oops";
	நிலை K_ERR_KERNEL_PANIC:
		திரும்பு "Kernel panic";
	இயல்பு:
		திரும்பு "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN வெறுமை k_fatal_halt(unsigned முழு reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

வெறுமை z_fatal_error(unsigned முழு reason, மாறிலி கட்டமைப்பு arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned முழு சாவி = arch_irq_lock();
	கட்டமைப்பு k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : வெற்று;

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
#எனில் defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	எனில் ((esf != வெற்று) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	எனில் (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned முழு coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(சாவி, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	சாவி = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#இல்லையெனில்
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
	எனில் (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#எனில் defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		எனில் ((esf != வெற்று) && arch_is_in_nested_exception(esf)) {
#எனில் defined(CONFIG_அடுக்கு_SENTINEL)
			எனில் (reason != K_ERR_அடுக்கு_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} இல்லையெனில் {
		/* Test mode */
#எனில் defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		எனில் ((esf != வெற்று) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#எனில் defined(CONFIG_அடுக்கு_SENTINEL)
			எனில் (reason != K_ERR_அடுக்கு_CHK_FAIL) {
				arch_irq_unlock(சாவி);
				திரும்பு;
			}
#இல்லையெனில்
			arch_irq_unlock(சாவி);
			திரும்பு;
#endif /* CONFIG_STACK_SENTINEL */
		} இல்லையெனில் {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			எனில் (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(சாவி);
				திரும்பு;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(சாவி);

	எனில் (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
