/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/toolchain.h>

സ്ഥിതം ഘടന k_object *validate_kernel_object(സ്ഥിരം ശൂന്യം *obj,
					       എണ്ണൽ k_objects otype,
					       എണ്ണൽ _obj_init_check init)
{
	ഘടന k_object *ko;
	പൂർണ്ണസംഖ്യ ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	എങ്കിൽ (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		തിരികെ ശൂന്യ;
	}

	തിരികെ ko;
}

സ്ഥിതം ALWAYS_INLINE ഘടന k_object *validate_any_object(സ്ഥിരം ശൂന്യം *obj)
{
	തിരികെ validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(സ്ഥിരം ശൂന്യം *obj, എണ്ണൽ k_objects otype)
{
	ഘടന k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	തിരികെ (ko != ശൂന്യ);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_object_access_grant(സ്ഥിരം ശൂന്യം *object,
						ഘടന k_thread *thread)
{
	ഘടന k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(ko != ശൂന്യ, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_object_access_check(സ്ഥിരം ശൂന്യം *object)
{
	തിരികെ z_impl_k_object_access_check(object);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_object_access_check_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_object_release(സ്ഥിരം ശൂന്യം *object)
{
	ഘടന k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(ko != ശൂന്യ, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_object_release_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം *z_vrfy_k_object_alloc(എണ്ണൽ k_objects otype)
{
	തിരികെ z_impl_k_object_alloc(otype);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_object_alloc_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം *z_vrfy_k_object_alloc_size(എണ്ണൽ k_objects otype, size_t size)
{
	തിരികെ z_impl_k_object_alloc_size(otype, size);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
