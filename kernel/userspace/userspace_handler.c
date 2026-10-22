/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/toolchain.h>

ساكن بنية k_object *validate_kernel_object(ثابت فراغ *obj,
					       تعداد k_objects otype,
					       تعداد _obj_init_check init)
{
	بنية k_object *ko;
	صحيح ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	𐡄𐡍 (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		أرجع عدم;
	}

	أرجع ko;
}

ساكن ALWAYS_INLINE بنية k_object *validate_any_object(ثابت فراغ *obj)
{
	أرجع validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ثابت فراغ *obj, تعداد k_objects otype)
{
	بنية k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	أرجع (ko != عدم);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
ساكن مضمن فراغ z_vrfy_k_object_access_grant(ثابت فراغ *object,
						بنية k_thread *thread)
{
	بنية k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_MSG(ko != عدم, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#ضمن <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
ساكن مضمن صحيح z_vrfy_k_object_access_check(ثابت فراغ *object)
{
	أرجع z_impl_k_object_access_check(object);
}
#ضمن <zephyr/syscalls/k_object_access_check_mrsh.c>

ساكن مضمن فراغ z_vrfy_k_object_release(ثابت فراغ *object)
{
	بنية k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_MSG(ko != عدم, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#ضمن <zephyr/syscalls/k_object_release_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_object_alloc(تعداد k_objects otype)
{
	أرجع z_impl_k_object_alloc(otype);
}
#ضمن <zephyr/syscalls/k_object_alloc_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_object_alloc_size(تعداد k_objects otype, size_t size)
{
	أرجع z_impl_k_object_alloc_size(otype, size);
}
#ضمن <zephyr/syscalls/k_object_alloc_size_mrsh.c>
