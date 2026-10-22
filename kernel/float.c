/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <kernel_arch_interface.h>

ಪೂರ್ಣಾಂಕ z_impl_k_float_disable(ರಚನೆ k_thread *thread)
{
#ಆದರೆ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ಹಿಂತಿರುಗಿ arch_float_disable(thread);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ಹಿಂತಿರುಗಿ -ENOTSUP;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ಪೂರ್ಣಾಂಕ z_impl_k_float_enable(ರಚನೆ k_thread *thread, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ options)
{
#ಆದರೆ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ಹಿಂತಿರುಗಿ arch_float_enable(thread, options);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(options);
	ಹಿಂತಿರುಗಿ -ENOTSUP;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_float_disable(ರಚನೆ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಹಿಂತಿರುಗಿ z_impl_k_float_disable(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_float_disable_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_float_enable(ರಚನೆ k_thread *thread, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಹಿಂತಿರುಗಿ z_impl_k_float_enable(thread, options);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_float_enable_mrsh.c>

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
