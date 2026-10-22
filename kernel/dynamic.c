/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kernel_internal.h"

#include <zephyr/kernel.h>
#include <ksched.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/bitarray.h>
#include <zephyr/sys/kobject.h>
#include <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#如果 CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#否则
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

struct dyn_cb_data {
	k_tid_t tid;
	k_thread_stack_t *stack;
};

static K_THREAD_STACK_ARRAY_DEFINE(dynamic_stack, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_STACK_LEN(CONFIG_DYNAMIC_THREAD_STACK_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

static k_thread_stack_t *z_thread_stack_alloc_pool(size_t size, 整数 flags)
{
	整数 rv;
	size_t offset;
	k_thread_stack_t *stack;

	如果 (size > CONFIG_DYNAMIC_THREAD_STACK_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_STACK_SIZE);
		返回 NULL;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	如果 (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		返回 NULL;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	stack = (k_thread_stack_t *)&dynamic_stack[offset];

	返回 stack;
}

static k_thread_stack_t *z_thread_stack_alloc_dyn(size_t size, 整数 flags)
{
	size_t stack_len;

	如果 ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		返回 k_object_alloc_size(K_OBJ_THREAD_STACK_ELEMENT, size);
#否则
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		返回 NULL;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	stack_len = K_KERNEL_STACK_LEN(size);
	如果 (stack_len < size) {
		返回 NULL;
	}

	返回 z_thread_aligned_alloc(Z_KERNEL_STACK_OBJ_ALIGN, stack_len);
}

k_thread_stack_t *z_impl_k_thread_stack_alloc(size_t size, 整数 flags)
{
	k_thread_stack_t *stack = NULL;

	如果 (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		stack = z_thread_stack_alloc_dyn(size, flags);
		如果 (stack == NULL && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}
	} 否则 如果 (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		如果 (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}

		如果 ((stack == NULL) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			stack = z_thread_stack_alloc_dyn(size, flags);
		}
	}

	返回 stack;
}

#ifdef CONFIG_USERSPACE
static inline k_thread_stack_t *z_vrfy_k_thread_stack_alloc(size_t size, 整数 flags)
{
	返回 z_impl_k_thread_stack_alloc(size, flags);
}
#include <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

static void dyn_cb(const struct k_thread *thread, void *user_data)
{
	struct dyn_cb_data *const data = (struct dyn_cb_data *)user_data;

	如果 (data->stack == (k_thread_stack_t *)thread->stack_info.start) {
		__ASSERT(data->tid == NULL, "stack %p is associated with more than one thread!",
			 (void *)thread->stack_info.start);
		data->tid = (k_tid_t)thread;
	}
}

整数 z_impl_k_thread_stack_free(k_thread_stack_t *stack)
{
	struct dyn_cb_data data = {.stack = stack};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	如果 (data.tid != NULL) {
		如果 (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			返回 -EBUSY;
		}
	}

	如果 (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		如果 (IS_ARRAY_ELEMENT(dynamic_stack, stack)) {
			如果 (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_stack, stack))) {
				LOG_ERR("stack %p is not allocated!", stack);
				返回 -EINVAL;
			}

			返回 0;
		}
	}

	如果 (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		如果 (k_object_find(stack)) {
			k_object_free(stack);
		} 否则 {
			k_free(stack);
		}
#否则
		k_free(stack);
#endif /* CONFIG_USERSPACE */
	} 否则 {
		LOG_DBG("Invalid stack %p", stack);
		返回 -EINVAL;
	}

	返回 0;
}

#ifdef CONFIG_USERSPACE
static inline 整数 z_vrfy_k_thread_stack_free(k_thread_stack_t *stack)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(stack, K_OBJ_THREAD_STACK_ELEMENT));

	返回 z_impl_k_thread_stack_free(stack);
}
#include <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
