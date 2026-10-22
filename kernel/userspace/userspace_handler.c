/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>

ಸ್ಥಿರ ಸಂರಚನಾ k_object *validate_kernel_object(ಧ್ರುವ ಶೂನ್ಯ *obj,
					       ಗಣನಾ k_objects otype,
					       ಗಣನಾ _obj_init_check init)
{
	ಸಂರಚನಾ k_object *ko;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	ಯದಿ (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ ko;
}

ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_object *validate_any_object(ಧ್ರುವ ಶೂನ್ಯ *obj)
{
	ಪ್ರತ್ಯಾಗಮ validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ಧ್ರುವ ಶೂನ್ಯ *obj, ಗಣನಾ k_objects otype)
{
	ಸಂರಚನಾ k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	ಪ್ರತ್ಯಾಗಮ (ko != ರಿಕ್ತ);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_object_access_grant(ಧ್ರುವ ಶೂನ್ಯ *object,
						ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(ko != ರಿಕ್ತ, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_object_access_check(ಧ್ರುವ ಶೂನ್ಯ *object)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_object_access_check(object);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_access_check_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_object_release(ಧ್ರುವ ಶೂನ್ಯ *object)
{
	ಸಂರಚನಾ k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(ko != ರಿಕ್ತ, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_release_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *z_vrfy_k_object_alloc(ಗಣನಾ k_objects otype)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_object_alloc(otype);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_alloc_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *z_vrfy_k_object_alloc_size(ಗಣನಾ k_objects otype, size_t size)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_object_alloc_size(otype, size);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
