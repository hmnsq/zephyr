/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ "kernel_internal.h"

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/kernel/thread_stack.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>
#അന്തര്ഭാവ <zephyr/sys/bitarray.h>
#അന്തര്ഭാവ <zephyr/sys/kobject.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#എങ്കിൽ CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#അല്ലെങ്കിൽ
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

ഘടന dyn_cb_data {
	k_tid_t tid;
	k_thread_അട്ടി_t *അട്ടി;
};

സ്ഥിതം K_THREAD_അട്ടി_ARRAY_DEFINE(dynamic_അട്ടി, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_അട്ടി_LEN(CONFIG_DYNAMIC_THREAD_അട്ടി_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

സ്ഥിതം k_thread_അട്ടി_t *z_thread_അട്ടി_alloc_pool(size_t size, പൂർണ്ണസംഖ്യ flags)
{
	പൂർണ്ണസംഖ്യ rv;
	size_t offset;
	k_thread_അട്ടി_t *അട്ടി;

	എങ്കിൽ (size > CONFIG_DYNAMIC_THREAD_അട്ടി_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_അട്ടി_SIZE);
		തിരികെ ശൂന്യ;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	എങ്കിൽ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		തിരികെ ശൂന്യ;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	അട്ടി = (k_thread_അട്ടി_t *)&dynamic_അട്ടി[offset];

	തിരികെ അട്ടി;
}

സ്ഥിതം k_thread_അട്ടി_t *z_thread_അട്ടി_alloc_dyn(size_t size, പൂർണ്ണസംഖ്യ flags)
{
	size_t അട്ടി_len;

	എങ്കിൽ ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		തിരികെ k_object_alloc_size(K_OBJ_THREAD_അട്ടി_ELEMENT, size);
#അല്ലെങ്കിൽ
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		തിരികെ ശൂന്യ;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	അട്ടി_len = K_KERNEL_അട്ടി_LEN(size);
	എങ്കിൽ (അട്ടി_len < size) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ z_thread_aligned_alloc(Z_KERNEL_അട്ടി_OBJ_ALIGN, അട്ടി_len);
}

k_thread_അട്ടി_t *z_impl_k_thread_അട്ടി_alloc(size_t size, പൂർണ്ണസംഖ്യ flags)
{
	k_thread_അട്ടി_t *അട്ടി = ശൂന്യ;

	എങ്കിൽ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		അട്ടി = z_thread_അട്ടി_alloc_dyn(size, flags);
		എങ്കിൽ (അട്ടി == ശൂന്യ && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			അട്ടി = z_thread_അട്ടി_alloc_pool(size, flags);
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		എങ്കിൽ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			അട്ടി = z_thread_അട്ടി_alloc_pool(size, flags);
		}

		എങ്കിൽ ((അട്ടി == ശൂന്യ) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			അട്ടി = z_thread_അട്ടി_alloc_dyn(size, flags);
		}
	}

	തിരികെ അട്ടി;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ k_thread_അട്ടി_t *z_vrfy_k_thread_അട്ടി_alloc(size_t size, പൂർണ്ണസംഖ്യ flags)
{
	തിരികെ z_impl_k_thread_അട്ടി_alloc(size, flags);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

സ്ഥിതം ശൂന്യം dyn_cb(സ്ഥിരം ഘടന k_thread *thread, ശൂന്യം *user_data)
{
	ഘടന dyn_cb_data *സ്ഥിരം data = (ഘടന dyn_cb_data *)user_data;

	എങ്കിൽ (data->അട്ടി == (k_thread_അട്ടി_t *)thread->അട്ടി_info.start) {
		__ASSERT(data->tid == ശൂന്യ, "stack %p is associated with more than one thread!",
			 (ശൂന്യം *)thread->അട്ടി_info.start);
		data->tid = (k_tid_t)thread;
	}
}

പൂർണ്ണസംഖ്യ z_impl_k_thread_അട്ടി_free(k_thread_അട്ടി_t *അട്ടി)
{
	ഘടന dyn_cb_data data = {.അട്ടി = അട്ടി};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	എങ്കിൽ (data.tid != ശൂന്യ) {
		എങ്കിൽ (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			തിരികെ -EBUSY;
		}
	}

	എങ്കിൽ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		എങ്കിൽ (IS_ARRAY_ELEMENT(dynamic_അട്ടി, അട്ടി)) {
			എങ്കിൽ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_അട്ടി, അട്ടി))) {
				LOG_ERR("stack %p is not allocated!", അട്ടി);
				തിരികെ -EINVAL;
			}

			തിരികെ 0;
		}
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		എങ്കിൽ (k_object_find(അട്ടി)) {
			k_object_free(അട്ടി);
		} അല്ലെങ്കിൽ {
			k_free(അട്ടി);
		}
#അല്ലെങ്കിൽ
		k_free(അട്ടി);
#endif /* CONFIG_USERSPACE */
	} അല്ലെങ്കിൽ {
		LOG_DBG("Invalid stack %p", അട്ടി);
		തിരികെ -EINVAL;
	}

	തിരികെ 0;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_thread_അട്ടി_free(k_thread_അട്ടി_t *അട്ടി)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(അട്ടി, K_OBJ_THREAD_അട്ടി_ELEMENT));

	തിരികെ z_impl_k_thread_അട്ടി_free(അട്ടി);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
