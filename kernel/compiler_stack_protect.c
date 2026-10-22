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

#అన్తర్భావ <zephyr/toolchain.h> /* compiler specific configurations */

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/linker/sections.h>
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
శూన్య దొంతర_Check_Handler(శూన్య)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_దొంతర_CHK_FAIL);
	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#నిర్వచితమైతే CONFIG_దొంతర_CANARIES_TLS
#నిర్వచితమైతే CONFIG_దొంతర_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL అస్థిర uintptr_t __stack_chk_guard;
#లేకపోతే
Z_THREAD_LOCAL అస్థిర uintptr_t __stack_chk_guard;
#అయితే_ముగింపు
#లేదావేళ CONFIG_USERSPACE
K_APP_DMEM(z_libc_విభజన) అస్థిర uintptr_t __stack_chk_guard;
#లేకపోతే
__noinit అస్థిర uintptr_t __stack_chk_guard;
#అయితే_ముగింపు

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(దొంతర_Check_Handler, __stack_chk_fail, శూన్య);
