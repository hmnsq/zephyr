/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <kernel_arch_interface.h>

പൂർണ്ണസംഖ്യ z_impl_k_float_disable(ഘടന k_thread *thread)
{
#എങ്കിൽ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	തിരികെ arch_float_disable(thread);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	തിരികെ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

പൂർണ്ണസംഖ്യ z_impl_k_float_enable(ഘടന k_thread *thread, unsigned പൂർണ്ണസംഖ്യ options)
{
#എങ്കിൽ defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	തിരികെ arch_float_enable(thread, options);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	തിരികെ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_float_disable(ഘടന k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	തിരികെ z_impl_k_float_disable(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_float_disable_mrsh.c>

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_float_enable(ഘടന k_thread *thread, unsigned പൂർണ്ണസംഖ്യ options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	തിരികെ z_impl_k_float_enable(thread, options);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
