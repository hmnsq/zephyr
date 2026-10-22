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

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h> /* compiler specific configurations */

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
ಶೂನ್ಯ ರಾಶಿ_Check_Handler(ಶೂನ್ಯ)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_ರಾಶಿ_CHK_FAIL);
	ಸಂಕೇತ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ರಾಶಿ_CANARIES_TLS
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ರಾಶಿ_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL ಅಸ್ಥಿರ uintptr_t __stack_chk_guard;
#ಇಲ್ಲದಿದ್ದರೆ
Z_THREAD_LOCAL ಅಸ್ಥಿರ uintptr_t __stack_chk_guard;
#ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ CONFIG_USERSPACE
K_APP_DMEM(z_libc_ವಿಭಜನೆ) ಅಸ್ಥಿರ uintptr_t __stack_chk_guard;
#ಇಲ್ಲದಿದ್ದರೆ
__noinit ಅಸ್ಥಿರ uintptr_t __stack_chk_guard;
#ಆದರೆ_ಅಂತ್ಯ

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(ರಾಶಿ_Check_Handler, __stack_chk_fail, ಶೂನ್ಯ);
