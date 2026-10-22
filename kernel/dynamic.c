/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव "kernel_internal.h"

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/kernel/thread_stack.h>
#अन्तर्भाव <zephyr/logging/log.h>
#अन्तर्भाव <zephyr/sys/bitarray.h>
#अन्तर्भाव <zephyr/sys/kobject.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#जइ CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#अण्णहा
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

संरचना dyn_cb_data {
	k_tid_t tid;
	k_thread_राशिः_t *राशिः;
};

स्थिर K_THREAD_राशिः_ARRAY_DEFINE(dynamic_राशिः, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_राशिः_LEN(CONFIG_DYNAMIC_THREAD_राशिः_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

स्थिर k_thread_राशिः_t *z_thread_राशिः_alloc_pool(size_t size, पूर्णाङ्क flags)
{
	पूर्णाङ्क rv;
	size_t offset;
	k_thread_राशिः_t *राशिः;

	जइ (size > CONFIG_DYNAMIC_THREAD_राशिः_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_राशिः_SIZE);
		प्रत्यागम रिक्त;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	जइ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		प्रत्यागम रिक्त;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	राशिः = (k_thread_राशिः_t *)&dynamic_राशिः[offset];

	प्रत्यागम राशिः;
}

स्थिर k_thread_राशिः_t *z_thread_राशिः_alloc_dyn(size_t size, पूर्णाङ्क flags)
{
	size_t राशिः_len;

	जइ ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		प्रत्यागम k_object_alloc_size(K_OBJ_THREAD_राशिः_ELEMENT, size);
#अण्णहा
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		प्रत्यागम रिक्त;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	राशिः_len = K_KERNEL_राशिः_LEN(size);
	जइ (राशिः_len < size) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम z_thread_aligned_alloc(Z_KERNEL_राशिः_OBJ_ALIGN, राशिः_len);
}

k_thread_राशिः_t *z_impl_k_thread_राशिः_alloc(size_t size, पूर्णाङ्क flags)
{
	k_thread_राशिः_t *राशिः = रिक्त;

	जइ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		राशिः = z_thread_राशिः_alloc_dyn(size, flags);
		जइ (राशिः == रिक्त && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			राशिः = z_thread_राशिः_alloc_pool(size, flags);
		}
	} अण्णहा जइ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		जइ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			राशिः = z_thread_राशिः_alloc_pool(size, flags);
		}

		जइ ((राशिः == रिक्त) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			राशिः = z_thread_राशिः_alloc_dyn(size, flags);
		}
	}

	प्रत्यागम राशिः;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ k_thread_राशिः_t *z_vrfy_k_thread_राशिः_alloc(size_t size, पूर्णाङ्क flags)
{
	प्रत्यागम z_impl_k_thread_राशिः_alloc(size, flags);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

स्थिर शून्य dyn_cb(ध्रुव संरचना k_thread *thread, शून्य *user_data)
{
	संरचना dyn_cb_data *ध्रुव data = (संरचना dyn_cb_data *)user_data;

	जइ (data->राशिः == (k_thread_राशिः_t *)thread->राशिः_info.start) {
		__ASSERT(data->tid == रिक्त, "stack %p is associated with more than one thread!",
			 (शून्य *)thread->राशिः_info.start);
		data->tid = (k_tid_t)thread;
	}
}

पूर्णाङ्क z_impl_k_thread_राशिः_free(k_thread_राशिः_t *राशिः)
{
	संरचना dyn_cb_data data = {.राशिः = राशिः};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	जइ (data.tid != रिक्त) {
		जइ (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			प्रत्यागम -EBUSY;
		}
	}

	जइ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		जइ (IS_ARRAY_ELEMENT(dynamic_राशिः, राशिः)) {
			जइ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_राशिः, राशिः))) {
				LOG_ERR("stack %p is not allocated!", राशिः);
				प्रत्यागम -EINVAL;
			}

			प्रत्यागम 0;
		}
	}

	जइ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		जइ (k_object_find(राशिः)) {
			k_object_free(राशिः);
		} अण्णहा {
			k_free(राशिः);
		}
#अण्णहा
		k_free(राशिः);
#endif /* CONFIG_USERSPACE */
	} अण्णहा {
		LOG_DBG("Invalid stack %p", राशिः);
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_राशिः_free(k_thread_राशिः_t *राशिः)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(राशिः, K_OBJ_THREAD_राशिः_ELEMENT));

	प्रत्यागम z_impl_k_thread_राशिः_free(राशिः);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
