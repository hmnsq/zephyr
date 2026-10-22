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

#ᨀᨘ CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#ᨀᨘᨁ
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

sa_nracanaa dyn_cb_data {
	k_tid_t tid;
	k_thread_stack_t *stack;
};

static K_THREAD_STACK_ARRAY_DEFINE(dynamic_stack, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_STACK_LEN(CONFIG_DYNAMIC_THREAD_STACK_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

static k_thread_stack_t *z_thread_stack_alloc_pool(size_t size, puur_naa_nka flags)
{
	puur_naa_nka rv;
	size_t offset;
	k_thread_stack_t *stack;

	ᨀᨘ (size > CONFIG_DYNAMIC_THREAD_STACK_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_STACK_SIZE);
		ᨀᨘᨒ NULL;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	ᨀᨘ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ᨀᨘᨒ NULL;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	stack = (k_thread_stack_t *)&dynamic_stack[offset];

	ᨀᨘᨒ stack;
}

static k_thread_stack_t *z_thread_stack_alloc_dyn(size_t size, puur_naa_nka flags)
{
	size_t stack_len;

	ᨀᨘ ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		ᨀᨘᨒ k_object_alloc_size(K_OBJ_THREAD_STACK_ELEMENT, size);
#ᨀᨘᨁ
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ᨀᨘᨒ NULL;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	stack_len = K_KERNEL_STACK_LEN(size);
	ᨀᨘ (stack_len < size) {
		ᨀᨘᨒ NULL;
	}

	ᨀᨘᨒ z_thread_aligned_alloc(Z_KERNEL_STACK_OBJ_ALIGN, stack_len);
}

k_thread_stack_t *z_impl_k_thread_stack_alloc(size_t size, puur_naa_nka flags)
{
	k_thread_stack_t *stack = NULL;

	ᨀᨘ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		stack = z_thread_stack_alloc_dyn(size, flags);
		ᨀᨘ (stack == NULL && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}
	} ᨀᨘᨁ ᨀᨘ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		ᨀᨘ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}

		ᨀᨘ ((stack == NULL) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			stack = z_thread_stack_alloc_dyn(size, flags);
		}
	}

	ᨀᨘᨒ stack;
}

#ifdef CONFIG_USERSPACE
static anta_hstha k_thread_stack_t *z_vrfy_k_thread_stack_alloc(size_t size, puur_naa_nka flags)
{
	ᨀᨘᨒ z_impl_k_thread_stack_alloc(size, flags);
}
#include <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

static void dyn_cb(const sa_nracanaa k_thread *thread, void *user_data)
{
	sa_nracanaa dyn_cb_data *const data = (sa_nracanaa dyn_cb_data *)user_data;

	ᨀᨘ (data->stack == (k_thread_stack_t *)thread->stack_info.start) {
		__ASSERT(data->tid == NULL, "stack %p is associated with more than one thread!",
			 (void *)thread->stack_info.start);
		data->tid = (k_tid_t)thread;
	}
}

puur_naa_nka z_impl_k_thread_stack_free(k_thread_stack_t *stack)
{
	sa_nracanaa dyn_cb_data data = {.stack = stack};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	ᨀᨘ (data.tid != NULL) {
		ᨀᨘ (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			ᨀᨘᨒ -EBUSY;
		}
	}

	ᨀᨘ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		ᨀᨘ (IS_ARRAY_ELEMENT(dynamic_stack, stack)) {
			ᨀᨘ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_stack, stack))) {
				LOG_ERR("stack %p is not allocated!", stack);
				ᨀᨘᨒ -EINVAL;
			}

			ᨀᨘᨒ 0;
		}
	}

	ᨀᨘ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		ᨀᨘ (k_object_find(stack)) {
			k_object_free(stack);
		} ᨀᨘᨁ {
			k_free(stack);
		}
#ᨀᨘᨁ
		k_free(stack);
#endif /* CONFIG_USERSPACE */
	} ᨀᨘᨁ {
		LOG_DBG("Invalid stack %p", stack);
		ᨀᨘᨒ -EINVAL;
	}

	ᨀᨘᨒ 0;
}

#ifdef CONFIG_USERSPACE
static anta_hstha puur_naa_nka z_vrfy_k_thread_stack_free(k_thread_stack_t *stack)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(stack, K_OBJ_THREAD_STACK_ELEMENT));

	ᨀᨘᨒ z_impl_k_thread_stack_free(stack);
}
#include <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
