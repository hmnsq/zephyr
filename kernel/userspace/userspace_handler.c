/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/toolchain.h>

अटल काष्ठा k_object *validate_kernel_object(स्थिर व्योम *obj,
					       क्रमागत k_objects otype,
					       क्रमागत _obj_init_check init)
{
	काष्ठा k_object *ko;
	पूर्णांक ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	अगर (ret != 0) {
#अगर_घोषित CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#पूर्ण_अगर /* CONFIG_LOG */
		वापस शून्य;
	}

	वापस ko;
}

अटल ALWAYS_INLINE काष्ठा k_object *validate_any_object(स्थिर व्योम *obj)
{
	वापस validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(स्थिर व्योम *obj, क्रमागत k_objects otype)
{
	काष्ठा k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	वापस (ko != शून्य);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
अटल अंतरभूत व्योम z_vrfy_k_object_access_grant(स्थिर व्योम *object,
						काष्ठा k_thread *thread)
{
	काष्ठा k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_संदेश(ko != शून्य, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#समावेश <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
अटल अंतरभूत पूर्णांक z_vrfy_k_object_access_check(स्थिर व्योम *object)
{
	वापस z_impl_k_object_access_check(object);
}
#समावेश <zephyr/syscalls/k_object_access_check_mrsh.c>

अटल अंतरभूत व्योम z_vrfy_k_object_release(स्थिर व्योम *object)
{
	काष्ठा k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_संदेश(ko != शून्य, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#समावेश <zephyr/syscalls/k_object_release_mrsh.c>

अटल अंतरभूत व्योम *z_vrfy_k_object_alloc(क्रमागत k_objects otype)
{
	वापस z_impl_k_object_alloc(otype);
}
#समावेश <zephyr/syscalls/k_object_alloc_mrsh.c>

अटल अंतरभूत व्योम *z_vrfy_k_object_alloc_आकार(क्रमागत k_objects otype, माप_प्रकार आकार)
{
	वापस z_impl_k_object_alloc_आकार(otype, आकार);
}
#समावेश <zephyr/syscalls/k_object_alloc_size_mrsh.c>
