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

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_

#समावेश <zephyr/linker/linker-defs.h>

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
अटल अंतरभूत FUNC_NO_ढेर_PROTECTOR माप_प्रकार z_tls_तथ्य_आकार(व्योम)
{
	माप_प्रकार आकार = (माप_प्रकार)(uintptr_t)__tdata_size + (माप_प्रकार)(uintptr_t)__tbss_size;

#अगर defined(CONFIG_ढेर_CANARIES_TLS_PREPEND)
	आकार += (माप_प्रकार)(uintptr_t)__stack_chk_size;
#पूर्ण_अगर
	वापस आकार;
}

/**
 * @brief Copy the TLS data/bss areas into destination
 *
 * This copies the TLS data into destination and clear the area
 * of TLS bss size after the data section.
 *
 * @param dest Pointer to destination
 */
अटल अंतरभूत FUNC_NO_ढेर_PROTECTOR व्योम z_tls_copy(अक्षर *dest)
{
#अगर defined(CONFIG_ढेर_CANARIES_TLS_PREPEND)
	/* Copy .stack_chk.guard and .tdata separately since the linker may insert
	 * padding between sections.
	 */
	स_नकल(dest, __stack_chk_start, (माप_प्रकार)(uintptr_t)__stack_chk_size);
	dest += (माप_प्रकार)(uintptr_t)__stack_chk_size;

	स_नकल(dest, __tdata_start, (माप_प्रकार)(uintptr_t)__tdata_size);
	dest += (माप_प्रकार)(uintptr_t)__tdata_size;
#अन्यथा
	/* Copy initialized data (tdata) */
	स_नकल(dest, __tdata_start, (माप_प्रकार)(uintptr_t)__tdata_size);
	dest += (माप_प्रकार)(uintptr_t)__tdata_size;
#पूर्ण_अगर
	/* Clear BSS data (tbss) */
	स_रखो(dest, 0, (माप_प्रकार)(uintptr_t)__tbss_size);
}

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_KERNEL_TLS_H_ */
