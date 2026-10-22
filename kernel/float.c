/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <kernel_arch_interface.h>

முழு z_impl_k_float_disable(கட்டமைப்பு k_thread *thread)
{
#எனில் defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	திரும்பு arch_float_disable(thread);
#இல்லையெனில்
	ARG_UNUSED(thread);
	திரும்பு -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

முழு z_impl_k_float_enable(கட்டமைப்பு k_thread *thread, unsigned முழு options)
{
#எனில் defined(CONFIG_FPU) && defined(CONFIG_FPU_SHARING)
	திரும்பு arch_float_enable(thread, options);
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
	திரும்பு -ENOTSUP;
#endif /* CONFIG_FPU && CONFIG_FPU_SHARING */
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_float_disable(கட்டமைப்பு k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	திரும்பு z_impl_k_float_disable(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_float_disable_mrsh.c>

நிலையான உள்வரி முழு z_vrfy_k_float_enable(கட்டமைப்பு k_thread *thread, unsigned முழு options)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	திரும்பு z_impl_k_float_enable(thread, options);
}
#அந்தர்பாவ <zephyr/syscalls/k_float_enable_mrsh.c>

#endif /* CONFIG_USERSPACE */
