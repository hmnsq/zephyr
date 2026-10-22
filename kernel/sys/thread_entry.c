/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Thread entry
 *
 * This file provides the common thread entry function
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/arch/cfi.h>

#यदि_परिभाषितम् CONFIG_CURRENT_THREAD_USE_TLS
Z_THREAD_LOCAL k_tid_t z_tls_current;
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_राशिः_CANARIES_TLS
#अन्तर्भाव <zephyr/random/random.h>
बाह्य Z_THREAD_LOCAL चञ्चल uintptr_t __stack_chk_guard;
#यदि_अन्तः /* CONFIG_STACK_CANARIES_TLS */

/*
 * Common thread entry point function (used by all threads)
 *
 * This routine invokes the actual thread entry point function and passes
 * it three arguments. It also handles graceful termination of the thread
 * if the entry point function ever returns.
 *
 * This routine does not return, and is marked as such so the compiler won't
 * generate preamble code that is only used by functions that actually return.
 */
FUNC_NORETURN शून्य z_thread_प्रविष्टिः(k_thread_प्रविष्टिः_t प्रविष्टिः,
				 शून्य *p1, शून्य *p2, शून्य *p3)
{
/*
 * Inform the unwinder that the current return address is undefined.
 *
 * This is typically used at points in the code where execution does not
 * return like a normal function (for example, thread entry routines).
 * Marking the return address as undefined prevents stack unwinding or
 * backtrace tools from following a bogus return address, which could
 * otherwise lead to reading invalid memory and cause faults during
 * unwinding or debugging.
 */
	ARCH_CFI_UNDEFINED_RETURN_स्थानाङ्कः();
#यदि_परिभाषितम् CONFIG_CURRENT_THREAD_USE_TLS
	z_tls_current = k_sched_current_thread_query();
#यदि_अन्तः
#यदि_परिभाषितम् CONFIG_राशिः_CANARIES_TLS
	uintptr_t राशिः_guard;

	sys_rand_get((uint8_t *)&राशिः_guard, परिमाण(राशिः_guard));
	__stack_chk_guard = राशिः_guard;
	__stack_chk_guard <<= 8;
#यदि_अन्तः	/* CONFIG_STACK_CANARIES_TLS */
	प्रविष्टिः(p1, p2, p3);

	k_thread_abort(k_current_get());

	/*
	 * Compiler can't tell that k_thread_abort() won't return and issues a
	 * warning unless we tell it that control never gets this far.
	 */

	संहिता_UNREACHABLE; /* LCOV_EXCL_LINE */
}
