/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/toolchain.h>

స్థిర నిర్మాణం k_object *validate_kernel_object(స్థిరరాశి శూన్య *obj,
					       గణన k_objects otype,
					       గణన _obj_init_check init)
{
	నిర్మాణం k_object *ko;
	పూర్ణాంకం ret;

	ko = k_object_కనుగొను(obj);

	ret = k_object_validate(ko, otype, init);
	ఒకవేళ (ret != 0) {
#నిర్వచితమైతే CONFIG_LOG
		k_object_dump_error(ret, obj, ko, otype);
#అయితే_ముగింపు /* CONFIG_LOG */
		తిరిగి ఖాళీ;
	}

	తిరిగి ko;
}

స్థిర ALWAYS_INLINE నిర్మాణం k_object *validate_any_object(స్థిరరాశి శూన్య *obj)
{
	తిరిగి validate_kernel_object(obj, K_OBJ_ANY, _OBJ_INIT_ANY);
}

bool k_object_is_valid(స్థిరరాశి శూన్య *obj, గణన k_objects otype)
{
	నిర్మాణం k_object *ko;

	ko = validate_kernel_object(obj, otype, _OBJ_INIT_TRUE);

	తిరిగి (ko != ఖాళీ);
}

/* Normally these would be included in userspace.c, but the way
 * syscall_dispatch.c declares weak handlers results in build errors if these
 * are located in userspace.c. Just put in a separate file.
 *
 * To avoid double k_object_find() lookups, we don't call the implementation
 * function, but call a level deeper.
 */
స్థిర అంతఃవరుస శూన్య z_vrfy_k_object_access_grant(స్థిరరాశి శూన్య *object,
						నిర్మాణం k_thread *thread)
{
	నిర్మాణం k_object *ko;

	K_OOPS(K_SYSCALL_OBJ_INIT(thread, K_OBJ_THREAD));
	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_సందేశం(ko != ఖాళీ, "object %p access denied",
				    object));
	k_thread_perms_సమితి(ko, thread);
}
#అన్తర్భావ <zephyr/syscalls/k_object_access_grant_mrsh.c>

/*
 * z_impl_k_object_access_check already checks if the object is valid.
 * No additional verification is required.
 */
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_object_access_check(స్థిరరాశి శూన్య *object)
{
	తిరిగి z_impl_k_object_access_check(object);
}
#అన్తర్భావ <zephyr/syscalls/k_object_access_check_mrsh.c>

స్థిర అంతఃవరుస శూన్య z_vrfy_k_object_release(స్థిరరాశి శూన్య *object)
{
	నిర్మాణం k_object *ko;

	ko = validate_any_object(object);
	K_OOPS(K_SYSCALL_VERIFY_సందేశం(ko != ఖాళీ, "object %p access denied", object));
	k_thread_perms_clear(ko, _current);
}
#అన్తర్భావ <zephyr/syscalls/k_object_release_mrsh.c>

స్థిర అంతఃవరుస శూన్య *z_vrfy_k_object_alloc(గణన k_objects otype)
{
	తిరిగి z_impl_k_object_alloc(otype);
}
#అన్తర్భావ <zephyr/syscalls/k_object_alloc_mrsh.c>

స్థిర అంతఃవరుస శూన్య *z_vrfy_k_object_alloc_కొలత(గణన k_objects otype, size_t కొలత)
{
	తిరిగి z_impl_k_object_alloc_కొలత(otype, కొలత);
}
#అన్తర్భావ <zephyr/syscalls/k_object_alloc_size_mrsh.c>
