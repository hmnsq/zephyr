/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <errno.h>

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/kernel/thread_stack.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>

k_thread_దొంతర_t *z_impl_k_thread_దొంతర_alloc(size_t కొలత, పూర్ణాంకం flags)
{
	ARG_వాడని(కొలత);
	ARG_వాడని(flags);

	తిరిగి ఖాళీ;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస k_thread_దొంతర_t *z_vrfy_k_thread_దొంతర_alloc(size_t కొలత, పూర్ణాంకం flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	తిరిగి z_impl_k_thread_దొంతర_alloc(కొలత, flags);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_thread_దొంతర_free(k_thread_దొంతర_t *దొంతర)
{
	ARG_వాడని(దొంతర);

	తిరిగి -ENOSYS;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_thread_దొంతర_free(k_thread_దొంతర_t *దొంతర)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	తిరిగి z_impl_k_thread_దొంతర_free(దొంతర);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
