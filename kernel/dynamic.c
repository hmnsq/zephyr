/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن "kernel_internal.h"

#ضمن <zephyr/kernel.h>
#ضمن <ksched.h>
#ضمن <zephyr/kernel/thread_stack.h>
#ضمن <zephyr/logging/log.h>
#ضمن <zephyr/sys/bitarray.h>
#ضمن <zephyr/sys/kobject.h>
#ضمن <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#إذا CONFIG_DYNAMIC_THREAD_POOL_اندازه > 0
#تعریف_کړه BA_اندازه CONFIG_DYNAMIC_THREAD_POOL_اندازه
#وإلا
#تعریف_کړه BA_اندازه 1
#د_که_پای /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

بنية dyn_cb_معلومات {
	k_tid_t tid;
	k_thread_ډېری_t *ډېری;
};

ساكن K_THREAD_ډېری_ARRAY_DEFINE(dynamic_ډېری, CONFIG_DYNAMIC_THREAD_POOL_اندازه,
				   K_THREAD_ډېری_LEN(CONFIG_DYNAMIC_THREAD_ډېری_اندازه));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_اندازه);

ساكن k_thread_ډېری_t *z_thread_ډېری_alloc_pool(size_t اندازه, صحيح flags)
{
	صحيح rv;
	size_t دننه_واټن;
	k_thread_ډېری_t *ډېری;

	إذا (اندازه > CONFIG_DYNAMIC_THREAD_ډېری_اندازه) {
		LOG_DBG("stack size %zu is > pool stack size %zu", اندازه,
			(size_t)CONFIG_DYNAMIC_THREAD_ډېری_اندازه);
		أرجع عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &دننه_واټن);
	إذا (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		أرجع عدم;
	}

	__ASSERT_NO_MSG(دننه_واټن < CONFIG_DYNAMIC_THREAD_POOL_اندازه);

	ډېری = (k_thread_ډېری_t *)&dynamic_ډېری[دننه_واټن];

	أرجع ډېری;
}

ساكن k_thread_ډېری_t *z_thread_ډېری_alloc_dyn(size_t اندازه, صحيح flags)
{
	size_t ډېری_len;

	إذا ((flags & K_USER) == K_USER) {
#که_تعریف_شوی CONFIG_DYNAMIC_OBJECTS
		أرجع k_object_alloc_اندازه(K_OBJ_THREAD_ډېری_ELEMENT, اندازه);
#وإلا
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		أرجع عدم;
#د_که_پای /* CONFIG_DYNAMIC_OBJECTS */
	}

	ډېری_len = K_KERNEL_ډېری_LEN(اندازه);
	إذا (ډېری_len < اندازه) {
		أرجع عدم;
	}

	أرجع z_thread_aligned_alloc(Z_KERNEL_ډېری_OBJ_ALIGN, ډېری_len);
}

k_thread_ډېری_t *z_impl_k_thread_ډېری_alloc(size_t اندازه, صحيح flags)
{
	k_thread_ډېری_t *ډېری = عدم;

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ډېری = z_thread_ډېری_alloc_dyn(اندازه, flags);
		إذا (ډېری == عدم && CONFIG_DYNAMIC_THREAD_POOL_اندازه > 0) {
			ډېری = z_thread_ډېری_alloc_pool(اندازه, flags);
		}
	} وإلا إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		إذا (CONFIG_DYNAMIC_THREAD_POOL_اندازه > 0) {
			ډېری = z_thread_ډېری_alloc_pool(اندازه, flags);
		}

		إذا ((ډېری == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ډېری = z_thread_ډېری_alloc_dyn(اندازه, flags);
		}
	}

	أرجع ډېری;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن k_thread_ډېری_t *z_vrfy_k_thread_ډېری_alloc(size_t اندازه, صحيح flags)
{
	أرجع z_impl_k_thread_ډېری_alloc(اندازه, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_معلومات)
{
	بنية dyn_cb_معلومات *ثابت معلومات = (بنية dyn_cb_معلومات *)user_معلومات;

	إذا (معلومات->ډېری == (k_thread_ډېری_t *)thread->ډېری_info.پیل) {
		__ASSERT(معلومات->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->ډېری_info.پیل);
		معلومات->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_ډېری_free(k_thread_ډېری_t *ډېری)
{
	بنية dyn_cb_معلومات معلومات = {.ډېری = ډېری};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &معلومات);

	إذا (معلومات.tid != عدم) {
		إذا (!z_is_thread_state_سېټ(معلومات.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", معلومات.tid);
			أرجع -EBUSY;
		}
	}

	إذا (CONFIG_DYNAMIC_THREAD_POOL_اندازه > 0) {
		إذا (IS_ARRAY_ELEMENT(dynamic_ډېری, ډېری)) {
			إذا (sys_bitarray_free(&dynamic_ba, 1, ARRAY_شاخص(dynamic_ډېری, ډېری))) {
				LOG_ERR("stack %p is not allocated!", ډېری);
				أرجع -EINVAL;
			}

			أرجع 0;
		}
	}

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#که_تعریف_شوی CONFIG_USERSPACE
		إذا (k_object_پیدا_کړه(ډېری)) {
			k_object_free(ډېری);
		} وإلا {
			k_free(ډېری);
		}
#وإلا
		k_free(ډېری);
#د_که_پای /* CONFIG_USERSPACE */
	} وإلا {
		LOG_DBG("Invalid stack %p", ډېری);
		أرجع -EINVAL;
	}

	أرجع 0;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ډېری_free(k_thread_ډېری_t *ډېری)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ډېری, K_OBJ_THREAD_ډېری_ELEMENT));

	أرجع z_impl_k_thread_ډېری_free(ډېری);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
