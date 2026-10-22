/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/thread_stack.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>

k_thread_ರಾಶಿ_t *z_impl_k_thread_ರಾಶಿ_alloc(size_t size, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_thread_ರಾಶಿ_t *z_vrfy_k_thread_ರಾಶಿ_alloc(size_t size, ಪೂರ್ಣಾಙ್ಕ flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_ರಾಶಿ_alloc(size, flags);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ARG_UNUSED(ರಾಶಿ);

	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_ರಾಶಿ_free(ರಾಶಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
