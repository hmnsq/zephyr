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
#تعریف_کن BA_اندازه CONFIG_DYNAMIC_THREAD_POOL_اندازه
#وإلا
#تعریف_کن BA_اندازه 1
#پایان_اگر /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

بنية dyn_cb_داده {
	k_tid_t tid;
	k_thread_پشته_t *پشته;
};

ساكن K_THREAD_پشته_ARRAY_DEFINE(dynamic_پشته, CONFIG_DYNAMIC_THREAD_POOL_اندازه,
				   K_THREAD_پشته_LEN(CONFIG_DYNAMIC_THREAD_پشته_اندازه));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_اندازه);

ساكن k_thread_پشته_t *z_thread_پشته_alloc_pool(size_t اندازه, صحيح flags)
{
	صحيح rv;
	size_t جابه_جایی;
	k_thread_پشته_t *پشته;

	إذا (اندازه > CONFIG_DYNAMIC_THREAD_پشته_اندازه) {
		LOG_DBG("stack size %zu is > pool stack size %zu", اندازه,
			(size_t)CONFIG_DYNAMIC_THREAD_پشته_اندازه);
		أرجع عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &جابه_جایی);
	إذا (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		أرجع عدم;
	}

	__ASSERT_NO_MSG(جابه_جایی < CONFIG_DYNAMIC_THREAD_POOL_اندازه);

	پشته = (k_thread_پشته_t *)&dynamic_پشته[جابه_جایی];

	أرجع پشته;
}

ساكن k_thread_پشته_t *z_thread_پشته_alloc_dyn(size_t اندازه, صحيح flags)
{
	size_t پشته_len;

	إذا ((flags & K_USER) == K_USER) {
#اگر_تعریف_شده CONFIG_DYNAMIC_OBJECTS
		أرجع k_object_alloc_اندازه(K_OBJ_THREAD_پشته_ELEMENT, اندازه);
#وإلا
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		أرجع عدم;
#پایان_اگر /* CONFIG_DYNAMIC_OBJECTS */
	}

	پشته_len = K_KERNEL_پشته_LEN(اندازه);
	إذا (پشته_len < اندازه) {
		أرجع عدم;
	}

	أرجع z_thread_aligned_alloc(Z_KERNEL_پشته_OBJ_ALIGN, پشته_len);
}

k_thread_پشته_t *z_impl_k_thread_پشته_alloc(size_t اندازه, صحيح flags)
{
	k_thread_پشته_t *پشته = عدم;

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		پشته = z_thread_پشته_alloc_dyn(اندازه, flags);
		إذا (پشته == عدم && CONFIG_DYNAMIC_THREAD_POOL_اندازه > 0) {
			پشته = z_thread_پشته_alloc_pool(اندازه, flags);
		}
	} وإلا إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		إذا (CONFIG_DYNAMIC_THREAD_POOL_اندازه > 0) {
			پشته = z_thread_پشته_alloc_pool(اندازه, flags);
		}

		إذا ((پشته == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			پشته = z_thread_پشته_alloc_dyn(اندازه, flags);
		}
	}

	أرجع پشته;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن k_thread_پشته_t *z_vrfy_k_thread_پشته_alloc(size_t اندازه, صحيح flags)
{
	أرجع z_impl_k_thread_پشته_alloc(اندازه, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_داده)
{
	بنية dyn_cb_داده *ثابت داده = (بنية dyn_cb_داده *)user_داده;

	إذا (داده->پشته == (k_thread_پشته_t *)thread->پشته_info.شروع) {
		__ASSERT(داده->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->پشته_info.شروع);
		داده->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	بنية dyn_cb_داده داده = {.پشته = پشته};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &داده);

	إذا (داده.tid != عدم) {
		إذا (!z_is_thread_state_مجموعه(داده.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", داده.tid);
			أرجع -EBUSY;
		}
	}

	إذا (CONFIG_DYNAMIC_THREAD_POOL_اندازه > 0) {
		إذا (IS_ARRAY_ELEMENT(dynamic_پشته, پشته)) {
			إذا (sys_bitarray_free(&dynamic_ba, 1, ARRAY_شاخص(dynamic_پشته, پشته))) {
				LOG_ERR("stack %p is not allocated!", پشته);
				أرجع -EINVAL;
			}

			أرجع 0;
		}
	}

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#اگر_تعریف_شده CONFIG_USERSPACE
		إذا (k_object_پیدا_کن(پشته)) {
			k_object_free(پشته);
		} وإلا {
			k_free(پشته);
		}
#وإلا
		k_free(پشته);
#پایان_اگر /* CONFIG_USERSPACE */
	} وإلا {
		LOG_DBG("Invalid stack %p", پشته);
		أرجع -EINVAL;
	}

	أرجع 0;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_پشته_free(k_thread_پشته_t *پشته)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(پشته, K_OBJ_THREAD_پشته_ELEMENT));

	أرجع z_impl_k_thread_پشته_free(پشته);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
