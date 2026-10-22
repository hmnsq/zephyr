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

#ܐܢ CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#ܐܚܪܢܐ
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

بنية dyn_cb_data {
	k_tid_t tid;
	k_thread_stack_t *stack;
};

ساكن K_THREAD_STACK_ARRAY_DEFINE(dynamic_stack, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_STACK_LEN(CONFIG_DYNAMIC_THREAD_STACK_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

ساكن k_thread_stack_t *z_thread_stack_alloc_pool(size_t size, صحيح flags)
{
	صحيح rv;
	size_t offset;
	k_thread_stack_t *stack;

	ܐܢ (size > CONFIG_DYNAMIC_THREAD_STACK_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_STACK_SIZE);
		ܦܢܐ عدم;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	ܐܢ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ܦܢܐ عدم;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	stack = (k_thread_stack_t *)&dynamic_stack[offset];

	ܦܢܐ stack;
}

ساكن k_thread_stack_t *z_thread_stack_alloc_dyn(size_t size, صحيح flags)
{
	size_t stack_len;

	ܐܢ ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		ܦܢܐ k_object_alloc_size(K_OBJ_THREAD_STACK_ELEMENT, size);
#ܐܚܪܢܐ
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ܦܢܐ عدم;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	stack_len = K_KERNEL_STACK_LEN(size);
	ܐܢ (stack_len < size) {
		ܦܢܐ عدم;
	}

	ܦܢܐ z_thread_aligned_alloc(Z_KERNEL_STACK_OBJ_ALIGN, stack_len);
}

k_thread_stack_t *z_impl_k_thread_stack_alloc(size_t size, صحيح flags)
{
	k_thread_stack_t *stack = عدم;

	ܐܢ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		stack = z_thread_stack_alloc_dyn(size, flags);
		ܐܢ (stack == عدم && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}
	} ܐܚܪܢܐ ܐܢ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		ܐܢ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			stack = z_thread_stack_alloc_pool(size, flags);
		}

		ܐܢ ((stack == عدم) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			stack = z_thread_stack_alloc_dyn(size, flags);
		}
	}

	ܦܢܐ stack;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_thread_stack_t *z_vrfy_k_thread_stack_alloc(size_t size, صحيح flags)
{
	ܦܢܐ z_impl_k_thread_stack_alloc(size, flags);
}
#ضمن <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ساكن فراغ dyn_cb(ثابت بنية k_thread *thread, فراغ *user_data)
{
	بنية dyn_cb_data *ثابت data = (بنية dyn_cb_data *)user_data;

	ܐܢ (data->stack == (k_thread_stack_t *)thread->stack_info.start) {
		__ASSERT(data->tid == عدم, "stack %p is associated with more than one thread!",
			 (فراغ *)thread->stack_info.start);
		data->tid = (k_tid_t)thread;
	}
}

صحيح z_impl_k_thread_stack_free(k_thread_stack_t *stack)
{
	بنية dyn_cb_data data = {.stack = stack};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	ܐܢ (data.tid != عدم) {
		ܐܢ (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			ܦܢܐ -EBUSY;
		}
	}

	ܐܢ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		ܐܢ (IS_ARRAY_ELEMENT(dynamic_stack, stack)) {
			ܐܢ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_stack, stack))) {
				LOG_ERR("stack %p is not allocated!", stack);
				ܦܢܐ -EINVAL;
			}

			ܦܢܐ 0;
		}
	}

	ܐܢ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		ܐܢ (k_object_find(stack)) {
			k_object_free(stack);
		} ܐܚܪܢܐ {
			k_free(stack);
		}
#ܐܚܪܢܐ
		k_free(stack);
#endif /* CONFIG_USERSPACE */
	} ܐܚܪܢܐ {
		LOG_DBG("Invalid stack %p", stack);
		ܦܢܐ -EINVAL;
	}

	ܦܢܐ 0;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_thread_stack_free(k_thread_stack_t *stack)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(stack, K_OBJ_THREAD_STACK_ELEMENT));

	ܦܢܐ z_impl_k_thread_stack_free(stack);
}
#ضمن <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
