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


#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <scheduler.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

கட்டமைப்பு alloc_முனை {
	sys_sfnode_t முனை;
	வெறுமை *data;
};

/* The queue must have its spinlock held before calling this function. */
நிலையான வெறுமை *z_queue_முனை_peek(sys_sfnode_t *முனை, bool needs_free)
{
	வெறுமை *ret;

	எனில் ((முனை != வெற்று) && (sys_sfnode_flags_get(முனை) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		கட்டமைப்பு alloc_முனை *anode;

		anode = CONTAINER_OF(முனை, கட்டமைப்பு alloc_முனை, முனை);
		ret = anode->data;
		எனில் (needs_free) {
			k_free(anode);
		}
	} இல்லையெனில் {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (வெறுமை *)முனை;
	}

	திரும்பு ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(கட்டமைப்பு k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(கட்டமைப்பு k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, வெற்று,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

வெறுமை z_impl_k_queue_init(கட்டமைப்பு k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (கட்டமைப்பு k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#எனில் defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_queue_init(கட்டமைப்பு k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

நிலையான உள்வரி bool queue_handle_poll_events(கட்டமைப்பு k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	திரும்பு z_handle_obj_poll_events(&queue->poll_events, state);
#இல்லையெனில்
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	திரும்பு false;
#endif /* CONFIG_POLL */
}

வெறுமை z_impl_k_queue_cancel_wait(கட்டமைப்பு k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_சாவி_t சாவி = k_spin_lock(&queue->lock);
	bool resched = false;

	எனில் (z_sched_wake(&queue->wait_q, 0, வெற்று)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	எனில் (resched) {
		z_reschedule(&queue->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&queue->lock, சாவி);
	}
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_queue_cancel_wait(கட்டமைப்பு k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
நிலையான int32_t queue_செருகு(கட்டமைப்பு k_queue *queue, வெறுமை *prev, வெறுமை *data,
			    bool alloc, bool is_append, k_spinlock_சாவி_t சாவி)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_செருகு, queue, alloc);

	எனில் (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	எனில் (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_செருகு, queue, alloc, K_FOREVER);
		resched = true;
		செல் out;
	}

	/* Only need to actually allocate if no threads are pending */
	எனில் (alloc) {
		கட்டமைப்பு alloc_முனை *anode;

		anode = z_thread_malloc(அளவு(*anode));
		எனில் (anode == வெற்று) {
			result = -ENOMEM;
			செல் out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->முனை, 0x1);
		data = anode;
	} இல்லையெனில் {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_செருகு(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	எனில் (resched) {
		z_reschedule(&queue->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&queue->lock, சாவி);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_செருகு, queue, alloc, result);

	திரும்பு result;
}

வெறுமை k_queue_செருகு(கட்டமைப்பு k_queue *queue, வெறுமை *prev, வெறுமை *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, செருகு, queue);

	(வெறுமை)queue_செருகு(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, செருகு, queue);
}

வெறுமை k_queue_append(கட்டமைப்பு k_queue *queue, வெறுமை *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(வெறுமை)queue_செருகு(queue, வெற்று, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

வெறுமை k_queue_prepend(கட்டமைப்பு k_queue *queue, வெறுமை *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(வெறுமை)queue_செருகு(queue, வெற்று, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(கட்டமைப்பு k_queue *queue, வெறுமை *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_செருகு(queue, வெற்று, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	திரும்பு ret;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி int32_t z_vrfy_k_queue_alloc_append(கட்டமைப்பு k_queue *queue,
						  வெறுமை *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	திரும்பு z_impl_k_queue_alloc_append(queue, data);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(கட்டமைப்பு k_queue *queue, வெறுமை *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_செருகு(queue, வெற்று, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	திரும்பு ret;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி int32_t z_vrfy_k_queue_alloc_prepend(கட்டமைப்பு k_queue *queue,
						   வெறுமை *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	திரும்பு z_impl_k_queue_alloc_prepend(queue, data);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு k_queue_append_list(கட்டமைப்பு k_queue *queue, வெறுமை *head, வெறுமை *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == வெற்று) || (tail == வெற்று)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		திரும்பு -EINVAL;
	}

	k_spinlock_சாவி_t சாவி = k_spin_lock(&queue->lock);

	போது ((head != வெற்று) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(வெறுமை **)head;
	}

	எனில் (head != வெற்று) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	எனில் (resched) {
		z_reschedule(&queue->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&queue->lock, சாவி);
	}

	திரும்பு 0;
}

முழு k_queue_merge_slist(கட்டமைப்பு k_queue *queue, sys_slist_t *list)
{
	முழு ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		திரும்பு -EINVAL;
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

		திரும்பு ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	திரும்பு 0;
}

வெறுமை *z_impl_k_queue_get(கட்டமைப்பு k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&queue->lock);
	வெறுமை *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	எனில் (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *முனை;

		முனை = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_முனை_peek(முனை, true);
		k_spin_unlock(&queue->lock, சாவி);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		திரும்பு data;
	}

	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, சாவி);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, வெற்று);

		திரும்பு வெற்று;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	முழு ret = z_pend_curr(&queue->lock, சாவி, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? வெற்று : _current->base.swap_data);

	திரும்பு (ret != 0) ? வெற்று : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(கட்டமைப்பு k_queue *queue, வெறுமை *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_சாவி_t சாவி = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = வெற்று;
	sys_sfnode_t *முனை = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	போது (முனை != வெற்று) {
		வெறுமை *peeked = z_queue_முனை_peek(முனை, false);

		எனில் (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, முனை);
			(வெறுமை)z_queue_முனை_peek(முனை, true);
			ret = true;
			நிறுத்து;
		}
		prev = முனை;
		முனை = sys_sflist_peek_next(முனை);
	}

	k_spin_unlock(&queue->lock, சாவி);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	திரும்பு ret;
}

bool k_queue_unique_append(கட்டமைப்பு k_queue *queue, வெறுமை *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_சாவி_t சாவி = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_முனை(&queue->data_q, test) {
		எனில் (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, சாவி);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			திரும்பு false;
		}
	}

	(வெறுமை)queue_செருகு(queue, வெற்று, data, false, true, சாவி);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	திரும்பு true;
}

வெறுமை *z_impl_k_queue_peek_head(கட்டமைப்பு k_queue *queue)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&queue->lock);
	வெறுமை *ret = z_queue_முனை_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, சாவி);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	திரும்பு ret;
}

வெறுமை *z_impl_k_queue_peek_tail(கட்டமைப்பு k_queue *queue)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&queue->lock);
	வெறுமை *ret = z_queue_முனை_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, சாவி);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	திரும்பு ret;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை *z_vrfy_k_queue_get(கட்டமைப்பு k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	திரும்பு z_impl_k_queue_get(queue, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_get_mrsh.c>

நிலையான உள்வரி முழு z_vrfy_k_queue_is_empty(கட்டமைப்பு k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	திரும்பு z_impl_k_queue_is_empty(queue);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

நிலையான உள்வரி வெறுமை *z_vrfy_k_queue_peek_head(கட்டமைப்பு k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	திரும்பு z_impl_k_queue_peek_head(queue);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

நிலையான உள்வரி வெறுமை *z_vrfy_k_queue_peek_tail(கட்டமைப்பு k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	திரும்பு z_impl_k_queue_peek_tail(queue);
}
#அந்தர்பாவ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, வெற்று, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, வெற்று, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
