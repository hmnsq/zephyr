/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/internal/syscall_handler.h>

k_thread_ډېری_t *z_impl_k_thread_ډېری_alloc(size_t اندازه, صحيح flags)
{
	ARG_ناکارول_شوی(اندازه);
	ARG_ناکارول_شوی(flags);

	أرجع عدم;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن k_thread_ډېری_t *z_vrfy_k_thread_ډېری_alloc(size_t اندازه, صحيح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	أرجع z_impl_k_thread_ډېری_alloc(اندازه, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_ډېری_free(k_thread_ډېری_t *ډېری)
{
	ARG_ناکارول_شوی(ډېری);

	أرجع -ENOSYS;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ډېری_free(k_thread_ډېری_t *ډېری)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	أرجع z_impl_k_thread_ډېری_free(ډېری);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
