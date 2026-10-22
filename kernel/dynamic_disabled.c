/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/kernel/thread_stack.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>

k_thread_ઢગલો_t *z_impl_k_thread_ઢગલો_alloc(size_t size, પૂર્ણાંક flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	પાછા ખાલી;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ k_thread_ઢગલો_t *z_vrfy_k_thread_ઢગલો_alloc(size_t size, પૂર્ણાંક flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	પાછા z_impl_k_thread_ઢગલો_alloc(size, flags);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_thread_ઢગલો_free(k_thread_ઢગલો_t *ઢગલો)
{
	ARG_UNUSED(ઢગલો);

	પાછા -ENOSYS;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_thread_ઢગલો_free(k_thread_ઢગલો_t *ઢગલો)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	પાછા z_impl_k_thread_ઢગલો_free(ઢગલો);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
