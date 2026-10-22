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

#အကယ်၍ CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#မဟုတ်ပါက
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

sa_nracanaa dyn_cb_data {
	k_tid_t tid;
	k_thread_stack_t *stack;
};

static K_THREAD_STACK_ARRAY_DEFINE(dynamic_stack, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_STACK_LEN(CONFIG_DYNAMIC_THREAD_STACK_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

static k_thread_stack_t *z_thread_stack_alloc_pool(size_t size, ကိန်းပြည့် flags)
{
	ကိန်းပြည့် rv;
	size_t offset;
	k_thread_stack_t *stack;

	အကယ်၍ (size > CONFIG_DYNAMIC_THREAD_STACK_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_STACK_SIZE);
		ပြန်ပေး NULL;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	အကယ်၍ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ပြန်ပေး NULL;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	stack = (k_thread_stack_t *)&dynamic_stack[offset];

	ပြန်ပေး stack;
}

static k_thread_stack_t *z_thread_stack_alloc_dyn(size_t size, ကိန်းပြည့် flags)
{
	size_t stack_len;

	အကယ်၍ ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		ပြန်ပေး k_object_alloc_size(K_OBJ_THREAD_STACK_ELEMENT, size);
#မဟုတ်ပါက
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ပြန်ပေး NULL;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	stack_len = K_KERNEL_STACK_LEN(size);
	အကယ်၍ (stack_len < size) {
		ပြန်ပေး NULL;
	}

	ပြန်ပေး z_thread_aligned_alloc(Z_KERNEL_STACK_OBJ_ALIGN, stack_len);
}

k_thread_stack_t *z_impl_k_thread_stack_alloc(size_t size, ကိန်းပြည့် flags)
{
	k_thread_stack_t *stack = NULL;

	အကယ်၍ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		stack = z_thread_stack_alloc_dyn(size, flags);
		အကယ်၍ (stack == NULL && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}
	} မဟုတ်ပါက အကယ်၍ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		အကယ်၍ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}

		အကယ်၍ ((stack == NULL) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			stack = z_thread_stack_alloc_dyn(size, flags);
		}
	}

	ပြန်ပေး stack;
}

#ifdef CONFIG_USERSPACE
static anta_hstha k_thread_stack_t *z_vrfy_k_thread_stack_alloc(size_t size, ကိန်းပြည့် flags)
{
	ပြန်ပေး z_impl_k_thread_stack_alloc(size, flags);
}
#include <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

static void dyn_cb(const sa_nracanaa k_thread *thread, void *user_data)
{
	sa_nracanaa dyn_cb_data *const data = (sa_nracanaa dyn_cb_data *)user_data;

	အကယ်၍ (data->stack == (k_thread_stack_t *)thread->stack_info.start) {
		__ASSERT(data->tid == NULL, "stack %p is associated with more than one thread!",
			 (void *)thread->stack_info.start);
		data->tid = (k_tid_t)thread;
	}
}

ကိန်းပြည့် z_impl_k_thread_stack_free(k_thread_stack_t *stack)
{
	sa_nracanaa dyn_cb_data data = {.stack = stack};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	အကယ်၍ (data.tid != NULL) {
		အကယ်၍ (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			ပြန်ပေး -EBUSY;
		}
	}

	အကယ်၍ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		အကယ်၍ (IS_ARRAY_ELEMENT(dynamic_stack, stack)) {
			အကယ်၍ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_stack, stack))) {
				LOG_ERR("stack %p is not allocated!", stack);
				ပြန်ပေး -EINVAL;
			}

			ပြန်ပေး 0;
		}
	}

	အကယ်၍ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		အကယ်၍ (k_object_find(stack)) {
			k_object_free(stack);
		} မဟုတ်ပါက {
			k_free(stack);
		}
#မဟုတ်ပါက
		k_free(stack);
#endif /* CONFIG_USERSPACE */
	} မဟုတ်ပါက {
		LOG_DBG("Invalid stack %p", stack);
		ပြန်ပေး -EINVAL;
	}

	ပြန်ပေး 0;
}

#ifdef CONFIG_USERSPACE
static anta_hstha ကိန်းပြည့် z_vrfy_k_thread_stack_free(k_thread_stack_t *stack)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(stack, K_OBJ_THREAD_STACK_ELEMENT));

	ပြန်ပေး z_impl_k_thread_stack_free(stack);
}
#include <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
