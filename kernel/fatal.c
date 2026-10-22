/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/sys/__assert.h>
#અન્તર્ભાવ <zephyr/arch/cpu.h>
#અન્તર્ભાવ <zephyr/arch/exception.h>
#અન્તર્ભાવ <zephyr/logging/log_ctrl.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
#અન્તર્ભાવ <zephyr/fatal.h>
#અન્તર્ભાવ <zephyr/debug/coredump.h>
#અન્તર્ભાવ <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak શૂન્ય arch_system_halt(unsigned પૂર્ણાંક reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(શૂન્ય)arch_irq_lock();
	માટે (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak શૂન્ય k_sys_fatal_error_handler(unsigned પૂર્ણાંક reason,
				      અચલ સંરચના arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#જો CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#નહીંતર
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

સ્થિર અચલ અક્ષર *thread_name_get(સંરચના k_thread *thread)
{
	અચલ અક્ષર *thread_name = (thread != ખાલી) ? k_thread_name_get(thread) : ખાલી;

	જો ((thread_name == ખાલી) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	પાછા thread_name;
}

સ્થિર અચલ અક્ષર *reason_to_str(unsigned પૂર્ણાંક reason)
{
	પસંદગી (reason) {
	સ્થિતિ K_ERR_CPU_EXCEPTION:
		પાછા "CPU exception";
	સ્થિતિ K_ERR_SPURIOUS_IRQ:
		પાછા "Unhandled interrupt";
	સ્થિતિ K_ERR_ઢગલો_CHK_FAIL:
		પાછા "Stack overflow";
	સ્થિતિ K_ERR_KERNEL_OOPS:
		પાછા "Kernel oops";
	સ્થિતિ K_ERR_KERNEL_PANIC:
		પાછા "Kernel panic";
	મૂળભૂત:
		પાછા "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN શૂન્ય k_fatal_halt(unsigned પૂર્ણાંક reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

શૂન્ય z_fatal_error(unsigned પૂર્ણાંક reason, અચલ સંરચના arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned પૂર્ણાંક ચાવી = arch_irq_lock();
	સંરચના k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ખાલી;

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
#જો defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	જો ((esf != ખાલી) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	જો (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned પૂર્ણાંક coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(ચાવી, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	ચાવી = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#નહીંતર
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
	જો (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#જો defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		જો ((esf != ખાલી) && arch_is_in_nested_exception(esf)) {
#જો defined(CONFIG_ઢગલો_SENTINEL)
			જો (reason != K_ERR_ઢગલો_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} નહીંતર {
		/* Test mode */
#જો defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		જો ((esf != ખાલી) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#જો defined(CONFIG_ઢગલો_SENTINEL)
			જો (reason != K_ERR_ઢગલો_CHK_FAIL) {
				arch_irq_unlock(ચાવી);
				પાછા;
			}
#નહીંતર
			arch_irq_unlock(ચાવી);
			પાછા;
#endif /* CONFIG_STACK_SENTINEL */
		} નહીંતર {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			જો (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(ચાવી);
				પાછા;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(ચાવી);

	જો (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
