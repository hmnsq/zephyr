/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ "kernel_internal.h"

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/kernel/thread_stack.h>
#అన్తర్భావ <zephyr/logging/log.h>
#అన్తర్భావ <zephyr/sys/bitarray.h>
#అన్తర్భావ <zephyr/sys/kobject.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ఒకవేళ CONFIG_DYNAMIC_THREAD_POOL_కొలత > 0
#నిర్వచించు BA_కొలత CONFIG_DYNAMIC_THREAD_POOL_కొలత
#లేకపోతే
#నిర్వచించు BA_కొలత 1
#అయితే_ముగింపు /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

నిర్మాణం dyn_cb_దత్తాంశం {
	k_tid_t tid;
	k_thread_దొంతర_t *దొంతర;
};

స్థిర K_THREAD_దొంతర_ARRAY_DEFINE(dynamic_దొంతర, CONFIG_DYNAMIC_THREAD_POOL_కొలత,
				   K_THREAD_దొంతర_LEN(CONFIG_DYNAMIC_THREAD_దొంతర_కొలత));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_కొలత);

స్థిర k_thread_దొంతర_t *z_thread_దొంతర_alloc_pool(size_t కొలత, పూర్ణాంకం flags)
{
	పూర్ణాంకం rv;
	size_t స్థానభ్రంశం;
	k_thread_దొంతర_t *దొంతర;

	ఒకవేళ (కొలత > CONFIG_DYNAMIC_THREAD_దొంతర_కొలత) {
		LOG_DBG("stack size %zu is > pool stack size %zu", కొలత,
			(size_t)CONFIG_DYNAMIC_THREAD_దొంతర_కొలత);
		తిరిగి ఖాళీ;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &స్థానభ్రంశం);
	ఒకవేళ (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		తిరిగి ఖాళీ;
	}

	__ASSERT_NO_MSG(స్థానభ్రంశం < CONFIG_DYNAMIC_THREAD_POOL_కొలత);

	దొంతర = (k_thread_దొంతర_t *)&dynamic_దొంతర[స్థానభ్రంశం];

	తిరిగి దొంతర;
}

స్థిర k_thread_దొంతర_t *z_thread_దొంతర_alloc_dyn(size_t కొలత, పూర్ణాంకం flags)
{
	size_t దొంతర_len;

	ఒకవేళ ((flags & K_USER) == K_USER) {
#నిర్వచితమైతే CONFIG_DYNAMIC_OBJECTS
		తిరిగి k_object_alloc_కొలత(K_OBJ_THREAD_దొంతర_ELEMENT, కొలత);
#లేకపోతే
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		తిరిగి ఖాళీ;
#అయితే_ముగింపు /* CONFIG_DYNAMIC_OBJECTS */
	}

	దొంతర_len = K_KERNEL_దొంతర_LEN(కొలత);
	ఒకవేళ (దొంతర_len < కొలత) {
		తిరిగి ఖాళీ;
	}

	తిరిగి z_thread_aligned_alloc(Z_KERNEL_దొంతర_OBJ_ALIGN, దొంతర_len);
}

k_thread_దొంతర_t *z_impl_k_thread_దొంతర_alloc(size_t కొలత, పూర్ణాంకం flags)
{
	k_thread_దొంతర_t *దొంతర = ఖాళీ;

	ఒకవేళ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		దొంతర = z_thread_దొంతర_alloc_dyn(కొలత, flags);
		ఒకవేళ (దొంతర == ఖాళీ && CONFIG_DYNAMIC_THREAD_POOL_కొలత > 0) {
			దొంతర = z_thread_దొంతర_alloc_pool(కొలత, flags);
		}
	} లేకపోతే ఒకవేళ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		ఒకవేళ (CONFIG_DYNAMIC_THREAD_POOL_కొలత > 0) {
			దొంతర = z_thread_దొంతర_alloc_pool(కొలత, flags);
		}

		ఒకవేళ ((దొంతర == ఖాళీ) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			దొంతర = z_thread_దొంతర_alloc_dyn(కొలత, flags);
		}
	}

	తిరిగి దొంతర;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస k_thread_దొంతర_t *z_vrfy_k_thread_దొంతర_alloc(size_t కొలత, పూర్ణాంకం flags)
{
	తిరిగి z_impl_k_thread_దొంతర_alloc(కొలత, flags);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

స్థిర శూన్య dyn_cb(స్థిరరాశి నిర్మాణం k_thread *thread, శూన్య *user_దత్తాంశం)
{
	నిర్మాణం dyn_cb_దత్తాంశం *స్థిరరాశి దత్తాంశం = (నిర్మాణం dyn_cb_దత్తాంశం *)user_దత్తాంశం;

	ఒకవేళ (దత్తాంశం->దొంతర == (k_thread_దొంతర_t *)thread->దొంతర_info.ప్రారంభం) {
		__ASSERT(దత్తాంశం->tid == ఖాళీ, "stack %p is associated with more than one thread!",
			 (శూన్య *)thread->దొంతర_info.ప్రారంభం);
		దత్తాంశం->tid = (k_tid_t)thread;
	}
}

పూర్ణాంకం z_impl_k_thread_దొంతర_free(k_thread_దొంతర_t *దొంతర)
{
	నిర్మాణం dyn_cb_దత్తాంశం దత్తాంశం = {.దొంతర = దొంతర};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &దత్తాంశం);

	ఒకవేళ (దత్తాంశం.tid != ఖాళీ) {
		ఒకవేళ (!z_is_thread_state_సమితి(దత్తాంశం.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", దత్తాంశం.tid);
			తిరిగి -EBUSY;
		}
	}

	ఒకవేళ (CONFIG_DYNAMIC_THREAD_POOL_కొలత > 0) {
		ఒకవేళ (IS_ARRAY_ELEMENT(dynamic_దొంతర, దొంతర)) {
			ఒకవేళ (sys_bitarray_free(&dynamic_ba, 1, ARRAY_సూచీ(dynamic_దొంతర, దొంతర))) {
				LOG_ERR("stack %p is not allocated!", దొంతర);
				తిరిగి -EINVAL;
			}

			తిరిగి 0;
		}
	}

	ఒకవేళ (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#నిర్వచితమైతే CONFIG_USERSPACE
		ఒకవేళ (k_object_కనుగొను(దొంతర)) {
			k_object_free(దొంతర);
		} లేకపోతే {
			k_free(దొంతర);
		}
#లేకపోతే
		k_free(దొంతర);
#అయితే_ముగింపు /* CONFIG_USERSPACE */
	} లేకపోతే {
		LOG_DBG("Invalid stack %p", దొంతర);
		తిరిగి -EINVAL;
	}

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_thread_దొంతర_free(k_thread_దొంతర_t *దొంతర)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(దొంతర, K_OBJ_THREAD_దొంతర_ELEMENT));

	తిరిగి z_impl_k_thread_దొంతర_free(దొంతర);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
