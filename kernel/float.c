/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <kernel_arch_interface.h>

পূর্ণাঙ্ক z_impl_k_float_disable(সংরচনা k_thread *thread)
{
#যদি defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	প্রত্যাগম arch_float_disable(thread);
#অন্যথা
	ARG_UNUSED(thread);
	প্রত্যাগম -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

পূর্ণাঙ্ক z_impl_k_float_enable(সংরচনা k_thread *thread, unsigned পূর্ণাঙ্ক options)
{
#যদি defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	প্রত্যাগম arch_float_enable(thread, options);
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	প্রত্যাগম -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_float_disable(সংরচনা k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	প্রত্যাগম z_impl_k_float_disable(thread);
}
#অন্তর্ভাব <zephyr/syscalls/k_float_disable_mrsh.c>

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_float_enable(সংরচনা k_thread *thread, unsigned পূর্ণাঙ্ক options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	প্রত্যাগম z_impl_k_float_enable(thread, options);
}
#অন্তর্ভাব <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
