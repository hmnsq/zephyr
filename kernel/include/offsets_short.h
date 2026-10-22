/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#ಅನ್ತರ್ಭಾವ <zephyr/offsets.h>
#ಅನ್ತರ್ಭಾವ <offsets_short_arch.h>

/* kernel */

/* main */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#ವ್ಯಾಖ್ಯಾನಿಸು kernel_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_nested \
	(___cpu_t_nested_OFFSET)

#ವ್ಯಾಖ್ಯಾನಿಸು kernel_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_irq_ರಾಶಿ \
	(___cpu_t_irq_stack_OFFSET)

#ವ್ಯಾಖ್ಯಾನಿಸು kernel_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_current \
	(___cpu_t_current_OFFSET)

#ಆದರೆ defined(CONFIG_FPU_SHARING)
#ವ್ಯಾಖ್ಯಾನಿಸು kernel_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_FPU_SHARING */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

#ವ್ಯಾಖ್ಯಾನಿಸು kernel_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_idle \
	(___kernel_t_idle_OFFSET)

#ವ್ಯಾಖ್ಯಾನಿಸು kernel_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*ಎಳೆಗಳು*/

/* main */

#ವ್ಯಾಖ್ಯಾನಿಸು thread_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_LOCAL_STORAGE
#ವ್ಯಾಖ್ಯಾನಿಸು thread_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_tls \
	(___thread_t_tls_OFFSET)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_LOCAL_STORAGE */

/*ತಳ*/

#ವ್ಯಾಖ್ಯಾನಿಸು thread_ಸ್ಥಾನಾಂತರ_ಗಮ್ಯ_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
