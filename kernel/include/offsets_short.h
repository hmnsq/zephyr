/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#परिभाषय ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#अन्तर्भाव <zephyr/offsets.h>
#अन्तर्भाव <offsets_short_arch.h>

/* kernel */

/* main */
#यदि_अपरिभाषितम् CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#परिभाषय kernel_विस्थापनम्_गन्तव्यम्_nested \
	(___cpu_t_nested_OFFSET)

#परिभाषय kernel_विस्थापनम्_गन्तव्यम्_irq_राशिः \
	(___cpu_t_irq_stack_OFFSET)

#परिभाषय kernel_विस्थापनम्_गन्तव्यम्_current \
	(___cpu_t_current_OFFSET)

#यदि defined(CONFIG_FPU_SHARING)
#परिभाषय kernel_विस्थापनम्_गन्तव्यम्_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#यदि_अन्तः /* CONFIG_FPU_SHARING */
#यदि_अन्तः /* CONFIG_SMP */

#परिभाषय kernel_विस्थापनम्_गन्तव्यम्_idle \
	(___kernel_t_idle_OFFSET)

#परिभाषय kernel_विस्थापनम्_गन्तव्यम्_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*तन्तवः*/

/* main */

#परिभाषय thread_विस्थापनम्_गन्तव्यम्_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#यदि_परिभाषितम् CONFIG_THREAD_LOCAL_STORAGE
#परिभाषय thread_विस्थापनम्_गन्तव्यम्_tls \
	(___thread_t_tls_OFFSET)
#यदि_अन्तः /* CONFIG_THREAD_LOCAL_STORAGE */

/*आधारः*/

#परिभाषय thread_विस्थापनम्_गन्तव्यम्_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
