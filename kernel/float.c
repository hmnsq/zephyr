/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

ཧྲིལ z_impl_k_float_disable(sa_nracanaa k_thread *thread)
{
#གལ་སྲིད defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ལོག arch_float_disable(thread);
#དེ་མིན
	ARG_UNUSED(thread);
	ལོག -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ཧྲིལ z_impl_k_float_enable(sa_nracanaa k_thread *thread, unsigned ཧྲིལ options)
{
#གལ་སྲིད defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ལོག arch_float_enable(thread, options);
#དེ་མིན
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	ལོག -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static anta_hstha ཧྲིལ z_vrfy_k_float_disable(sa_nracanaa k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ལོག z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static anta_hstha ཧྲིལ z_vrfy_k_float_enable(sa_nracanaa k_thread *thread, unsigned ཧྲིལ options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ལོག z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
