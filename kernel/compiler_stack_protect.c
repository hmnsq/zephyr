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

#अन्तर्भाव <zephyr/toolchain.h> /* compiler specific configurations */

#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <zephyr/linker/sections.h>
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/app_memory/app_memdomain.h>

/**
 *
 * @brief Stack canary error handler
 *
 * This function is invoked when a stack canary error is detected.
 *
 * @note This function does not return.
 */
शून्य चळत_Check_Handler(शून्य)
{
	/* Stack canary error is a software fatal condition; treat it as such.
	 */
	z_except_reason(K_ERR_चळत_CHK_FAIL);
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

/* Global variable */

/*
 * Symbol referenced by GCC compiler generated code for canary value.
 * The canary value gets initialized in z_cstart().
 */
#ifdef CONFIG_चळत_CANARIES_TLS
#ifdef CONFIG_चळत_CANARIES_TLS_PREPEND
__attribute__((section(".stack_chk.guard"))) Z_THREAD_LOCAL चञ्चल uintptr_t __stack_chk_guard;
#अन्यथा
Z_THREAD_LOCAL चञ्चल uintptr_t __stack_chk_guard;
#endif
#नोचेद्यदि CONFIG_USERSPACE
K_APP_DMEM(z_libc_partition) चञ्चल uintptr_t __stack_chk_guard;
#अन्यथा
__noinit चञ्चल uintptr_t __stack_chk_guard;
#endif

/**
 *
 * @brief Referenced by GCC compiler generated code
 *
 * This routine is invoked when a stack canary error is detected, indicating
 * a buffer overflow or stack corruption problem.
 */
FUNC_ALIAS(चळत_Check_Handler, __stack_chk_fail, शून्य);
