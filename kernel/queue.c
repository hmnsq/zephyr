/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief dynamic-size QUEUE object.
 */


#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_ਬਿੱਟ >= 1, "k_queue needs one sflist flag bit");

ਬਣਤਰ alloc_ਗੰਢ {
	sys_sfnode_t ਗੰਢ;
	ਖਾਲੀ *ਅੰਕੜੇ;
};

/* The queue must have its spinlock held before calling this function. */
ਸਥਿਰ ਖਾਲੀ *z_ਕਤਾਰ_ਗੰਢ_peek(sys_sfnode_t *ਗੰਢ, bool needs_free)
{
	ਖਾਲੀ *ret;

	ਜੇ ((ਗੰਢ != ਨਿਰਾ) && (sys_sfnode_flags_get(ਗੰਢ) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		ਬਣਤਰ alloc_ਗੰਢ *anode;

		anode = CONTAINER_ਦਾ(ਗੰਢ, ਬਣਤਰ alloc_ਗੰਢ, ਗੰਢ);
		ret = anode->ਅੰਕੜੇ;
		ਜੇ (needs_free) {
			k_free(anode);
		}
	} ਨਹੀਂਤਾਂ {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (ਖਾਲੀ *)ਗੰਢ;
	}

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_ਕਤਾਰ
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(ਬਣਤਰ k_fifo, ਕਤਾਰ_2) == 0);
BUILD_ASSERT(offsetof(ਬਣਤਰ k_lifo, ਕਤਾਰ_2) == 0);
STRUCT_SECTION_ਸ਼ੁਰੂ_EXTERN(k_fifo);
STRUCT_SECTION_ਸਿਰਾ_EXTERN(k_fifo);
STRUCT_SECTION_ਸ਼ੁਰੂ_EXTERN(k_lifo);
STRUCT_SECTION_ਸਿਰਾ_EXTERN(k_lifo);
STRUCT_SECTION_ਸ਼ੁਰੂ_EXTERN(k_ਕਤਾਰ);
STRUCT_SECTION_ਸਿਰਾ_EXTERN(k_ਕਤਾਰ);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_ਕਤਾਰ, k_ਕਤਾਰ, K_OBJ_TYPE_ਕਤਾਰ_ID, ਨਿਰਾ,
			 K_OBJ_RANGE_SECTION(k_ਕਤਾਰ), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_QUEUE */

ਖਾਲੀ z_impl_k_ਕਤਾਰ_init(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	sys_sflist_init(&ਕਤਾਰ->ਅੰਕੜੇ_q);
	ਕਤਾਰ->ਤਾਲਾ = (ਬਣਤਰ k_spinlock) {};
	z_waitq_init(&ਕਤਾਰ->wait_q);
#ਜੇ defined(CONFIG_POLL)
	sys_dlist_init(&ਕਤਾਰ->poll_events);
#ਜੇ_ਅੰਤ

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_ਕਤਾਰ, ਕਤਾਰ);

	k_object_init(ਕਤਾਰ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_ਕਤਾਰ
	k_obj_core_init_and_link(K_OBJ_CORE(ਕਤਾਰ), &obj_type_ਕਤਾਰ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_QUEUE */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_ਕਤਾਰ_init(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	z_impl_k_ਕਤਾਰ_init(ਕਤਾਰ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਸਥਿਰ ਸਤਰਅੰਦਰ bool ਕਤਾਰ_handle_poll_events(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, uint32_t state)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POLL
	ਵਾਪਸ z_handle_obj_poll_events(&ਕਤਾਰ->poll_events, state);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(ਕਤਾਰ);
	ARG_ਅਣਵਰਤਿਆ(state);

	ਵਾਪਸ false;
#ਜੇ_ਅੰਤ /* CONFIG_POLL */
}

ਖਾਲੀ z_impl_k_ਕਤਾਰ_cancel_wait(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_ਕਤਾਰ, cancel_wait, ਕਤਾਰ);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ);
	bool resched = false;

	ਜੇ (z_sched_wake(&ਕਤਾਰ->wait_q, 0, ਨਿਰਾ)) {
		resched = true;
	}

	resched = ਕਤਾਰ_handle_poll_events(ਕਤਾਰ, K_POLL_STATE_CANCELLED) || resched;

	ਜੇ (resched) {
		z_reschedule(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_ਕਤਾਰ_cancel_wait(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	K_OOPS(K_SYSCALL_OBJ(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	z_impl_k_ਕਤਾਰ_cancel_wait(ਕਤਾਰ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ਸਥਿਰ int32_t ਕਤਾਰ_ਪਾਓ(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *prev, ਖਾਲੀ *ਅੰਕੜੇ,
			    bool alloc, bool is_append, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	int32_t ਨਤੀਜਾ = 0;
	bool resched = false;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, ਕਤਾਰ_ਪਾਓ, ਕਤਾਰ, alloc);

	ਜੇ (is_append) {
		prev = sys_sflist_peek_ਪੂਛ(&ਕਤਾਰ->ਅੰਕੜੇ_q);
	}

	ਜੇ (z_sched_wake(&ਕਤਾਰ->wait_q, 0, ਅੰਕੜੇ)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_ਕਤਾਰ, ਕਤਾਰ_ਪਾਓ, ਕਤਾਰ, alloc, K_FOREVER);
		resched = true;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/* Only need to actually allocate if no threads are pending */
	ਜੇ (alloc) {
		ਬਣਤਰ alloc_ਗੰਢ *anode;

		anode = z_thread_malloc(ਆਕਾਰ(*anode));
		ਜੇ (anode == ਨਿਰਾ) {
			ਨਤੀਜਾ = -ENOMEM;
			ਜਾਓ ਬਾਹਰਲਾ;
		}
		anode->ਅੰਕੜੇ = ਅੰਕੜੇ;
		sys_sfnode_init(&anode->ਗੰਢ, 0x1);
		ਅੰਕੜੇ = anode;
	} ਨਹੀਂਤਾਂ {
		sys_sfnode_init(ਅੰਕੜੇ, 0x0);
	}

	sys_sflist_ਪਾਓ(&ਕਤਾਰ->ਅੰਕੜੇ_q, prev, ਅੰਕੜੇ);
	resched = ਕਤਾਰ_handle_poll_events(ਕਤਾਰ, K_POLL_STATE_ਅੰਕੜੇ_ਉਪਲਬਧ);

ਬਾਹਰਲਾ:
	ਜੇ (resched) {
		z_reschedule(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, ਕਤਾਰ_ਪਾਓ, ਕਤਾਰ, alloc, ਨਤੀਜਾ);

	ਵਾਪਸ ਨਤੀਜਾ;
}

ਖਾਲੀ k_ਕਤਾਰ_ਪਾਓ(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *prev, ਖਾਲੀ *ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, ਪਾਓ, ਕਤਾਰ);

	(ਖਾਲੀ)ਕਤਾਰ_ਪਾਓ(ਕਤਾਰ, prev, ਅੰਕੜੇ, false, false,
			   k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ));

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, ਪਾਓ, ਕਤਾਰ);
}

ਖਾਲੀ k_ਕਤਾਰ_append(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, append, ਕਤਾਰ);

	(ਖਾਲੀ)ਕਤਾਰ_ਪਾਓ(ਕਤਾਰ, ਨਿਰਾ, ਅੰਕੜੇ, false, true,
			   k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ));

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, append, ਕਤਾਰ);
}

ਖਾਲੀ k_ਕਤਾਰ_prepend(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, prepend, ਕਤਾਰ);

	(ਖਾਲੀ)ਕਤਾਰ_ਪਾਓ(ਕਤਾਰ, ਨਿਰਾ, ਅੰਕੜੇ, false, false,
			   k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ));

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, prepend, ਕਤਾਰ);
}

int32_t z_impl_k_ਕਤਾਰ_alloc_append(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, alloc_append, ਕਤਾਰ);

	int32_t ret = ਕਤਾਰ_ਪਾਓ(ਕਤਾਰ, ਨਿਰਾ, ਅੰਕੜੇ, true, true,
				   k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ));

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, alloc_append, ਕਤਾਰ, ret);

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ int32_t z_vrfy_k_ਕਤਾਰ_alloc_append(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ,
						  ਖਾਲੀ *ਅੰਕੜੇ)
{
	K_OOPS(K_SYSCALL_OBJ(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	ਵਾਪਸ z_impl_k_ਕਤਾਰ_alloc_append(ਕਤਾਰ, ਅੰਕੜੇ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

int32_t z_impl_k_ਕਤਾਰ_alloc_prepend(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, alloc_prepend, ਕਤਾਰ);

	int32_t ret = ਕਤਾਰ_ਪਾਓ(ਕਤਾਰ, ਨਿਰਾ, ਅੰਕੜੇ, true, false,
				   k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ));

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, alloc_prepend, ਕਤਾਰ, ret);

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ int32_t z_vrfy_k_ਕਤਾਰ_alloc_prepend(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ,
						   ਖਾਲੀ *ਅੰਕੜੇ)
{
	K_OOPS(K_SYSCALL_OBJ(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	ਵਾਪਸ z_impl_k_ਕਤਾਰ_alloc_prepend(ਕਤਾਰ, ਅੰਕੜੇ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ k_ਕਤਾਰ_append_list(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *ਸਿਰ, ਖਾਲੀ *ਪੂਛ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, append_list, ਕਤਾਰ);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((ਸਿਰ == ਨਿਰਾ) || (ਪੂਛ == ਨਿਰਾ)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, append_list, ਕਤਾਰ, -EINVAL);

		ਵਾਪਸ -EINVAL;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ);

	ਜਦੋਂ ((ਸਿਰ != ਨਿਰਾ) && z_sched_wake(&ਕਤਾਰ->wait_q, 0, ਸਿਰ)) {
		resched = true;
		ਸਿਰ = *(ਖਾਲੀ **)ਸਿਰ;
	}

	ਜੇ (ਸਿਰ != ਨਿਰਾ) {
		sys_sflist_append_list(&ਕਤਾਰ->ਅੰਕੜੇ_q, ਸਿਰ, ਪੂਛ);

		resched = ਕਤਾਰ_handle_poll_events(ਕਤਾਰ, K_POLL_STATE_ਅੰਕੜੇ_ਉਪਲਬਧ) || resched;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, append_list, ਕਤਾਰ, 0);

	ਜੇ (resched) {
		z_reschedule(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_ਕਤਾਰ_ਮਿਲਾਓ_slist(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, sys_slist_t *list)
{
	ਪੂਰਨ ret;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, ਮਿਲਾਓ_slist, ਕਤਾਰ);

	/* list must not be empty */
	CHECKIF(sys_slist_is_ਖਾਲੀ_ਥਾਵਾਂ(list)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, ਮਿਲਾਓ_slist, ਕਤਾਰ, -EINVAL);

		ਵਾਪਸ -EINVAL;
	}

	/*
	 * note: this works as long as:
	 * - the slist implementation keeps the next pointer as the first
	 *   field of the node object type
	 * - list->tail->next = NULL.
	 * - sflist implementation only differs from slist by stuffing
	 *   flag bytes in the lower order bits of the data pointer
	 * - source list is really an slist and not an sflist with flags set
	 */
	ret = k_ਕਤਾਰ_append_list(ਕਤਾਰ, list->ਸਿਰ, list->ਪੂਛ);
	CHECKIF(ret != 0) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, ਮਿਲਾਓ_slist, ਕਤਾਰ, ret);

		ਵਾਪਸ ret;
	}
	sys_slist_init(list);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, ਮਿਲਾਓ_slist, ਕਤਾਰ, 0);

	ਵਾਪਸ 0;
}

ਖਾਲੀ *z_impl_k_ਕਤਾਰ_get(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, k_timeout_t timeout)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ);
	ਖਾਲੀ *ਅੰਕੜੇ;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, get, ਕਤਾਰ, timeout);

	ਜੇ (likely(!sys_sflist_is_ਖਾਲੀ_ਥਾਵਾਂ(&ਕਤਾਰ->ਅੰਕੜੇ_q))) {
		sys_sfnode_t *ਗੰਢ;

		ਗੰਢ = sys_sflist_get_not_ਖਾਲੀ_ਥਾਵਾਂ(&ਕਤਾਰ->ਅੰਕੜੇ_q);
		ਅੰਕੜੇ = z_ਕਤਾਰ_ਗੰਢ_peek(ਗੰਢ, true);
		k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, get, ਕਤਾਰ, timeout, ਅੰਕੜੇ);

		ਵਾਪਸ ਅੰਕੜੇ;
	}

	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, get, ਕਤਾਰ, timeout, ਨਿਰਾ);

		ਵਾਪਸ ਨਿਰਾ;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_ਕਤਾਰ, get, ਕਤਾਰ, timeout);

	ਪੂਰਨ ret = z_pend_curr(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ, &ਕਤਾਰ->wait_q, timeout);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, get, ਕਤਾਰ, timeout,
		(ret != 0) ? ਨਿਰਾ : _current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ);

	ਵਾਪਸ (ret != 0) ? ਨਿਰਾ : _current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_ਕਤਾਰ_remove(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, remove, ਕਤਾਰ);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ);
	sys_sfnode_t *prev = ਨਿਰਾ;
	sys_sfnode_t *ਗੰਢ = sys_sflist_peek_ਸਿਰ(&ਕਤਾਰ->ਅੰਕੜੇ_q);
	bool ret = false;

	ਜਦੋਂ (ਗੰਢ != ਨਿਰਾ) {
		ਖਾਲੀ *peeked = z_ਕਤਾਰ_ਗੰਢ_peek(ਗੰਢ, false);

		ਜੇ (peeked == ਅੰਕੜੇ) {
			sys_sflist_remove(&ਕਤਾਰ->ਅੰਕੜੇ_q, prev, ਗੰਢ);
			(ਖਾਲੀ)z_ਕਤਾਰ_ਗੰਢ_peek(ਗੰਢ, true);
			ret = true;
			ਰੋਕੋ;
		}
		prev = ਗੰਢ;
		ਗੰਢ = sys_sflist_peek_ਅਗਲਾ(ਗੰਢ);
	}

	k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, remove, ਕਤਾਰ, ret);

	ਵਾਪਸ ret;
}

bool k_ਕਤਾਰ_unique_append(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_ਕਤਾਰ, unique_append, ਕਤਾਰ);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_ਗੰਢ(&ਕਤਾਰ->ਅੰਕੜੇ_q, test) {
		ਜੇ (test == (sys_sfnode_t *) ਅੰਕੜੇ) {
			k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);
			SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, unique_append, ਕਤਾਰ, false);

			ਵਾਪਸ false;
		}
	}

	(ਖਾਲੀ)ਕਤਾਰ_ਪਾਓ(ਕਤਾਰ, ਨਿਰਾ, ਅੰਕੜੇ, false, true, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_ਕਤਾਰ, unique_append, ਕਤਾਰ, true);

	ਵਾਪਸ true;
}

ਖਾਲੀ *z_impl_k_ਕਤਾਰ_peek_ਸਿਰ(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ);
	ਖਾਲੀ *ret = z_ਕਤਾਰ_ਗੰਢ_peek(sys_sflist_peek_ਸਿਰ(&ਕਤਾਰ->ਅੰਕੜੇ_q), false);

	k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_ਕਤਾਰ, peek_ਸਿਰ, ਕਤਾਰ, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ *z_impl_k_ਕਤਾਰ_peek_ਪੂਛ(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ਕਤਾਰ->ਤਾਲਾ);
	ਖਾਲੀ *ret = z_ਕਤਾਰ_ਗੰਢ_peek(sys_sflist_peek_ਪੂਛ(&ਕਤਾਰ->ਅੰਕੜੇ_q), false);

	k_spin_unlock(&ਕਤਾਰ->ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_ਕਤਾਰ, peek_ਪੂਛ, ਕਤਾਰ, ret);

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *z_vrfy_k_ਕਤਾਰ_get(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	ਵਾਪਸ z_impl_k_ਕਤਾਰ_get(ਕਤਾਰ, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_get_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_ਕਤਾਰ_is_ਖਾਲੀ_ਥਾਵਾਂ(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	K_OOPS(K_SYSCALL_OBJ(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	ਵਾਪਸ z_impl_k_ਕਤਾਰ_is_ਖਾਲੀ_ਥਾਵਾਂ(ਕਤਾਰ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *z_vrfy_k_ਕਤਾਰ_peek_ਸਿਰ(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	K_OOPS(K_SYSCALL_OBJ(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	ਵਾਪਸ z_impl_k_ਕਤਾਰ_peek_ਸਿਰ(ਕਤਾਰ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *z_vrfy_k_ਕਤਾਰ_peek_ਪੂਛ(ਬਣਤਰ k_ਕਤਾਰ *ਕਤਾਰ)
{
	K_OOPS(K_SYSCALL_OBJ(ਕਤਾਰ, K_OBJ_ਕਤਾਰ));
	ਵਾਪਸ z_impl_k_ਕਤਾਰ_peek_ਪੂਛ(ਕਤਾਰ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_ਸ਼ੁਰੂ_EXTERN(k_fifo);
STRUCT_SECTION_ਸਿਰਾ_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ਨਿਰਾ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_FIFO */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_ਸ਼ੁਰੂ_EXTERN(k_lifo);
STRUCT_SECTION_ਸਿਰਾ_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ਨਿਰਾ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_LIFO */
