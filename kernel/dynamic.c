/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश "kernel_internal.h"

#समावेश <zephyr/kernel.h>
#समावेश <ksched.h>
#समावेश <zephyr/kernel/thread_stack.h>
#समावेश <zephyr/logging/log.h>
#समावेश <zephyr/sys/bitarray.h>
#समावेश <zephyr/sys/kobject.h>
#समावेश <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#अगर CONFIG_DYNAMIC_THREAD_POOL_आकार > 0
#घोषणा BA_आकार CONFIG_DYNAMIC_THREAD_POOL_आकार
#अन्यथा
#घोषणा BA_आकार 1
#पूर्ण_अगर /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

काष्ठा dyn_cb_तथ्य {
	k_tid_t tid;
	k_thread_ढेर_t *ढेर;
};

अटल K_THREAD_ढेर_ARRAY_DEFINE(dynamic_ढेर, CONFIG_DYNAMIC_THREAD_POOL_आकार,
				   K_THREAD_ढेर_LEN(CONFIG_DYNAMIC_THREAD_ढेर_आकार));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_आकार);

अटल k_thread_ढेर_t *z_thread_ढेर_alloc_pool(माप_प्रकार आकार, पूर्णांक flags)
{
	पूर्णांक rv;
	माप_प्रकार offset;
	k_thread_ढेर_t *ढेर;

	अगर (आकार > CONFIG_DYNAMIC_THREAD_ढेर_आकार) {
		LOG_DBG("stack size %zu is > pool stack size %zu", आकार,
			(माप_प्रकार)CONFIG_DYNAMIC_THREAD_ढेर_आकार);
		वापस शून्य;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	अगर (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		वापस शून्य;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_आकार);

	ढेर = (k_thread_ढेर_t *)&dynamic_ढेर[offset];

	वापस ढेर;
}

अटल k_thread_ढेर_t *z_thread_ढेर_alloc_dyn(माप_प्रकार आकार, पूर्णांक flags)
{
	माप_प्रकार ढेर_len;

	अगर ((flags & K_USER) == K_USER) {
#अगर_घोषित CONFIG_DYNAMIC_OBJECTS
		वापस k_object_alloc_आकार(K_OBJ_THREAD_ढेर_ELEMENT, आकार);
#अन्यथा
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		वापस शून्य;
#पूर्ण_अगर /* CONFIG_DYNAMIC_OBJECTS */
	}

	ढेर_len = K_KERNEL_ढेर_LEN(आकार);
	अगर (ढेर_len < आकार) {
		वापस शून्य;
	}

	वापस z_thread_aligned_alloc(Z_KERNEL_ढेर_OBJ_ALIGN, ढेर_len);
}

k_thread_ढेर_t *z_impl_k_thread_ढेर_alloc(माप_प्रकार आकार, पूर्णांक flags)
{
	k_thread_ढेर_t *ढेर = शून्य;

	अगर (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ढेर = z_thread_ढेर_alloc_dyn(आकार, flags);
		अगर (ढेर == शून्य && CONFIG_DYNAMIC_THREAD_POOL_आकार > 0) {
			ढेर = z_thread_ढेर_alloc_pool(आकार, flags);
		}
	} अन्यथा अगर (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		अगर (CONFIG_DYNAMIC_THREAD_POOL_आकार > 0) {
			ढेर = z_thread_ढेर_alloc_pool(आकार, flags);
		}

		अगर ((ढेर == शून्य) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ढेर = z_thread_ढेर_alloc_dyn(आकार, flags);
		}
	}

	वापस ढेर;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत k_thread_ढेर_t *z_vrfy_k_thread_ढेर_alloc(माप_प्रकार आकार, पूर्णांक flags)
{
	वापस z_impl_k_thread_ढेर_alloc(आकार, flags);
}
#समावेश <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

अटल व्योम dyn_cb(स्थिर काष्ठा k_thread *thread, व्योम *user_तथ्य)
{
	काष्ठा dyn_cb_तथ्य *स्थिर तथ्य = (काष्ठा dyn_cb_तथ्य *)user_तथ्य;

	अगर (तथ्य->ढेर == (k_thread_ढेर_t *)thread->ढेर_info.start) {
		__ASSERT(तथ्य->tid == शून्य, "stack %p is associated with more than one thread!",
			 (व्योम *)thread->ढेर_info.start);
		तथ्य->tid = (k_tid_t)thread;
	}
}

पूर्णांक z_impl_k_thread_ढेर_free(k_thread_ढेर_t *ढेर)
{
	काष्ठा dyn_cb_तथ्य तथ्य = {.ढेर = ढेर};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &तथ्य);

	अगर (तथ्य.tid != शून्य) {
		अगर (!z_is_thread_state_set(तथ्य.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", तथ्य.tid);
			वापस -EBUSY;
		}
	}

	अगर (CONFIG_DYNAMIC_THREAD_POOL_आकार > 0) {
		अगर (IS_ARRAY_ELEMENT(dynamic_ढेर, ढेर)) {
			अगर (sys_bitarray_free(&dynamic_ba, 1, ARRAY_सूचकांक(dynamic_ढेर, ढेर))) {
				LOG_ERR("stack %p is not allocated!", ढेर);
				वापस -EINVAL;
			}

			वापस 0;
		}
	}

	अगर (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#अगर_घोषित CONFIG_USERSPACE
		अगर (k_object_find(ढेर)) {
			k_object_free(ढेर);
		} अन्यथा {
			k_free(ढेर);
		}
#अन्यथा
		k_free(ढेर);
#पूर्ण_अगर /* CONFIG_USERSPACE */
	} अन्यथा {
		LOG_DBG("Invalid stack %p", ढेर);
		वापस -EINVAL;
	}

	वापस 0;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_thread_ढेर_free(k_thread_ढेर_t *ढेर)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ढेर, K_OBJ_THREAD_ढेर_ELEMENT));

	वापस z_impl_k_thread_ढेर_free(ढेर);
}
#समावेश <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
