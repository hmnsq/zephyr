/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

số_nguyên z_impl_k_float_disable(struct k_thread *thread)
{
#nếu defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	trả_về arch_float_disable(thread);
#không_thì
	ARG_UNUSED(thread);
	trả_về -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

số_nguyên z_impl_k_float_enable(struct k_thread *thread, unsigned số_nguyên options)
{
#nếu defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	trả_về arch_float_enable(thread, options);
#không_thì
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	trả_về -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static inline số_nguyên z_vrfy_k_float_disable(struct k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	trả_về z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static inline số_nguyên z_vrfy_k_float_enable(struct k_thread *thread, unsigned số_nguyên options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	trả_về z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
