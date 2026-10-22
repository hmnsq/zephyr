/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ضمن <zephyr/kernel.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <kernel_arch_interface.h>

صحيح z_impl_k_float_disable(بنية k_thread *thread)
{
#ܐܢ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ܦܢܐ arch_float_disable(thread);
#ܐܚܪܢܐ
	ARG_UNUSED(thread);
	ܦܢܐ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

صحيح z_impl_k_float_enable(بنية k_thread *thread, unsigned صحيح options)
{
#ܐܢ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ܦܢܐ arch_float_enable(thread, options);
#ܐܚܪܢܐ
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	ܦܢܐ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_float_disable(بنية k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ܦܢܐ z_impl_k_float_disable(thread);
}
#ضمن <zephyr/syscalls/k_float_disable_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_float_enable(بنية k_thread *thread, unsigned صحيح options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ܦܢܐ z_impl_k_float_enable(thread, options);
}
#ضمن <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
