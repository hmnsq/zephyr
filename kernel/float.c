/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/kernel.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <kernel_arch_interface.h>

صحیح z_impl_k_float_disable(ساخت k_thread *thread)
{
#اگر defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	واپس arch_float_disable(thread);
#ورنہ
	ARG_غیر_مستعمل(thread);
	واپس -ENOTSUP;
#اگر_ختم /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

صحیح z_impl_k_float_enable(ساخت k_thread *thread, غیرعلامتی صحیح options)
{
#اگر defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	واپس arch_float_enable(thread, options);
#ورنہ
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(options);
	واپس -ENOTSUP;
#اگر_ختم /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_float_disable(ساخت k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	واپس z_impl_k_float_disable(thread);
}
#شامل <zephyr/syscalls/k_float_disable_mrsh.c>

ساکن اندرونی صحیح z_vrfy_k_float_enable(ساخت k_thread *thread, غیرعلامتی صحیح options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	واپس z_impl_k_float_enable(thread, options);
}
#شامل <zephyr/syscalls/k_float_enable_mrsh.c>

#اگر_ختم /* CONFIG_USERSPACE */
