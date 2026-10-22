/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/internal/syscall_handler.h>

k_thread_پشته_t *z_impl_k_thread_پشته_alloc(size_t اندازه, صحيح flags)
{
	ARG_استفادهنشده(اندازه);
	ARG_استفادهنشده(flags);

	أرجع عدم;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن k_thread_پشته_t *z_vrfy_k_thread_پشته_alloc(size_t اندازه, صحيح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	أرجع z_impl_k_thread_پشته_alloc(اندازه, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	ARG_استفادهنشده(پشته);

	أرجع -ENOSYS;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	أرجع z_impl_k_thread_پشته_free(پشته);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
