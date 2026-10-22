/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <errno.h>

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/kernel/thread_stack.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>

k_thread_राशिः_t *z_impl_k_thread_राशिः_alloc(size_t size, पूर्णाङ्क flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	प्रत्यागम रिक्त;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ k_thread_राशिः_t *z_vrfy_k_thread_राशिः_alloc(size_t size, पूर्णाङ्क flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	प्रत्यागम z_impl_k_thread_राशिः_alloc(size, flags);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_thread_राशिः_free(k_thread_राशिः_t *राशिः)
{
	ARG_UNUSED(राशिः);

	प्रत्यागम -ENOSYS;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_राशिः_free(k_thread_राशिः_t *राशिः)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	प्रत्यागम z_impl_k_thread_राशिः_free(राशिः);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
