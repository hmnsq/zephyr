/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/kernel/thread_stack.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>

k_thread_അട്ടി_t *z_impl_k_thread_അട്ടി_alloc(size_t size, പൂർണ്ണസംഖ്യ flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	തിരികെ ശൂന്യ;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ k_thread_അട്ടി_t *z_vrfy_k_thread_അട്ടി_alloc(size_t size, പൂർണ്ണസംഖ്യ flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	തിരികെ z_impl_k_thread_അട്ടി_alloc(size, flags);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_thread_അട്ടി_free(k_thread_അട്ടി_t *അട്ടി)
{
	ARG_UNUSED(അട്ടി);

	തിരികെ -ENOSYS;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_thread_അട്ടി_free(k_thread_അട്ടി_t *അട്ടി)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	തിരികെ z_impl_k_thread_അട്ടി_free(അട്ടി);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
