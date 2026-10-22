/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#అన్తర్భావ <zephyr/offsets.h>
#అన్తర్భావ <offsets_short_arch.h>

/* kernel */

/* main */
#నిర్వచితం_కాకపోతే CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#నిర్వచించు kernel_స్థానభ్రంశం_గమ్యం_nested \
	(___cpu_t_nested_OFFSET)

#నిర్వచించు kernel_స్థానభ్రంశం_గమ్యం_irq_దొంతర \
	(___cpu_t_irq_stack_OFFSET)

#నిర్వచించు kernel_స్థానభ్రంశం_గమ్యం_current \
	(___cpu_t_current_OFFSET)

#ఒకవేళ defined(CONFIG_FPU_SHARING)
#నిర్వచించు kernel_స్థానభ్రంశం_గమ్యం_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#అయితే_ముగింపు /* CONFIG_FPU_SHARING */
#అయితే_ముగింపు /* CONFIG_SMP */

#నిర్వచించు kernel_స్థానభ్రంశం_గమ్యం_idle \
	(___kernel_t_idle_OFFSET)

#నిర్వచించు kernel_స్థానభ్రంశం_గమ్యం_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*దారాలు*/

/* main */

#నిర్వచించు thread_స్థానభ్రంశం_గమ్యం_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#నిర్వచితమైతే CONFIG_THREAD_LOCAL_STORAGE
#నిర్వచించు thread_స్థానభ్రంశం_గమ్యం_tls \
	(___thread_t_tls_OFFSET)
#అయితే_ముగింపు /* CONFIG_THREAD_LOCAL_STORAGE */

/*పునాది*/

#నిర్వచించు thread_స్థానభ్రంశం_గమ్యం_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
