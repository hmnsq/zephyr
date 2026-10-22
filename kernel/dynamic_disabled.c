/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <errno.h>

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/kernel/thread_stack.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>

k_thread_স্তূপ_t *z_impl_k_thread_স্তূপ_alloc(size_t size, পূর্ণাঙ্ক flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	ঘূৰাই রিক্ত;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ k_thread_স্তূপ_t *z_vrfy_k_thread_স্তূপ_alloc(size_t size, পূর্ণাঙ্ক flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	ঘূৰাই z_impl_k_thread_স্তূপ_alloc(size, flags);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	ARG_UNUSED(স্তূপ);

	ঘূৰাই -ENOSYS;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	ঘূৰাই z_impl_k_thread_স্তূপ_free(স্তূপ);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
