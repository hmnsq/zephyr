/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <kernel_arch_interface.h>

పూర్ణాంకం z_impl_k_float_disable(నిర్మాణం k_thread *thread)
{
#ఒకవేళ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	తిరిగి arch_float_disable(thread);
#లేకపోతే
	ARG_వాడని(thread);
	తిరిగి -ENOTSUP;
#అయితే_ముగింపు /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

పూర్ణాంకం z_impl_k_float_enable(నిర్మాణం k_thread *thread, గుర్తులేని పూర్ణాంకం options)
{
#ఒకవేళ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	తిరిగి arch_float_enable(thread, options);
#లేకపోతే
	ARG_వాడని(thread);
	ARG_వాడని(options);
	తిరిగి -ENOTSUP;
#అయితే_ముగింపు /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_float_disable(నిర్మాణం k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	తిరిగి z_impl_k_float_disable(thread);
}
#అన్తర్భావ <zephyr/syscalls/k_float_disable_mrsh.c>

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_float_enable(నిర్మాణం k_thread *thread, గుర్తులేని పూర్ణాంకం options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	తిరిగి z_impl_k_float_enable(thread, options);
}
#అన్తర్భావ <zephyr/syscalls/k_float_enable_mrsh.c>

#అయితే_ముగింపు /* CONFIG_USERSPACE */
