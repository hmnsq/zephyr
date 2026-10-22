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

#ضمن <zephyr/toolchain.h> /* compiler specific configurations */

#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/linker/sections.h>
#ضمن <zephyr/kernel.h>
#ضمن <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
فراغ پشته_Check_Handler(فراغ)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_پشته_CHK_FAIL);
	کد_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#اگر_تعریف_شده CONFIG_پشته_CANARIES_TLS
#اگر_تعریف_شده CONFIG_پشته_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL متطاير uintptr_t __stack_chk_guard;
#وإلا
Z_THREAD_LOCAL متطاير uintptr_t __stack_chk_guard;
#پایان_اگر
#وإلاإذا CONFIG_USERSPACE
K_APP_DMEM(z_libc_افراز) متطاير uintptr_t __stack_chk_guard;
#وإلا
__noinit متطاير uintptr_t __stack_chk_guard;
#پایان_اگر

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(پشته_Check_Handler, __stack_chk_fail, فراغ);
