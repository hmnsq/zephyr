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


#include <zephyr/kernel.h>

#include <zephyr/toolchain.h>
#include <wait_q.h>
#include <ksched.h>
#include <scheduler.h>
#include <zephyr/init.h>
#include <zephyr/internal/syscall_handler.h>
#include <kernel_internal.h>
#include <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

sa_nracanaa alloc_node {
	sys_sfnode_t node;
	void *data;
};

/* The queue must have its spinlock held before calling this function. */
static void *z_queue_node_peek(sys_sfnode_t *node, bool needs_free)
{
	void *ret;

	གལ་སྲིད ((node != NULL) && (sys_sfnode_flags_get(node) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		sa_nracanaa alloc_node *anode;

		anode = CONTAINER_OF(node, sa_nracanaa alloc_node, node);
		ret = anode->data;
		གལ་སྲིད (needs_free) {
			k_free(anode);
		}
	} དེ་མིན {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (void *)node;
	}

	ལོག ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(sa_nracanaa k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(sa_nracanaa k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, NULL,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

void z_impl_k_queue_init(sa_nracanaa k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (sa_nracanaa k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#གལ་སྲིད defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
static anta_hstha void z_vrfy_k_queue_init(sa_nracanaa k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#include <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

static anta_hstha bool queue_handle_poll_events(sa_nracanaa k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	ལོག z_handle_obj_poll_events(&queue->poll_events, state);
#དེ་མིན
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	ལོག false;
#endif /* CONFIG_POLL */
}

void z_impl_k_queue_cancel_wait(sa_nracanaa k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	bool resched = false;

	གལ་སྲིད (z_sched_wake(&queue->wait_q, 0, NULL)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	གལ་སྲིད (resched) {
		z_reschedule(&queue->lock, key);
	} དེ་མིན {
		k_spin_unlock(&queue->lock, key);
	}
}

#ifdef CONFIG_USERSPACE
static anta_hstha void z_vrfy_k_queue_cancel_wait(sa_nracanaa k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#include <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
static int32_t queue_insert(sa_nracanaa k_queue *queue, void *prev, void *data,
			    bool alloc, bool is_append, k_spinlock_key_t key)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_insert, queue, alloc);

	གལ་སྲིད (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	གལ་སྲིད (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_insert, queue, alloc, K_FOREVER);
		resched = true;
		goto out;
	}

	/* Only need to actually allocate if no threads are pending */
	གལ་སྲིད (alloc) {
		sa_nracanaa alloc_node *anode;

		anode = z_thread_malloc(parimaa_na(*anode));
		གལ་སྲིད (anode == NULL) {
			result = -ENOMEM;
			goto out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->node, 0x1);
		data = anode;
	} དེ་མིན {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_insert(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	གལ་སྲིད (resched) {
		z_reschedule(&queue->lock, key);
	} དེ་མིན {
		k_spin_unlock(&queue->lock, key);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_insert, queue, alloc, result);

	ལོག result;
}

void k_queue_insert(sa_nracanaa k_queue *queue, void *prev, void *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, insert, queue);

	(void)queue_insert(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, insert, queue);
}

void k_queue_append(sa_nracanaa k_queue *queue, void *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(void)queue_insert(queue, NULL, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

void k_queue_prepend(sa_nracanaa k_queue *queue, void *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(void)queue_insert(queue, NULL, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(sa_nracanaa k_queue *queue, void *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_insert(queue, NULL, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	ལོག ret;
}

#ifdef CONFIG_USERSPACE
static anta_hstha int32_t z_vrfy_k_queue_alloc_append(sa_nracanaa k_queue *queue,
						  void *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ལོག z_impl_k_queue_alloc_append(queue, data);
}
#include <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(sa_nracanaa k_queue *queue, void *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_insert(queue, NULL, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	ལོག ret;
}

#ifdef CONFIG_USERSPACE
static anta_hstha int32_t z_vrfy_k_queue_alloc_prepend(sa_nracanaa k_queue *queue,
						   void *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ལོག z_impl_k_queue_alloc_prepend(queue, data);
}
#include <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

ཧྲིལ k_queue_append_list(sa_nracanaa k_queue *queue, void *head, void *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == NULL) || (tail == NULL)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		ལོག -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&queue->lock);

	སྐབས ((head != NULL) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(void **)head;
	}

	གལ་སྲིད (head != NULL) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	གལ་སྲིད (resched) {
		z_reschedule(&queue->lock, key);
	} དེ་མིན {
		k_spin_unlock(&queue->lock, key);
	}

	ལོག 0;
}

ཧྲིལ k_queue_merge_slist(sa_nracanaa k_queue *queue, sys_slist_t *list)
{
	ཧྲིལ ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		ལོག -EINVAL;
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

		ལོག ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	ལོག 0;
}

void *z_impl_k_queue_get(sa_nracanaa k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	void *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	གལ་སྲིད (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *node;

		node = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_node_peek(node, true);
		k_spin_unlock(&queue->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		ལོག data;
	}

	གལ་སྲིད (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, NULL);

		ལོག NULL;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	ཧྲིལ ret = z_pend_curr(&queue->lock, key, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? NULL : _current->base.swap_data);

	ལོག (ret != 0) ? NULL : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(sa_nracanaa k_queue *queue, void *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = NULL;
	sys_sfnode_t *node = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	སྐབས (node != NULL) {
		void *peeked = z_queue_node_peek(node, false);

		གལ་སྲིད (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, node);
			(void)z_queue_node_peek(node, true);
			ret = true;
			bha_nga;
		}
		prev = node;
		node = sys_sflist_peek_next(node);
	}

	k_spin_unlock(&queue->lock, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	ལོག ret;
}

bool k_queue_unique_append(sa_nracanaa k_queue *queue, void *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_key_t key = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_NODE(&queue->data_q, test) {
		གལ་སྲིད (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, key);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			ལོག false;
		}
	}

	(void)queue_insert(queue, NULL, data, false, true, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	ལོག true;
}

void *z_impl_k_queue_peek_head(sa_nracanaa k_queue *queue)
{
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	void *ret = z_queue_node_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, key);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	ལོག ret;
}

void *z_impl_k_queue_peek_tail(sa_nracanaa k_queue *queue)
{
	k_spinlock_key_t key = k_spin_lock(&queue->lock);
	void *ret = z_queue_node_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, key);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	ལོག ret;
}

#ifdef CONFIG_USERSPACE
static anta_hstha void *z_vrfy_k_queue_get(sa_nracanaa k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ལོག z_impl_k_queue_get(queue, timeout);
}
#include <zephyr/syscalls/k_queue_get_mrsh.c>

static anta_hstha ཧྲིལ z_vrfy_k_queue_is_empty(sa_nracanaa k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ལོག z_impl_k_queue_is_empty(queue);
}
#include <zephyr/syscalls/k_queue_is_empty_mrsh.c>

static anta_hstha void *z_vrfy_k_queue_peek_head(sa_nracanaa k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ལོག z_impl_k_queue_peek_head(queue);
}
#include <zephyr/syscalls/k_queue_peek_head_mrsh.c>

static anta_hstha void *z_vrfy_k_queue_peek_tail(sa_nracanaa k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ལོག z_impl_k_queue_peek_tail(queue);
}
#include <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, NULL, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, NULL, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
