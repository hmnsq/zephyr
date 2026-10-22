/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel Thread Local Storage APIs.
 *
 * Kernel APIs related to thread local storage.
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_

#అన్తర్భావ <zephyr/linker/linker-defs.h>

/**
 * @brief Return the total size of TLS data/bss areas
 *
 * This returns the total size of thread local storage (TLS)
 * data and bss areas as defined in the linker script.
 * Note that this does not include any architecture specific
 * bits required for proper functionality of TLS.
 *
 * @return Total size of TLS data/bss areas
 */
స్థిర అంతఃవరుస FUNC_NO_దొంతర_PROTECTOR size_t z_tls_దత్తాంశం_కొలత(శూన్య)
{
	size_t కొలత = (size_t)(uintptr_t)__tdata_size + (size_t)(uintptr_t)__tbss_size;

#ఒకవేళ defined(CONFIG_దొంతర_CANARIES_TLS_PREPEND)
	కొలత += (size_t)(uintptr_t)__stack_chk_size;
#అయితే_ముగింపు
	తిరిగి కొలత;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
స్థిర అంతఃవరుస FUNC_NO_దొంతర_PROTECTOR శూన్య z_tls_copy(అక్షరం *dest)
{
#ఒకవేళ defined(CONFIG_దొంతర_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	జ్ఞాపకం_నకలు(dest, __stack_chk_start, (size_t)(uintptr_t)__stack_chk_size);
	dest += (size_t)(uintptr_t)__stack_chk_size;

	జ్ఞాపకం_నకలు(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#లేకపోతే
	/* Copy initialized data (tdata) */
	జ్ఞాపకం_నకలు(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#అయితే_ముగింపు
	/* Clear BSS data (tbss) */
	memset(dest, 0, (size_t)(uintptr_t)__tbss_size);
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
