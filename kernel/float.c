/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <kernel_arch_interface.h>

पूर्णांक z_impl_k_float_disable(काष्ठा k_thread *thread)
{
#अगर defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	वापस arch_float_disable(thread);
#अन्यथा
	ARG_UNUSED(thread);
	वापस -ENOTSUP;
#पूर्ण_अगर /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

पूर्णांक z_impl_k_float_enable(काष्ठा k_thread *thread, अचिन्हित पूर्णांक options)
{
#अगर defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	वापस arch_float_enable(thread, options);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	वापस -ENOTSUP;
#पूर्ण_अगर /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_float_disable(काष्ठा k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	वापस z_impl_k_float_disable(thread);
}
#समावेश <zephyr/syscalls/k_float_disable_mrsh.c>

अटल अंतरभूत पूर्णांक z_vrfy_k_float_enable(काष्ठा k_thread *thread, अचिन्हित पूर्णांक options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	वापस z_impl_k_float_enable(thread, options);
}
#समावेश <zephyr/syscalls/k_float_enable_mrsh.c>

#पूर्ण_अगर /* CONFIG_USERSPACE */
