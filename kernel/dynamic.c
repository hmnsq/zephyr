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

#ಆದರೆ CONFIG_DYNAMIC_THREAD_POOL_ಗಾತ್ರ > 0
#ವ್ಯಾಖ್ಯಾನಿಸು BA_ಗಾತ್ರ CONFIG_DYNAMIC_THREAD_POOL_ಗಾತ್ರ
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು BA_ಗಾತ್ರ 1
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

ರಚನೆ dyn_cb_ದತ್ತಾಂಶ {
	k_tid_t tid;
	k_thread_ರಾಶಿ_t *ರಾಶಿ;
};

ಸ್ಥಿರ K_THREAD_ರಾಶಿ_ARRAY_DEFINE(dynamic_ರಾಶಿ, CONFIG_DYNAMIC_THREAD_POOL_ಗಾತ್ರ,
				   K_THREAD_ರಾಶಿ_LEN(CONFIG_DYNAMIC_THREAD_ರಾಶಿ_ಗಾತ್ರ));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_ಗಾತ್ರ);

ಸ್ಥಿರ k_thread_ರಾಶಿ_t *z_thread_ರಾಶಿ_alloc_pool(size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ flags)
{
	ಪೂರ್ಣಾಂಕ rv;
	size_t ಸ್ಥಾನಾಂತರ;
	k_thread_ರಾಶಿ_t *ರಾಶಿ;

	ಆದರೆ (ಗಾತ್ರ_2 > CONFIG_DYNAMIC_THREAD_ರಾಶಿ_ಗಾತ್ರ) {
		LOG_DBG("stack size %zu is > pool stack size %zu", ಗಾತ್ರ_2,
			(size_t)CONFIG_DYNAMIC_THREAD_ರಾಶಿ_ಗಾತ್ರ);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &ಸ್ಥಾನಾಂತರ);
	ಆದರೆ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	__ASSERT_NO_MSG(ಸ್ಥಾನಾಂತರ < CONFIG_DYNAMIC_THREAD_POOL_ಗಾತ್ರ);

	ರಾಶಿ = (k_thread_ರಾಶಿ_t *)&dynamic_ರಾಶಿ[ಸ್ಥಾನಾಂತರ];

	ಹಿಂತಿರುಗಿ ರಾಶಿ;
}

ಸ್ಥಿರ k_thread_ರಾಶಿ_t *z_thread_ರಾಶಿ_alloc_dyn(size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ flags)
{
	size_t ರಾಶಿ_len;

	ಆದರೆ ((flags & K_USER) == K_USER) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DYNAMIC_OBJECTS
		ಹಿಂತಿರುಗಿ k_object_alloc_ಗಾತ್ರ(K_OBJ_THREAD_ರಾಶಿ_ELEMENT, ಗಾತ್ರ_2);
#ಇಲ್ಲದಿದ್ದರೆ
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_OBJECTS */
	}

	ರಾಶಿ_len = K_KERNEL_ರಾಶಿ_LEN(ಗಾತ್ರ_2);
	ಆದರೆ (ರಾಶಿ_len < ಗಾತ್ರ_2) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ z_thread_aligned_alloc(Z_KERNEL_ರಾಶಿ_OBJ_ALIGN, ರಾಶಿ_len);
}

k_thread_ರಾಶಿ_t *z_impl_k_thread_ರಾಶಿ_alloc(size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ flags)
{
	k_thread_ರಾಶಿ_t *ರಾಶಿ = ಖಾಲಿ;

	ಆದರೆ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ರಾಶಿ = z_thread_ರಾಶಿ_alloc_dyn(ಗಾತ್ರ_2, flags);
		ಆದರೆ (ರಾಶಿ == ಖಾಲಿ && CONFIG_DYNAMIC_THREAD_POOL_ಗಾತ್ರ > 0) {
			ರಾಶಿ = z_thread_ರಾಶಿ_alloc_pool(ಗಾತ್ರ_2, flags);
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		ಆದರೆ (CONFIG_DYNAMIC_THREAD_POOL_ಗಾತ್ರ > 0) {
			ರಾಶಿ = z_thread_ರಾಶಿ_alloc_pool(ಗಾತ್ರ_2, flags);
		}

		ಆದರೆ ((ರಾಶಿ == ಖಾಲಿ) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ರಾಶಿ = z_thread_ರಾಶಿ_alloc_dyn(ಗಾತ್ರ_2, flags);
		}
	}

	ಹಿಂತಿರುಗಿ ರಾಶಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_thread_ರಾಶಿ_t *z_vrfy_k_thread_ರಾಶಿ_alloc(size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ flags)
{
	ಹಿಂತಿರುಗಿ z_impl_k_thread_ರಾಶಿ_alloc(ಗಾತ್ರ_2, flags);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಶೂನ್ಯ dyn_cb(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread, ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
	ರಚನೆ dyn_cb_ದತ್ತಾಂಶ *ಸ್ಥಿರಾಂಕ ದತ್ತಾಂಶ = (ರಚನೆ dyn_cb_ದತ್ತಾಂಶ *)user_ದತ್ತಾಂಶ;

	ಆದರೆ (ದತ್ತಾಂಶ->ರಾಶಿ == (k_thread_ರಾಶಿ_t *)thread->ರಾಶಿ_info.ಆರಂಭ) {
		__ASSERT(ದತ್ತಾಂಶ->tid == ಖಾಲಿ, "stack %p is associated with more than one thread!",
			 (ಶೂನ್ಯ *)thread->ರಾಶಿ_info.ಆರಂಭ);
		ದತ್ತಾಂಶ->tid = (k_tid_t)thread;
	}
}

ಪೂರ್ಣಾಂಕ z_impl_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ರಚನೆ dyn_cb_ದತ್ತಾಂಶ ದತ್ತಾಂಶ = {.ರಾಶಿ = ರಾಶಿ};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &ದತ್ತಾಂಶ);

	ಆದರೆ (ದತ್ತಾಂಶ.tid != ಖಾಲಿ) {
		ಆದರೆ (!z_is_thread_state_ಗಣ(ದತ್ತಾಂಶ.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", ದತ್ತಾಂಶ.tid);
			ಹಿಂತಿರುಗಿ -EBUSY;
		}
	}

	ಆದರೆ (CONFIG_DYNAMIC_THREAD_POOL_ಗಾತ್ರ > 0) {
		ಆದರೆ (IS_ARRAY_ELEMENT(dynamic_ರಾಶಿ, ರಾಶಿ)) {
			ಆದರೆ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_ಸೂಚ್ಯಂಕ(dynamic_ರಾಶಿ, ರಾಶಿ))) {
				LOG_ERR("stack %p is not allocated!", ರಾಶಿ);
				ಹಿಂತಿರುಗಿ -EINVAL;
			}

			ಹಿಂತಿರುಗಿ 0;
		}
	}

	ಆದರೆ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
		ಆದರೆ (k_object_ಕಂಡುಹಿಡಿ(ರಾಶಿ)) {
			k_object_free(ರಾಶಿ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			k_free(ರಾಶಿ);
		}
#ಇಲ್ಲದಿದ್ದರೆ
		k_free(ರಾಶಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
	} ಇಲ್ಲದಿದ್ದರೆ {
		LOG_DBG("Invalid stack %p", ರಾಶಿ);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_ರಾಶಿ_free(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ರಾಶಿ, K_OBJ_THREAD_ರಾಶಿ_ELEMENT));

	ಹಿಂತಿರುಗಿ z_impl_k_thread_ರಾಶಿ_free(ರಾಶಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
