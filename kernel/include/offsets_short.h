/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_تعریف_نشده ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#تعریف_کن ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#ضمن <zephyr/offsets.h>
#ضمن <offsets_short_arch.h>

/* kernel */

/* main */
#اگر_تعریف_نشده CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#تعریف_کن kernel_جابه_جایی_مقصد_nested \
	(___cpu_t_nested_OFFSET)

#تعریف_کن kernel_جابه_جایی_مقصد_irq_پشته \
	(___cpu_t_irq_stack_OFFSET)

#تعریف_کن kernel_جابه_جایی_مقصد_current \
	(___cpu_t_current_OFFSET)

#إذا defined(CONFIG_FPU_SHARING)
#تعریف_کن kernel_جابه_جایی_مقصد_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#پایان_اگر /* CONFIG_FPU_SHARING */
#پایان_اگر /* CONFIG_SMP */

#تعریف_کن kernel_جابه_جایی_مقصد_idle \
	(___kernel_t_idle_OFFSET)

#تعریف_کن kernel_جابه_جایی_مقصد_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*نخها*/

/* main */

#تعریف_کن thread_جابه_جایی_مقصد_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#اگر_تعریف_شده CONFIG_THREAD_LOCAL_STORAGE
#تعریف_کن thread_جابه_جایی_مقصد_tls \
	(___thread_t_tls_OFFSET)
#پایان_اگر /* CONFIG_THREAD_LOCAL_STORAGE */

/*پایه*/

#تعریف_کن thread_جابه_جایی_مقصد_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#پایان_اگر /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
