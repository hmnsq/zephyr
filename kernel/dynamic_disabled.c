/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/internal/syscall_handler.h>

k_thread_دۆۋە_t *z_impl_k_thread_دۆۋە_alloc(size_t size, صحيح flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	أرجع عدم;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_thread_دۆۋە_t *z_vrfy_k_thread_دۆۋە_alloc(size_t size, صحيح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	أرجع z_impl_k_thread_دۆۋە_alloc(size, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_دۆۋە_free(k_thread_دۆۋە_t *دۆۋە)
{
	ARG_UNUSED(دۆۋە);

	أرجع -ENOSYS;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_دۆۋە_free(k_thread_دۆۋە_t *دۆۋە)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	أرجع z_impl_k_thread_دۆۋە_free(دۆۋە);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
