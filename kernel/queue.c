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


#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <scheduler.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

ഘടന alloc_കണ്ണി {
	sys_sfnode_t കണ്ണി;
	ശൂന്യം *data;
};

/* The queue must have its spinlock held before calling this function. */
സ്ഥിതം ശൂന്യം *z_queue_കണ്ണി_peek(sys_sfnode_t *കണ്ണി, bool needs_free)
{
	ശൂന്യം *ret;

	എങ്കിൽ ((കണ്ണി != ശൂന്യ) && (sys_sfnode_flags_get(കണ്ണി) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		ഘടന alloc_കണ്ണി *anode;

		anode = CONTAINER_OF(കണ്ണി, ഘടന alloc_കണ്ണി, കണ്ണി);
		ret = anode->data;
		എങ്കിൽ (needs_free) {
			k_free(anode);
		}
	} അല്ലെങ്കിൽ {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (ശൂന്യം *)കണ്ണി;
	}

	തിരികെ ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(ഘടന k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(ഘടന k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, ശൂന്യ,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

ശൂന്യം z_impl_k_queue_init(ഘടന k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (ഘടന k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#എങ്കിൽ defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_queue_init(ഘടന k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

സ്ഥിതം വരിയിൽ bool queue_handle_poll_events(ഘടന k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	തിരികെ z_handle_obj_poll_events(&queue->poll_events, state);
#അല്ലെങ്കിൽ
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	തിരികെ false;
#endif /* CONFIG_POLL */
}

ശൂന്യം z_impl_k_queue_cancel_wait(ഘടന k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&queue->lock);
	bool resched = false;

	എങ്കിൽ (z_sched_wake(&queue->wait_q, 0, ശൂന്യ)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	എങ്കിൽ (resched) {
		z_reschedule(&queue->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&queue->lock, താക്കോൽ);
	}
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_queue_cancel_wait(ഘടന k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
സ്ഥിതം int32_t queue_തിരുകുക(ഘടന k_queue *queue, ശൂന്യം *prev, ശൂന്യം *data,
			    bool alloc, bool is_append, k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_തിരുകുക, queue, alloc);

	എങ്കിൽ (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	എങ്കിൽ (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_തിരുകുക, queue, alloc, K_FOREVER);
		resched = true;
		പോകുക out;
	}

	/* Only need to actually allocate if no threads are pending */
	എങ്കിൽ (alloc) {
		ഘടന alloc_കണ്ണി *anode;

		anode = z_thread_malloc(വലുപ്പം(*anode));
		എങ്കിൽ (anode == ശൂന്യ) {
			result = -ENOMEM;
			പോകുക out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->കണ്ണി, 0x1);
		data = anode;
	} അല്ലെങ്കിൽ {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_തിരുകുക(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	എങ്കിൽ (resched) {
		z_reschedule(&queue->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&queue->lock, താക്കോൽ);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_തിരുകുക, queue, alloc, result);

	തിരികെ result;
}

ശൂന്യം k_queue_തിരുകുക(ഘടന k_queue *queue, ശൂന്യം *prev, ശൂന്യം *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, തിരുകുക, queue);

	(ശൂന്യം)queue_തിരുകുക(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, തിരുകുക, queue);
}

ശൂന്യം k_queue_append(ഘടന k_queue *queue, ശൂന്യം *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(ശൂന്യം)queue_തിരുകുക(queue, ശൂന്യ, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

ശൂന്യം k_queue_prepend(ഘടന k_queue *queue, ശൂന്യം *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(ശൂന്യം)queue_തിരുകുക(queue, ശൂന്യ, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(ഘടന k_queue *queue, ശൂന്യം *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_തിരുകുക(queue, ശൂന്യ, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	തിരികെ ret;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ int32_t z_vrfy_k_queue_alloc_append(ഘടന k_queue *queue,
						  ശൂന്യം *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	തിരികെ z_impl_k_queue_alloc_append(queue, data);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(ഘടന k_queue *queue, ശൂന്യം *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_തിരുകുക(queue, ശൂന്യ, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	തിരികെ ret;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ int32_t z_vrfy_k_queue_alloc_prepend(ഘടന k_queue *queue,
						   ശൂന്യം *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	തിരികെ z_impl_k_queue_alloc_prepend(queue, data);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ k_queue_append_list(ഘടന k_queue *queue, ശൂന്യം *head, ശൂന്യം *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == ശൂന്യ) || (tail == ശൂന്യ)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		തിരികെ -EINVAL;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&queue->lock);

	ആകുമ്പോൾ ((head != ശൂന്യ) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(ശൂന്യം **)head;
	}

	എങ്കിൽ (head != ശൂന്യ) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	എങ്കിൽ (resched) {
		z_reschedule(&queue->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&queue->lock, താക്കോൽ);
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ k_queue_merge_slist(ഘടന k_queue *queue, sys_slist_t *list)
{
	പൂർണ്ണസംഖ്യ ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		തിരികെ -EINVAL;
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

		തിരികെ ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	തിരികെ 0;
}

ശൂന്യം *z_impl_k_queue_get(ഘടന k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&queue->lock);
	ശൂന്യം *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	എങ്കിൽ (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *കണ്ണി;

		കണ്ണി = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_കണ്ണി_peek(കണ്ണി, true);
		k_spin_unlock(&queue->lock, താക്കോൽ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		തിരികെ data;
	}

	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, താക്കോൽ);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, ശൂന്യ);

		തിരികെ ശൂന്യ;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	പൂർണ്ണസംഖ്യ ret = z_pend_curr(&queue->lock, താക്കോൽ, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? ശൂന്യ : _current->base.swap_data);

	തിരികെ (ret != 0) ? ശൂന്യ : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(ഘടന k_queue *queue, ശൂന്യം *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = ശൂന്യ;
	sys_sfnode_t *കണ്ണി = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	ആകുമ്പോൾ (കണ്ണി != ശൂന്യ) {
		ശൂന്യം *peeked = z_queue_കണ്ണി_peek(കണ്ണി, false);

		എങ്കിൽ (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, കണ്ണി);
			(ശൂന്യം)z_queue_കണ്ണി_peek(കണ്ണി, true);
			ret = true;
			നിർത്തുക;
		}
		prev = കണ്ണി;
		കണ്ണി = sys_sflist_peek_next(കണ്ണി);
	}

	k_spin_unlock(&queue->lock, താക്കോൽ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	തിരികെ ret;
}

bool k_queue_unique_append(ഘടന k_queue *queue, ശൂന്യം *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_കണ്ണി(&queue->data_q, test) {
		എങ്കിൽ (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, താക്കോൽ);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			തിരികെ false;
		}
	}

	(ശൂന്യം)queue_തിരുകുക(queue, ശൂന്യ, data, false, true, താക്കോൽ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	തിരികെ true;
}

ശൂന്യം *z_impl_k_queue_peek_head(ഘടന k_queue *queue)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&queue->lock);
	ശൂന്യം *ret = z_queue_കണ്ണി_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, താക്കോൽ);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	തിരികെ ret;
}

ശൂന്യം *z_impl_k_queue_peek_tail(ഘടന k_queue *queue)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&queue->lock);
	ശൂന്യം *ret = z_queue_കണ്ണി_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, താക്കോൽ);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	തിരികെ ret;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം *z_vrfy_k_queue_get(ഘടന k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	തിരികെ z_impl_k_queue_get(queue, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_get_mrsh.c>

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_queue_is_empty(ഘടന k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	തിരികെ z_impl_k_queue_is_empty(queue);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം *z_vrfy_k_queue_peek_head(ഘടന k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	തിരികെ z_impl_k_queue_peek_head(queue);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം *z_vrfy_k_queue_peek_tail(ഘടന k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	തിരികെ z_impl_k_queue_peek_tail(queue);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ശൂന്യ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ശൂന്യ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
