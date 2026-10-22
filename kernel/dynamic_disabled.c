/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <errno.h>

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/kernel/thread_stack.h>
#समावेश <zephyr/internal/syscall_handler.h>

k_thread_ढेर_t *z_impl_k_thread_ढेर_alloc(माप_प्रकार आकार, पूर्णांक flags)
{
	ARG_UNUSED(आकार);
	ARG_UNUSED(flags);

	वापस शून्य;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत k_thread_ढेर_t *z_vrfy_k_thread_ढेर_alloc(माप_प्रकार आकार, पूर्णांक flags)
{
	/* No check needed: neither argument is dereferenced, allocation always fails. */
	वापस z_impl_k_thread_ढेर_alloc(आकार, flags);
}
#समावेश <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_thread_ढेर_free(k_thread_ढेर_t *ढेर)
{
	ARG_UNUSED(ढेर);

	वापस -ENOSYS;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_thread_ढेर_free(k_thread_ढेर_t *ढेर)
{
	/* No check needed: stack is not dereferenced, -ENOSYS is always returned. */
	वापस z_impl_k_thread_ढेर_free(ढेर);
}
#समावेश <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
