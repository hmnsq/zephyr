/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <kernel_arch_interface.h>

पूर्णाङ्क z_impl_k_float_disable(संरचना k_thread *thread)
{
#सचे defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	पटिनिवत्त arch_float_disable(thread);
#अञ्ञथा
	ARG_UNUSED(thread);
	पटिनिवत्त -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

पूर्णाङ्क z_impl_k_float_enable(संरचना k_thread *thread, unsigned पूर्णाङ्क options)
{
#सचे defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	पटिनिवत्त arch_float_enable(thread, options);
#अञ्ञथा
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	पटिनिवत्त -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_float_disable(संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	पटिनिवत्त z_impl_k_float_disable(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_float_disable_mrsh.c>

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_float_enable(संरचना k_thread *thread, unsigned पूर्णाङ्क options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	पटिनिवत्त z_impl_k_float_enable(thread, options);
}
#अन्तर्भाव <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
