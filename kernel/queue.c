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


#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <wait_q.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

بنية alloc_node {
	sys_sfnode_t node;
	فراغ *data;
};

/* The queue must have its spinlock held before calling this function. */
ساكن فراغ *z_queue_node_peek(sys_sfnode_t *node, bool needs_free)
{
	فراغ *ret;

	𐡄𐡍 ((node != عدم) && (sys_sfnode_flags_get(node) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		بنية alloc_node *anode;

		anode = CONTAINER_OF(node, بنية alloc_node, node);
		ret = anode->data;
		𐡄𐡍 (needs_free) {
			k_free(anode);
		}
	} وإلا {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (فراغ *)node;
	}

	أرجع ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(بنية k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(بنية k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, عدم,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

فراغ z_impl_k_queue_init(بنية k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (بنية k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#𐡄𐡍 defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_queue_init(بنية k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#ضمن <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ساكن مضمن bool queue_handle_poll_events(بنية k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	أرجع z_handle_obj_poll_events(&queue->poll_events, state);
#وإلا
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	أرجع false;
#endif /* CONFIG_POLL */
}

فراغ z_impl_k_queue_cancel_wait(بنية k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	bool resched = false;

	𐡄𐡍 (z_sched_wake(&queue->wait_q, 0, عدم)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	𐡄𐡍 (resched) {
		z_reschedule(&queue->lock, key);
	} وإلا {
		k_spin_unlock(&queue->lock, key);
	}
}

#ifdef CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_queue_cancel_wait(بنية k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#ضمن <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ساكن int32_t queue_insert(بنية k_queue *queue, فراغ *prev, فراغ *data,
			    bool alloc, bool is_append, k_spinlock_key_t key)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_insert, queue, alloc);

	𐡄𐡍 (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	𐡄𐡍 (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_insert, queue, alloc, K_FOREVER);
		resched = true;
		اذهب out;
	}

	/* Only need to actually allocate if no threads are pending */
	𐡄𐡍 (alloc) {
		بنية alloc_node *anode;

		anode = z_thread_malloc(حجم(*anode));
		𐡄𐡍 (anode == عدم) {
			result = -ENOMEM;
			اذهب out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->node, 0x1);
		data = anode;
	} وإلا {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_insert(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	𐡄𐡍 (resched) {
		z_reschedule(&queue->lock, key);
	} وإلا {
		k_spin_unlock(&queue->lock, key);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_insert, queue, alloc, result);

	أرجع result;
}

فراغ k_queue_insert(بنية k_queue *queue, فراغ *prev, فراغ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, insert, queue);

	(فراغ)queue_insert(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, insert, queue);
}

فراغ k_queue_append(بنية k_queue *queue, فراغ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(فراغ)queue_insert(queue, عدم, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

فراغ k_queue_prepend(بنية k_queue *queue, فراغ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(فراغ)queue_insert(queue, عدم, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(بنية k_queue *queue, فراغ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_insert(queue, عدم, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_queue_alloc_append(بنية k_queue *queue,
						  فراغ *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	أرجع z_impl_k_queue_alloc_append(queue, data);
}
#ضمن <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(بنية k_queue *queue, فراغ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_insert(queue, عدم, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_queue_alloc_prepend(بنية k_queue *queue,
						   فراغ *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	أرجع z_impl_k_queue_alloc_prepend(queue, data);
}
#ضمن <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح k_queue_append_list(بنية k_queue *queue, فراغ *head, فراغ *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == عدم) || (tail == عدم)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		أرجع -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&queue->lock);

	طالما ((head != عدم) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(فراغ **)head;
	}

	𐡄𐡍 (head != عدم) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	𐡄𐡍 (resched) {
		z_reschedule(&queue->lock, key);
	} وإلا {
		k_spin_unlock(&queue->lock, key);
	}

	أرجع 0;
}

صحيح k_queue_merge_slist(بنية k_queue *queue, sys_slist_t *list)
{
	صحيح ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		أرجع -EINVAL;
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

		أرجع ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	أرجع 0;
}

فراغ *z_impl_k_queue_get(بنية k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	فراغ *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	𐡄𐡍 (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *node;

		node = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_node_peek(node, true);
		k_spin_unlock(&queue->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		أرجع data;
	}

	𐡄𐡍 (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, عدم);

		أرجع عدم;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	صحيح ret = z_pend_curr(&queue->lock, key, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? عدم : _current->base.swap_data);

	أرجع (ret != 0) ? عدم : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(بنية k_queue *queue, فراغ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = عدم;
	sys_sfnode_t *node = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	طالما (node != عدم) {
		فراغ *peeked = z_queue_node_peek(node, false);

		𐡄𐡍 (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, node);
			(فراغ)z_queue_node_peek(node, true);
			ret = true;
			اكسر;
		}
		prev = node;
		node = sys_sflist_peek_next(node);
	}

	k_spin_unlock(&queue->lock, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	أرجع ret;
}

bool k_queue_unique_append(بنية k_queue *queue, فراغ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_key_t key = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_NODE(&queue->data_q, test) {
		𐡄𐡍 (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, key);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			أرجع false;
		}
	}

	(فراغ)queue_insert(queue, عدم, data, false, true, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	أرجع true;
}

فراغ *z_impl_k_queue_peek_head(بنية k_queue *queue)
{
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	فراغ *ret = z_queue_node_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, key);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	أرجع ret;
}

فراغ *z_impl_k_queue_peek_tail(بنية k_queue *queue)
{
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	فراغ *ret = z_queue_node_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, key);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	أرجع ret;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_queue_get(بنية k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	أرجع z_impl_k_queue_get(queue, timeout);
}
#ضمن <zephyr/syscalls/k_queue_get_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_queue_is_empty(بنية k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	أرجع z_impl_k_queue_is_empty(queue);
}
#ضمن <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_queue_peek_head(بنية k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	أرجع z_impl_k_queue_peek_head(queue);
}
#ضمن <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_queue_peek_tail(بنية k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	أرجع z_impl_k_queue_peek_tail(queue);
}
#ضمن <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
