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

#അന്തര്ഭാവ <zephyr/toolchain.h> /* compiler specific configurations */

#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <zephyr/linker/sections.h>
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
ശൂന്യം അട്ടി_Check_Handler(ശൂന്യം)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_അട്ടി_CHK_FAIL);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#ifdef CONFIG_അട്ടി_CANARIES_TLS
#ifdef CONFIG_അട്ടി_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL അസ്ഥിര uintptr_t __stack_chk_guard;
#അല്ലെങ്കിൽ
Z_THREAD_LOCAL അസ്ഥിര uintptr_t __stack_chk_guard;
#endif
#അല്ലെങ്കിലെങ്കിൽ CONFIG_USERSPACE
K_APP_DMEM(z_libc_partition) അസ്ഥിര uintptr_t __stack_chk_guard;
#അല്ലെങ്കിൽ
__noinit അസ്ഥിര uintptr_t __stack_chk_guard;
#endif

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(അട്ടി_Check_Handler, __stack_chk_fail, ശൂന്യം);
