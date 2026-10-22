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

#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h> /* compiler specific configurations */

#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/linker/sections.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
ଶୂନ୍ୟ ଗଦା_Check_Handler(ଶୂନ୍ୟ)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_ଗଦା_CHK_FAIL);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#ifdef CONFIG_ଗଦା_CANARIES_TLS
#ifdef CONFIG_ଗଦା_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL ଅସ୍ଥିର uintptr_t __stack_chk_guard;
#ଅନ୍ୟଥା
Z_THREAD_LOCAL ଅସ୍ଥିର uintptr_t __stack_chk_guard;
#endif
#ନଚେତଯଦି CONFIG_USERSPACE
K_APP_DMEM(z_libc_partition) ଅସ୍ଥିର uintptr_t __stack_chk_guard;
#ଅନ୍ୟଥା
__noinit ଅସ୍ଥିର uintptr_t __stack_chk_guard;
#endif

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(ଗଦା_Check_Handler, __stack_chk_fail, ଶୂନ୍ୟ);
