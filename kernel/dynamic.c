/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ "kernel_internal.h"

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <zephyr/kernel/thread_stack.h>
#অন্তর্ভাৱ <zephyr/logging/log.h>
#অন্তর্ভাৱ <zephyr/sys/bitarray.h>
#অন্তর্ভাৱ <zephyr/sys/kobject.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#যদি CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#অন্যথা
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

সংরচনা dyn_cb_data {
	k_tid_t tid;
	k_thread_স্তূপ_t *স্তূপ;
};

স্থির K_THREAD_স্তূপ_ARRAY_DEFINE(dynamic_স্তূপ, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_স্তূপ_LEN(CONFIG_DYNAMIC_THREAD_স্তূপ_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

স্থির k_thread_স্তূপ_t *z_thread_স্তূপ_alloc_pool(size_t size, পূর্ণাঙ্ক flags)
{
	পূর্ণাঙ্ক rv;
	size_t offset;
	k_thread_স্তূপ_t *স্তূপ;

	যদি (size > CONFIG_DYNAMIC_THREAD_স্তূপ_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_স্তূপ_SIZE);
		ঘূৰাই রিক্ত;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	যদি (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ঘূৰাই রিক্ত;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	স্তূপ = (k_thread_স্তূপ_t *)&dynamic_স্তূপ[offset];

	ঘূৰাই স্তূপ;
}

স্থির k_thread_স্তূপ_t *z_thread_স্তূপ_alloc_dyn(size_t size, পূর্ণাঙ্ক flags)
{
	size_t স্তূপ_len;

	যদি ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		ঘূৰাই k_object_alloc_size(K_OBJ_THREAD_স্তূপ_ELEMENT, size);
#অন্যথা
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ঘূৰাই রিক্ত;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	স্তূপ_len = K_KERNEL_স্তূপ_LEN(size);
	যদি (স্তূপ_len < size) {
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই z_thread_aligned_alloc(Z_KERNEL_স্তূপ_OBJ_ALIGN, স্তূপ_len);
}

k_thread_স্তূপ_t *z_impl_k_thread_স্তূপ_alloc(size_t size, পূর্ণাঙ্ক flags)
{
	k_thread_স্তূপ_t *স্তূপ = রিক্ত;

	যদি (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		স্তূপ = z_thread_স্তূপ_alloc_dyn(size, flags);
		যদি (স্তূপ == রিক্ত && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			স্তূপ = z_thread_স্তূপ_alloc_pool(size, flags);
		}
	} অন্যথা যদি (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		যদি (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			স্তূপ = z_thread_স্তূপ_alloc_pool(size, flags);
		}

		যদি ((স্তূপ == রিক্ত) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			স্তূপ = z_thread_স্তূপ_alloc_dyn(size, flags);
		}
	}

	ঘূৰাই স্তূপ;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ k_thread_স্তূপ_t *z_vrfy_k_thread_স্তূপ_alloc(size_t size, পূর্ণাঙ্ক flags)
{
	ঘূৰাই z_impl_k_thread_স্তূপ_alloc(size, flags);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

স্থির শূন্য dyn_cb(ধ্রুৱ সংরচনা k_thread *thread, শূন্য *user_data)
{
	সংরচনা dyn_cb_data *ধ্রুৱ data = (সংরচনা dyn_cb_data *)user_data;

	যদি (data->স্তূপ == (k_thread_স্তূপ_t *)thread->স্তূপ_info.start) {
		__ASSERT(data->tid == রিক্ত, "stack %p is associated with more than one thread!",
			 (শূন্য *)thread->স্তূপ_info.start);
		data->tid = (k_tid_t)thread;
	}
}

পূর্ণাঙ্ক z_impl_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	সংরচনা dyn_cb_data data = {.স্তূপ = স্তূপ};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	যদি (data.tid != রিক্ত) {
		যদি (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			ঘূৰাই -EBUSY;
		}
	}

	যদি (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		যদি (IS_ARRAY_ELEMENT(dynamic_স্তূপ, স্তূপ)) {
			যদি (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_স্তূপ, স্তূপ))) {
				LOG_ERR("stack %p is not allocated!", স্তূপ);
				ঘূৰাই -EINVAL;
			}

			ঘূৰাই 0;
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
		ঘূৰাই -EINVAL;
	}

	ঘূৰাই 0;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_thread_স্তূপ_free(k_thread_স্তূপ_t *স্তূপ)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(স্তূপ, K_OBJ_THREAD_স্তূপ_ELEMENT));

	ঘূৰাই z_impl_k_thread_স্তূপ_free(স্তূপ);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
