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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_

#ಅನ್ತರ್ಭಾವ <zephyr/linker/linker-defs.h>

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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ FUNC_NO_ರಾಶಿ_PROTECTOR size_t z_tls_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಶೂನ್ಯ)
{
	size_t ಗಾತ್ರ_2 = (size_t)(uintptr_t)__tdata_size + (size_t)(uintptr_t)__tbss_size;

#ಆದರೆ defined(CONFIG_ರಾಶಿ_CANARIES_TLS_PREPEND)
	ಗಾತ್ರ_2 += (size_t)(uintptr_t)__stack_chk_size;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ FUNC_NO_ರಾಶಿ_PROTECTOR ಶೂನ್ಯ z_tls_copy(ಅಕ್ಷರ *dest)
{
#ಆದರೆ defined(CONFIG_ರಾಶಿ_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	ಸ್ಮರಣೆ_ನಕಲು(dest, __stack_chk_start, (size_t)(uintptr_t)__stack_chk_size);
	dest += (size_t)(uintptr_t)__stack_chk_size;

	ಸ್ಮರಣೆ_ನಕಲು(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#ಇಲ್ಲದಿದ್ದರೆ
	/* Copy initialized data (tdata) */
	ಸ್ಮರಣೆ_ನಕಲು(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#ಆದರೆ_ಅಂತ್ಯ
	/* Clear BSS data (tbss) */
	memset(dest, 0, (size_t)(uintptr_t)__tbss_size);
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
