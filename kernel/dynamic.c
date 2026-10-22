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

#यदि CONFIG_DYNAMIC_THREAD_POOL_आकार > 0
#परिभाषय BA_आकार CONFIG_DYNAMIC_THREAD_POOL_आकार
#अन्यथा
#परिभाषय BA_आकार 1
#यदि_अन्तः /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

संरचना dyn_cb_दत्त {
	k_tid_t tid;
	k_thread_राशिः_t *राशिः;
};

स्थिर K_THREAD_राशिः_ARRAY_DEFINE(dynamic_राशिः, CONFIG_DYNAMIC_THREAD_POOL_आकार,
				   K_THREAD_राशिः_LEN(CONFIG_DYNAMIC_THREAD_राशिः_आकार));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_आकार);

स्थिर k_thread_राशिः_t *z_thread_राशिः_alloc_pool(size_t आकार, पूर्णाङ्क flags)
{
	पूर्णाङ्क rv;
	size_t विस्थापनम्;
	k_thread_राशिः_t *राशिः;

	यदि (आकार > CONFIG_DYNAMIC_THREAD_राशिः_आकार) {
		LOG_DBG("stack size %zu is > pool stack size %zu", आकार,
			(size_t)CONFIG_DYNAMIC_THREAD_राशिः_आकार);
		प्रत्यागम रिक्त;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &विस्थापनम्);
	यदि (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		प्रत्यागम रिक्त;
	}

	__ASSERT_NO_MSG(विस्थापनम् < CONFIG_DYNAMIC_THREAD_POOL_आकार);

	राशिः = (k_thread_राशिः_t *)&dynamic_राशिः[विस्थापनम्];

	प्रत्यागम राशिः;
}

स्थिर k_thread_राशिः_t *z_thread_राशिः_alloc_dyn(size_t आकार, पूर्णाङ्क flags)
{
	size_t राशिः_len;

	यदि ((flags & K_USER) == K_USER) {
#यदि_परिभाषितम् CONFIG_DYNAMIC_OBJECTS
		प्रत्यागम k_object_alloc_आकार(K_OBJ_THREAD_राशिः_ELEMENT, आकार);
#अन्यथा
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		प्रत्यागम रिक्त;
#यदि_अन्तः /* CONFIG_DYNAMIC_OBJECTS */
	}

	राशिः_len = K_KERNEL_राशिः_LEN(आकार);
	यदि (राशिः_len < आकार) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम z_thread_aligned_alloc(Z_KERNEL_राशिः_OBJ_ALIGN, राशिः_len);
}

k_thread_राशिः_t *z_impl_k_thread_राशिः_alloc(size_t आकार, पूर्णाङ्क flags)
{
	k_thread_राशिः_t *राशिः = रिक्त;

	यदि (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		राशिः = z_thread_राशिः_alloc_dyn(आकार, flags);
		यदि (राशिः == रिक्त && CONFIG_DYNAMIC_THREAD_POOL_आकार > 0) {
			राशिः = z_thread_राशिः_alloc_pool(आकार, flags);
		}
	} अन्यथा यदि (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		यदि (CONFIG_DYNAMIC_THREAD_POOL_आकार > 0) {
			राशिः = z_thread_राशिः_alloc_pool(आकार, flags);
		}

		यदि ((राशिः == रिक्त) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			राशिः = z_thread_राशिः_alloc_dyn(आकार, flags);
		}
	}

	प्रत्यागम राशिः;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ k_thread_राशिः_t *z_vrfy_k_thread_राशिः_alloc(size_t आकार, पूर्णाङ्क flags)
{
	प्रत्यागम z_impl_k_thread_राशिः_alloc(आकार, flags);
}
#अन्तर्भाव <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

स्थिर शून्य dyn_cb(ध्रुव संरचना k_thread *thread, शून्य *user_दत्त)
{
	संरचना dyn_cb_दत्त *ध्रुव दत्त = (संरचना dyn_cb_दत्त *)user_दत्त;

	यदि (दत्त->राशिः == (k_thread_राशिः_t *)thread->राशिः_info.start) {
		__ASSERT(दत्त->tid == रिक्त, "stack %p is associated with more than one thread!",
			 (शून्य *)thread->राशिः_info.start);
		दत्त->tid = (k_tid_t)thread;
	}
}

पूर्णाङ्क z_impl_k_thread_राशिः_free(k_thread_राशिः_t *राशिः)
{
	संरचना dyn_cb_दत्त दत्त = {.राशिः = राशिः};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &दत्त);

	यदि (दत्त.tid != रिक्त) {
		यदि (!z_is_thread_state_समुच्चयः(दत्त.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", दत्त.tid);
			प्रत्यागम -EBUSY;
		}
	}

	यदि (CONFIG_DYNAMIC_THREAD_POOL_आकार > 0) {
		यदि (IS_ARRAY_ELEMENT(dynamic_राशिः, राशिः)) {
			यदि (sys_bitarray_free(&dynamic_ba, 1, ARRAY_सूचकांक(dynamic_राशिः, राशिः))) {
				LOG_ERR("stack %p is not allocated!", राशिः);
				प्रत्यागम -EINVAL;
			}

			प्रत्यागम 0;
		}
	}

	यदि (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#यदि_परिभाषितम् CONFIG_USERSPACE
		यदि (k_object_find(राशिः)) {
			k_object_free(राशिः);
		} अन्यथा {
			k_free(राशिः);
		}
#अन्यथा
		k_free(राशिः);
#यदि_अन्तः /* CONFIG_USERSPACE */
	} अन्यथा {
		LOG_DBG("Invalid stack %p", राशिः);
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
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
#यदि_अन्तः /* CONFIG_USERSPACE */
