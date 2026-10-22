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


#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

ಸಂರಚನಾ alloc_ಗಂಟು {
	sys_sfnode_t ಗಂಟು;
	ಶೂನ್ಯ *data;
};

/* The queue must have its spinlock held before calling this function. */
ಸ್ಥಿರ ಶೂನ್ಯ *z_queue_ಗಂಟು_peek(sys_sfnode_t *ಗಂಟು, bool needs_free)
{
	ಶೂನ್ಯ *ret;

	ಯದಿ ((ಗಂಟು != ರಿಕ್ತ) && (sys_sfnode_flags_get(ಗಂಟು) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		ಸಂರಚನಾ alloc_ಗಂಟು *anode;

		anode = CONTAINER_OF(ಗಂಟು, ಸಂರಚನಾ alloc_ಗಂಟು, ಗಂಟು);
		ret = anode->data;
		ಯದಿ (needs_free) {
			k_free(anode);
		}
	} ಅನ್ಯಥಾ {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (ಶೂನ್ಯ *)ಗಂಟು;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(ಸಂರಚನಾ k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(ಸಂರಚನಾ k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, ರಿಕ್ತ,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

ಶೂನ್ಯ z_impl_k_queue_init(ಸಂರಚನಾ k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (ಸಂರಚನಾ k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#ಯದಿ defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_queue_init(ಸಂರಚನಾ k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool queue_handle_poll_events(ಸಂರಚನಾ k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	ಪ್ರತ್ಯಾಗಮ z_handle_obj_poll_events(&queue->poll_events, state);
#ಅನ್ಯಥಾ
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	ಪ್ರತ್ಯಾಗಮ false;
#endif /* CONFIG_POLL */
}

ಶೂನ್ಯ z_impl_k_queue_cancel_wait(ಸಂರಚನಾ k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&queue->lock);
	bool resched = false;

	ಯದಿ (z_sched_wake(&queue->wait_q, 0, ರಿಕ್ತ)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	ಯದಿ (resched) {
		z_reschedule(&queue->lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&queue->lock, ಕೀಲಿ);
	}
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_queue_cancel_wait(ಸಂರಚನಾ k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ಸ್ಥಿರ int32_t queue_ಸೇರಿಸು(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *prev, ಶೂನ್ಯ *data,
			    bool alloc, bool is_append, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_ಸೇರಿಸು, queue, alloc);

	ಯದಿ (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	ಯದಿ (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_ಸೇರಿಸು, queue, alloc, K_FOREVER);
		resched = true;
		ಗಚ್ಛ out;
	}

	/* Only need to actually allocate if no threads are pending */
	ಯದಿ (alloc) {
		ಸಂರಚನಾ alloc_ಗಂಟು *anode;

		anode = z_thread_malloc(ಪರಿಮಾಣ(*anode));
		ಯದಿ (anode == ರಿಕ್ತ) {
			result = -ENOMEM;
			ಗಚ್ಛ out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->ಗಂಟು, 0x1);
		data = anode;
	} ಅನ್ಯಥಾ {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_ಸೇರಿಸು(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	ಯದಿ (resched) {
		z_reschedule(&queue->lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&queue->lock, ಕೀಲಿ);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_ಸೇರಿಸು, queue, alloc, result);

	ಪ್ರತ್ಯಾಗಮ result;
}

ಶೂನ್ಯ k_queue_ಸೇರಿಸು(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *prev, ಶೂನ್ಯ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, ಸೇರಿಸು, queue);

	(ಶೂನ್ಯ)queue_ಸೇರಿಸು(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, ಸೇರಿಸು, queue);
}

ಶೂನ್ಯ k_queue_append(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(ಶೂನ್ಯ)queue_ಸೇರಿಸು(queue, ರಿಕ್ತ, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

ಶೂನ್ಯ k_queue_prepend(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(ಶೂನ್ಯ)queue_ಸೇರಿಸು(queue, ರಿಕ್ತ, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_ಸೇರಿಸು(queue, ರಿಕ್ತ, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ int32_t z_vrfy_k_queue_alloc_append(ಸಂರಚನಾ k_queue *queue,
						  ಶೂನ್ಯ *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_queue_alloc_append(queue, data);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_ಸೇರಿಸು(queue, ರಿಕ್ತ, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ int32_t z_vrfy_k_queue_alloc_prepend(ಸಂರಚನಾ k_queue *queue,
						   ಶೂನ್ಯ *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_queue_alloc_prepend(queue, data);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ k_queue_append_list(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *head, ಶೂನ್ಯ *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == ರಿಕ್ತ) || (tail == ರಿಕ್ತ)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&queue->lock);

	ಯಾವತ್ ((head != ರಿಕ್ತ) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(ಶೂನ್ಯ **)head;
	}

	ಯದಿ (head != ರಿಕ್ತ) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	ಯದಿ (resched) {
		z_reschedule(&queue->lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&queue->lock, ಕೀಲಿ);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ k_queue_merge_slist(ಸಂರಚನಾ k_queue *queue, sys_slist_t *list)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		ಪ್ರತ್ಯಾಗಮ -EINVAL;
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
	ret = k_queue_append_list(queue, list->head, list->tail);
	CHECKIF(ret != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, ret);

		ಪ್ರತ್ಯಾಗಮ ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ *z_impl_k_queue_get(ಸಂರಚನಾ k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&queue->lock);
	ಶೂನ್ಯ *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	ಯದಿ (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *ಗಂಟು;

		ಗಂಟು = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_ಗಂಟು_peek(ಗಂಟು, true);
		k_spin_unlock(&queue->lock, ಕೀಲಿ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		ಪ್ರತ್ಯಾಗಮ data;
	}

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, ಕೀಲಿ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, ರಿಕ್ತ);

		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	ಪೂರ್ಣಾಙ್ಕ ret = z_pend_curr(&queue->lock, ಕೀಲಿ, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? ರಿಕ್ತ : _current->base.swap_data);

	ಪ್ರತ್ಯಾಗಮ (ret != 0) ? ರಿಕ್ತ : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = ರಿಕ್ತ;
	sys_sfnode_t *ಗಂಟು = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	ಯಾವತ್ (ಗಂಟು != ರಿಕ್ತ) {
		ಶೂನ್ಯ *peeked = z_queue_ಗಂಟು_peek(ಗಂಟು, false);

		ಯದಿ (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, ಗಂಟು);
			(ಶೂನ್ಯ)z_queue_ಗಂಟು_peek(ಗಂಟು, true);
			ret = true;
			ಭಙ್ಗ;
		}
		prev = ಗಂಟು;
		ಗಂಟು = sys_sflist_peek_next(ಗಂಟು);
	}

	k_spin_unlock(&queue->lock, ಕೀಲಿ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

bool k_queue_unique_append(ಸಂರಚನಾ k_queue *queue, ಶೂನ್ಯ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_ಗಂಟು(&queue->data_q, test) {
		ಯದಿ (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, ಕೀಲಿ);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	(ಶೂನ್ಯ)queue_ಸೇರಿಸು(queue, ರಿಕ್ತ, data, false, true, ಕೀಲಿ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	ಪ್ರತ್ಯಾಗಮ true;
}

ಶೂನ್ಯ *z_impl_k_queue_peek_head(ಸಂರಚನಾ k_queue *queue)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&queue->lock);
	ಶೂನ್ಯ *ret = z_queue_ಗಂಟು_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, ಕೀಲಿ);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ *z_impl_k_queue_peek_tail(ಸಂರಚನಾ k_queue *queue)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&queue->lock);
	ಶೂನ್ಯ *ret = z_queue_ಗಂಟು_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, ಕೀಲಿ);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *z_vrfy_k_queue_get(ಸಂರಚನಾ k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_queue_get(queue, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_get_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_queue_is_empty(ಸಂರಚನಾ k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_queue_is_empty(queue);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *z_vrfy_k_queue_peek_head(ಸಂರಚನಾ k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_queue_peek_head(queue);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *z_vrfy_k_queue_peek_tail(ಸಂರಚನಾ k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_queue_peek_tail(queue);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ರಿಕ್ತ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ರಿಕ್ತ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
