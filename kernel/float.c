/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/kernel.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <kernel_arch_interface.h>

صحیح z_impl_k_float_disable(ساختار k_thread *thread)
{
#اگر defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	بازگشت arch_float_disable(thread);
#وگرنه
	ARG_UNUSED(thread);
	بازگشت -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

صحیح z_impl_k_float_enable(ساختار k_thread *thread, unsigned صحیح options)
{
#اگر defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	بازگشت arch_float_enable(thread, options);
#وگرنه
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	بازگشت -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_float_disable(ساختار k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	بازگشت z_impl_k_float_disable(thread);
}
#شامل <zephyr/syscalls/k_float_disable_mrsh.c>

ایستا درونخطی صحیح z_vrfy_k_float_enable(ساختار k_thread *thread, unsigned صحیح options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	بازگشت z_impl_k_float_enable(thread, options);
}
#شامل <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
