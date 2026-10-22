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
	k_thread_کەڵەکە_t *کەڵەکە;
};

ساكن K_THREAD_کەڵەکە_ARRAY_DEFINE(dynamic_کەڵەکە, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_کەڵەکە_LEN(CONFIG_DYNAMIC_THREAD_کەڵەکە_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

ساكن k_thread_کەڵەکە_t *z_thread_کەڵەکە_alloc_pool(size_t size, صحيح flags)
{
	صحيح rv;
	size_t offset;
	k_thread_کەڵەکە_t *کەڵەکە;

	إذا (size > CONFIG_DYNAMIC_THREAD_کەڵەکە_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_کەڵەکە_SIZE);
		أرجع عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	إذا (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		أرجع عدم;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	کەڵەکە = (k_thread_کەڵەکە_t *)&dynamic_کەڵەکە[offset];

	أرجع کەڵەکە;
}

ساكن k_thread_کەڵەکە_t *z_thread_کەڵەکە_alloc_dyn(size_t size, صحيح flags)
{
	size_t کەڵەکە_len;

	إذا ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		أرجع k_object_alloc_size(K_OBJ_THREAD_کەڵەکە_ELEMENT, size);
#وإلا
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		أرجع عدم;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	کەڵەکە_len = K_KERNEL_کەڵەکە_LEN(size);
	إذا (کەڵەکە_len < size) {
		أرجع عدم;
	}

	أرجع z_thread_aligned_alloc(Z_KERNEL_کەڵەکە_OBJ_ALIGN, کەڵەکە_len);
}

k_thread_کەڵەکە_t *z_impl_k_thread_کەڵەکە_alloc(size_t size, صحيح flags)
{
	k_thread_کەڵەکە_t *کەڵەکە = عدم;

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		کەڵەکە = z_thread_کەڵەکە_alloc_dyn(size, flags);
		إذا (کەڵەکە == عدم && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			کەڵەکە = z_thread_کەڵەکە_alloc_pool(size, flags);
		}
	} وإلا إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		إذا (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			کەڵەکە = z_thread_کەڵەکە_alloc_pool(size, flags);
		}

		إذا ((کەڵەکە == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			کەڵەکە = z_thread_کەڵەکە_alloc_dyn(size, flags);
		}
	}

	أرجع کەڵەکە;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_thread_کەڵەکە_t *z_vrfy_k_thread_کەڵەکە_alloc(size_t size, صحيح flags)
{
	أرجع z_impl_k_thread_کەڵەکە_alloc(size, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_data)
{
	بنية dyn_cb_data *ثابت data = (بنية dyn_cb_data *)user_data;

	إذا (data->کەڵەکە == (k_thread_کەڵەکە_t *)thread->کەڵەکە_info.start) {
		__ASSERT(data->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->کەڵەکە_info.start);
		data->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_کەڵەکە_free(k_thread_کەڵەکە_t *کەڵەکە)
{
	بنية dyn_cb_data data = {.کەڵەکە = کەڵەکە};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	إذا (data.tid != عدم) {
		إذا (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			أرجع -EBUSY;
		}
	}

	إذا (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		إذا (IS_ARRAY_ELEMENT(dynamic_کەڵەکە, کەڵەکە)) {
			إذا (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_کەڵەکە, کەڵەکە))) {
				LOG_ERR("stack %p is not allocated!", کەڵەکە);
				أرجع -EINVAL;
			}

			أرجع 0;
		}
	}

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		إذا (k_object_find(کەڵەکە)) {
			k_object_free(کەڵەکە);
		} وإلا {
			k_free(کەڵەکە);
		}
#وإلا
		k_free(کەڵەکە);
#endif /* CONFIG_USERSPACE */
	} وإلا {
		LOG_DBG("Invalid stack %p", کەڵەکە);
		أرجع -EINVAL;
	}

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_کەڵەکە_free(k_thread_کەڵەکە_t *کەڵەکە)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(کەڵەکە, K_OBJ_THREAD_کەڵەکە_ELEMENT));

	أرجع z_impl_k_thread_کەڵەکە_free(کەڵەکە);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
