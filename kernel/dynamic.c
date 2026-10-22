/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ "kernel_internal.h"

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <zephyr/kernel/thread_stack.h>
#அந்தர்பாவ <zephyr/logging/log.h>
#அந்தர்பாவ <zephyr/sys/bitarray.h>
#அந்தர்பாவ <zephyr/sys/kobject.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#எனில் CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#இல்லையெனில்
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

கட்டமைப்பு dyn_cb_data {
	k_tid_t tid;
	k_thread_அடுக்கு_t *அடுக்கு;
};

நிலையான K_THREAD_அடுக்கு_ARRAY_DEFINE(dynamic_அடுக்கு, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_அடுக்கு_LEN(CONFIG_DYNAMIC_THREAD_அடுக்கு_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

நிலையான k_thread_அடுக்கு_t *z_thread_அடுக்கு_alloc_pool(size_t size, முழு flags)
{
	முழு rv;
	size_t offset;
	k_thread_அடுக்கு_t *அடுக்கு;

	எனில் (size > CONFIG_DYNAMIC_THREAD_அடுக்கு_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_அடுக்கு_SIZE);
		திரும்பு வெற்று;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	எனில் (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		திரும்பு வெற்று;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	அடுக்கு = (k_thread_அடுக்கு_t *)&dynamic_அடுக்கு[offset];

	திரும்பு அடுக்கு;
}

நிலையான k_thread_அடுக்கு_t *z_thread_அடுக்கு_alloc_dyn(size_t size, முழு flags)
{
	size_t அடுக்கு_len;

	எனில் ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		திரும்பு k_object_alloc_size(K_OBJ_THREAD_அடுக்கு_ELEMENT, size);
#இல்லையெனில்
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		திரும்பு வெற்று;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	அடுக்கு_len = K_KERNEL_அடுக்கு_LEN(size);
	எனில் (அடுக்கு_len < size) {
		திரும்பு வெற்று;
	}

	திரும்பு z_thread_aligned_alloc(Z_KERNEL_அடுக்கு_OBJ_ALIGN, அடுக்கு_len);
}

k_thread_அடுக்கு_t *z_impl_k_thread_அடுக்கு_alloc(size_t size, முழு flags)
{
	k_thread_அடுக்கு_t *அடுக்கு = வெற்று;

	எனில் (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		அடுக்கு = z_thread_அடுக்கு_alloc_dyn(size, flags);
		எனில் (அடுக்கு == வெற்று && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			அடுக்கு = z_thread_அடுக்கு_alloc_pool(size, flags);
		}
	} இல்லையெனில் எனில் (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		எனில் (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			அடுக்கு = z_thread_அடுக்கு_alloc_pool(size, flags);
		}

		எனில் ((அடுக்கு == வெற்று) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			அடுக்கு = z_thread_அடுக்கு_alloc_dyn(size, flags);
		}
	}

	திரும்பு அடுக்கு;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி k_thread_அடுக்கு_t *z_vrfy_k_thread_அடுக்கு_alloc(size_t size, முழு flags)
{
	திரும்பு z_impl_k_thread_அடுக்கு_alloc(size, flags);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

நிலையான வெறுமை dyn_cb(மாறிலி கட்டமைப்பு k_thread *thread, வெறுமை *user_data)
{
	கட்டமைப்பு dyn_cb_data *மாறிலி data = (கட்டமைப்பு dyn_cb_data *)user_data;

	எனில் (data->அடுக்கு == (k_thread_அடுக்கு_t *)thread->அடுக்கு_info.start) {
		__ASSERT(data->tid == வெற்று, "stack %p is associated with more than one thread!",
			 (வெறுமை *)thread->அடுக்கு_info.start);
		data->tid = (k_tid_t)thread;
	}
}

முழு z_impl_k_thread_அடுக்கு_free(k_thread_அடுக்கு_t *அடுக்கு)
{
	கட்டமைப்பு dyn_cb_data data = {.அடுக்கு = அடுக்கு};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	எனில் (data.tid != வெற்று) {
		எனில் (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			திரும்பு -EBUSY;
		}
	}

	எனில் (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		எனில் (IS_ARRAY_ELEMENT(dynamic_அடுக்கு, அடுக்கு)) {
			எனில் (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_அடுக்கு, அடுக்கு))) {
				LOG_ERR("stack %p is not allocated!", அடுக்கு);
				திரும்பு -EINVAL;
			}

			திரும்பு 0;
		}
	}

	எனில் (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		எனில் (k_object_find(அடுக்கு)) {
			k_object_free(அடுக்கு);
		} இல்லையெனில் {
			k_free(அடுக்கு);
		}
#இல்லையெனில்
		k_free(அடுக்கு);
#endif /* CONFIG_USERSPACE */
	} இல்லையெனில் {
		LOG_DBG("Invalid stack %p", அடுக்கு);
		திரும்பு -EINVAL;
	}

	திரும்பு 0;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_thread_அடுக்கு_free(k_thread_அடுக்கு_t *அடுக்கு)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(அடுக்கு, K_OBJ_THREAD_அடுக்கு_ELEMENT));

	திரும்பு z_impl_k_thread_அடுக்கு_free(அடுக்கு);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
