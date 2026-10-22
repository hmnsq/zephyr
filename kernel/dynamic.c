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

#إذا CONFIG_DYNAMIC_THREAD_POOL_سائيز > 0
#وصف_ڪريو BA_سائيز CONFIG_DYNAMIC_THREAD_POOL_سائيز
#وإلا
#وصف_ڪريو BA_سائيز 1
#جيڪڏهن_پورو /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

بنية dyn_cb_معلومات {
	k_tid_t tid;
	k_thread_ڍير_t *ڍير;
};

ساكن K_THREAD_ڍير_ARRAY_DEFINE(dynamic_ڍير, CONFIG_DYNAMIC_THREAD_POOL_سائيز,
				   K_THREAD_ڍير_LEN(CONFIG_DYNAMIC_THREAD_ڍير_سائيز));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_سائيز);

ساكن k_thread_ڍير_t *z_thread_ڍير_alloc_pool(size_t سائيز, صحيح flags)
{
	صحيح rv;
	size_t وٿي;
	k_thread_ڍير_t *ڍير;

	إذا (سائيز > CONFIG_DYNAMIC_THREAD_ڍير_سائيز) {
		LOG_DBG("stack size %zu is > pool stack size %zu", سائيز,
			(size_t)CONFIG_DYNAMIC_THREAD_ڍير_سائيز);
		أرجع عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &وٿي);
	إذا (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		أرجع عدم;
	}

	__ASSERT_NO_MSG(وٿي < CONFIG_DYNAMIC_THREAD_POOL_سائيز);

	ڍير = (k_thread_ڍير_t *)&dynamic_ڍير[وٿي];

	أرجع ڍير;
}

ساكن k_thread_ڍير_t *z_thread_ڍير_alloc_dyn(size_t سائيز, صحيح flags)
{
	size_t ڍير_len;

	إذا ((flags & K_USER) == K_USER) {
#جيڪڏهن_وصف_ٿيل CONFIG_DYNAMIC_OBJECTS
		أرجع k_object_alloc_سائيز(K_OBJ_THREAD_ڍير_ELEMENT, سائيز);
#وإلا
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		أرجع عدم;
#جيڪڏهن_پورو /* CONFIG_DYNAMIC_OBJECTS */
	}

	ڍير_len = K_KERNEL_ڍير_LEN(سائيز);
	إذا (ڍير_len < سائيز) {
		أرجع عدم;
	}

	أرجع z_thread_aligned_alloc(Z_KERNEL_ڍير_OBJ_ALIGN, ڍير_len);
}

k_thread_ڍير_t *z_impl_k_thread_ڍير_alloc(size_t سائيز, صحيح flags)
{
	k_thread_ڍير_t *ڍير = عدم;

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ڍير = z_thread_ڍير_alloc_dyn(سائيز, flags);
		إذا (ڍير == عدم && CONFIG_DYNAMIC_THREAD_POOL_سائيز > 0) {
			ڍير = z_thread_ڍير_alloc_pool(سائيز, flags);
		}
	} وإلا إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		إذا (CONFIG_DYNAMIC_THREAD_POOL_سائيز > 0) {
			ڍير = z_thread_ڍير_alloc_pool(سائيز, flags);
		}

		إذا ((ڍير == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ڍير = z_thread_ڍير_alloc_dyn(سائيز, flags);
		}
	}

	أرجع ڍير;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن k_thread_ڍير_t *z_vrfy_k_thread_ڍير_alloc(size_t سائيز, صحيح flags)
{
	أرجع z_impl_k_thread_ڍير_alloc(سائيز, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_معلومات)
{
	بنية dyn_cb_معلومات *ثابت معلومات = (بنية dyn_cb_معلومات *)user_معلومات;

	إذا (معلومات->ڍير == (k_thread_ڍير_t *)thread->ڍير_info.شروعات) {
		__ASSERT(معلومات->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->ڍير_info.شروعات);
		معلومات->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_ڍير_free(k_thread_ڍير_t *ڍير)
{
	بنية dyn_cb_معلومات معلومات = {.ڍير = ڍير};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &معلومات);

	إذا (معلومات.tid != عدم) {
		إذا (!z_is_thread_state_سيٽ(معلومات.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", معلومات.tid);
			أرجع -EBUSY;
		}
	}

	إذا (CONFIG_DYNAMIC_THREAD_POOL_سائيز > 0) {
		إذا (IS_ARRAY_ELEMENT(dynamic_ڍير, ڍير)) {
			إذا (sys_bitarray_free(&dynamic_ba, 1, ARRAY_اشاريو(dynamic_ڍير, ڍير))) {
				LOG_ERR("stack %p is not allocated!", ڍير);
				أرجع -EINVAL;
			}

			أرجع 0;
		}
	}

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
		إذا (k_object_ڳوليو(ڍير)) {
			k_object_free(ڍير);
		} وإلا {
			k_free(ڍير);
		}
#وإلا
		k_free(ڍير);
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
	} وإلا {
		LOG_DBG("Invalid stack %p", ڍير);
		أرجع -EINVAL;
	}

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ڍير_free(k_thread_ڍير_t *ڍير)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ڍير, K_OBJ_THREAD_ڍير_ELEMENT));

	أرجع z_impl_k_thread_ڍير_free(ڍير);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
