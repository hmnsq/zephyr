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

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/arch/cfi.h>

#నిర్వచితమైతే CONFIG_CURRENT_THREAD_USE_TLS
Z_THREAD_LOCAL k_tid_t z_tls_current;
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_దొంతర_CANARIES_TLS
#అన్తర్భావ <zephyr/random/random.h>
బాహ్య Z_THREAD_LOCAL అస్థిర uintptr_t __stack_chk_guard;
#అయితే_ముగింపు /* CONFIG_STACK_CANARIES_TLS */

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
FUNC_NORETURN శూన్య z_thread_నమోదు(k_thread_నమోదు_t నమోదు_2,
				 శూన్య *p1, శూన్య *p2, శూన్య *p3)
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
	ARCH_CFI_UNDEFINED_RETURN_చిరునామా();
#నిర్వచితమైతే CONFIG_CURRENT_THREAD_USE_TLS
	z_tls_current = k_sched_current_thread_query();
#అయితే_ముగింపు
#నిర్వచితమైతే CONFIG_దొంతర_CANARIES_TLS
	uintptr_t దొంతర_guard;

	sys_rand_get((uint8_t *)&దొంతర_guard, పరిమాణం(దొంతర_guard));
	__stack_chk_guard = దొంతర_guard;
	__stack_chk_guard <<= 8;
#అయితే_ముగింపు	/* CONFIG_STACK_CANARIES_TLS */
	నమోదు_2(p1, p2, p3);

	k_thread_abort(k_current_get());

	/*
	 * Compiler can't tell that k_thread_abort() won't return and issues a
	 * warning unless we tell it that control never gets this far.
	 */

	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}
