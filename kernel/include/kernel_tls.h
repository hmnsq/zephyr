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

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_
#परिभाषय ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_

#अन्तर्भाव <zephyr/linker/linker-defs.h>

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
स्थिर अन्तःस्थ FUNC_NO_राशिः_PROTECTOR size_t z_tls_दत्त_आकार(शून्य)
{
	size_t आकार = (size_t)(uintptr_t)__tdata_size + (size_t)(uintptr_t)__tbss_size;

#यदि defined(CONFIG_राशिः_CANARIES_TLS_PREPEND)
	आकार += (size_t)(uintptr_t)__stack_chk_size;
#यदि_अन्तः
	प्रत्यागम आकार;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
स्थिर अन्तःस्थ FUNC_NO_राशिः_PROTECTOR शून्य z_tls_copy(वर्ण *dest)
{
#यदि defined(CONFIG_राशिः_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	स्मृतिं_प्रतिलिख(dest, __stack_chk_start, (size_t)(uintptr_t)__stack_chk_size);
	dest += (size_t)(uintptr_t)__stack_chk_size;

	स्मृतिं_प्रतिलिख(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#अन्यथा
	/* Copy initialized data (tdata) */
	स्मृतिं_प्रतिलिख(dest, __tdata_start, (size_t)(uintptr_t)__tdata_size);
	dest += (size_t)(uintptr_t)__tdata_size;
#यदि_अन्तः
	/* Clear BSS data (tbss) */
	स्मृतिं_पूरय(dest, 0, (size_t)(uintptr_t)__tbss_size);
}

#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
