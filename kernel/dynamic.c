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

#जर CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#अन्यथा
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

संरचना dyn_cb_data {
	k_tid_t tid;
	k_thread_चळत_t *चळत;
};

स्थिर K_THREAD_चळत_ARRAY_DEFINE(dynamic_चळत, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_चळत_LEN(CONFIG_DYNAMIC_THREAD_चळत_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

स्थिर k_thread_चळत_t *z_thread_चळत_alloc_pool(माप_प्रकार size, पूर्णांक flags)
{
	पूर्णांक rv;
	माप_प्रकार offset;
	k_thread_चळत_t *चळत;

	जर (size > CONFIG_DYNAMIC_THREAD_चळत_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(माप_प्रकार)CONFIG_DYNAMIC_THREAD_चळत_SIZE);
		परत रिक्त;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	जर (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		परत रिक्त;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	चळत = (k_thread_चळत_t *)&dynamic_चळत[offset];

	परत चळत;
}

स्थिर k_thread_चळत_t *z_thread_चळत_alloc_dyn(माप_प्रकार size, पूर्णांक flags)
{
	माप_प्रकार चळत_len;

	जर ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		परत k_object_alloc_size(K_OBJ_THREAD_चळत_ELEMENT, size);
#अन्यथा
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		परत रिक्त;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	चळत_len = K_KERNEL_चळत_LEN(size);
	जर (चळत_len < size) {
		परत रिक्त;
	}

	परत z_thread_aligned_alloc(Z_KERNEL_चळत_OBJ_ALIGN, चळत_len);
}

k_thread_चळत_t *z_impl_k_thread_चळत_alloc(माप_प्रकार size, पूर्णांक flags)
{
	k_thread_चळत_t *चळत = रिक्त;

	जर (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		चळत = z_thread_चळत_alloc_dyn(size, flags);
		जर (चळत == रिक्त && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			चळत = z_thread_चळत_alloc_pool(size, flags);
		}
	} अन्यथा जर (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		जर (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			चळत = z_thread_चळत_alloc_pool(size, flags);
		}

		जर ((चळत == रिक्त) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			चळत = z_thread_चळत_alloc_dyn(size, flags);
		}
	}

	परत चळत;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत k_thread_चळत_t *z_vrfy_k_thread_चळत_alloc(माप_प्रकार size, पूर्णांक flags)
{
	परत z_impl_k_thread_चळत_alloc(size, flags);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

स्थिर शून्य dyn_cb(स्थिरांक संरचना k_thread *thread, शून्य *user_data)
{
	संरचना dyn_cb_data *स्थिरांक data = (संरचना dyn_cb_data *)user_data;

	जर (data->चळत == (k_thread_चळत_t *)thread->चळत_info.start) {
		__ASSERT(data->tid == रिक्त, "stack %p is associated with more than one thread!",
			 (शून्य *)thread->चळत_info.start);
		data->tid = (k_tid_t)thread;
	}
}

पूर्णांक z_impl_k_thread_चळत_free(k_thread_चळत_t *चळत)
{
	संरचना dyn_cb_data data = {.चळत = चळत};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	जर (data.tid != रिक्त) {
		जर (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			परत -EBUSY;
		}
	}

	जर (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		जर (IS_ARRAY_ELEMENT(dynamic_चळत, चळत)) {
			जर (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_चळत, चळत))) {
				LOG_ERR("stack %p is not allocated!", चळत);
				परत -EINVAL;
			}

			परत 0;
		}
	}

	जर (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		जर (k_object_find(चळत)) {
			k_object_free(चळत);
		} अन्यथा {
			k_free(चळत);
		}
#अन्यथा
		k_free(चळत);
#endif /* CONFIG_USERSPACE */
	} अन्यथा {
		LOG_DBG("Invalid stack %p", चळत);
		परत -EINVAL;
	}

	परत 0;
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_thread_चळत_free(k_thread_चळत_t *चळत)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(चळत, K_OBJ_THREAD_चळत_ELEMENT));

	परत z_impl_k_thread_चळत_free(चळत);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
