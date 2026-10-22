/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <errno.h>

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/internal/syscall_handler.h>

k_thread_ڈھیر_t *z_impl_k_thread_ڈھیر_alloc(size_t ناپ, صحيح flags)
{
	ARG_بن_ورتیا(ناپ);
	ARG_بن_ورتیا(flags);

	أرجع عدم;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن k_thread_ڈھیر_t *z_vrfy_k_thread_ڈھیر_alloc(size_t ناپ, صحيح flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	أرجع z_impl_k_thread_ڈھیر_alloc(ناپ, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح z_impl_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	ARG_بن_ورتیا(ڈھیر);

	أرجع -ENOSYS;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	أرجع z_impl_k_thread_ڈھیر_free(ڈھیر);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */
