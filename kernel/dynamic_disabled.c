/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <errno.h>

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/kernel/thread_stack.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>

k_thread_चळत_t *z_impl_k_thread_चळत_alloc(माप_प्रकार size, पूर्णांक flags)
{
	ARG_UNUSED(size);
	ARG_UNUSED(flags);

	परत रिक्त;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत k_thread_चळत_t *z_vrfy_k_thread_चळत_alloc(माप_प्रकार size, पूर्णांक flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	परत z_impl_k_thread_चळत_alloc(size, flags);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_thread_चळत_free(k_thread_चळत_t *चळत)
{
	ARG_UNUSED(चळत);

	परत -ENOSYS;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_thread_चळत_free(k_thread_चळत_t *चळत)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	परत z_impl_k_thread_चळत_free(चळत);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
