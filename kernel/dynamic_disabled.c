/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <errno.h>

#شامل <zephyr/kernel.h>
#شامل <zephyr/kernel/thread_stack.h>
#شامل <zephyr/internal/syscall_handler.h>

k_thread_پشته_t *z_impl_k_thread_پشته_alloc(size_t size, صحیح flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	بازگشت پوچ;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی k_thread_پشته_t *z_vrfy_k_thread_پشته_alloc(size_t size, صحیح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	بازگشت z_impl_k_thread_پشته_alloc(size, flags);
}
#شامل <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	ARG_UNUSED(پشته);

	بازگشت -ENOSYS;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	بازگشت z_impl_k_thread_پشته_free(پشته);
}
#شامل <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
