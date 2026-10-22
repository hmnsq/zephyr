/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#ضمن <zephyr/offsets.h>
#ضمن <offsets_short_arch.h>

/* kernel */

/* main */
#که_نه_تعریف_شوی CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#تعریف_کړه kernel_دننه_واټن_منزل_nested \
	(___cpu_t_nested_OFFSET)

#تعریف_کړه kernel_دننه_واټن_منزل_irq_ډېری \
	(___cpu_t_irq_stack_OFFSET)

#تعریف_کړه kernel_دننه_واټن_منزل_current \
	(___cpu_t_current_OFFSET)

#إذا defined(CONFIG_FPU_SHARING)
#تعریف_کړه kernel_دننه_واټن_منزل_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#د_که_پای /* CONFIG_FPU_SHARING */
#د_که_پای /* CONFIG_SMP */

#تعریف_کړه kernel_دننه_واټن_منزل_idle \
	(___kernel_t_idle_OFFSET)

#تعریف_کړه kernel_دننه_واټن_منزل_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*تارونه*/

/* main */

#تعریف_کړه thread_دننه_واټن_منزل_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#که_تعریف_شوی CONFIG_THREAD_LOCAL_STORAGE
#تعریف_کړه thread_دننه_واټن_منزل_tls \
	(___thread_t_tls_OFFSET)
#د_که_پای /* CONFIG_THREAD_LOCAL_STORAGE */

/*بنسټ*/

#تعریف_کړه thread_دننه_واټن_منزل_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
