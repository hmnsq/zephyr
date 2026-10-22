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

#ଅନ୍ତର୍ଭାଵ <zephyr/linker/linker-defs.h>

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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ FUNC_NO_ଗଦା_PROTECTOR size_t z_tls_data_size(ଶୂନ୍ୟ)
{
	size_t size = (size_t)(uintptr_t)__tdata_size + (size_t)(uintptr_t)__tbss_size;

#ଯଦି defined(CONFIG_ଗଦା_CANARIES_TLS_PREPEND)
	size += (size_t)(uintptr_t)__stack_chk_size;
#endif
	ଫେରନ୍ତୁ size;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ FUNC_NO_ଗଦା_PROTECTOR ଶୂନ୍ୟ z_tls_copy(ଅକ୍ଷର *dest)
{
#ଯଦି defined(CONFIG_ଗଦା_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	memcpy(dest, __stack_chk_start, (size_t)(uintptr_t)__stack_chk_size);
	dest += (size_t)(uintptr_t)__stack_chk_size;

	memcpy(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#ଅନ୍ୟଥା
	/* Copy initialized data (tdata) */
	memcpy(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#endif
	/* Clear BSS data (tbss) */
	memset(dest, 0, (size_t)(uintptr_t)__tbss_size);
}

#endif /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
