/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Compiler stack protection (kernel part)
 *
 * This module provides functions to support compiler stack protection
 * using canaries.  This feature is enabled with configuration
 * CONFIG_STACK_CANARIES=y or CONFIG_STACK_CANARIES_STRONG=y or
 * CONFIG_STACK_CANARIES_ALL=y or CONFIG_STACK_CANARIES_EXPLICIT=y.
 *
 * When this feature is enabled, the compiler generated code refers to
 * function __stack_chk_fail and global variable __stack_chk_guard.
 */

#અન્તર્ભાવ <zephyr/toolchain.h> /* compiler specific configurations */

#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/linker/sections.h>
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
શૂન્ય ઢગલો_Check_Handler(શૂન્ય)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_ઢગલો_CHK_FAIL);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#ifdef CONFIG_ઢગલો_CANARIES_TLS
#ifdef CONFIG_ઢગલો_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL અસ્થિર uintptr_t __stack_chk_guard;
#નહીંતર
Z_THREAD_LOCAL અસ્થિર uintptr_t __stack_chk_guard;
#endif
#અથવાજો CONFIG_USERSPACE
K_APP_DMEM(z_libc_partition) અસ્થિર uintptr_t __stack_chk_guard;
#નહીંતર
__noinit અસ્થિર uintptr_t __stack_chk_guard;
#endif

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(ઢગલો_Check_Handler, __stack_chk_fail, શૂન્ય);
