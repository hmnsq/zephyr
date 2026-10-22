/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

целое z_impl_k_float_disable(struct k_thread *thread)
{
#если defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	вернуть arch_float_disable(thread);
#иначе
	ARG_UNUSED(thread);
	вернуть -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

целое z_impl_k_float_enable(struct k_thread *thread, unsigned целое options)
{
#если defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	вернуть arch_float_enable(thread, options);
#иначе
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	вернуть -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static inline целое z_vrfy_k_float_disable(struct k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	вернуть z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static inline целое z_vrfy_k_float_enable(struct k_thread *thread, unsigned целое options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	вернуть z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
