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


#অন্তর্ভাৱ <zephyr/kernel.h>

#অন্তর্ভাৱ <zephyr/toolchain.h>
#অন্তর্ভাৱ <wait_q.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <scheduler.h>
#অন্তর্ভাৱ <zephyr/init.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <kernel_internal.h>
#অন্তর্ভাৱ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

সংরচনা alloc_গ্ৰন্থি {
	sys_sfnode_t গ্ৰন্থি;
	শূন্য *data;
};

/* The queue must have its spinlock held before calling this function. */
স্থির শূন্য *z_queue_গ্ৰন্থি_peek(sys_sfnode_t *গ্ৰন্থি, bool needs_free)
{
	শূন্য *ret;

	যদি ((গ্ৰন্থি != রিক্ত) && (sys_sfnode_flags_get(গ্ৰন্থি) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		সংরচনা alloc_গ্ৰন্থি *anode;

		anode = CONTAINER_OF(গ্ৰন্থি, সংরচনা alloc_গ্ৰন্থি, গ্ৰন্থি);
		ret = anode->data;
		যদি (needs_free) {
			k_free(anode);
		}
	} অন্যথা {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (শূন্য *)গ্ৰন্থি;
	}

	ঘূৰাই ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(সংরচনা k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(সংরচনা k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, রিক্ত,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

শূন্য z_impl_k_queue_init(সংরচনা k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (সংরচনা k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#যদি defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_queue_init(সংরচনা k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

স্থির অন্তঃস্থ bool queue_handle_poll_events(সংরচনা k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	ঘূৰাই z_handle_obj_poll_events(&queue->poll_events, state);
#অন্যথা
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	ঘূৰাই false;
#endif /* CONFIG_POLL */
}

শূন্য z_impl_k_queue_cancel_wait(সংরচনা k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	bool resched = false;

	যদি (z_sched_wake(&queue->wait_q, 0, রিক্ত)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	যদি (resched) {
		z_reschedule(&queue->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&queue->lock, চাবি);
	}
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_queue_cancel_wait(সংরচনা k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
স্থির int32_t queue_ঢোকাও(সংরচনা k_queue *queue, শূন্য *prev, শূন্য *data,
			    bool alloc, bool is_append, k_spinlock_চাবি_t চাবি)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_ঢোকাও, queue, alloc);

	যদি (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	যদি (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_ঢোকাও, queue, alloc, K_FOREVER);
		resched = true;
		গচ্ছ out;
	}

	/* Only need to actually allocate if no threads are pending */
	যদি (alloc) {
		সংরচনা alloc_গ্ৰন্থি *anode;

		anode = z_thread_malloc(পরিমাণ(*anode));
		যদি (anode == রিক্ত) {
			result = -ENOMEM;
			গচ্ছ out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->গ্ৰন্থি, 0x1);
		data = anode;
	} অন্যথা {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_ঢোকাও(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	যদি (resched) {
		z_reschedule(&queue->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&queue->lock, চাবি);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_ঢোকাও, queue, alloc, result);

	ঘূৰাই result;
}

শূন্য k_queue_ঢোকাও(সংরচনা k_queue *queue, শূন্য *prev, শূন্য *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, ঢোকাও, queue);

	(শূন্য)queue_ঢোকাও(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, ঢোকাও, queue);
}

শূন্য k_queue_append(সংরচনা k_queue *queue, শূন্য *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(শূন্য)queue_ঢোকাও(queue, রিক্ত, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

শূন্য k_queue_prepend(সংরচনা k_queue *queue, শূন্য *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(শূন্য)queue_ঢোকাও(queue, রিক্ত, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(সংরচনা k_queue *queue, শূন্য *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_ঢোকাও(queue, রিক্ত, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	ঘূৰাই ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ int32_t z_vrfy_k_queue_alloc_append(সংরচনা k_queue *queue,
						  শূন্য *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ঘূৰাই z_impl_k_queue_alloc_append(queue, data);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(সংরচনা k_queue *queue, শূন্য *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_ঢোকাও(queue, রিক্ত, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	ঘূৰাই ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ int32_t z_vrfy_k_queue_alloc_prepend(সংরচনা k_queue *queue,
						   শূন্য *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ঘূৰাই z_impl_k_queue_alloc_prepend(queue, data);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক k_queue_append_list(সংরচনা k_queue *queue, শূন্য *head, শূন্য *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == রিক্ত) || (tail == রিক্ত)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		ঘূৰাই -EINVAL;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);

	যেতিয়ালৈ ((head != রিক্ত) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(শূন্য **)head;
	}

	যদি (head != রিক্ত) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	যদি (resched) {
		z_reschedule(&queue->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&queue->lock, চাবি);
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক k_queue_merge_slist(সংরচনা k_queue *queue, sys_slist_t *list)
{
	পূর্ণাঙ্ক ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		ঘূৰাই -EINVAL;
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

		ঘূৰাই ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	ঘূৰাই 0;
}

শূন্য *z_impl_k_queue_get(সংরচনা k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	শূন্য *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	যদি (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *গ্ৰন্থি;

		গ্ৰন্থি = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_গ্ৰন্থি_peek(গ্ৰন্থি, true);
		k_spin_unlock(&queue->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		ঘূৰাই data;
	}

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, রিক্ত);

		ঘূৰাই রিক্ত;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	পূর্ণাঙ্ক ret = z_pend_curr(&queue->lock, চাবি, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? রিক্ত : _current->base.swap_data);

	ঘূৰাই (ret != 0) ? রিক্ত : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(সংরচনা k_queue *queue, শূন্য *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = রিক্ত;
	sys_sfnode_t *গ্ৰন্থি = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	যেতিয়ালৈ (গ্ৰন্থি != রিক্ত) {
		শূন্য *peeked = z_queue_গ্ৰন্থি_peek(গ্ৰন্থি, false);

		যদি (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, গ্ৰন্থি);
			(শূন্য)z_queue_গ্ৰন্থি_peek(গ্ৰন্থি, true);
			ret = true;
			ভঙ্গ;
		}
		prev = গ্ৰন্থি;
		গ্ৰন্থি = sys_sflist_peek_next(গ্ৰন্থি);
	}

	k_spin_unlock(&queue->lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	ঘূৰাই ret;
}

bool k_queue_unique_append(সংরচনা k_queue *queue, শূন্য *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_গ্ৰন্থি(&queue->data_q, test) {
		যদি (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, চাবি);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			ঘূৰাই false;
		}
	}

	(শূন্য)queue_ঢোকাও(queue, রিক্ত, data, false, true, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	ঘূৰাই true;
}

শূন্য *z_impl_k_queue_peek_head(সংরচনা k_queue *queue)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	শূন্য *ret = z_queue_গ্ৰন্থি_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	ঘূৰাই ret;
}

শূন্য *z_impl_k_queue_peek_tail(সংরচনা k_queue *queue)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	শূন্য *ret = z_queue_গ্ৰন্থি_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	ঘূৰাই ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য *z_vrfy_k_queue_get(সংরচনা k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ঘূৰাই z_impl_k_queue_get(queue, timeout);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_get_mrsh.c>

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_queue_is_empty(সংরচনা k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ঘূৰাই z_impl_k_queue_is_empty(queue);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

স্থির অন্তঃস্থ শূন্য *z_vrfy_k_queue_peek_head(সংরচনা k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ঘূৰাই z_impl_k_queue_peek_head(queue);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

স্থির অন্তঃস্থ শূন্য *z_vrfy_k_queue_peek_tail(সংরচনা k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ঘূৰাই z_impl_k_queue_peek_tail(queue);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, রিক্ত, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, রিক্ত, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
