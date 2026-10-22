/*
 * Copyright (c) 2019 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>

#समावेश <kernel_internal.h>
#समावेश <zephyr/sys/__assert.h>
#समावेश <zephyr/arch/cpu.h>
#समावेश <zephyr/arch/exception.h>
#समावेश <zephyr/logging/log_ctrl.h>
#समावेश <zephyr/logging/log.h>
#समावेश <zephyr/fatal.h>
#समावेश <zephyr/debug/coredump.h>
#समावेश <zephyr/sys/reboot.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/* LCOV_EXCL_START */
FUNC_NORETURN __weak व्योम arch_system_halt(अचिन्हित पूर्णांक reason)
{
	ARG_UNUSED(reason);

	/* TODO: What's the best way to totally halt the system if SMP
	 * is enabled?
	 */

	(व्योम)arch_irq_lock();
	क्रम (;;) {
		/* Spin endlessly */
	}
}
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
__weak व्योम k_sys_fatal_error_handler(अचिन्हित पूर्णांक reason,
				      स्थिर काष्ठा arch_esf *esf)
{
	ARG_UNUSED(esf);
	LOG_PANIC();

#अगर CONFIG_RESET_ON_FATAL_ERROR
	EXCEPTION_DUMP("Resetting system");
	sys_reboot(SYS_REBOOT_WARM);
#अन्यथा
	EXCEPTION_DUMP("Halting system");
	arch_system_halt(reason);
#पूर्ण_अगर /* CONFIG_RESET_ON_FATAL_ERROR */

	CODE_UNREACHABLE;
}
/* LCOV_EXCL_STOP */

अटल स्थिर अक्षर *thread_name_get(काष्ठा k_thread *thread)
{
	स्थिर अक्षर *thread_name = (thread != शून्य) ? k_thread_name_get(thread) : शून्य;

	अगर ((thread_name == शून्य) || (thread_name[0] == '\0')) {
		thread_name = "unknown";
	}

	वापस thread_name;
}

अटल स्थिर अक्षर *reason_to_str(अचिन्हित पूर्णांक reason)
{
	चयन (reason) {
	हाल K_ERR_CPU_EXCEPTION:
		वापस "CPU exception";
	हाल K_ERR_SPURIOUS_IRQ:
		वापस "Unhandled interrupt";
	हाल K_ERR_ढेर_CHK_FAIL:
		वापस "Stack overflow";
	हाल K_ERR_KERNEL_OOPS:
		वापस "Kernel oops";
	हाल K_ERR_KERNEL_PANIC:
		वापस "Kernel panic";
	शेष:
		वापस "Unknown error";
	}
}

/* LCOV_EXCL_START */
FUNC_NORETURN व्योम k_fatal_halt(अचिन्हित पूर्णांक reason)
{
	arch_system_halt(reason);
}
/* LCOV_EXCL_STOP */

व्योम z_fatal_error(अचिन्हित पूर्णांक reason, स्थिर काष्ठा arch_esf *esf)
{
	/* We can't allow this code to be preempted, but don't need to
	 * synchronize between CPUs, so an arch-layer lock is
	 * appropriate.
	 */
	अचिन्हित पूर्णांक कुंजी = arch_irq_lock();
	काष्ठा k_thread *thread = IS_ENABLED(CONFIG_MULTITHREADING) ?
			_current : शून्य;

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
#अगर defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
	अगर ((esf != शून्य) && arch_is_in_nested_exception(esf)) {
		EXCEPTION_DUMP("Fault during interrupt handling");
	}
#पूर्ण_अगर /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */

	अगर (IS_ENABLED(CONFIG_MULTITHREADING)) {
		EXCEPTION_DUMP("Current thread: %p (%s)", thread, thread_name_get(thread));
	}

	/*
	 * Must use preprocessor guard, not if (IS_ENABLED(...)): the arch hook
	 * prototypes exist only when CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS is
	 * set (kernel_arch_interface.h). IS_ENABLED still parses both branches,
	 * which breaks builds when the option is off (implicit declaration).
	 */
#अगर_घोषित CONFIG_DEBUG_COREDUMP_FATAL_UNLOCK_IRQS
	अचिन्हित पूर्णांक coredump_irq_cookie;

	arch_coredump_fatal_irq_unlock(कुंजी, &coredump_irq_cookie);
	coredump(reason, esf, thread);
	कुंजी = arch_coredump_fatal_irq_lock(&coredump_irq_cookie);
#अन्यथा
	coredump(reason, esf, thread);
#पूर्ण_अगर

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
	अगर (!IS_ENABLED(CONFIG_TEST)) {
		__ASSERT(reason != K_ERR_KERNEL_PANIC,
			 "Attempted to recover from a kernel panic condition");
		/* FIXME: #17656 */
#अगर defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		अगर ((esf != शून्य) && arch_is_in_nested_exception(esf)) {
#अगर defined(CONFIG_ढेर_SENTINEL)
			अगर (reason != K_ERR_ढेर_CHK_FAIL) {
				__ASSERT(0,
				 "Attempted to recover from a fatal error in ISR");
			 }
#पूर्ण_अगर /* CONFIG_STACK_SENTINEL */
		}
#पूर्ण_अगर /* CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	} अन्यथा {
		/* Test mode */
#अगर defined(CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION)
		अगर ((esf != शून्य) && arch_is_in_nested_exception(esf)) {
			/* Abort the thread only on STACK Sentinel check fail. */
#अगर defined(CONFIG_ढेर_SENTINEL)
			अगर (reason != K_ERR_ढेर_CHK_FAIL) {
				arch_irq_unlock(कुंजी);
				वापस;
			}
#अन्यथा
			arch_irq_unlock(कुंजी);
			वापस;
#पूर्ण_अगर /* CONFIG_STACK_SENTINEL */
		} अन्यथा {
			/* Abort the thread only if the fault is not due to
			 * a spurious ISR handler triggered.
			 */
			अगर (reason == K_ERR_SPURIOUS_IRQ) {
				arch_irq_unlock(कुंजी);
				वापस;
			}
		}
#पूर्ण_अगर /*CONFIG_ARCH_HAS_NESTED_EXCEPTION_DETECTION */
	}

	arch_irq_unlock(कुंजी);

	अगर (IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_thread_abort(thread);
	}
}
