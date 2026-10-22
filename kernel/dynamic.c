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

#если CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#иначе
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

struct dyn_cb_data {
	k_tid_t tid;
	k_thread_stack_t *stack;
};

static K_THREAD_STACK_ARRAY_DEFINE(dynamic_stack, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_STACK_LEN(CONFIG_DYNAMIC_THREAD_STACK_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

static k_thread_stack_t *z_thread_stack_alloc_pool(size_t size, целое flags)
{
	целое rv;
	size_t offset;
	k_thread_stack_t *stack;

	если (size > CONFIG_DYNAMIC_THREAD_STACK_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_STACK_SIZE);
		вернуть NULL;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	если (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		вернуть NULL;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	stack = (k_thread_stack_t *)&dynamic_stack[offset];

	вернуть stack;
}

static k_thread_stack_t *z_thread_stack_alloc_dyn(size_t size, целое flags)
{
	size_t stack_len;

	если ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		вернуть k_object_alloc_size(K_OBJ_THREAD_STACK_ELEMENT, size);
#иначе
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		вернуть NULL;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	stack_len = K_KERNEL_STACK_LEN(size);
	если (stack_len < size) {
		вернуть NULL;
	}

	вернуть z_thread_aligned_alloc(Z_KERNEL_STACK_OBJ_ALIGN, stack_len);
}

k_thread_stack_t *z_impl_k_thread_stack_alloc(size_t size, целое flags)
{
	k_thread_stack_t *stack = NULL;

	если (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		stack = z_thread_stack_alloc_dyn(size, flags);
		если (stack == NULL && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}
	} иначе если (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		если (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}

		если ((stack == NULL) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			stack = z_thread_stack_alloc_dyn(size, flags);
		}
	}

	вернуть stack;
}

#ifdef CONFIG_USERSPACE
static inline k_thread_stack_t *z_vrfy_k_thread_stack_alloc(size_t size, целое flags)
{
	вернуть z_impl_k_thread_stack_alloc(size, flags);
}
#include <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

static void dyn_cb(const struct k_thread *thread, void *user_data)
{
	struct dyn_cb_data *const data = (struct dyn_cb_data *)user_data;

	если (data->stack == (k_thread_stack_t *)thread->stack_info.start) {
		__ASSERT(data->tid == NULL, "stack %p is associated with more than one thread!",
			 (void *)thread->stack_info.start);
		data->tid = (k_tid_t)thread;
	}
}

целое z_impl_k_thread_stack_free(k_thread_stack_t *stack)
{
	struct dyn_cb_data data = {.stack = stack};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	если (data.tid != NULL) {
		если (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			вернуть -EBUSY;
		}
	}

	если (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		если (IS_ARRAY_ELEMENT(dynamic_stack, stack)) {
			если (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_stack, stack))) {
				LOG_ERR("stack %p is not allocated!", stack);
				вернуть -EINVAL;
			}

			вернуть 0;
		}
	}

	если (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		если (k_object_find(stack)) {
			k_object_free(stack);
		} иначе {
			k_free(stack);
		}
#иначе
		k_free(stack);
#endif /* CONFIG_USERSPACE */
	} иначе {
		LOG_DBG("Invalid stack %p", stack);
		вернуть -EINVAL;
	}

	вернуть 0;
}

#ifdef CONFIG_USERSPACE
static inline целое z_vrfy_k_thread_stack_free(k_thread_stack_t *stack)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(stack, K_OBJ_THREAD_STACK_ELEMENT));

	вернуть z_impl_k_thread_stack_free(stack);
}
#include <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
