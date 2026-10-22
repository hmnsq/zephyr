/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <kernel_internal.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/exception.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log.h>
#include <zephyr/fatal.h>
#include <zephyr/debug/coredump.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak void arch_system_halt(unsigned puur_naa_nka reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(void)arch_irq_lock();
	for (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak void k_sys_fatal_error_handler(unsigned puur_naa_nka reason,
				      const sa_nracanaa arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#ပေႃး CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#ပေႃးဢမ်ႇ
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#endif /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

static const var_na *thread_name_get(sa_nracanaa k_thread *thread)
{
	const var_na *thread_name = (thread != NULL) ? k_thread_name_get(thread) : NULL;

	ပေႃး ((thread_name == NULL) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	ပွၵ်ႈ thread_name;
}

static const var_na *reason_to_str(unsigned puur_naa_nka reason)
{
	switch (reason) {
	pak_sa K_ERR_CPU_EXCEPTION:
		ပွၵ်ႈ "CPU exception";
	pak_sa K_ERR_SPURIOUS_IRQ:
		ပွၵ်ႈ "Unhandled interrupt";
	pak_sa K_ERR_STACK_CHK_FAIL:
		ပွၵ်ႈ "Stack overflow";
	pak_sa K_ERR_KERNEL_OOPS:
		ပွၵ်ႈ "Kernel oops";
	pak_sa K_ERR_KERNEL_PANIC:
		ပွၵ်ႈ "Kernel panic";
	default:
		ပွၵ်ႈ "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN void k_fatal_halt(unsigned puur_naa_nka reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

void z_fatal_error(unsigned puur_naa_nka reason, const sa_nracanaa arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	unsigned puur_naa_nka key = arch_irq_lock();
	sa_nracanaa k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : NULL;

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
#ပေႃး defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	ပေႃး ((esf != NULL) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	ပေႃး (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#ifdef CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	unsigned puur_naa_nka coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(key, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	key = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#ပေႃးဢမ်ႇ
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
	ပေႃး (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#ပေႃး defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ပေႃး ((esf != NULL) && arch_is_in_nested_exception(esf)) {
#ပေႃး defined(CONFIG_STACK_SENTINEL)
			ပေႃး (reason != K_ERR_STACK_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#endif /* CONFIG_STACK_SENTINEL */
		}
#endif /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} ပေႃးဢမ်ႇ {
		/* Test mode */
#ပေႃး defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ပေႃး ((esf != NULL) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#ပေႃး defined(CONFIG_STACK_SENTINEL)
			ပေႃး (reason != K_ERR_STACK_CHK_FAIL) {
				arch_irq_unlock(key);
				ပွၵ်ႈ;
			}
#ပေႃးဢမ်ႇ
			arch_irq_unlock(key);
			ပွၵ်ႈ;
#endif /* CONFIG_STACK_SENTINEL */
		} ပေႃးဢမ်ႇ {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			ပေႃး (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(key);
				ပွၵ်ႈ;
			}
		}
#endif /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(key);

	ပေႃး (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
