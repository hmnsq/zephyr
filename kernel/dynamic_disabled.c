/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/internal/syscall_handler.h>

k_thread_stack_t *z_impl_k_thread_stack_alloc(size_t size, صحيح flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	ܦܢܐ عدم;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_thread_stack_t *z_vrfy_k_thread_stack_alloc(size_t size, صحيح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	ܦܢܐ z_impl_k_thread_stack_alloc(size, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_stack_free(k_thread_stack_t *stack)
{
	ARG_UNUSED(stack);

	ܦܢܐ -ENOSYS;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_stack_free(k_thread_stack_t *stack)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	ܦܢܐ z_impl_k_thread_stack_free(stack);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
