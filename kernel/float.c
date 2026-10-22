/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

පූර්ණ_සංඛ්‍යා z_impl_k_float_disable(sa_nracanaa k_thread *thread)
{
#නම් defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ආපසු arch_float_disable(thread);
#නැතහොත්
	ARG_UNUSED(thread);
	ආපසු -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

පූර්ණ_සංඛ්‍යා z_impl_k_float_enable(sa_nracanaa k_thread *thread, unsigned පූර්ණ_සංඛ්‍යා options)
{
#නම් defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ආපසු arch_float_enable(thread, options);
#නැතහොත්
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	ආපසු -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static anta_hstha පූර්ණ_සංඛ්‍යා z_vrfy_k_float_disable(sa_nracanaa k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ආපසු z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static anta_hstha පූර්ණ_සංඛ්‍යා z_vrfy_k_float_enable(sa_nracanaa k_thread *thread, unsigned පූර්ණ_සංඛ්‍යා options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ආපසු z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
