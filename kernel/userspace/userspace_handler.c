/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/toolchain.h>

स्थिर संरचना k_object *validate_kernel_object(स्थिरांक शून्य *obj,
					       गणना k_objects otype,
					       गणना _obj_init_check init)
{
	संरचना k_object *ko;
	पूर्णांक ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	जर (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		परत रिक्त;
	}

	परत ko;
}

स्थिर ALWAYS_INLINE संरचना k_object *validate_any_object(स्थिरांक शून्य *obj)
{
	परत validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(स्थिरांक शून्य *obj, गणना k_objects otype)
{
	संरचना k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	परत (ko != रिक्त);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
स्थिर ओळीत शून्य z_vrfy_k_object_access_grant(स्थिरांक शून्य *object,
						संरचना k_thread *thread)
{
	संरचना k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_संदेश(ko != रिक्त, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#अन्तर्भाव <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
स्थिर ओळीत पूर्णांक z_vrfy_k_object_access_check(स्थिरांक शून्य *object)
{
	परत z_impl_k_object_access_check(object);
}
#अन्तर्भाव <zephyr/syscalls/k_object_access_check_mrsh.c>

स्थिर ओळीत शून्य z_vrfy_k_object_release(स्थिरांक शून्य *object)
{
	संरचना k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_संदेश(ko != रिक्त, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#अन्तर्भाव <zephyr/syscalls/k_object_release_mrsh.c>

स्थिर ओळीत शून्य *z_vrfy_k_object_alloc(गणना k_objects otype)
{
	परत z_impl_k_object_alloc(otype);
}
#अन्तर्भाव <zephyr/syscalls/k_object_alloc_mrsh.c>

स्थिर ओळीत शून्य *z_vrfy_k_object_alloc_size(गणना k_objects otype, माप_प्रकार size)
{
	परत z_impl_k_object_alloc_size(otype, size);
}
#अन्तर्भाव <zephyr/syscalls/k_object_alloc_size_mrsh.c>
