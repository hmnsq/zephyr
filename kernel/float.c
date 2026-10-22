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
#यदि defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	फर्काऊ arch_float_disable(thread);
#अन्यथा
	ARG_UNUSED(thread);
	फर्काऊ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

पूर्णाङ्क z_impl_k_float_enable(संरचना k_thread *thread, अचिन्हित पूर्णाङ्क options)
{
#यदि defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	फर्काऊ arch_float_enable(thread, options);
#अन्यथा
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	फर्काऊ -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_float_disable(संरचना k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	फर्काऊ z_impl_k_float_disable(thread);
}
#अन्तर्भाव <zephyr/syscalls/k_float_disable_mrsh.c>

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_float_enable(संरचना k_thread *thread, अचिन्हित पूर्णाङ्क options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	फर्काऊ z_impl_k_float_enable(thread, options);
}
#अन्तर्भाव <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
