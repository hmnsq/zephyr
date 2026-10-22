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

#إذا CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#وإلا
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

بنية dyn_cb_data {
	k_tid_t tid;
	k_thread_مكدس_t *مكدس;
};

ساكن K_THREAD_مكدس_ARRAY_DEFINE(dynamic_مكدس, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_مكدس_LEN(CONFIG_DYNAMIC_THREAD_مكدس_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

ساكن k_thread_مكدس_t *z_thread_مكدس_alloc_pool(size_t size, صحيح flags)
{
	صحيح rv;
	size_t offset;
	k_thread_مكدس_t *مكدس;

	إذا (size > CONFIG_DYNAMIC_THREAD_مكدس_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_مكدس_SIZE);
		إرجاع عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	إذا (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		إرجاع عدم;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	مكدس = (k_thread_مكدس_t *)&dynamic_مكدس[offset];

	إرجاع مكدس;
}

ساكن k_thread_مكدس_t *z_thread_مكدس_alloc_dyn(size_t size, صحيح flags)
{
	size_t مكدس_len;

	إذا ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		إرجاع k_object_alloc_size(K_OBJ_THREAD_مكدس_ELEMENT, size);
#وإلا
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		إرجاع عدم;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	مكدس_len = K_KERNEL_مكدس_LEN(size);
	إذا (مكدس_len < size) {
		إرجاع عدم;
	}

	إرجاع z_thread_aligned_alloc(Z_KERNEL_مكدس_OBJ_ALIGN, مكدس_len);
}

k_thread_مكدس_t *z_impl_k_thread_مكدس_alloc(size_t size, صحيح flags)
{
	k_thread_مكدس_t *مكدس = عدم;

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		مكدس = z_thread_مكدس_alloc_dyn(size, flags);
		إذا (مكدس == عدم && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			مكدس = z_thread_مكدس_alloc_pool(size, flags);
		}
	} وإلا إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		إذا (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			مكدس = z_thread_مكدس_alloc_pool(size, flags);
		}

		إذا ((مكدس == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			مكدس = z_thread_مكدس_alloc_dyn(size, flags);
		}
	}

	إرجاع مكدس;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_thread_مكدس_t *z_vrfy_k_thread_مكدس_alloc(size_t size, صحيح flags)
{
	إرجاع z_impl_k_thread_مكدس_alloc(size, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_data)
{
	بنية dyn_cb_data *ثابت data = (بنية dyn_cb_data *)user_data;

	إذا (data->مكدس == (k_thread_مكدس_t *)thread->مكدس_info.start) {
		__ASSERT(data->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->مكدس_info.start);
		data->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_مكدس_free(k_thread_مكدس_t *مكدس)
{
	بنية dyn_cb_data data = {.مكدس = مكدس};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	إذا (data.tid != عدم) {
		إذا (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			إرجاع -EBUSY;
		}
	}

	إذا (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		إذا (IS_ARRAY_ELEMENT(dynamic_مكدس, مكدس)) {
			إذا (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_مكدس, مكدس))) {
				LOG_ERR("stack %p is not allocated!", مكدس);
				إرجاع -EINVAL;
			}

			إرجاع 0;
		}
	}

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		إذا (k_object_find(مكدس)) {
			k_object_free(مكدس);
		} وإلا {
			k_free(مكدس);
		}
#وإلا
		k_free(مكدس);
#endif /* CONFIG_USERSPACE */
	} وإلا {
		LOG_DBG("Invalid stack %p", مكدس);
		إرجاع -EINVAL;
	}

	إرجاع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_مكدس_free(k_thread_مكدس_t *مكدس)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(مكدس, K_OBJ_THREAD_مكدس_ELEMENT));

	إرجاع z_impl_k_thread_مكدس_free(مكدس);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
