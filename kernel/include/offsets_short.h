/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/offsets.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <offsets_short_arch.h>

/* kernel */

/* main */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
/* Relies on _kernel.cpu being the first member of _kernel and having 1 element
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ kernel_ਵਿੱਥ_ਮੰਜ਼ਿਲ_nested \
	(___cpu_t_nested_OFFSET)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ kernel_ਵਿੱਥ_ਮੰਜ਼ਿਲ_irq_ਢੇਰ \
	(___cpu_t_irq_stack_OFFSET)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ kernel_ਵਿੱਥ_ਮੰਜ਼ਿਲ_current \
	(___cpu_t_current_OFFSET)

#ਜੇ defined(CONFIG_FPU_SHARING)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ kernel_ਵਿੱਥ_ਮੰਜ਼ਿਲ_fp_ctx \
	(___cpu_t_fp_ctx_OFFSET)
#ਜੇ_ਅੰਤ /* CONFIG_FPU_SHARING */
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ kernel_ਵਿੱਥ_ਮੰਜ਼ਿਲ_idle \
	(___kernel_t_idle_OFFSET)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ kernel_ਵਿੱਥ_ਮੰਜ਼ਿਲ_ready_q_cache \
	(___kernel_t_ready_q_OFFSET + ___ready_q_t_cache_OFFSET)

/* end - kernel */

/*ਧਾਗੇ*/

/* main */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ thread_ਵਿੱਥ_ਮੰਜ਼ਿਲ_callee_saved \
	(___thread_t_callee_saved_OFFSET)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_LOCAL_STORAGE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ thread_ਵਿੱਥ_ਮੰਜ਼ਿਲ_tls \
	(___thread_t_tls_OFFSET)
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_LOCAL_STORAGE */

/*ਅਧਾਰ*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ thread_ਵਿੱਥ_ਮੰਜ਼ਿਲ_user_options \
	(___thread_t_base_OFFSET + ___thread_base_t_user_options_OFFSET)

/* end - threads */

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_OFFSETS_SHORT_H_ */
