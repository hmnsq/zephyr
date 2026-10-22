/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل "kernel_internal.h"

#شامل <zephyr/kernel.h>
#شامل <ksched.h>
#شامل <zephyr/kernel/thread_stack.h>
#شامل <zephyr/logging/log.h>
#شامل <zephyr/sys/bitarray.h>
#شامل <zephyr/sys/kobject.h>
#شامل <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#اگر CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#وگرنه
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

ساختار dyn_cb_data {
	k_tid_t tid;
	k_thread_پشته_t *پشته;
};

ایستا K_THREAD_پشته_ARRAY_DEFINE(dynamic_پشته, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_پشته_LEN(CONFIG_DYNAMIC_THREAD_پشته_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

ایستا k_thread_پشته_t *z_thread_پشته_alloc_pool(size_t size, صحیح flags)
{
	صحیح rv;
	size_t offset;
	k_thread_پشته_t *پشته;

	اگر (size > CONFIG_DYNAMIC_THREAD_پشته_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_پشته_SIZE);
		بازگشت پوچ;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	اگر (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		بازگشت پوچ;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	پشته = (k_thread_پشته_t *)&dynamic_پشته[offset];

	بازگشت پشته;
}

ایستا k_thread_پشته_t *z_thread_پشته_alloc_dyn(size_t size, صحیح flags)
{
	size_t پشته_len;

	اگر ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		بازگشت k_object_alloc_size(K_OBJ_THREAD_پشته_ELEMENT, size);
#وگرنه
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		بازگشت پوچ;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	پشته_len = K_KERNEL_پشته_LEN(size);
	اگر (پشته_len < size) {
		بازگشت پوچ;
	}

	بازگشت z_thread_aligned_alloc(Z_KERNEL_پشته_OBJ_ALIGN, پشته_len);
}

k_thread_پشته_t *z_impl_k_thread_پشته_alloc(size_t size, صحیح flags)
{
	k_thread_پشته_t *پشته = پوچ;

	اگر (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		پشته = z_thread_پشته_alloc_dyn(size, flags);
		اگر (پشته == پوچ && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			پشته = z_thread_پشته_alloc_pool(size, flags);
		}
	} وگرنه اگر (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		اگر (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			پشته = z_thread_پشته_alloc_pool(size, flags);
		}

		اگر ((پشته == پوچ) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			پشته = z_thread_پشته_alloc_dyn(size, flags);
		}
	}

	بازگشت پشته;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی k_thread_پشته_t *z_vrfy_k_thread_پشته_alloc(size_t size, صحیح flags)
{
	بازگشت z_impl_k_thread_پشته_alloc(size, flags);
}
#شامل <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ایستا تهی dyn_cb(ثابت ساختار k_thread *thread, تهی *user_data)
{
	ساختار dyn_cb_data *ثابت data = (ساختار dyn_cb_data *)user_data;

	اگر (data->پشته == (k_thread_پشته_t *)thread->پشته_info.start) {
		__ASSERT(data->tid == پوچ, "stack %p is associated with more than one thread!",
			 (تهی *)thread->پشته_info.start);
		data->tid = (k_tid_t)thread;
	}
}

صحیح z_impl_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	ساختار dyn_cb_data data = {.پشته = پشته};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	اگر (data.tid != پوچ) {
		اگر (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			بازگشت -EBUSY;
		}
	}

	اگر (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		اگر (IS_ARRAY_ELEMENT(dynamic_پشته, پشته)) {
			اگر (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_پشته, پشته))) {
				LOG_ERR("stack %p is not allocated!", پشته);
				بازگشت -EINVAL;
			}

			بازگشت 0;
		}
	}

	اگر (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		اگر (k_object_find(پشته)) {
			k_object_free(پشته);
		} وگرنه {
			k_free(پشته);
		}
#وگرنه
		k_free(پشته);
#endif /* CONFIG_USERSPACE */
	} وگرنه {
		LOG_DBG("Invalid stack %p", پشته);
		بازگشت -EINVAL;
	}

	بازگشت 0;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(پشته, K_OBJ_THREAD_پشته_ELEMENT));

	بازگشت z_impl_k_thread_پشته_free(پشته);
}
#شامل <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
