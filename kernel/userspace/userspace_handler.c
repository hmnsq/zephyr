/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/toolchain.h>

સ્થિર સંરચના k_object *validate_kernel_object(અચલ શૂન્ય *obj,
					       ગણના k_objects otype,
					       ગણના _obj_init_check init)
{
	સંરચના k_object *ko;
	પૂર્ણાંક ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	જો (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		પાછા ખાલી;
	}

	પાછા ko;
}

સ્થિર ALWAYS_INLINE સંરચના k_object *validate_any_object(અચલ શૂન્ય *obj)
{
	પાછા validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(અચલ શૂન્ય *obj, ગણના k_objects otype)
{
	સંરચના k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	પાછા (ko != ખાલી);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_object_access_grant(અચલ શૂન્ય *object,
						સંરચના k_thread *thread)
{
	સંરચના k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_સંદેશ(ko != ખાલી, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_object_access_check(અચલ શૂન્ય *object)
{
	પાછા z_impl_k_object_access_check(object);
}
#અન્તર્ભાવ <zephyr/syscalls/k_object_access_check_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_object_release(અચલ શૂન્ય *object)
{
	સંરચના k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_સંદેશ(ko != ખાલી, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#અન્તર્ભાવ <zephyr/syscalls/k_object_release_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય *z_vrfy_k_object_alloc(ગણના k_objects otype)
{
	પાછા z_impl_k_object_alloc(otype);
}
#અન્તર્ભાવ <zephyr/syscalls/k_object_alloc_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય *z_vrfy_k_object_alloc_size(ગણના k_objects otype, size_t size)
{
	પાછા z_impl_k_object_alloc_size(otype, size);
}
#અન્તર્ભાવ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
