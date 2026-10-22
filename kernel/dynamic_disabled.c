/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <errno.h>

#شامل <zephyr/kernel.h>
#شامل <zephyr/kernel/thread_stack.h>
#شامل <zephyr/internal/syscall_handler.h>

k_thread_ڈھیر_t *z_impl_k_thread_ڈھیر_alloc(size_t سائز, صحیح flags)
{
	ARG_غیر_مستعمل(سائز);
	ARG_غیر_مستعمل(flags);

	واپس تہی;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی k_thread_ڈھیر_t *z_vrfy_k_thread_ڈھیر_alloc(size_t سائز, صحیح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	واپس z_impl_k_thread_ڈھیر_alloc(سائز, flags);
}
#شامل <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	ARG_غیر_مستعمل(ڈھیر);

	واپس -ENOSYS;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	واپس z_impl_k_thread_ڈھیر_free(ڈھیر);
}
#شامل <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
