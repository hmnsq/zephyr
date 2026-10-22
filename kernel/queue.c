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


#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <scheduler.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

સંરચના alloc_ગાંઠ {
	sys_sfnode_t ગાંઠ;
	શૂન્ય *data;
};

/* The queue must have its spinlock held before calling this function. */
સ્થિર શૂન્ય *z_queue_ગાંઠ_peek(sys_sfnode_t *ગાંઠ, bool needs_free)
{
	શૂન્ય *ret;

	જો ((ગાંઠ != ખાલી) && (sys_sfnode_flags_get(ગાંઠ) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		સંરચના alloc_ગાંઠ *anode;

		anode = CONTAINER_OF(ગાંઠ, સંરચના alloc_ગાંઠ, ગાંઠ);
		ret = anode->data;
		જો (needs_free) {
			k_free(anode);
		}
	} નહીંતર {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (શૂન્ય *)ગાંઠ;
	}

	પાછા ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(સંરચના k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(સંરચના k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, ખાલી,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

શૂન્ય z_impl_k_queue_init(સંરચના k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (સંરચના k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#જો defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_queue_init(સંરચના k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

સ્થિર અંતઃરેખ bool queue_handle_poll_events(સંરચના k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	પાછા z_handle_obj_poll_events(&queue->poll_events, state);
#નહીંતર
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	પાછા false;
#endif /* CONFIG_POLL */
}

શૂન્ય z_impl_k_queue_cancel_wait(સંરચના k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&queue->lock);
	bool resched = false;

	જો (z_sched_wake(&queue->wait_q, 0, ખાલી)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	જો (resched) {
		z_reschedule(&queue->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&queue->lock, ચાવી);
	}
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_queue_cancel_wait(સંરચના k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
સ્થિર int32_t queue_ઉમેરો(સંરચના k_queue *queue, શૂન્ય *prev, શૂન્ય *data,
			    bool alloc, bool is_append, k_spinlock_ચાવી_t ચાવી)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_ઉમેરો, queue, alloc);

	જો (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	જો (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_ઉમેરો, queue, alloc, K_FOREVER);
		resched = true;
		જાઓ out;
	}

	/* Only need to actually allocate if no threads are pending */
	જો (alloc) {
		સંરચના alloc_ગાંઠ *anode;

		anode = z_thread_malloc(કદ(*anode));
		જો (anode == ખાલી) {
			result = -ENOMEM;
			જાઓ out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->ગાંઠ, 0x1);
		data = anode;
	} નહીંતર {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_ઉમેરો(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	જો (resched) {
		z_reschedule(&queue->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&queue->lock, ચાવી);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_ઉમેરો, queue, alloc, result);

	પાછા result;
}

શૂન્ય k_queue_ઉમેરો(સંરચના k_queue *queue, શૂન્ય *prev, શૂન્ય *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, ઉમેરો, queue);

	(શૂન્ય)queue_ઉમેરો(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, ઉમેરો, queue);
}

શૂન્ય k_queue_append(સંરચના k_queue *queue, શૂન્ય *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(શૂન્ય)queue_ઉમેરો(queue, ખાલી, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

શૂન્ય k_queue_prepend(સંરચના k_queue *queue, શૂન્ય *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(શૂન્ય)queue_ઉમેરો(queue, ખાલી, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(સંરચના k_queue *queue, શૂન્ય *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_ઉમેરો(queue, ખાલી, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	પાછા ret;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ int32_t z_vrfy_k_queue_alloc_append(સંરચના k_queue *queue,
						  શૂન્ય *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	પાછા z_impl_k_queue_alloc_append(queue, data);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(સંરચના k_queue *queue, શૂન્ય *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_ઉમેરો(queue, ખાલી, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	પાછા ret;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ int32_t z_vrfy_k_queue_alloc_prepend(સંરચના k_queue *queue,
						   શૂન્ય *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	પાછા z_impl_k_queue_alloc_prepend(queue, data);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક k_queue_append_list(સંરચના k_queue *queue, શૂન્ય *head, શૂન્ય *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == ખાલી) || (tail == ખાલી)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		પાછા -EINVAL;
	}

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&queue->lock);

	જ્યારે ((head != ખાલી) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(શૂન્ય **)head;
	}

	જો (head != ખાલી) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	જો (resched) {
		z_reschedule(&queue->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&queue->lock, ચાવી);
	}

	પાછા 0;
}

પૂર્ણાંક k_queue_merge_slist(સંરચના k_queue *queue, sys_slist_t *list)
{
	પૂર્ણાંક ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		પાછા -EINVAL;
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

		પાછા ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	પાછા 0;
}

શૂન્ય *z_impl_k_queue_get(સંરચના k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&queue->lock);
	શૂન્ય *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	જો (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *ગાંઠ;

		ગાંઠ = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_ગાંઠ_peek(ગાંઠ, true);
		k_spin_unlock(&queue->lock, ચાવી);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		પાછા data;
	}

	જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, ચાવી);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, ખાલી);

		પાછા ખાલી;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	પૂર્ણાંક ret = z_pend_curr(&queue->lock, ચાવી, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? ખાલી : _current->base.swap_data);

	પાછા (ret != 0) ? ખાલી : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(સંરચના k_queue *queue, શૂન્ય *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = ખાલી;
	sys_sfnode_t *ગાંઠ = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	જ્યારે (ગાંઠ != ખાલી) {
		શૂન્ય *peeked = z_queue_ગાંઠ_peek(ગાંઠ, false);

		જો (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, ગાંઠ);
			(શૂન્ય)z_queue_ગાંઠ_peek(ગાંઠ, true);
			ret = true;
			તોડો;
		}
		prev = ગાંઠ;
		ગાંઠ = sys_sflist_peek_next(ગાંઠ);
	}

	k_spin_unlock(&queue->lock, ચાવી);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	પાછા ret;
}

bool k_queue_unique_append(સંરચના k_queue *queue, શૂન્ય *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_ગાંઠ(&queue->data_q, test) {
		જો (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, ચાવી);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			પાછા false;
		}
	}

	(શૂન્ય)queue_ઉમેરો(queue, ખાલી, data, false, true, ચાવી);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	પાછા true;
}

શૂન્ય *z_impl_k_queue_peek_head(સંરચના k_queue *queue)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&queue->lock);
	શૂન્ય *ret = z_queue_ગાંઠ_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, ચાવી);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	પાછા ret;
}

શૂન્ય *z_impl_k_queue_peek_tail(સંરચના k_queue *queue)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&queue->lock);
	શૂન્ય *ret = z_queue_ગાંઠ_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, ચાવી);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	પાછા ret;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય *z_vrfy_k_queue_get(સંરચના k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	પાછા z_impl_k_queue_get(queue, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_get_mrsh.c>

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_queue_is_empty(સંરચના k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	પાછા z_impl_k_queue_is_empty(queue);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય *z_vrfy_k_queue_peek_head(સંરચના k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	પાછા z_impl_k_queue_peek_head(queue);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય *z_vrfy_k_queue_peek_tail(સંરચના k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	પાછા z_impl_k_queue_peek_tail(queue);
}
#અન્તર્ભાવ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ખાલી, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ખાલી, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
