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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/linker-defs.h>

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
ਸਥਿਰ ਸਤਰਅੰਦਰ FUNC_NO_ਢੇਰ_PROTECTOR size_t z_tls_ਅੰਕੜੇ_ਮਾਪ(ਖਾਲੀ)
{
	size_t ਮਾਪ = (size_t)(uintptr_t)__tdata_size + (size_t)(uintptr_t)__tbss_size;

#ਜੇ defined(CONFIG_ਢੇਰ_CANARIES_TLS_PREPEND)
	ਮਾਪ += (size_t)(uintptr_t)__stack_chk_size;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ਮਾਪ;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ FUNC_NO_ਢੇਰ_PROTECTOR ਖਾਲੀ z_tls_copy(ਅੱਖਰ *dest)
{
#ਜੇ defined(CONFIG_ਢੇਰ_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, __stack_chk_start, (size_t)(uintptr_t)__stack_chk_size);
	dest += (size_t)(uintptr_t)__stack_chk_size;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#ਨਹੀਂਤਾਂ
	/* Copy initialized data (tdata) */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#ਜੇ_ਅੰਤ
	/* Clear BSS data (tbss) */
	memset(dest, 0, (size_t)(uintptr_t)__tbss_size);
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
