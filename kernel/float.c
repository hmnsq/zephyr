/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_arch_interface.h>

จำนวนเต็ม z_impl_k_float_disable(sa_nracanaa k_thread *thread)
{
#ถ้า defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	คืน arch_float_disable(thread);
#มิฉะนั้น
	ARG_UNUSED(thread);
	คืน -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

จำนวนเต็ม z_impl_k_float_enable(sa_nracanaa k_thread *thread, unsigned จำนวนเต็ม options)
{
#ถ้า defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	คืน arch_float_enable(thread, options);
#มิฉะนั้น
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	คืน -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
static anta_hstha จำนวนเต็ม z_vrfy_k_float_disable(sa_nracanaa k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	คืน z_impl_k_float_disable(thread);
}
#include <zephyr/syscalls/k_float_disable_mrsh.c>

static anta_hstha จำนวนเต็ม z_vrfy_k_float_enable(sa_nracanaa k_thread *thread, unsigned จำนวนเต็ม options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	คืน z_impl_k_float_enable(thread, options);
}
#include <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
