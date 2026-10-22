/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/internal/syscall_handler.h>

k_thread_ڍير_t *z_impl_k_thread_ڍير_alloc(size_t سائيز, صحيح flags)
{
	ARG_اڻ_استعمال_ٿيل(سائيز);
	ARG_اڻ_استعمال_ٿيل(flags);

	أرجع عدم;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن k_thread_ڍير_t *z_vrfy_k_thread_ڍير_alloc(size_t سائيز, صحيح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	أرجع z_impl_k_thread_ڍير_alloc(سائيز, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_ڍير_free(k_thread_ڍير_t *ڍير)
{
	ARG_اڻ_استعمال_ٿيل(ڍير);

	أرجع -ENOSYS;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ڍير_free(k_thread_ڍير_t *ڍير)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	أرجع z_impl_k_thread_ڍير_free(ڍير);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
