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

#اگر CONFIG_DYNAMIC_THREAD_POOL_سائز > 0
#تعریف_کرو BA_سائز CONFIG_DYNAMIC_THREAD_POOL_سائز
#ورنہ
#تعریف_کرو BA_سائز 1
#اگر_ختم /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

ساخت dyn_cb_ڈیٹا {
	k_tid_t tid;
	k_thread_ڈھیر_t *ڈھیر;
};

ساکن K_THREAD_ڈھیر_ARRAY_DEFINE(dynamic_ڈھیر, CONFIG_DYNAMIC_THREAD_POOL_سائز,
				   K_THREAD_ڈھیر_LEN(CONFIG_DYNAMIC_THREAD_ڈھیر_سائز));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_سائز);

ساکن k_thread_ڈھیر_t *z_thread_ڈھیر_alloc_pool(size_t سائز, صحیح flags)
{
	صحیح rv;
	size_t انحراف;
	k_thread_ڈھیر_t *ڈھیر;

	اگر (سائز > CONFIG_DYNAMIC_THREAD_ڈھیر_سائز) {
		LOG_DBG("stack size %zu is > pool stack size %zu", سائز,
			(size_t)CONFIG_DYNAMIC_THREAD_ڈھیر_سائز);
		واپس تہی;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &انحراف);
	اگر (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		واپس تہی;
	}

	__ASSERT_NO_MSG(انحراف < CONFIG_DYNAMIC_THREAD_POOL_سائز);

	ڈھیر = (k_thread_ڈھیر_t *)&dynamic_ڈھیر[انحراف];

	واپس ڈھیر;
}

ساکن k_thread_ڈھیر_t *z_thread_ڈھیر_alloc_dyn(size_t سائز, صحیح flags)
{
	size_t ڈھیر_len;

	اگر ((flags & K_USER) == K_USER) {
#اگر_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
		واپس k_object_alloc_سائز(K_OBJ_THREAD_ڈھیر_ELEMENT, سائز);
#ورنہ
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		واپس تہی;
#اگر_ختم /* CONFIG_DYNAMIC_OBJECTS */
	}

	ڈھیر_len = K_KERNEL_ڈھیر_LEN(سائز);
	اگر (ڈھیر_len < سائز) {
		واپس تہی;
	}

	واپس z_thread_aligned_alloc(Z_KERNEL_ڈھیر_OBJ_ALIGN, ڈھیر_len);
}

k_thread_ڈھیر_t *z_impl_k_thread_ڈھیر_alloc(size_t سائز, صحیح flags)
{
	k_thread_ڈھیر_t *ڈھیر = تہی;

	اگر (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ڈھیر = z_thread_ڈھیر_alloc_dyn(سائز, flags);
		اگر (ڈھیر == تہی && CONFIG_DYNAMIC_THREAD_POOL_سائز > 0) {
			ڈھیر = z_thread_ڈھیر_alloc_pool(سائز, flags);
		}
	} ورنہ اگر (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		اگر (CONFIG_DYNAMIC_THREAD_POOL_سائز > 0) {
			ڈھیر = z_thread_ڈھیر_alloc_pool(سائز, flags);
		}

		اگر ((ڈھیر == تہی) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ڈھیر = z_thread_ڈھیر_alloc_dyn(سائز, flags);
		}
	}

	واپس ڈھیر;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی k_thread_ڈھیر_t *z_vrfy_k_thread_ڈھیر_alloc(size_t سائز, صحیح flags)
{
	واپس z_impl_k_thread_ڈھیر_alloc(سائز, flags);
}
#شامل <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

ساکن خالی dyn_cb(ثابت ساخت k_thread *thread, خالی *user_ڈیٹا)
{
	ساخت dyn_cb_ڈیٹا *ثابت ڈیٹا = (ساخت dyn_cb_ڈیٹا *)user_ڈیٹا;

	اگر (ڈیٹا->ڈھیر == (k_thread_ڈھیر_t *)thread->ڈھیر_info.آغاز) {
		__ASSERT(ڈیٹا->tid == تہی, "stack %p is associated with more than one thread!",
			 (خالی *)thread->ڈھیر_info.آغاز);
		ڈیٹا->tid = (k_tid_t)thread;
	}
}

صحیح z_impl_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	ساخت dyn_cb_ڈیٹا ڈیٹا = {.ڈھیر = ڈھیر};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &ڈیٹا);

	اگر (ڈیٹا.tid != تہی) {
		اگر (!z_is_thread_state_سیٹ(ڈیٹا.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", ڈیٹا.tid);
			واپس -EBUSY;
		}
	}

	اگر (CONFIG_DYNAMIC_THREAD_POOL_سائز > 0) {
		اگر (IS_ARRAY_ELEMENT(dynamic_ڈھیر, ڈھیر)) {
			اگر (sys_bitarray_free(&dynamic_ba, 1, ARRAY_اشاریہ(dynamic_ڈھیر, ڈھیر))) {
				LOG_ERR("stack %p is not allocated!", ڈھیر);
				واپس -EINVAL;
			}

			واپس 0;
		}
	}

	اگر (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#اگر_تعریف_شدہ CONFIG_USERSPACE
		اگر (k_object_ڈھونڈو(ڈھیر)) {
			k_object_free(ڈھیر);
		} ورنہ {
			k_free(ڈھیر);
		}
#ورنہ
		k_free(ڈھیر);
#اگر_ختم /* CONFIG_USERSPACE */
	} ورنہ {
		LOG_DBG("Invalid stack %p", ڈھیر);
		واپس -EINVAL;
	}

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ڈھیر, K_OBJ_THREAD_ڈھیر_ELEMENT));

	واپس z_impl_k_thread_ڈھیر_free(ڈھیر);
}
#شامل <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
