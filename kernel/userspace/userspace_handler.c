/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>

ସ୍ଥିର ସଂରଚନା k_object *validate_kernel_object(ଅଚଳ ଶୂନ୍ୟ *obj,
					       ଗଣନା k_objects otype,
					       ଗଣନା _obj_init_check init)
{
	ସଂରଚନା k_object *ko;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ko = k_object_find(obj);

	ret = k_object_validate(ko, otype, init);
	ଯଦି (ret != 0) {
#ifdef CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#endif /* CONFIG_LOG */
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ ko;
}

ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_object *validate_any_object(ଅଚଳ ଶୂନ୍ୟ *obj)
{
	ଫେରନ୍ତୁ validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(ଅଚଳ ଶୂନ୍ୟ *obj, ଗଣନା k_objects otype)
{
	ସଂରଚନା k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	ଫେରନ୍ତୁ (ko != ଖାଲି);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_object_access_grant(ଅଚଳ ଶୂନ୍ୟ *object,
						ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(ko != ଖାଲି, "object %p access denied",
				    object));
	k_thread_perms_set(ko, thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_object_access_check(ଅଚଳ ଶୂନ୍ୟ *object)
{
	ଫେରନ୍ତୁ z_impl_k_object_access_check(object);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_object_access_check_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_object_release(ଅଚଳ ଶୂନ୍ୟ *object)
{
	ସଂରଚନା k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(ko != ଖାଲି, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_object_release_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *z_vrfy_k_object_alloc(ଗଣନା k_objects otype)
{
	ଫେରନ୍ତୁ z_impl_k_object_alloc(otype);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_object_alloc_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *z_vrfy_k_object_alloc_size(ଗଣନା k_objects otype, size_t size)
{
	ଫେରନ୍ତୁ z_impl_k_object_alloc_size(otype, size);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
