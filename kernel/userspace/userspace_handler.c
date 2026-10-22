/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/toolchain.h>

ایستا ساختار k_object *validate_kernel_object(ثابت تهی *obj,
					       شمارش k_objects otype,
					       شمارش _obj_init_check init)
{
	ساختار k_object *ko;
	صحیح ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	اگر (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		بازگشت پوچ;
	}

	بازگشت ko;
}

ایستا ALWAYS_INLINE ساختار k_object *validate_any_object(ثابت تهی *obj)
{
	بازگشت validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ثابت تهی *obj, شمارش k_objects otype)
{
	ساختار k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	بازگشت (ko != پوچ);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
ایستا درونخطی تهی z_vrfy_k_object_access_grant(ثابت تهی *object,
						ساختار k_thread *thread)
{
	ساختار k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_پیام(ko != پوچ, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#شامل <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
ایستا درونخطی صحیح z_vrfy_k_object_access_check(ثابت تهی *object)
{
	بازگشت z_impl_k_object_access_check(object);
}
#شامل <zephyr/syscalls/k_object_access_check_mrsh.c>

ایستا درونخطی تهی z_vrfy_k_object_release(ثابت تهی *object)
{
	ساختار k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_پیام(ko != پوچ, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#شامل <zephyr/syscalls/k_object_release_mrsh.c>

ایستا درونخطی تهی *z_vrfy_k_object_alloc(شمارش k_objects otype)
{
	بازگشت z_impl_k_object_alloc(otype);
}
#شامل <zephyr/syscalls/k_object_alloc_mrsh.c>

ایستا درونخطی تهی *z_vrfy_k_object_alloc_size(شمارش k_objects otype, size_t size)
{
	بازگشت z_impl_k_object_alloc_size(otype, size);
}
#شامل <zephyr/syscalls/k_object_alloc_size_mrsh.c>
