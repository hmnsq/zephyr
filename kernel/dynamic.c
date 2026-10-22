/*
 * Copyright (c) 2022, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ "kernel_internal.h"

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/kernel/thread_stack.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
#અન્તર્ભાવ <zephyr/sys/bitarray.h>
#અન્તર્ભાવ <zephyr/sys/kobject.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#જો CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0
#define BA_SIZE CONFIG_DYNAMIC_THREAD_POOL_SIZE
#નહીંતર
#define BA_SIZE 1
#endif /* CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0 */

સંરચના dyn_cb_data {
	k_tid_t tid;
	k_thread_ઢગલો_t *ઢગલો;
};

સ્થિર K_THREAD_ઢગલો_ARRAY_DEFINE(dynamic_ઢગલો, CONFIG_DYNAMIC_THREAD_POOL_SIZE,
				   K_THREAD_ઢગલો_LEN(CONFIG_DYNAMIC_THREAD_ઢગલો_SIZE));
SYS_BITARRAY_DEFINE_STATIC(dynamic_ba, BA_SIZE);

સ્થિર k_thread_ઢગલો_t *z_thread_ઢગલો_alloc_pool(size_t size, પૂર્ણાંક flags)
{
	પૂર્ણાંક rv;
	size_t offset;
	k_thread_ઢગલો_t *ઢગલો;

	જો (size > CONFIG_DYNAMIC_THREAD_ઢગલો_SIZE) {
		LOG_DBG("stack size %zu is > pool stack size %zu", size,
			(size_t)CONFIG_DYNAMIC_THREAD_ઢગલો_SIZE);
		પાછા ખાલી;
	}

	rv = sys_bitarray_alloc(&dynamic_ba, 1, &offset);
	જો (rv < 0) {
		LOG_DBG("unable to allocate stack from pool");
		પાછા ખાલી;
	}

	__ASSERT_NO_MSG(offset < CONFIG_DYNAMIC_THREAD_POOL_SIZE);

	ઢગલો = (k_thread_ઢગલો_t *)&dynamic_ઢગલો[offset];

	પાછા ઢગલો;
}

સ્થિર k_thread_ઢગલો_t *z_thread_ઢગલો_alloc_dyn(size_t size, પૂર્ણાંક flags)
{
	size_t ઢગલો_len;

	જો ((flags & K_USER) == K_USER) {
#ifdef CONFIG_DYNAMIC_OBJECTS
		પાછા k_object_alloc_size(K_OBJ_THREAD_ઢગલો_ELEMENT, size);
#નહીંતર
		/* Dynamic user stack needs a kobject, so if this option is not
		 * enabled we can't proceed.
		 */
		પાછા ખાલી;
#endif /* CONFIG_DYNAMIC_OBJECTS */
	}

	ઢગલો_len = K_KERNEL_ઢગલો_LEN(size);
	જો (ઢગલો_len < size) {
		પાછા ખાલી;
	}

	પાછા z_thread_aligned_alloc(Z_KERNEL_ઢગલો_OBJ_ALIGN, ઢગલો_len);
}

k_thread_ઢગલો_t *z_impl_k_thread_ઢગલો_alloc(size_t size, પૂર્ણાંક flags)
{
	k_thread_ઢગલો_t *ઢગલો = ખાલી;

	જો (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_ALLOC)) {
		ઢગલો = z_thread_ઢગલો_alloc_dyn(size, flags);
		જો (ઢગલો == ખાલી && CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			ઢગલો = z_thread_ઢગલો_alloc_pool(size, flags);
		}
	} નહીંતર જો (IS_ENABLED(CONFIG_DYNAMIC_THREAD_PREFER_POOL)) {
		જો (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
			ઢગલો = z_thread_ઢગલો_alloc_pool(size, flags);
		}

		જો ((ઢગલો == ખાલી) && IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
			ઢગલો = z_thread_ઢગલો_alloc_dyn(size, flags);
		}
	}

	પાછા ઢગલો;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ k_thread_ઢગલો_t *z_vrfy_k_thread_ઢગલો_alloc(size_t size, પૂર્ણાંક flags)
{
	પાછા z_impl_k_thread_ઢગલો_alloc(size, flags);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_stack_alloc_mrsh.c>
#endif /* CONFIG_USERSPACE */

સ્થિર શૂન્ય dyn_cb(અચલ સંરચના k_thread *thread, શૂન્ય *user_data)
{
	સંરચના dyn_cb_data *અચલ data = (સંરચના dyn_cb_data *)user_data;

	જો (data->ઢગલો == (k_thread_ઢગલો_t *)thread->ઢગલો_info.start) {
		__ASSERT(data->tid == ખાલી, "stack %p is associated with more than one thread!",
			 (શૂન્ય *)thread->ઢગલો_info.start);
		data->tid = (k_tid_t)thread;
	}
}

પૂર્ણાંક z_impl_k_thread_ઢગલો_free(k_thread_ઢગલો_t *ઢગલો)
{
	સંરચના dyn_cb_data data = {.ઢગલો = ઢગલો};

	/* Get a possible tid associated with stack */
	k_thread_foreach(dyn_cb, &data);

	જો (data.tid != ખાલી) {
		જો (!z_is_thread_state_set(data.tid, _THREAD_DUMMY | _THREAD_DEAD)) {
			LOG_ERR("tid %p is in use!", data.tid);
			પાછા -EBUSY;
		}
	}

	જો (CONFIG_DYNAMIC_THREAD_POOL_SIZE > 0) {
		જો (IS_ARRAY_ELEMENT(dynamic_ઢગલો, ઢગલો)) {
			જો (sys_bitarray_free(&dynamic_ba, 1, ARRAY_INDEX(dynamic_ઢગલો, ઢગલો))) {
				LOG_ERR("stack %p is not allocated!", ઢગલો);
				પાછા -EINVAL;
			}

			પાછા 0;
		}
	}

	જો (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
#ifdef CONFIG_USERSPACE
		જો (k_object_find(ઢગલો)) {
			k_object_free(ઢગલો);
		} નહીંતર {
			k_free(ઢગલો);
		}
#નહીંતર
		k_free(ઢગલો);
#endif /* CONFIG_USERSPACE */
	} નહીંતર {
		LOG_DBG("Invalid stack %p", ઢગલો);
		પાછા -EINVAL;
	}

	પાછા 0;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_thread_ઢગલો_free(k_thread_ઢગલો_t *ઢગલો)
{
	/* The thread stack object must not be in initialized state.
	 *
	 * Thread stack objects are initialized when the thread is created
	 * and de-initialized when the thread is destroyed. Since we can't
	 * free a stack that is in use, we have to check that the caller
	 * has access to the object but that it is not in use anymore.
	 */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ઢગલો, K_OBJ_THREAD_ઢગલો_ELEMENT));

	પાછા z_impl_k_thread_ઢગલો_free(ઢગલો);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_stack_free_mrsh.c>
#endif /* CONFIG_USERSPACE */
