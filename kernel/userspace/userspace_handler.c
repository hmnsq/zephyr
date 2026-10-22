/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/toolchain.h>

स्थिर संरचना k_object *validate_kernel_object(ध्रुव शून्य *obj,
					       गणना k_objects otype,
					       गणना _obj_init_check init)
{
	संरचना k_object *ko;
	पूर्णाङ्क ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	यदि (ret != 0) {
#यदि_परिभाषितम् CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#यदि_अन्तः /* CONFIG_LOG */
		प्रत्यागम रिक्त;
	}

	प्रत्यागम ko;
}

स्थिर ALWAYS_INLINE संरचना k_object *validate_any_object(ध्रुव शून्य *obj)
{
	प्रत्यागम validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ध्रुव शून्य *obj, गणना k_objects otype)
{
	संरचना k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	प्रत्यागम (ko != रिक्त);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
स्थिर अन्तःस्थ शून्य z_vrfy_k_object_access_grant(ध्रुव शून्य *object,
						संरचना k_thread *thread)
{
	संरचना k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(ko != रिक्त, "object %p access denied",
				    object));
	k_thread_perms_समुच्चयः(ko, thread);
}
#अन्तर्भाव <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_object_access_check(ध्रुव शून्य *object)
{
	प्रत्यागम z_impl_k_object_access_check(object);
}
#अन्तर्भाव <zephyr/syscalls/k_object_access_check_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_object_release(ध्रुव शून्य *object)
{
	संरचना k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(ko != रिक्त, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#अन्तर्भाव <zephyr/syscalls/k_object_release_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_object_alloc(गणना k_objects otype)
{
	प्रत्यागम z_impl_k_object_alloc(otype);
}
#अन्तर्भाव <zephyr/syscalls/k_object_alloc_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_object_alloc_आकार(गणना k_objects otype, size_t आकार)
{
	प्रत्यागम z_impl_k_object_alloc_आकार(otype, आकार);
}
#अन्तर्भाव <zephyr/syscalls/k_object_alloc_size_mrsh.c>
