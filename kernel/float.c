/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

ចំនួនគត់ z_impl_k_float_disable(sa_nracanaa k_thread *thread)
{
#ប្រសិនបើ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ត្រឡប់ arch_float_disable(thread);
#បើមិនដូច្នេះ
	ARG_UNUSED(thread);
	ត្រឡប់ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ចំនួនគត់ z_impl_k_float_enable(sa_nracanaa k_thread *thread, unsigned ចំនួនគត់ options)
{
#ប្រសិនបើ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ត្រឡប់ arch_float_enable(thread, options);
#បើមិនដូច្នេះ
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	ត្រឡប់ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static anta_hstha ចំនួនគត់ z_vrfy_k_float_disable(sa_nracanaa k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ត្រឡប់ z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static anta_hstha ចំនួនគត់ z_vrfy_k_float_enable(sa_nracanaa k_thread *thread, unsigned ចំនួនគត់ options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ត្រឡប់ z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
