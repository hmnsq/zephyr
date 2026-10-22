/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#ضمن <zephyr/offsets.h>
#ضمن <offsets_short_arch.h>

/* kernel */

/* main */
#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#وصف_ڪريو kernel_وٿي_منزل_nested \
	(___cpu_t_nested_OFFSET)

#وصف_ڪريو kernel_وٿي_منزل_irq_ڍير \
	(___cpu_t_irq_stack_OFFSET)

#وصف_ڪريو kernel_وٿي_منزل_current \
	(___cpu_t_current_OFFSET)

#إذا defined(CONFIG_FPU_SHARING)
#وصف_ڪريو kernel_وٿي_منزل_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#جيڪڏهن_پورو /* CONFIG_FPU_SHARING */
#جيڪڏهن_پورو /* CONFIG_SMP */

#وصف_ڪريو kernel_وٿي_منزل_idle \
	(___kernel_t_idle_OFFSET)

#وصف_ڪريو kernel_وٿي_منزل_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*ڌاڳا*/

/* main */

#وصف_ڪريو thread_وٿي_منزل_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_LOCAL_STORAGE
#وصف_ڪريو thread_وٿي_منزل_tls \
	(___thread_t_tls_OFFSET)
#جيڪڏهن_پورو /* CONFIG_THREAD_LOCAL_STORAGE */

/*بنياد*/

#وصف_ڪريو thread_وٿي_منزل_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
