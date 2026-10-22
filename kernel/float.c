/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <kernel_arch_interface.h>

ಪೂರ್ಣಾಙ್ಕ z_impl_k_float_disable(ಸಂರಚನಾ k_thread *thread)
{
#ಯದಿ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ಪ್ರತ್ಯಾಗಮ arch_float_disable(thread);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_float_enable(ಸಂರಚನಾ k_thread *thread, unsigned ಪೂರ್ಣಾಙ್ಕ options)
{
#ಯದಿ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ಪ್ರತ್ಯಾಗಮ arch_float_enable(thread, options);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_float_disable(ಸಂರಚನಾ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_float_disable(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_float_disable_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_float_enable(ಸಂರಚನಾ k_thread *thread, unsigned ಪೂರ್ಣಾಙ್ಕ options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_float_enable(thread, options);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
