/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/thread_stack.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>

k_thread_ರಾಶಿ_t *z_impl_k_thread_ರಾಶಿ_alloc(size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ flags)
{
	ARG_ಬಳಸದ(ಗಾತ್ರ_2);
	ARG_ಬಳಸದ(flags);

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_thread_ರಾಶಿ_t *z_vrfy_k_thread_ರಾಶಿ_alloc(size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	ಹಿಂತಿರುಗಿ z_impl_k_thread_ರಾಶಿ_alloc(ಗಾತ್ರ_2, flags);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ARG_ಬಳಸದ(ರಾಶಿ);

	ಹಿಂತಿರುಗಿ -ENOSYS;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	ಹಿಂತಿರುಗಿ z_impl_k_thread_ರಾಶಿ_free(ರಾಶಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
