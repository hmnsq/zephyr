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
	k_thread_دۆۋە_t *دۆۋە;
};

ساكن K_THREAD_دۆۋە_ARRAY_DEFINE(dynamic_دۆۋە, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_دۆۋە_LEN(CONFIG_DYNAMIC_THREAD_دۆۋە_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

ساكن k_thread_دۆۋە_t *z_thread_دۆۋە_alloc_pool(size_t size, صحيح flags)
{
	صحيح rv;
	size_t offset;
	k_thread_دۆۋە_t *دۆۋە;

	إذا (size > CONFIG_DYNAMIC_THREAD_دۆۋە_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_دۆۋە_SIZE);
		أرجع عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	إذا (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		أرجع عدم;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	دۆۋە = (k_thread_دۆۋە_t *)&dynamic_دۆۋە[offset];

	أرجع دۆۋە;
}

ساكن k_thread_دۆۋە_t *z_thread_دۆۋە_alloc_dyn(size_t size, صحيح flags)
{
	size_t دۆۋە_len;

	إذا ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		أرجع k_object_alloc_size(K_OBJ_THREAD_دۆۋە_ELEMENT, size);
#وإلا
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		أرجع عدم;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	دۆۋە_len = K_KERNEL_دۆۋە_LEN(size);
	إذا (دۆۋە_len < size) {
		أرجع عدم;
	}

	أرجع z_thread_aligned_alloc(Z_KERNEL_دۆۋە_OBJ_ALIGN, دۆۋە_len);
}

k_thread_دۆۋە_t *z_impl_k_thread_دۆۋە_alloc(size_t size, صحيح flags)
{
	k_thread_دۆۋە_t *دۆۋە = عدم;

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		دۆۋە = z_thread_دۆۋە_alloc_dyn(size, flags);
		إذا (دۆۋە == عدم && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			دۆۋە = z_thread_دۆۋە_alloc_pool(size, flags);
		}
	} وإلا إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		إذا (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			دۆۋە = z_thread_دۆۋە_alloc_pool(size, flags);
		}

		إذا ((دۆۋە == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			دۆۋە = z_thread_دۆۋە_alloc_dyn(size, flags);
		}
	}

	أرجع دۆۋە;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_thread_دۆۋە_t *z_vrfy_k_thread_دۆۋە_alloc(size_t size, صحيح flags)
{
	أرجع z_impl_k_thread_دۆۋە_alloc(size, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_data)
{
	بنية dyn_cb_data *ثابت data = (بنية dyn_cb_data *)user_data;

	إذا (data->دۆۋە == (k_thread_دۆۋە_t *)thread->دۆۋە_info.start) {
		__ASSERT(data->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->دۆۋە_info.start);
		data->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_دۆۋە_free(k_thread_دۆۋە_t *دۆۋە)
{
	بنية dyn_cb_data data = {.دۆۋە = دۆۋە};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	إذا (data.tid != عدم) {
		إذا (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			أرجع -EBUSY;
		}
	}

	إذا (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		إذا (IS_ARRAY_ELEMENT(dynamic_دۆۋە, دۆۋە)) {
			إذا (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_دۆۋە, دۆۋە))) {
				LOG_ERR("stack %p is not allocated!", دۆۋە);
				أرجع -EINVAL;
			}

			أرجع 0;
		}
	}

	إذا (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		إذا (k_object_find(دۆۋە)) {
			k_object_free(دۆۋە);
		} وإلا {
			k_free(دۆۋە);
		}
#وإلا
		k_free(دۆۋە);
#endif /* CONFIG_USERSPACE */
	} وإلا {
		LOG_DBG("Invalid stack %p", دۆۋە);
		أرجع -EINVAL;
	}

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_دۆۋە_free(k_thread_دۆۋە_t *دۆۋە)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(دۆۋە, K_OBJ_THREAD_دۆۋە_ELEMENT));

	أرجع z_impl_k_thread_دۆۋە_free(دۆۋە);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
