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

#ifndef ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_
#define ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_

#અન્તર્ભાવ <zephyr/linker/linker-defs.h>

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
સ્થિર અંતઃરેખ FUNC_NO_ઢગલો_PROTECTOR size_t z_tls_data_size(શૂન્ય)
{
	size_t size = (size_t)(uintptr_t)__tdata_size + (size_t)(uintptr_t)__tbss_size;

#જો defined(CONFIG_ઢગલો_CANARIES_TLS_PREPEND)
	size += (size_t)(uintptr_t)__stack_chk_size;
#endif
	પાછા size;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
સ્થિર અંતઃરેખ FUNC_NO_ઢગલો_PROTECTOR શૂન્ય z_tls_copy(અક્ષર *dest)
{
#જો defined(CONFIG_ઢગલો_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	memcpy(dest, __stack_chk_start, (size_t)(uintptr_t)__stack_chk_size);
	dest += (size_t)(uintptr_t)__stack_chk_size;

	memcpy(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#નહીંતર
	/* Copy initialized data (tdata) */
	memcpy(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#endif
	/* Clear BSS data (tbss) */
	memset(dest, 0, (size_t)(uintptr_t)__tbss_size);
}

#endif /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
