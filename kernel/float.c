/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

ကိန်းပြည့် z_impl_k_float_disable(sa_nracanaa k_thread *thread)
{
#အကယ်၍ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ပြန်ပေး arch_float_disable(thread);
#မဟုတ်ပါက
	ARG_UNUSED(thread);
	ပြန်ပေး -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ကိန်းပြည့် z_impl_k_float_enable(sa_nracanaa k_thread *thread, unsigned ကိန်းပြည့် options)
{
#အကယ်၍ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ပြန်ပေး arch_float_enable(thread, options);
#မဟုတ်ပါက
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	ပြန်ပေး -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static anta_hstha ကိန်းပြည့် z_vrfy_k_float_disable(sa_nracanaa k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ပြန်ပေး z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static anta_hstha ကိန်းပြည့် z_vrfy_k_float_enable(sa_nracanaa k_thread *thread, unsigned ကိန်းပြည့် options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ပြန်ပေး z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
