/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ "kernel_internal.h"

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel/thread_stack.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/bitarray.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/kobject.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ଯଦି CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#ଅନ୍ୟଥା
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

ସଂରଚନା dyn_cb_data {
	k_tid_t tid;
	k_thread_ଗଦା_t *ଗଦା;
};

ସ୍ଥିର K_THREAD_ଗଦା_ARRAY_DEFINE(dynamic_ଗଦା, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_ଗଦା_LEN(CONFIG_DYNAMIC_THREAD_ଗଦା_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

ସ୍ଥିର k_thread_ଗଦା_t *z_thread_ଗଦା_alloc_pool(size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rv;
	size_t offset;
	k_thread_ଗଦା_t *ଗଦା;

	ଯଦି (size > CONFIG_DYNAMIC_THREAD_ଗଦା_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_ଗଦା_SIZE);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	ଯଦି (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	ଗଦା = (k_thread_ଗଦା_t *)&dynamic_ଗଦା[offset];

	ଫେରନ୍ତୁ ଗଦା;
}

ସ୍ଥିର k_thread_ଗଦା_t *z_thread_ଗଦା_alloc_dyn(size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	size_t ଗଦା_len;

	ଯଦି ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		ଫେରନ୍ତୁ k_object_alloc_size(K_OBJ_THREAD_ଗଦା_ELEMENT, size);
#ଅନ୍ୟଥା
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ଫେରନ୍ତୁ ଖାଲି;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	ଗଦା_len = K_KERNEL_ଗଦା_LEN(size);
	ଯଦି (ଗଦା_len < size) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ z_thread_aligned_alloc(Z_KERNEL_ଗଦା_OBJ_ALIGN, ଗଦା_len);
}

k_thread_ଗଦା_t *z_impl_k_thread_ଗଦା_alloc(size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	k_thread_ଗଦା_t *ଗଦା = ଖାଲି;

	ଯଦି (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ଗଦା = z_thread_ଗଦା_alloc_dyn(size, flags);
		ଯଦି (ଗଦା == ଖାଲି && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			ଗଦା = z_thread_ଗଦା_alloc_pool(size, flags);
		}
	} ଅନ୍ୟଥା ଯଦି (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		ଯଦି (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			ଗଦା = z_thread_ଗଦା_alloc_pool(size, flags);
		}

		ଯଦି ((ଗଦା == ଖାଲି) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ଗଦା = z_thread_ଗଦା_alloc_dyn(size, flags);
		}
	}

	ଫେରନ୍ତୁ ଗଦା;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_thread_ଗଦା_t *z_vrfy_k_thread_ଗଦା_alloc(size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ଫେରନ୍ତୁ z_impl_k_thread_ଗଦା_alloc(size, flags);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ସ୍ଥିର ଶୂନ୍ୟ dyn_cb(ଅଚଳ ସଂରଚନା k_thread *thread, ଶୂନ୍ୟ *user_data)
{
	ସଂରଚନା dyn_cb_data *ଅଚଳ data = (ସଂରଚନା dyn_cb_data *)user_data;

	ଯଦି (data->ଗଦା == (k_thread_ଗଦା_t *)thread->ଗଦା_info.start) {
		__ASSERT(data->tid == ଖାଲି, "stack %p is associated with more than one thread!",
			 (ଶୂନ୍ୟ *)thread->ଗଦା_info.start);
		data->tid = (k_tid_t)thread;
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_ଗଦା_free(k_thread_ଗଦା_t *ଗଦା)
{
	ସଂରଚନା dyn_cb_data data = {.ଗଦା = ଗଦା};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	ଯଦି (data.tid != ଖାଲି) {
		ଯଦି (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			ଫେରନ୍ତୁ -EBUSY;
		}
	}

	ଯଦି (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		ଯଦି (IS_ARRAY_ELEMENT(dynamic_ଗଦା, ଗଦା)) {
			ଯଦି (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_ଗଦା, ଗଦା))) {
				LOG_ERR("stack %p is not allocated!", ଗଦା);
				ଫେରନ୍ତୁ -EINVAL;
			}

			ଫେରନ୍ତୁ 0;
		}
	}

	ଯଦି (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		ଯଦି (k_object_find(ଗଦା)) {
			k_object_free(ଗଦା);
		} ଅନ୍ୟଥା {
			k_free(ଗଦା);
		}
#ଅନ୍ୟଥା
		k_free(ଗଦା);
#endif /* CONFIG_USERSPACE */
	} ଅନ୍ୟଥା {
		LOG_DBG("Invalid stack %p", ଗଦା);
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_ଗଦା_free(k_thread_ଗଦା_t *ଗଦା)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ଗଦା, K_OBJ_THREAD_ଗଦା_ELEMENT));

	ଫେରନ୍ତୁ z_impl_k_thread_ଗଦା_free(ଗଦା);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
