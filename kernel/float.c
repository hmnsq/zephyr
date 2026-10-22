/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <kernel_arch_interface.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_float_disable(ସଂରଚନା k_thread *thread)
{
#ଯଦି defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ଫେରନ୍ତୁ arch_float_disable(thread);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ଫେରନ୍ତୁ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_float_enable(ସଂରଚନା k_thread *thread, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
#ଯଦି defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	ଫେରନ୍ତୁ arch_float_enable(thread, options);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	ଫେରନ୍ତୁ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_float_disable(ସଂରଚନା k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ଫେରନ୍ତୁ z_impl_k_float_disable(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_float_disable_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_float_enable(ସଂରଚନା k_thread *thread, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ଫେରନ୍ତୁ z_impl_k_float_enable(thread, options);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
