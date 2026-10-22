/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

ціле z_impl_k_float_disable(struct k_thread *thread)
{
#якщо defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	повернути arch_float_disable(thread);
#інакше
	ARG_UNUSED(thread);
	повернути -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ціле z_impl_k_float_enable(struct k_thread *thread, unsigned ціле options)
{
#якщо defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	повернути arch_float_enable(thread, options);
#інакше
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	повернути -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static inline ціле z_vrfy_k_float_disable(struct k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	повернути z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static inline ціле z_vrfy_k_float_enable(struct k_thread *thread, unsigned ціле options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	повернути z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
