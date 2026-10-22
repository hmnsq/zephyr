/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব "kernel_internal.h"

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <zephyr/kernel/thread_stack.h>
#অন্তর্ভাব <zephyr/logging/log.h>
#অন্তর্ভাব <zephyr/sys/bitarray.h>
#অন্তর্ভাব <zephyr/sys/kobject.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#যদি CONFIG_DYNAMIC_THREAD_POOL_আকার > 0
#define BA_আকার CONFIG_DYNAMIC_THREAD_POOL_আকার
#অন্যথা
#define BA_আকার 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

কাঠামো dyn_cb_তথ্য {
	k_tid_t tid;
	k_thread_স্তূপ_t *স্তূপ;
};

স্থির K_THREAD_স্তূপ_ARRAY_DEFINE(dynamic_স্তূপ, CONFIG_DYNAMIC_THREAD_POOL_আকার,
				   K_THREAD_স্তূপ_LEN(CONFIG_DYNAMIC_THREAD_স্তূপ_আকার));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_আকার);

স্থির k_thread_স্তূপ_t *z_thread_স্তূপ_alloc_pool(size_t আকার_2, পূর্ণসংখ্যা flags)
{
	পূর্ণসংখ্যা rv;
	size_t offset;
	k_thread_স্তূপ_t *স্তূপ;

	যদি (আকার_2 > CONFIG_DYNAMIC_THREAD_স্তূপ_আকার) {
		LOG_DBG("stack size %zu is > pool stack size %zu", আকার_2,
			(size_t)CONFIG_DYNAMIC_THREAD_স্তূপ_আকার);
		ফেরত ফাঁকা;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	যদি (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ফেরত ফাঁকা;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_আকার);

	স্তূপ = (k_thread_স্তূপ_t *)&dynamic_স্তূপ[offset];

	ফেরত স্তূপ;
}

স্থির k_thread_স্তূপ_t *z_thread_স্তূপ_alloc_dyn(size_t আকার_2, পূর্ণসংখ্যা flags)
{
	size_t স্তূপ_len;

	যদি ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		ফেরত k_object_alloc_আকার(K_OBJ_THREAD_স্তূপ_ELEMENT, আকার_2);
#অন্যথা
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ফেরত ফাঁকা;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	স্তূপ_len = K_KERNEL_স্তূপ_LEN(আকার_2);
	যদি (স্তূপ_len < আকার_2) {
		ফেরত ফাঁকা;
	}

	ফেরত z_thread_aligned_alloc(Z_KERNEL_স্তূপ_OBJ_ALIGN, স্তূপ_len);
}

k_thread_স্তূপ_t *z_impl_k_thread_স্তূপ_alloc(size_t আকার_2, পূর্ণসংখ্যা flags)
{
	k_thread_স্তূপ_t *স্তূপ = ফাঁকা;

	যদি (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		স্তূপ = z_thread_স্তূপ_alloc_dyn(আকার_2, flags);
		যদি (স্তূপ == ফাঁকা && CONFIG_DYNAMIC_THREAD_POOL_আকার > 0) {
			স্তূপ = z_thread_স্তূপ_alloc_pool(আকার_2, flags);
		}
	} অন্যথা যদি (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		যদি (CONFIG_DYNAMIC_THREAD_POOL_আকার > 0) {
			স্তূপ = z_thread_স্তূপ_alloc_pool(আকার_2, flags);
		}

		যদি ((স্তূপ == ফাঁকা) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			স্তূপ = z_thread_স্তূপ_alloc_dyn(আকার_2, flags);
		}
	}

	ফেরত স্তূপ;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ k_thread_স্তূপ_t *z_vrfy_k_thread_স্তূপ_alloc(size_t আকার_2, পূর্ণসংখ্যা flags)
{
	ফেরত z_impl_k_thread_স্তূপ_alloc(আকার_2, flags);
}
#অন্তর্ভাব <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

স্থির শূন্য dyn_cb(ধ্রুবক কাঠামো k_thread *thread, শূন্য *user_তথ্য)
{
	কাঠামো dyn_cb_তথ্য *ধ্রুবক তথ্য = (কাঠামো dyn_cb_তথ্য *)user_তথ্য;

	যদি (তথ্য->স্তূপ == (k_thread_স্তূপ_t *)thread->স্তূপ_info.start) {
		__ASSERT(তথ্য->tid == ফাঁকা, "stack %p is associated with more than one thread!",
			 (শূন্য *)thread->স্তূপ_info.start);
		তথ্য->tid = (k_tid_t)thread;
	}
}

পূর্ণসংখ্যা z_impl_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	কাঠামো dyn_cb_তথ্য তথ্য = {.স্তূপ = স্তূপ};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &তথ্য);

	যদি (তথ্য.tid != ফাঁকা) {
		যদি (!z_is_thread_state_set(তথ্য.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", তথ্য.tid);
			ফেরত -EBUSY;
		}
	}

	যদি (CONFIG_DYNAMIC_THREAD_POOL_আকার > 0) {
		যদি (IS_ARRAY_ELEMENT(dynamic_স্তূপ, স্তূপ)) {
			যদি (sys_bitarray_free(&dynamic_ba, 1, ARRAY_সূচক(dynamic_স্তূপ, স্তূপ))) {
				LOG_ERR("stack %p is not allocated!", স্তূপ);
				ফেরত -EINVAL;
			}

			ফেরত 0;
		}
	}

	যদি (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		যদি (k_object_find(স্তূপ)) {
			k_object_free(স্তূপ);
		} অন্যথা {
			k_free(স্তূপ);
		}
#অন্যথা
		k_free(স্তূপ);
#endif /* CONFIG_USERSPACE */
	} অন্যথা {
		LOG_DBG("Invalid stack %p", স্তূপ);
		ফেরত -EINVAL;
	}

	ফেরত 0;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(স্তূপ, K_OBJ_THREAD_স্তূপ_ELEMENT));

	ফেরত z_impl_k_thread_স্তূপ_free(স্তূপ);
}
#অন্তর্ভাব <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
