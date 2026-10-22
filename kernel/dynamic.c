/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "kernel_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel/thread_stack.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/bitarray.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/kobject.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ਜੇ CONFIG_DYNAMIC_THREAD_POOL_ਮਾਪ > 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BA_ਮਾਪ CONFIG_DYNAMIC_THREAD_POOL_ਮਾਪ
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BA_ਮਾਪ 1
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

ਬਣਤਰ dyn_cb_ਅੰਕੜੇ {
	k_tid_t tid;
	k_thread_ਢੇਰ_t *ਢੇਰ;
};

ਸਥਿਰ K_THREAD_ਢੇਰ_ARRAY_DEFINE(dynamic_ਢੇਰ, CONFIG_DYNAMIC_THREAD_POOL_ਮਾਪ,
				   K_THREAD_ਢੇਰ_LEN(CONFIG_DYNAMIC_THREAD_ਢੇਰ_ਮਾਪ));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_ਮਾਪ);

ਸਥਿਰ k_thread_ਢੇਰ_t *z_thread_ਢੇਰ_alloc_pool(size_t ਮਾਪ, ਪੂਰਨ flags)
{
	ਪੂਰਨ rv;
	size_t ਵਿੱਥ;
	k_thread_ਢੇਰ_t *ਢੇਰ;

	ਜੇ (ਮਾਪ > CONFIG_DYNAMIC_THREAD_ਢੇਰ_ਮਾਪ) {
		LOG_DBG("stack size %zu is > pool stack size %zu", ਮਾਪ,
			(size_t)CONFIG_DYNAMIC_THREAD_ਢੇਰ_ਮਾਪ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &ਵਿੱਥ);
	ਜੇ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		ਵਾਪਸ ਨਿਰਾ;
	}

	__ASSERT_NO_MSG(ਵਿੱਥ < CONFIG_DYNAMIC_THREAD_POOL_ਮਾਪ);

	ਢੇਰ = (k_thread_ਢੇਰ_t *)&dynamic_ਢੇਰ[ਵਿੱਥ];

	ਵਾਪਸ ਢੇਰ;
}

ਸਥਿਰ k_thread_ਢੇਰ_t *z_thread_ਢੇਰ_alloc_dyn(size_t ਮਾਪ, ਪੂਰਨ flags)
{
	size_t ਢੇਰ_len;

	ਜੇ ((flags & K_USER) == K_USER) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_OBJECTS
		ਵਾਪਸ k_object_alloc_ਮਾਪ(K_OBJ_THREAD_ਢੇਰ_ELEMENT, ਮਾਪ);
#ਨਹੀਂਤਾਂ
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		ਵਾਪਸ ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_OBJECTS */
	}

	ਢੇਰ_len = K_KERNEL_ਢੇਰ_LEN(ਮਾਪ);
	ਜੇ (ਢੇਰ_len < ਮਾਪ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ z_thread_aligned_alloc(Z_KERNEL_ਢੇਰ_OBJ_ALIGN, ਢੇਰ_len);
}

k_thread_ਢੇਰ_t *z_impl_k_thread_ਢੇਰ_alloc(size_t ਮਾਪ, ਪੂਰਨ flags)
{
	k_thread_ਢੇਰ_t *ਢੇਰ = ਨਿਰਾ;

	ਜੇ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ਢੇਰ = z_thread_ਢੇਰ_alloc_dyn(ਮਾਪ, flags);
		ਜੇ (ਢੇਰ == ਨਿਰਾ && CONFIG_DYNAMIC_THREAD_POOL_ਮਾਪ > 0) {
			ਢੇਰ = z_thread_ਢੇਰ_alloc_pool(ਮਾਪ, flags);
		}
	} ਨਹੀਂਤਾਂ ਜੇ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		ਜੇ (CONFIG_DYNAMIC_THREAD_POOL_ਮਾਪ > 0) {
			ਢੇਰ = z_thread_ਢੇਰ_alloc_pool(ਮਾਪ, flags);
		}

		ਜੇ ((ਢੇਰ == ਨਿਰਾ) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ਢੇਰ = z_thread_ਢੇਰ_alloc_dyn(ਮਾਪ, flags);
		}
	}

	ਵਾਪਸ ਢੇਰ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ k_thread_ਢੇਰ_t *z_vrfy_k_thread_ਢੇਰ_alloc(size_t ਮਾਪ, ਪੂਰਨ flags)
{
	ਵਾਪਸ z_impl_k_thread_ਢੇਰ_alloc(ਮਾਪ, flags);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਸਥਿਰ ਖਾਲੀ dyn_cb(ਅਚਲ ਬਣਤਰ k_thread *thread, ਖਾਲੀ *user_ਅੰਕੜੇ)
{
	ਬਣਤਰ dyn_cb_ਅੰਕੜੇ *ਅਚਲ ਅੰਕੜੇ = (ਬਣਤਰ dyn_cb_ਅੰਕੜੇ *)user_ਅੰਕੜੇ;

	ਜੇ (ਅੰਕੜੇ->ਢੇਰ == (k_thread_ਢੇਰ_t *)thread->ਢੇਰ_info.ਸ਼ੁਰੂ) {
		__ASSERT(ਅੰਕੜੇ->tid == ਨਿਰਾ, "stack %p is associated with more than one thread!",
			 (ਖਾਲੀ *)thread->ਢੇਰ_info.ਸ਼ੁਰੂ);
		ਅੰਕੜੇ->tid = (k_tid_t)thread;
	}
}

ਪੂਰਨ z_impl_k_thread_ਢੇਰ_free(k_thread_ਢੇਰ_t *ਢੇਰ)
{
	ਬਣਤਰ dyn_cb_ਅੰਕੜੇ ਅੰਕੜੇ = {.ਢੇਰ = ਢੇਰ};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &ਅੰਕੜੇ);

	ਜੇ (ਅੰਕੜੇ.tid != ਨਿਰਾ) {
		ਜੇ (!z_is_thread_state_ਸੈੱਟ(ਅੰਕੜੇ.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", ਅੰਕੜੇ.tid);
			ਵਾਪਸ -EBUSY;
		}
	}

	ਜੇ (CONFIG_DYNAMIC_THREAD_POOL_ਮਾਪ > 0) {
		ਜੇ (IS_ARRAY_ELEMENT(dynamic_ਢੇਰ, ਢੇਰ)) {
			ਜੇ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_ਸੂਚਕਾਂਕ(dynamic_ਢੇਰ, ਢੇਰ))) {
				LOG_ERR("stack %p is not allocated!", ਢੇਰ);
				ਵਾਪਸ -EINVAL;
			}

			ਵਾਪਸ 0;
		}
	}

	ਜੇ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
		ਜੇ (k_object_ਲੱਭੋ(ਢੇਰ)) {
			k_object_free(ਢੇਰ);
		} ਨਹੀਂਤਾਂ {
			k_free(ਢੇਰ);
		}
#ਨਹੀਂਤਾਂ
		k_free(ਢੇਰ);
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
	} ਨਹੀਂਤਾਂ {
		LOG_DBG("Invalid stack %p", ਢੇਰ);
		ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_thread_ਢੇਰ_free(k_thread_ਢੇਰ_t *ਢੇਰ)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ਢੇਰ, K_OBJ_THREAD_ਢੇਰ_ELEMENT));

	ਵਾਪਸ z_impl_k_thread_ਢੇਰ_free(ਢੇਰ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
