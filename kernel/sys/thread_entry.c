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

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/arch/cfi.h>

#جے_تعریف_شدہ CONFIG_CURRENT_THREAD_USE_TLS
Z_THREAD_LOCAL k_tid_t z_tls_current;
#جے_انت

#جے_تعریف_شدہ CONFIG_ڈھیر_CANARIES_TLS
#ضمن <zephyr/random/random.h>
خارجي Z_THREAD_LOCAL متطاير uintptr_t __stack_chk_guard;
#جے_انت /* CONFIG_STACK_CANARIES_TLS */

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
FUNC_NORETURN فراغ z_thread_اندراج(k_thread_اندراج_t اندراج,
				 فراغ *p1, فراغ *p2, فراغ *p3)
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
	ARCH_CFI_UNDEFINED_RETURN_پتہ();
#جے_تعریف_شدہ CONFIG_CURRENT_THREAD_USE_TLS
	z_tls_current = k_sched_current_thread_query();
#جے_انت
#جے_تعریف_شدہ CONFIG_ڈھیر_CANARIES_TLS
	uintptr_t ڈھیر_guard;

	sys_rand_get((uint8_t *)&ڈھیر_guard, حجم(ڈھیر_guard));
	__stack_chk_guard = ڈھیر_guard;
	__stack_chk_guard <<= 8;
#جے_انت	/* CONFIG_STACK_CANARIES_TLS */
	اندراج(p1, p2, p3);

	k_thread_abort(k_current_get());

	/*
	 * Compiler can't tell that k_thread_abort() won't return and issues a
	 * warning unless we tell it that control never gets this far.
	 */

	کوڈ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
