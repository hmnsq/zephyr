/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#شامل <zephyr/offsets.h>
#شامل <offsets_short_arch.h>

/* kernel */

/* main */
#اگر_غیر_تعریف_شدہ CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#تعریف_کرو kernel_انحراف_مقصد_nested \
	(___cpu_t_nested_OFFSET)

#تعریف_کرو kernel_انحراف_مقصد_irq_ڈھیر \
	(___cpu_t_irq_stack_OFFSET)

#تعریف_کرو kernel_انحراف_مقصد_current \
	(___cpu_t_current_OFFSET)

#اگر defined(CONFIG_FPU_SHARING)
#تعریف_کرو kernel_انحراف_مقصد_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#اگر_ختم /* CONFIG_FPU_SHARING */
#اگر_ختم /* CONFIG_SMP */

#تعریف_کرو kernel_انحراف_مقصد_idle \
	(___kernel_t_idle_OFFSET)

#تعریف_کرو kernel_انحراف_مقصد_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*دھاگے*/

/* main */

#تعریف_کرو thread_انحراف_مقصد_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#اگر_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
#تعریف_کرو thread_انحراف_مقصد_tls \
	(___thread_t_tls_OFFSET)
#اگر_ختم /* CONFIG_THREAD_LOCAL_STORAGE */

/*بنیاد*/

#تعریف_کرو thread_انحراف_مقصد_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
