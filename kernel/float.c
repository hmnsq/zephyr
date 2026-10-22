/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <kernel_arch_interface.h>

પૂર્ણાંક z_impl_k_float_disable(સંરચના k_thread *thread)
{
#જો defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	પાછા arch_float_disable(thread);
#નહીંતર
	ARG_UNUSED(thread);
	પાછા -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

પૂર્ણાંક z_impl_k_float_enable(સંરચના k_thread *thread, unsigned પૂર્ણાંક options)
{
#જો defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	પાછા arch_float_enable(thread, options);
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	પાછા -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_float_disable(સંરચના k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	પાછા z_impl_k_float_disable(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_float_disable_mrsh.c>

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_float_enable(સંરચના k_thread *thread, unsigned પૂર્ણાંક options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	પાછા z_impl_k_float_enable(thread, options);
}
#અન્તર્ભાવ <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
