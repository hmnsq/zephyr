/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جے_بنا_تعریف ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#ضمن <zephyr/offsets.h>
#ضمن <offsets_short_arch.h>

/* kernel */

/* main */
#جے_بنا_تعریف CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#تعریف_کرو kernel_وتھ_منزل_nested \
	(___cpu_t_nested_OFFSET)

#تعریف_کرو kernel_وتھ_منزل_irq_ڈھیر \
	(___cpu_t_irq_stack_OFFSET)

#تعریف_کرو kernel_وتھ_منزل_current \
	(___cpu_t_current_OFFSET)

#إذا defined(CONFIG_FPU_SHARING)
#تعریف_کرو kernel_وتھ_منزل_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#جے_انت /* CONFIG_FPU_SHARING */
#جے_انت /* CONFIG_SMP */

#تعریف_کرو kernel_وتھ_منزل_idle \
	(___kernel_t_idle_OFFSET)

#تعریف_کرو kernel_وتھ_منزل_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*دھاگے*/

/* main */

#تعریف_کرو thread_وتھ_منزل_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#جے_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
#تعریف_کرو thread_وتھ_منزل_tls \
	(___thread_t_tls_OFFSET)
#جے_انت /* CONFIG_THREAD_LOCAL_STORAGE */

/*ادھار*/

#تعریف_کرو thread_وتھ_منزل_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#جے_انت /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
