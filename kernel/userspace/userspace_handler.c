/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/toolchain.h>

static sa_nracanaa k_object *validate_kernel_object(const void *obj,
					       ga_nanaa k_objects otype,
					       ga_nanaa _obj_init_check init)
{
	sa_nracanaa k_object *ko;
	පූර්ණ_සංඛ්‍යා ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	නම් (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		ආපසු NULL;
	}

	ආපසු ko;
}

static ALWAYS_INLINE sa_nracanaa k_object *validate_any_object(const void *obj)
{
	ආපසු validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(const void *obj, ga_nanaa k_objects otype)
{
	sa_nracanaa k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	ආපසු (ko != NULL);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
static anta_hstha void z_vrfy_k_object_access_grant(const void *object,
						sa_nracanaa k_thread *thread)
{
	sa_nracanaa k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_MSG(ko != NULL, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#include <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
static anta_hstha පූර්ණ_සංඛ්‍යා z_vrfy_k_object_access_check(const void *object)
{
	ආපසු z_impl_k_object_access_check(object);
}
#include <zephyr/syscalls/k_object_access_check_mrsh.c>

static anta_hstha void z_vrfy_k_object_release(const void *object)
{
	sa_nracanaa k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_MSG(ko != NULL, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#include <zephyr/syscalls/k_object_release_mrsh.c>

static anta_hstha void *z_vrfy_k_object_alloc(ga_nanaa k_objects otype)
{
	ආපසු z_impl_k_object_alloc(otype);
}
#include <zephyr/syscalls/k_object_alloc_mrsh.c>

static anta_hstha void *z_vrfy_k_object_alloc_size(ga_nanaa k_objects otype, size_t size)
{
	ආපසු z_impl_k_object_alloc_size(otype, size);
}
#include <zephyr/syscalls/k_object_alloc_size_mrsh.c>
