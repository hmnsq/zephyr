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

#إذا CONFIG_DYNAMIC_THREAD_POOL_ناپ > 0
#تعریف_کرو BA_ناپ CONFIG_DYNAMIC_THREAD_POOL_ناپ
#وإلا
#تعریف_کرو BA_ناپ 1
#جے_انت /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

بنية dyn_cb_انکڑے {
	k_tid_t tid;
	k_thread_ڈھیر_t *ڈھیر;
};

ساكن K_THREAD_ڈھیر_ARRAY_DEFINE(dynamic_ڈھیر, CONFIG_DYNAMIC_THREAD_POOL_ناپ,
				   K_THREAD_ڈھیر_LEN(CONFIG_DYNAMIC_THREAD_ڈھیر_ناپ));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_ناپ);

ساكن k_thread_ڈھیر_t *z_thread_ڈھیر_alloc_pool(size_t ناپ, صحيح flags)
{
	صحيح rv;
	size_t وتھ;
	k_thread_ڈھیر_t *ڈھیر;

	إذا (ناپ > CONFIG_DYNAMIC_THREAD_ڈھیر_ناپ) {
		LOG_DBG("stack size %zu is > pool stack size %zu", ناپ,
			(size_t)CONFIG_DYNAMIC_THREAD_ڈھیر_ناپ);
		أرجع عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &وتھ);
	إذا (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		أرجع عدم;
	}

	__ASSERT_NO_MSG(وتھ < CONFIG_DYNAMIC_THREAD_POOL_ناپ);

	ڈھیر = (k_thread_ڈھیر_t *)&dynamic_ڈھیر[وتھ];

	أرجع ڈھیر;
}

ساكن k_thread_ڈھیر_t *z_thread_ڈھیر_alloc_dyn(size_t ناپ, صحيح flags)
{
	size_t ڈھیر_len;

	إذا ((flags & K_USER) == K_USER) {
#جے_تعریف_شدہ CONFIG_DYNAMIC_OBJECTS
		أرجع k_object_alloc_ناپ(K_OBJ_THREAD_ڈھیر_ELEMENT, ناپ);
#وإلا
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		أرجع عدم;
#جے_انت /* CONFIG_DYNAMIC_OBJECTS */
	}

	ڈھیر_len = K_KERNEL_ڈھیر_LEN(ناپ);
	إذا (ڈھیر_len < ناپ) {
		أرجع عدم;
	}

	أرجع z_thread_aligned_alloc(Z_KERNEL_ڈھیر_OBJ_ALIGN, ڈھیر_len);
}

k_thread_ڈھیر_t *z_impl_k_thread_ڈھیر_alloc(size_t ناپ, صحيح flags)
{
	k_thread_ڈھیر_t *ڈھیر = عدم;

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ڈھیر = z_thread_ڈھیر_alloc_dyn(ناپ, flags);
		إذا (ڈھیر == عدم && CONFIG_DYNAMIC_THREAD_POOL_ناپ > 0) {
			ڈھیر = z_thread_ڈھیر_alloc_pool(ناپ, flags);
		}
	} وإلا إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		إذا (CONFIG_DYNAMIC_THREAD_POOL_ناپ > 0) {
			ڈھیر = z_thread_ڈھیر_alloc_pool(ناپ, flags);
		}

		إذا ((ڈھیر == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ڈھیر = z_thread_ڈھیر_alloc_dyn(ناپ, flags);
		}
	}

	أرجع ڈھیر;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن k_thread_ڈھیر_t *z_vrfy_k_thread_ڈھیر_alloc(size_t ناپ, صحيح flags)
{
	أرجع z_impl_k_thread_ڈھیر_alloc(ناپ, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_انکڑے)
{
	بنية dyn_cb_انکڑے *ثابت انکڑے = (بنية dyn_cb_انکڑے *)user_انکڑے;

	إذا (انکڑے->ڈھیر == (k_thread_ڈھیر_t *)thread->ڈھیر_info.شروع) {
		__ASSERT(انکڑے->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->ڈھیر_info.شروع);
		انکڑے->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	بنية dyn_cb_انکڑے انکڑے = {.ڈھیر = ڈھیر};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &انکڑے);

	إذا (انکڑے.tid != عدم) {
		إذا (!z_is_thread_state_سیٹ(انکڑے.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", انکڑے.tid);
			أرجع -EBUSY;
		}
	}

	إذا (CONFIG_DYNAMIC_THREAD_POOL_ناپ > 0) {
		إذا (IS_ARRAY_ELEMENT(dynamic_ڈھیر, ڈھیر)) {
			إذا (sys_bitarray_free(&dynamic_ba, 1, ARRAY_اشاریہ(dynamic_ڈھیر, ڈھیر))) {
				LOG_ERR("stack %p is not allocated!", ڈھیر);
				أرجع -EINVAL;
			}

			أرجع 0;
		}
	}

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#جے_تعریف_شدہ CONFIG_USERSPACE
		إذا (k_object_لبھو(ڈھیر)) {
			k_object_free(ڈھیر);
		} وإلا {
			k_free(ڈھیر);
		}
#وإلا
		k_free(ڈھیر);
#جے_انت /* CONFIG_USERSPACE */
	} وإلا {
		LOG_DBG("Invalid stack %p", ڈھیر);
		أرجع -EINVAL;
	}

	أرجع 0;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_ڈھیر_free(k_thread_ڈھیر_t *ڈھیر)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ڈھیر, K_OBJ_THREAD_ڈھیر_ELEMENT));

	أرجع z_impl_k_thread_ڈھیر_free(ڈھیر);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */
