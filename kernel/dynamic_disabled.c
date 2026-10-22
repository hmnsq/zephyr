/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel/thread_stack.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>

k_thread_ਢੇਰ_t *z_impl_k_thread_ਢੇਰ_alloc(size_t ਮਾਪ, ਪੂਰਨ flags)
{
	ARG_ਅਣਵਰਤਿਆ(ਮਾਪ);
	ARG_ਅਣਵਰਤਿਆ(flags);

	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ k_thread_ਢੇਰ_t *z_vrfy_k_thread_ਢੇਰ_alloc(size_t ਮਾਪ, ਪੂਰਨ flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	ਵਾਪਸ z_impl_k_thread_ਢੇਰ_alloc(ਮਾਪ, flags);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_thread_ਢੇਰ_free(k_thread_ਢੇਰ_t *ਢੇਰ)
{
	ARG_ਅਣਵਰਤਿਆ(ਢੇਰ);

	ਵਾਪਸ -ENOSYS;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_thread_ਢੇਰ_free(k_thread_ਢੇਰ_t *ਢੇਰ)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	ਵਾਪਸ z_impl_k_thread_ਢੇਰ_free(ਢੇਰ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
