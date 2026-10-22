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


#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

ସଂରଚନା alloc_ଗଣ୍ଠି {
	sys_sfnode_t ଗଣ୍ଠି;
	ଶୂନ୍ୟ *data;
};

/* The queue must have its spinlock held before calling this function. */
ସ୍ଥିର ଶୂନ୍ୟ *z_queue_ଗଣ୍ଠି_peek(sys_sfnode_t *ଗଣ୍ଠି, bool needs_free)
{
	ଶୂନ୍ୟ *ret;

	ଯଦି ((ଗଣ୍ଠି != ଖାଲି) && (sys_sfnode_flags_get(ଗଣ୍ଠି) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		ସଂରଚନା alloc_ଗଣ୍ଠି *anode;

		anode = CONTAINER_OF(ଗଣ୍ଠି, ସଂରଚନା alloc_ଗଣ୍ଠି, ଗଣ୍ଠି);
		ret = anode->data;
		ଯଦି (needs_free) {
			k_free(anode);
		}
	} ଅନ୍ୟଥା {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (ଶୂନ୍ୟ *)ଗଣ୍ଠି;
	}

	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(ସଂରଚନା k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(ସଂରଚନା k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, ଖାଲି,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

ଶୂନ୍ୟ z_impl_k_queue_init(ସଂରଚନା k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (ସଂରଚନା k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#ଯଦି defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_queue_init(ସଂରଚନା k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool queue_handle_poll_events(ସଂରଚନା k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	ଫେରନ୍ତୁ z_handle_obj_poll_events(&queue->poll_events, state);
#ଅନ୍ୟଥା
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	ଫେରନ୍ତୁ false;
#endif /* CONFIG_POLL */
}

ଶୂନ୍ୟ z_impl_k_queue_cancel_wait(ସଂରଚନା k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&queue->lock);
	bool resched = false;

	ଯଦି (z_sched_wake(&queue->wait_q, 0, ଖାଲି)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	ଯଦି (resched) {
		z_reschedule(&queue->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&queue->lock, ଚାବି);
	}
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_queue_cancel_wait(ସଂରଚନା k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ସ୍ଥିର int32_t queue_ଭର୍ତ୍ତି(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *prev, ଶୂନ୍ୟ *data,
			    bool alloc, bool is_append, k_spinlock_ଚାବି_t ଚାବି)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_ଭର୍ତ୍ତି, queue, alloc);

	ଯଦି (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	ଯଦି (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_ଭର୍ତ୍ତି, queue, alloc, K_FOREVER);
		resched = true;
		ଯାଅ out;
	}

	/* Only need to actually allocate if no threads are pending */
	ଯଦି (alloc) {
		ସଂରଚନା alloc_ଗଣ୍ଠି *anode;

		anode = z_thread_malloc(ଆକାର(*anode));
		ଯଦି (anode == ଖାଲି) {
			result = -ENOMEM;
			ଯାଅ out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->ଗଣ୍ଠି, 0x1);
		data = anode;
	} ଅନ୍ୟଥା {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_ଭର୍ତ୍ତି(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	ଯଦି (resched) {
		z_reschedule(&queue->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&queue->lock, ଚାବି);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_ଭର୍ତ୍ତି, queue, alloc, result);

	ଫେରନ୍ତୁ result;
}

ଶୂନ୍ୟ k_queue_ଭର୍ତ୍ତି(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *prev, ଶୂନ୍ୟ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, ଭର୍ତ୍ତି, queue);

	(ଶୂନ୍ୟ)queue_ଭର୍ତ୍ତି(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, ଭର୍ତ୍ତି, queue);
}

ଶୂନ୍ୟ k_queue_append(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(ଶୂନ୍ୟ)queue_ଭର୍ତ୍ତି(queue, ଖାଲି, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

ଶୂନ୍ୟ k_queue_prepend(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(ଶୂନ୍ୟ)queue_ଭର୍ତ୍ତି(queue, ଖାଲି, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_ଭର୍ତ୍ତି(queue, ଖାଲି, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ int32_t z_vrfy_k_queue_alloc_append(ସଂରଚନା k_queue *queue,
						  ଶୂନ୍ୟ *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ଫେରନ୍ତୁ z_impl_k_queue_alloc_append(queue, data);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_ଭର୍ତ୍ତି(queue, ଖାଲି, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ int32_t z_vrfy_k_queue_alloc_prepend(ସଂରଚନା k_queue *queue,
						   ଶୂନ୍ୟ *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ଫେରନ୍ତୁ z_impl_k_queue_alloc_prepend(queue, data);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_queue_append_list(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *head, ଶୂନ୍ୟ *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == ଖାଲି) || (tail == ଖାଲି)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		ଫେରନ୍ତୁ -EINVAL;
	}

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&queue->lock);

	ଯେତେବେଳେ ((head != ଖାଲି) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(ଶୂନ୍ୟ **)head;
	}

	ଯଦି (head != ଖାଲି) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	ଯଦି (resched) {
		z_reschedule(&queue->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&queue->lock, ଚାବି);
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_queue_merge_slist(ସଂରଚନା k_queue *queue, sys_slist_t *list)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		ଫେରନ୍ତୁ -EINVAL;
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

		ଫେରନ୍ତୁ ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ *z_impl_k_queue_get(ସଂରଚନା k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&queue->lock);
	ଶୂନ୍ୟ *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	ଯଦି (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *ଗଣ୍ଠି;

		ଗଣ୍ଠି = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_ଗଣ୍ଠି_peek(ଗଣ୍ଠି, true);
		k_spin_unlock(&queue->lock, ଚାବି);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		ଫେରନ୍ତୁ data;
	}

	ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, ଚାବି);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, ଖାଲି);

		ଫେରନ୍ତୁ ଖାଲି;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = z_pend_curr(&queue->lock, ଚାବି, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? ଖାଲି : _current->base.swap_data);

	ଫେରନ୍ତୁ (ret != 0) ? ଖାଲି : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = ଖାଲି;
	sys_sfnode_t *ଗଣ୍ଠି = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	ଯେତେବେଳେ (ଗଣ୍ଠି != ଖାଲି) {
		ଶୂନ୍ୟ *peeked = z_queue_ଗଣ୍ଠି_peek(ଗଣ୍ଠି, false);

		ଯଦି (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, ଗଣ୍ଠି);
			(ଶୂନ୍ୟ)z_queue_ଗଣ୍ଠି_peek(ଗଣ୍ଠି, true);
			ret = true;
			ଭାଙ୍ଗ;
		}
		prev = ଗଣ୍ଠି;
		ଗଣ୍ଠି = sys_sflist_peek_next(ଗଣ୍ଠି);
	}

	k_spin_unlock(&queue->lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	ଫେରନ୍ତୁ ret;
}

bool k_queue_unique_append(ସଂରଚନା k_queue *queue, ଶୂନ୍ୟ *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_ଗଣ୍ଠି(&queue->data_q, test) {
		ଯଦି (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, ଚାବି);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			ଫେରନ୍ତୁ false;
		}
	}

	(ଶୂନ୍ୟ)queue_ଭର୍ତ୍ତି(queue, ଖାଲି, data, false, true, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	ଫେରନ୍ତୁ true;
}

ଶୂନ୍ୟ *z_impl_k_queue_peek_head(ସଂରଚନା k_queue *queue)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&queue->lock);
	ଶୂନ୍ୟ *ret = z_queue_ଗଣ୍ଠି_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ *z_impl_k_queue_peek_tail(ସଂରଚନା k_queue *queue)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&queue->lock);
	ଶୂନ୍ୟ *ret = z_queue_ଗଣ୍ଠି_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *z_vrfy_k_queue_get(ସଂରଚନା k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ଫେରନ୍ତୁ z_impl_k_queue_get(queue, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_get_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_queue_is_empty(ସଂରଚନା k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ଫେରନ୍ତୁ z_impl_k_queue_is_empty(queue);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *z_vrfy_k_queue_peek_head(ସଂରଚନା k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ଫେରନ୍ତୁ z_impl_k_queue_peek_head(queue);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *z_vrfy_k_queue_peek_tail(ସଂରଚନା k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ଫେରନ୍ତୁ z_impl_k_queue_peek_tail(queue);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ଖାଲି, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ଖାଲି, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
