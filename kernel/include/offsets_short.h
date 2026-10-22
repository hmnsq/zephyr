/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#समावेश <zephyr/offsets.h>
#समावेश <offsets_short_arch.h>

/* kernel */

/* main */
#अगर_अघोषित CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#घोषणा _kernel_offset_to_nested \
	(___cpu_t_nested_OFFSET)

#घोषणा kernel_offset_to_irq_ढेर \
	(___cpu_t_irq_stack_OFFSET)

#घोषणा _kernel_offset_to_current \
	(___cpu_t_current_OFFSET)

#अगर defined(CONFIG_FPU_SHARING)
#घोषणा _kernel_offset_to_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#पूर्ण_अगर /* CONFIG_FPU_SHARING */
#पूर्ण_अगर /* CONFIG_SMP */

#घोषणा _kernel_offset_to_idle \
	(___kernel_t_idle_OFFSET)

#घोषणा _kernel_offset_to_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/* threads */

/* main */

#घोषणा _thread_offset_to_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#अगर_घोषित CONFIG_THREAD_LOCAL_STORAGE
#घोषणा _thread_offset_to_tls \
	(___thread_t_tls_OFFSET)
#पूर्ण_अगर /* CONFIG_THREAD_LOCAL_STORAGE */

/* base */

#घोषणा _thread_offset_to_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
