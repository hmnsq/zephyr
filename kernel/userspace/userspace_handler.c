/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>

ಸ್ಥಿರ ರಚನೆ k_object *validate_kernel_object(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj,
					       ಎಣಿಕೆ k_objects otype,
					       ಎಣಿಕೆ _obj_init_check init)
{
	ರಚನೆ k_object *ko;
	ಪೂರ್ಣಾಂಕ ret;

	ko = k_object_ಕಂಡುಹಿಡಿ(obj);

	ret = k_object_validate(ko, otype, init);
	ಆದರೆ (ret != 0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_LOG */
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ ko;
}

ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_object *validate_any_object(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj)
{
	ಹಿಂತಿರುಗಿ validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *obj, ಎಣಿಕೆ k_objects otype)
{
	ರಚನೆ k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	ಹಿಂತಿರುಗಿ (ko != ಖಾಲಿ);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_object_access_grant(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object,
						ರಚನೆ k_thread *thread)
{
	ರಚನೆ k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(ko != ಖಾಲಿ, "object %p access denied",
				    object));
	k_thread_perms_ಗಣ(ko, thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_object_access_check(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object)
{
	ಹಿಂತಿರುಗಿ z_impl_k_object_access_check(object);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_access_check_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_object_release(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *object)
{
	ರಚನೆ k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(ko != ಖಾಲಿ, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_release_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *z_vrfy_k_object_alloc(ಎಣಿಕೆ k_objects otype)
{
	ಹಿಂತಿರುಗಿ z_impl_k_object_alloc(otype);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_alloc_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *z_vrfy_k_object_alloc_ಗಾತ್ರ(ಎಣಿಕೆ k_objects otype, size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ z_impl_k_object_alloc_ಗಾತ್ರ(otype, ಗಾತ್ರ_2);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
