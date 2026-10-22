/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <zephyr/toolchain.h>

স্থির সংরচনা k_object *validate_kernel_object(ধ্রুৱ শূন্য *obj,
					       গণনা k_objects otype,
					       গণনা _obj_init_check init)
{
	সংরচনা k_object *ko;
	পূর্ণাঙ্ক ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	যদি (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই ko;
}

স্থির ALWAYS_INLINE সংরচনা k_object *validate_any_object(ধ্রুৱ শূন্য *obj)
{
	ঘূৰাই validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ধ্রুৱ শূন্য *obj, গণনা k_objects otype)
{
	সংরচনা k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	ঘূৰাই (ko != রিক্ত);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
স্থির অন্তঃস্থ শূন্য z_vrfy_k_object_access_grant(ধ্রুৱ শূন্য *object,
						সংরচনা k_thread *thread)
{
	সংরচনা k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(ko != রিক্ত, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_object_access_check(ধ্রুৱ শূন্য *object)
{
	ঘূৰাই z_impl_k_object_access_check(object);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_object_access_check_mrsh.c>

স্থির অন্তঃস্থ শূন্য z_vrfy_k_object_release(ধ্রুৱ শূন্য *object)
{
	সংরচনা k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(ko != রিক্ত, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_object_release_mrsh.c>

স্থির অন্তঃস্থ শূন্য *z_vrfy_k_object_alloc(গণনা k_objects otype)
{
	ঘূৰাই z_impl_k_object_alloc(otype);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_object_alloc_mrsh.c>

স্থির অন্তঃস্থ শূন্য *z_vrfy_k_object_alloc_size(গণনা k_objects otype, size_t size)
{
	ঘূৰাই z_impl_k_object_alloc_size(otype, size);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
