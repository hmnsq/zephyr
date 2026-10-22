/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>

ਸਥਿਰ ਬਣਤਰ k_object *validate_kernel_object(ਅਚਲ ਖਾਲੀ *obj,
					       ਗਿਣਤੀ k_objects otype,
					       ਗਿਣਤੀ _obj_init_check init)
{
	ਬਣਤਰ k_object *ko;
	ਪੂਰਨ ret;

	ko = k_object_ਲੱਭੋ(obj);

	ret = k_object_validate(ko, otype, init);
	ਜੇ (ret != 0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#ਜੇ_ਅੰਤ /* CONFIG_LOG */
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ ko;
}

ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_object *validate_any_object(ਅਚਲ ਖਾਲੀ *obj)
{
	ਵਾਪਸ validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ਅਚਲ ਖਾਲੀ *obj, ਗਿਣਤੀ k_objects otype)
{
	ਬਣਤਰ k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	ਵਾਪਸ (ko != ਨਿਰਾ);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_object_access_grant(ਅਚਲ ਖਾਲੀ *object,
						ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(ko != ਨਿਰਾ, "object %p access denied",
				    object));
	k_thread_perms_ਸੈੱਟ(ko, thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_object_access_check(ਅਚਲ ਖਾਲੀ *object)
{
	ਵਾਪਸ z_impl_k_object_access_check(object);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_object_access_check_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_object_release(ਅਚਲ ਖਾਲੀ *object)
{
	ਬਣਤਰ k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(ko != ਨਿਰਾ, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_object_release_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *z_vrfy_k_object_alloc(ਗਿਣਤੀ k_objects otype)
{
	ਵਾਪਸ z_impl_k_object_alloc(otype);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_object_alloc_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *z_vrfy_k_object_alloc_ਮਾਪ(ਗਿਣਤੀ k_objects otype, size_t ਮਾਪ)
{
	ਵਾਪਸ z_impl_k_object_alloc_ਮਾਪ(otype, ਮਾਪ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
