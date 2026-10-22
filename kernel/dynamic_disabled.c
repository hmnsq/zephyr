/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/internal/syscall_handler.h>

k_thread_مكدس_t *z_impl_k_thread_مكدس_alloc(size_t size, صحيح flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	إرجاع عدم;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_thread_مكدس_t *z_vrfy_k_thread_مكدس_alloc(size_t size, صحيح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	إرجاع z_impl_k_thread_مكدس_alloc(size, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_مكدس_free(k_thread_مكدس_t *مكدس)
{
	ARG_UNUSED(مكدس);

	إرجاع -ENOSYS;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_مكدس_free(k_thread_مكدس_t *مكدس)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	إرجاع z_impl_k_thread_مكدس_free(مكدس);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
