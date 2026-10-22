/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/toolchain.h>

स्थिर संरचना k_object *validate_kernel_object(अचर शून्य *obj,
					       गणना k_objects otype,
					       गणना _obj_init_check init)
{
	संरचना k_object *ko;
	पूर्णाङ्क ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	यदि (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		फर्काऊ रिक्त;
	}

	फर्काऊ ko;
}

स्थिर ALWAYS_INLINE संरचना k_object *validate_any_object(अचर शून्य *obj)
{
	फर्काऊ validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(अचर शून्य *obj, गणना k_objects otype)
{
	संरचना k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	फर्काऊ (ko != रिक्त);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
स्थिर अन्तःस्थ शून्य z_vrfy_k_object_access_grant(अचर शून्य *object,
						संरचना k_thread *thread)
{
	संरचना k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_सन्देश(ko != रिक्त, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#अन्तर्भाव <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_object_access_check(अचर शून्य *object)
{
	फर्काऊ z_impl_k_object_access_check(object);
}
#अन्तर्भाव <zephyr/syscalls/k_object_access_check_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_object_release(अचर शून्य *object)
{
	संरचना k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_सन्देश(ko != रिक्त, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#अन्तर्भाव <zephyr/syscalls/k_object_release_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_object_alloc(गणना k_objects otype)
{
	फर्काऊ z_impl_k_object_alloc(otype);
}
#अन्तर्भाव <zephyr/syscalls/k_object_alloc_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_object_alloc_size(गणना k_objects otype, माप_प्रकार size)
{
	फर्काऊ z_impl_k_object_alloc_size(otype, size);
}
#अन्तर्भाव <zephyr/syscalls/k_object_alloc_size_mrsh.c>
