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

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_

#شامل <zephyr/linker/linker-defs.h>

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
ساکن اندرونی FUNC_NO_ڈھیر_PROTECTOR size_t z_tls_ڈیٹا_سائز(خالی)
{
	size_t سائز = (size_t)(uintptr_t)__tdata_size + (size_t)(uintptr_t)__tbss_size;

#اگر defined(CONFIG_ڈھیر_CANARIES_TLS_PREPEND)
	سائز += (size_t)(uintptr_t)__stack_chk_size;
#اگر_ختم
	واپس سائز;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
ساکن اندرونی FUNC_NO_ڈھیر_PROTECTOR خالی z_tls_copy(حرف *dest)
{
#اگر defined(CONFIG_ڈھیر_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	حافظہ_نقل(dest, __stack_chk_start, (size_t)(uintptr_t)__stack_chk_size);
	dest += (size_t)(uintptr_t)__stack_chk_size;

	حافظہ_نقل(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#ورنہ
	/* Copy initialized data (tdata) */
	حافظہ_نقل(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#اگر_ختم
	/* Clear BSS data (tbss) */
	memset(dest, 0, (size_t)(uintptr_t)__tbss_size);
}

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
