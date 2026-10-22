/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <zephyr/toolchain.h>

স্থির কাঠামো k_object *validate_kernel_object(ধ্রুবক শূন্য *obj,
					       গণনা k_objects otype,
					       গণনা _obj_init_check init)
{
	কাঠামো k_object *ko;
	পূর্ণসংখ্যা ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	যদি (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		ফেরত ফাঁকা;
	}

	ফেরত ko;
}

স্থির ALWAYS_INLINE কাঠামো k_object *validate_any_object(ধ্রুবক শূন্য *obj)
{
	ফেরত validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ধ্রুবক শূন্য *obj, গণনা k_objects otype)
{
	কাঠামো k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	ফেরত (ko != ফাঁকা);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
স্থির অন্তঃরেখ শূন্য z_vrfy_k_object_access_grant(ধ্রুবক শূন্য *object,
						কাঠামো k_thread *thread)
{
	কাঠামো k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_বার্তা(ko != ফাঁকা, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#অন্তর্ভাব <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_object_access_check(ধ্রুবক শূন্য *object)
{
	ফেরত z_impl_k_object_access_check(object);
}
#অন্তর্ভাব <zephyr/syscalls/k_object_access_check_mrsh.c>

স্থির অন্তঃরেখ শূন্য z_vrfy_k_object_release(ধ্রুবক শূন্য *object)
{
	কাঠামো k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_বার্তা(ko != ফাঁকা, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#অন্তর্ভাব <zephyr/syscalls/k_object_release_mrsh.c>

স্থির অন্তঃরেখ শূন্য *z_vrfy_k_object_alloc(গণনা k_objects otype)
{
	ফেরত z_impl_k_object_alloc(otype);
}
#অন্তর্ভাব <zephyr/syscalls/k_object_alloc_mrsh.c>

স্থির অন্তঃরেখ শূন্য *z_vrfy_k_object_alloc_আকার(গণনা k_objects otype, size_t আকার_2)
{
	ফেরত z_impl_k_object_alloc_আকার(otype, আকার_2);
}
#অন্তর্ভাব <zephyr/syscalls/k_object_alloc_size_mrsh.c>
