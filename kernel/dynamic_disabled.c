/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <errno.h>

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/kernel/thread_stack.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>

k_thread_স্তূপ_t *z_impl_k_thread_স্তূপ_alloc(size_t আকার_2, পূর্ণসংখ্যা flags)
{
	ARG_UNUSED(আকার_2);
	ARG_UNUSED(flags);

	ফেরত ফাঁকা;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ k_thread_স্তূপ_t *z_vrfy_k_thread_স্তূপ_alloc(size_t আকার_2, পূর্ণসংখ্যা flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	ফেরত z_impl_k_thread_স্তূপ_alloc(আকার_2, flags);
}
#অন্তর্ভাব <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	ARG_UNUSED(স্তূপ);

	ফেরত -ENOSYS;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	ফেরত z_impl_k_thread_স্তূপ_free(স্তূপ);
}
#অন্তর্ভাব <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
