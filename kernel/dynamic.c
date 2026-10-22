/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ "kernel_internal.h"

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel/thread_stack.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/bitarray.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/kobject.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ಯದಿ CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#ಅನ್ಯಥಾ
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

ಸಂರಚನಾ dyn_cb_data {
	k_tid_t tid;
	k_thread_ರಾಶಿ_t *ರಾಶಿ;
};

ಸ್ಥಿರ K_THREAD_ರಾಶಿ_ARRAY_DEFINE(dynamic_ರಾಶಿ, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_ರಾಶಿ_LEN(CONFIG_DYNAMIC_THREAD_ರಾಶಿ_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

ಸ್ಥಿರ k_thread_ರಾಶಿ_t *z_thread_ರಾಶಿ_alloc_pool(size_t size, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪೂರ್ಣಾಙ್ಕ rv;
	size_t offset;
	k_thread_ರಾಶಿ_t *ರಾಶಿ;

	ಯದಿ (size > CONFIG_DYNAMIC_THREAD_ರಾಶಿ_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_ರಾಶಿ_SIZE);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	ಯದಿ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	ರಾಶಿ = (k_thread_ರಾಶಿ_t *)&dynamic_ರಾಶಿ[offset];

	ಪ್ರತ್ಯಾಗಮ ರಾಶಿ;
}

ಸ್ಥಿರ k_thread_ರಾಶಿ_t *z_thread_ರಾಶಿ_alloc_dyn(size_t size, ಪೂರ್ಣಾಙ್ಕ flags)
{
	size_t ರಾಶಿ_len;

	ಯದಿ ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		ಪ್ರತ್ಯಾಗಮ k_object_alloc_size(K_OBJ_THREAD_ರಾಶಿ_ELEMENT, size);
#ಅನ್ಯಥಾ
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	ರಾಶಿ_len = K_KERNEL_ರಾಶಿ_LEN(size);
	ಯದಿ (ರಾಶಿ_len < size) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ z_thread_aligned_alloc(Z_KERNEL_ರಾಶಿ_OBJ_ALIGN, ರಾಶಿ_len);
}

k_thread_ರಾಶಿ_t *z_impl_k_thread_ರಾಶಿ_alloc(size_t size, ಪೂರ್ಣಾಙ್ಕ flags)
{
	k_thread_ರಾಶಿ_t *ರಾಶಿ = ರಿಕ್ತ;

	ಯದಿ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ರಾಶಿ = z_thread_ರಾಶಿ_alloc_dyn(size, flags);
		ಯದಿ (ರಾಶಿ == ರಿಕ್ತ && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			ರಾಶಿ = z_thread_ರಾಶಿ_alloc_pool(size, flags);
		}
	} ಅನ್ಯಥಾ ಯದಿ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		ಯದಿ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			ರಾಶಿ = z_thread_ರಾಶಿ_alloc_pool(size, flags);
		}

		ಯದಿ ((ರಾಶಿ == ರಿಕ್ತ) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ರಾಶಿ = z_thread_ರಾಶಿ_alloc_dyn(size, flags);
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಾಶಿ;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_thread_ರಾಶಿ_t *z_vrfy_k_thread_ರಾಶಿ_alloc(size_t size, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_ರಾಶಿ_alloc(size, flags);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಶೂನ್ಯ dyn_cb(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread, ಶೂನ್ಯ *user_data)
{
	ಸಂರಚನಾ dyn_cb_data *ಧ್ರುವ data = (ಸಂರಚನಾ dyn_cb_data *)user_data;

	ಯದಿ (data->ರಾಶಿ == (k_thread_ರಾಶಿ_t *)thread->ರಾಶಿ_info.start) {
		__ASSERT(data->tid == ರಿಕ್ತ, "stack %p is associated with more than one thread!",
			 (ಶೂನ್ಯ *)thread->ರಾಶಿ_info.start);
		data->tid = (k_tid_t)thread;
	}
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ಸಂರಚನಾ dyn_cb_data data = {.ರಾಶಿ = ರಾಶಿ};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	ಯದಿ (data.tid != ರಿಕ್ತ) {
		ಯದಿ (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			ಪ್ರತ್ಯಾಗಮ -EBUSY;
		}
	}

	ಯದಿ (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		ಯದಿ (IS_ARRAY_ELEMENT(dynamic_ರಾಶಿ, ರಾಶಿ)) {
			ಯದಿ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_ರಾಶಿ, ರಾಶಿ))) {
				LOG_ERR("stack %p is not allocated!", ರಾಶಿ);
				ಪ್ರತ್ಯಾಗಮ -EINVAL;
			}

			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}

	ಯದಿ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		ಯದಿ (k_object_find(ರಾಶಿ)) {
			k_object_free(ರಾಶಿ);
		} ಅನ್ಯಥಾ {
			k_free(ರಾಶಿ);
		}
#ಅನ್ಯಥಾ
		k_free(ರಾಶಿ);
#endif /* CONFIG_USERSPACE */
	} ಅನ್ಯಥಾ {
		LOG_DBG("Invalid stack %p", ರಾಶಿ);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ರಾಶಿ, K_OBJ_THREAD_ರಾಶಿ_ELEMENT));

	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_ರಾಶಿ_free(ರಾಶಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
