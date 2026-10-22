/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/toolchain.h>

நிலையான கட்டமைப்பு k_object *validate_kernel_object(மாறிலி வெறுமை *obj,
					       எண்ணிக்கை k_objects otype,
					       எண்ணிக்கை _obj_init_check init)
{
	கட்டமைப்பு k_object *ko;
	முழு ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	எனில் (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		திரும்பு வெற்று;
	}

	திரும்பு ko;
}

நிலையான ALWAYS_INLINE கட்டமைப்பு k_object *validate_any_object(மாறிலி வெறுமை *obj)
{
	திரும்பு validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(மாறிலி வெறுமை *obj, எண்ணிக்கை k_objects otype)
{
	கட்டமைப்பு k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	திரும்பு (ko != வெற்று);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
நிலையான உள்வரி வெறுமை z_vrfy_k_object_access_grant(மாறிலி வெறுமை *object,
						கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_செய்தி(ko != வெற்று, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
நிலையான உள்வரி முழு z_vrfy_k_object_access_check(மாறிலி வெறுமை *object)
{
	திரும்பு z_impl_k_object_access_check(object);
}
#அந்தர்பாவ <zephyr/syscalls/k_object_access_check_mrsh.c>

நிலையான உள்வரி வெறுமை z_vrfy_k_object_release(மாறிலி வெறுமை *object)
{
	கட்டமைப்பு k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_செய்தி(ko != வெற்று, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#அந்தர்பாவ <zephyr/syscalls/k_object_release_mrsh.c>

நிலையான உள்வரி வெறுமை *z_vrfy_k_object_alloc(எண்ணிக்கை k_objects otype)
{
	திரும்பு z_impl_k_object_alloc(otype);
}
#அந்தர்பாவ <zephyr/syscalls/k_object_alloc_mrsh.c>

நிலையான உள்வரி வெறுமை *z_vrfy_k_object_alloc_size(எண்ணிக்கை k_objects otype, size_t size)
{
	திரும்பு z_impl_k_object_alloc_size(otype, size);
}
#அந்தர்பாவ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
