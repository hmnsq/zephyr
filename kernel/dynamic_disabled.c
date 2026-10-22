/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <errno.h>

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/kernel/thread_stack.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>

k_thread_அடுக்கு_t *z_impl_k_thread_அடுக்கு_alloc(size_t size, முழு flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	திரும்பு வெற்று;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி k_thread_அடுக்கு_t *z_vrfy_k_thread_அடுக்கு_alloc(size_t size, முழு flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	திரும்பு z_impl_k_thread_அடுக்கு_alloc(size, flags);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_thread_அடுக்கு_free(k_thread_அடுக்கு_t *அடுக்கு)
{
	ARG_UNUSED(அடுக்கு);

	திரும்பு -ENOSYS;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_thread_அடுக்கு_free(k_thread_அடுக்கு_t *அடுக்கு)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	திரும்பு z_impl_k_thread_அடுக்கு_free(அடுக்கு);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
