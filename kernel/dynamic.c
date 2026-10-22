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

#यदि CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#अन्यथा
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

संरचना dyn_cb_data {
	k_tid_t tid;
	k_thread_थाक_t *थाक;
};

स्थिर K_THREAD_थाक_ARRAY_DEFINE(dynamic_थाक, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_थाक_LEN(CONFIG_DYNAMIC_THREAD_थाक_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

स्थिर k_thread_थाक_t *z_thread_थाक_alloc_pool(माप_प्रकार size, पूर्णाङ्क flags)
{
	पूर्णाङ्क rv;
	माप_प्रकार offset;
	k_thread_थाक_t *थाक;

	यदि (size > CONFIG_DYNAMIC_THREAD_थाक_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(माप_प्रकार)CONFIG_DYNAMIC_THREAD_थाक_SIZE);
		फर्काऊ रिक्त;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	यदि (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		फर्काऊ रिक्त;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	थाक = (k_thread_थाक_t *)&dynamic_थाक[offset];

	फर्काऊ थाक;
}

स्थिर k_thread_थाक_t *z_thread_थाक_alloc_dyn(माप_प्रकार size, पूर्णाङ्क flags)
{
	माप_प्रकार थाक_len;

	यदि ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		फर्काऊ k_object_alloc_size(K_OBJ_THREAD_थाक_ELEMENT, size);
#अन्यथा
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		फर्काऊ रिक्त;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	थाक_len = K_KERNEL_थाक_LEN(size);
	यदि (थाक_len < size) {
		फर्काऊ रिक्त;
	}

	फर्काऊ z_thread_aligned_alloc(Z_KERNEL_थाक_OBJ_ALIGN, थाक_len);
}

k_thread_थाक_t *z_impl_k_thread_थाक_alloc(माप_प्रकार size, पूर्णाङ्क flags)
{
	k_thread_थाक_t *थाक = रिक्त;

	यदि (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		थाक = z_thread_थाक_alloc_dyn(size, flags);
		यदि (थाक == रिक्त && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			थाक = z_thread_थाक_alloc_pool(size, flags);
		}
	} अन्यथा यदि (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		यदि (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			थाक = z_thread_थाक_alloc_pool(size, flags);
		}

		यदि ((थाक == रिक्त) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			थाक = z_thread_थाक_alloc_dyn(size, flags);
		}
	}

	फर्काऊ थाक;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ k_thread_थाक_t *z_vrfy_k_thread_थाक_alloc(माप_प्रकार size, पूर्णाङ्क flags)
{
	फर्काऊ z_impl_k_thread_थाक_alloc(size, flags);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

स्थिर शून्य dyn_cb(अचर संरचना k_thread *thread, शून्य *user_data)
{
	संरचना dyn_cb_data *अचर data = (संरचना dyn_cb_data *)user_data;

	यदि (data->थाक == (k_thread_थाक_t *)thread->थाक_info.start) {
		__ASSERT(data->tid == रिक्त, "stack %p is associated with more than one thread!",
			 (शून्य *)thread->थाक_info.start);
		data->tid = (k_tid_t)thread;
	}
}

पूर्णाङ्क z_impl_k_thread_थाक_free(k_thread_थाक_t *थाक)
{
	संरचना dyn_cb_data data = {.थाक = थाक};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	यदि (data.tid != रिक्त) {
		यदि (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			फर्काऊ -EBUSY;
		}
	}

	यदि (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		यदि (IS_ARRAY_ELEMENT(dynamic_थाक, थाक)) {
			यदि (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_थाक, थाक))) {
				LOG_ERR("stack %p is not allocated!", थाक);
				फर्काऊ -EINVAL;
			}

			फर्काऊ 0;
		}
	}

	यदि (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		यदि (k_object_find(थाक)) {
			k_object_free(थाक);
		} अन्यथा {
			k_free(थाक);
		}
#अन्यथा
		k_free(थाक);
#endif /* CONFIG_USERSPACE */
	} अन्यथा {
		LOG_DBG("Invalid stack %p", थाक);
		फर्काऊ -EINVAL;
	}

	फर्काऊ 0;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_thread_थाक_free(k_thread_थाक_t *थाक)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(थाक, K_OBJ_THREAD_थाक_ELEMENT));

	फर्काऊ z_impl_k_thread_थाक_free(थाक);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
