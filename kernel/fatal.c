/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/sys/__assert.h>
#అన్తర్భావ <zephyr/arch/cpu.h>
#అన్తర్భావ <zephyr/arch/exception.h>
#అన్తర్భావ <zephyr/logging/log_ctrl.h>
#అన్తర్భావ <zephyr/logging/log.h>
#అన్తర్భావ <zephyr/fatal.h>
#అన్తర్భావ <zephyr/debug/coredump.h>
#అన్తర్భావ <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak శూన్య arch_system_halt(గుర్తులేని పూర్ణాంకం reason)
{
	ARG_వాడని(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(శూన్య)arch_irq_తాళం();
	కొరకు (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak శూన్య k_sys_fatal_error_handler(గుర్తులేని పూర్ణాంకం reason,
				      స్థిరరాశి నిర్మాణం arch_esf *esf)
{
	ARG_వాడని(esf);
	LOG_PANIC();

#ఒకవేళ CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#లేకపోతే
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#అయితే_ముగింపు /* CONFIG_RESET_ON_FATAL_ERROR */

	సంకేతం_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

స్థిర స్థిరరాశి అక్షరం *thread_name_get(నిర్మాణం k_thread *thread)
{
	స్థిరరాశి అక్షరం *thread_name = (thread != ఖాళీ) ? k_thread_name_get(thread) : ఖాళీ;

	ఒకవేళ ((thread_name == ఖాళీ) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	తిరిగి thread_name;
}

స్థిర స్థిరరాశి అక్షరం *reason_గమ్యం_str(గుర్తులేని పూర్ణాంకం reason)
{
	ఎంపిక (reason) {
	స్థితి K_ERR_CPU_EXCEPTION:
		తిరిగి "CPU exception";
	స్థితి K_ERR_SPURIOUS_IRQ:
		తిరిగి "Unhandled interrupt";
	స్థితి K_ERR_దొంతర_CHK_FAIL:
		తిరిగి "Stack overflow";
	స్థితి K_ERR_KERNEL_OOPS:
		తిరిగి "Kernel oops";
	స్థితి K_ERR_KERNEL_PANIC:
		తిరిగి "Kernel panic";
	అప్రమేయ:
		తిరిగి "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN శూన్య k_fatal_halt(గుర్తులేని పూర్ణాంకం reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

శూన్య z_fatal_error(గుర్తులేని పూర్ణాంకం reason, స్థిరరాశి నిర్మాణం arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	గుర్తులేని పూర్ణాంకం కీలకం = arch_irq_తాళం();
	నిర్మాణం k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : ఖాళీ;

	/* The printk lock is not trustworthy from here on: this context may
	 * have failed while holding it, or a CPU that is never coming back
	 * may own it. Give up on serialized output rather than on output.
	 */
	printk_panic();

	/* twister looks for the "ZEPHYR FATAL ERROR" string, don't
	 * change it without also updating twister
	 */
	EXCEPTION_DUMP(">>> ZEPHYR FATAL ERROR %d: %s on CPU %d", reason, reason_గమ్యం_str(reason),
		       _current_cpu->id);

	/* FIXME: This doesn't seem to work as expected on all arches.
	 * Need a reliable way to determine whether the fault happened when
	 * an IRQ or exception was being handled, or thread context.
	 *
	 * See #17656
	 */
#ఒకవేళ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	ఒకవేళ ((esf != ఖాళీ) && arch_is_లోపల_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#అయితే_ముగింపు /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	ఒకవేళ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#నిర్వచితమైతే CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	గుర్తులేని పూర్ణాంకం coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(కీలకం, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	కీలకం = arch_coredump_fatal_irq_తాళం(&coredump_irq_cookie);
#లేకపోతే
	coredump(reason, esf, thread);
#అయితే_ముగింపు

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
	ఒకవేళ (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#ఒకవేళ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ఒకవేళ ((esf != ఖాళీ) && arch_is_లోపల_nested_exception(esf)) {
#ఒకవేళ defined(CONFIG_దొంతర_SENTINEL)
			ఒకవేళ (reason != K_ERR_దొంతర_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#అయితే_ముగింపు /* CONFIG_STACK_SENTINEL */
		}
#అయితే_ముగింపు /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} లేకపోతే {
		/* Test mode */
#ఒకవేళ defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		ఒకవేళ ((esf != ఖాళీ) && arch_is_లోపల_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#ఒకవేళ defined(CONFIG_దొంతర_SENTINEL)
			ఒకవేళ (reason != K_ERR_దొంతర_CHK_FAIL) {
				arch_irq_unlock(కీలకం);
				తిరిగి;
			}
#లేకపోతే
			arch_irq_unlock(కీలకం);
			తిరిగి;
#అయితే_ముగింపు /* CONFIG_STACK_SENTINEL */
		} లేకపోతే {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			ఒకవేళ (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(కీలకం);
				తిరిగి;
			}
		}
#అయితే_ముగింపు /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(కీలకం);

	ఒకవేళ (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
