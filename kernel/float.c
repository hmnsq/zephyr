/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

정수 z_impl_k_float_disable(struct k_thread *thread)
{
#만약 defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	반환 arch_float_disable(thread);
#아니면
	ARG_UNUSED(thread);
	반환 -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

정수 z_impl_k_float_enable(struct k_thread *thread, unsigned 정수 options)
{
#만약 defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	반환 arch_float_enable(thread, options);
#아니면
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	반환 -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static inline 정수 z_vrfy_k_float_disable(struct k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	반환 z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static inline 정수 z_vrfy_k_float_enable(struct k_thread *thread, unsigned 정수 options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	반환 z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
