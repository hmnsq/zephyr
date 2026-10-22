/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/toolchain.h>

ساکن ساخت k_object *validate_kernel_object(ثابت خالی *obj,
					       شمار k_objects otype,
					       شمار _obj_init_check init)
{
	ساخت k_object *ko;
	صحیح ret;

	ko = k_object_ڈھونڈو(obj);

	ret = k_object_validate(ko, otype, init);
	اگر (ret != 0) {
#اگر_تعریف_شدہ CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#اگر_ختم /* CONFIG_LOG */
		واپس تہی;
	}

	واپس ko;
}

ساکن ALWAYS_INLINE ساخت k_object *validate_any_object(ثابت خالی *obj)
{
	واپس validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ثابت خالی *obj, شمار k_objects otype)
{
	ساخت k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	واپس (ko != تہی);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
ساکن اندرونی خالی z_vrfy_k_object_access_grant(ثابت خالی *object,
						ساخت k_thread *thread)
{
	ساخت k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_پیغام(ko != تہی, "object %p access denied",
				    object));
	k_thread_perms_سیٹ(ko, thread);
}
#شامل <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
ساکن اندرونی صحیح z_vrfy_k_object_access_check(ثابت خالی *object)
{
	واپس z_impl_k_object_access_check(object);
}
#شامل <zephyr/syscalls/k_object_access_check_mrsh.c>

ساکن اندرونی خالی z_vrfy_k_object_release(ثابت خالی *object)
{
	ساخت k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_پیغام(ko != تہی, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#شامل <zephyr/syscalls/k_object_release_mrsh.c>

ساکن اندرونی خالی *z_vrfy_k_object_alloc(شمار k_objects otype)
{
	واپس z_impl_k_object_alloc(otype);
}
#شامل <zephyr/syscalls/k_object_alloc_mrsh.c>

ساکن اندرونی خالی *z_vrfy_k_object_alloc_سائز(شمار k_objects otype, size_t سائز)
{
	واپس z_impl_k_object_alloc_سائز(otype, سائز);
}
#شامل <zephyr/syscalls/k_object_alloc_size_mrsh.c>
