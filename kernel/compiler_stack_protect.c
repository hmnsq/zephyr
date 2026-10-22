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

#அந்தர்பாவ <zephyr/toolchain.h> /* compiler specific configurations */

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <zephyr/linker/sections.h>
#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
வெறுமை அடுக்கு_Check_Handler(வெறுமை)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_அடுக்கு_CHK_FAIL);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#ifdef CONFIG_அடுக்கு_CANARIES_TLS
#ifdef CONFIG_அடுக்கு_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL நிலையற்ற uintptr_t __stack_chk_guard;
#இல்லையெனில்
Z_THREAD_LOCAL நிலையற்ற uintptr_t __stack_chk_guard;
#endif
#அல்லதெனில் CONFIG_USERSPACE
K_APP_DMEM(z_libc_partition) நிலையற்ற uintptr_t __stack_chk_guard;
#இல்லையெனில்
__noinit நிலையற்ற uintptr_t __stack_chk_guard;
#endif

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(அடுக்கு_Check_Handler, __stack_chk_fail, வெறுமை);
