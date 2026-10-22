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


#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <wait_q.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <kernel_internal.h>
#شامل <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

ساختار alloc_گره {
	sys_sfnode_t گره;
	تهی *data;
};

/* The queue must have its spinlock held before calling this function. */
ایستا تهی *z_queue_گره_peek(sys_sfnode_t *گره, bool needs_free)
{
	تهی *ret;

	اگر ((گره != پوچ) && (sys_sfnode_flags_get(گره) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		ساختار alloc_گره *anode;

		anode = CONTAINER_OF(گره, ساختار alloc_گره, گره);
		ret = anode->data;
		اگر (needs_free) {
			k_free(anode);
		}
	} وگرنه {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (تهی *)گره;
	}

	بازگشت ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(ساختار k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(ساختار k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, پوچ,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

تهی z_impl_k_queue_init(ساختار k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (ساختار k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#اگر defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_queue_init(ساختار k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#شامل <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ایستا درونخطی bool queue_handle_poll_events(ساختار k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	بازگشت z_handle_obj_poll_events(&queue->poll_events, state);
#وگرنه
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	بازگشت false;
#endif /* CONFIG_POLL */
}

تهی z_impl_k_queue_cancel_wait(ساختار k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_کلید_t کلید = k_spin_lock(&queue->lock);
	bool resched = false;

	اگر (z_sched_wake(&queue->wait_q, 0, پوچ)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	اگر (resched) {
		z_reschedule(&queue->lock, کلید);
	} وگرنه {
		k_spin_unlock(&queue->lock, کلید);
	}
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_queue_cancel_wait(ساختار k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#شامل <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ایستا int32_t queue_درج(ساختار k_queue *queue, تهی *prev, تهی *data,
			    bool alloc, bool is_append, k_spinlock_کلید_t کلید)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_درج, queue, alloc);

	اگر (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	اگر (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_درج, queue, alloc, K_FOREVER);
		resched = true;
		برو out;
	}

	/* Only need to actually allocate if no threads are pending */
	اگر (alloc) {
		ساختار alloc_گره *anode;

		anode = z_thread_malloc(اندازه(*anode));
		اگر (anode == پوچ) {
			result = -ENOMEM;
			برو out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->گره, 0x1);
		data = anode;
	} وگرنه {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_درج(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	اگر (resched) {
		z_reschedule(&queue->lock, کلید);
	} وگرنه {
		k_spin_unlock(&queue->lock, کلید);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_درج, queue, alloc, result);

	بازگشت result;
}

تهی k_queue_درج(ساختار k_queue *queue, تهی *prev, تهی *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, درج, queue);

	(تهی)queue_درج(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, درج, queue);
}

تهی k_queue_append(ساختار k_queue *queue, تهی *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(تهی)queue_درج(queue, پوچ, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

تهی k_queue_prepend(ساختار k_queue *queue, تهی *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(تهی)queue_درج(queue, پوچ, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(ساختار k_queue *queue, تهی *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_درج(queue, پوچ, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	بازگشت ret;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی int32_t z_vrfy_k_queue_alloc_append(ساختار k_queue *queue,
						  تهی *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	بازگشت z_impl_k_queue_alloc_append(queue, data);
}
#شامل <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(ساختار k_queue *queue, تهی *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_درج(queue, پوچ, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	بازگشت ret;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی int32_t z_vrfy_k_queue_alloc_prepend(ساختار k_queue *queue,
						   تهی *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	بازگشت z_impl_k_queue_alloc_prepend(queue, data);
}
#شامل <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح k_queue_append_list(ساختار k_queue *queue, تهی *head, تهی *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == پوچ) || (tail == پوچ)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		بازگشت -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_lock(&queue->lock);

	وقتی ((head != پوچ) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(تهی **)head;
	}

	اگر (head != پوچ) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	اگر (resched) {
		z_reschedule(&queue->lock, کلید);
	} وگرنه {
		k_spin_unlock(&queue->lock, کلید);
	}

	بازگشت 0;
}

صحیح k_queue_merge_slist(ساختار k_queue *queue, sys_slist_t *list)
{
	صحیح ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		بازگشت -EINVAL;
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

		بازگشت ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	بازگشت 0;
}

تهی *z_impl_k_queue_get(ساختار k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&queue->lock);
	تهی *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	اگر (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *گره;

		گره = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_گره_peek(گره, true);
		k_spin_unlock(&queue->lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		بازگشت data;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, کلید);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, پوچ);

		بازگشت پوچ;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	صحیح ret = z_pend_curr(&queue->lock, کلید, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? پوچ : _current->base.swap_data);

	بازگشت (ret != 0) ? پوچ : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(ساختار k_queue *queue, تهی *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_کلید_t کلید = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = پوچ;
	sys_sfnode_t *گره = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	وقتی (گره != پوچ) {
		تهی *peeked = z_queue_گره_peek(گره, false);

		اگر (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, گره);
			(تهی)z_queue_گره_peek(گره, true);
			ret = true;
			بشکن;
		}
		prev = گره;
		گره = sys_sflist_peek_next(گره);
	}

	k_spin_unlock(&queue->lock, کلید);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	بازگشت ret;
}

bool k_queue_unique_append(ساختار k_queue *queue, تهی *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_کلید_t کلید = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_گره(&queue->data_q, test) {
		اگر (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, کلید);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			بازگشت false;
		}
	}

	(تهی)queue_درج(queue, پوچ, data, false, true, کلید);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	بازگشت true;
}

تهی *z_impl_k_queue_peek_head(ساختار k_queue *queue)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&queue->lock);
	تهی *ret = z_queue_گره_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, کلید);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	بازگشت ret;
}

تهی *z_impl_k_queue_peek_tail(ساختار k_queue *queue)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&queue->lock);
	تهی *ret = z_queue_گره_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, کلید);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	بازگشت ret;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی *z_vrfy_k_queue_get(ساختار k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	بازگشت z_impl_k_queue_get(queue, timeout);
}
#شامل <zephyr/syscalls/k_queue_get_mrsh.c>

ایستا درونخطی صحیح z_vrfy_k_queue_is_empty(ساختار k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	بازگشت z_impl_k_queue_is_empty(queue);
}
#شامل <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ایستا درونخطی تهی *z_vrfy_k_queue_peek_head(ساختار k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	بازگشت z_impl_k_queue_peek_head(queue);
}
#شامل <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ایستا درونخطی تهی *z_vrfy_k_queue_peek_tail(ساختار k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	بازگشت z_impl_k_queue_peek_tail(queue);
}
#شامل <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, پوچ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, پوچ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
