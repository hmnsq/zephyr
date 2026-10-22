/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_arch_interface.h>

ਪੂਰਨ z_impl_k_float_disable(ਬਣਤਰ k_thread *thread)
{
#ਜੇ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ਵਾਪਸ arch_float_disable(thread);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ਵਾਪਸ -ENOTSUP;
#ਜੇ_ਅੰਤ /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ਪੂਰਨ z_impl_k_float_enable(ਬਣਤਰ k_thread *thread, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ options)
{
#ਜੇ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ਵਾਪਸ arch_float_enable(thread, options);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(options);
	ਵਾਪਸ -ENOTSUP;
#ਜੇ_ਅੰਤ /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_float_disable(ਬਣਤਰ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ਵਾਪਸ z_impl_k_float_disable(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_float_disable_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_float_enable(ਬਣਤਰ k_thread *thread, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ਵਾਪਸ z_impl_k_float_enable(thread, options);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_float_enable_mrsh.c>

#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
