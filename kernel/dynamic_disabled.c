/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel/thread_stack.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>

k_thread_ଗଦା_t *z_impl_k_thread_ଗଦା_alloc(size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	ଫେରନ୍ତୁ ଖାଲି;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_thread_ଗଦା_t *z_vrfy_k_thread_ଗଦା_alloc(size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	ଫେରନ୍ତୁ z_impl_k_thread_ଗଦା_alloc(size, flags);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_ଗଦା_free(k_thread_ଗଦା_t *ଗଦା)
{
	ARG_UNUSED(ଗଦା);

	ଫେରନ୍ତୁ -ENOSYS;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_ଗଦା_free(k_thread_ଗଦା_t *ଗଦା)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	ଫେରନ୍ତୁ z_impl_k_thread_ଗଦା_free(ଗଦା);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
