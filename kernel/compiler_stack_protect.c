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

#समावेश <zephyr/toolchain.h> /* compiler specific configurations */

#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/linker/sections.h>
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
व्योम ढेर_Check_Handler(व्योम)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_ढेर_CHK_FAIL);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#अगर_घोषित CONFIG_ढेर_CANARIES_TLS
#अगर_घोषित CONFIG_ढेर_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL अस्थिर uintptr_t __stack_chk_guard;
#अन्यथा
Z_THREAD_LOCAL अस्थिर uintptr_t __stack_chk_guard;
#पूर्ण_अगर
#या_अगर CONFIG_USERSPACE
K_APP_DMEM(z_libc_partition) अस्थिर uintptr_t __stack_chk_guard;
#अन्यथा
__noinit अस्थिर uintptr_t __stack_chk_guard;
#पूर्ण_अगर

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(ढेर_Check_Handler, __stack_chk_fail, व्योम);
