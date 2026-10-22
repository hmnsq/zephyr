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


#অন্তর্ভাব <zephyr/kernel.h>

#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <scheduler.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

কাঠামো alloc_গ্রন্থি {
	sys_sfnode_t গ্রন্থি;
	শূন্য *তথ্য;
};

/* The queue must have its spinlock held before calling this function. */
স্থির শূন্য *z_queue_গ্রন্থি_peek(sys_sfnode_t *গ্রন্থি, bool needs_free)
{
	শূন্য *ret;

	যদি ((গ্রন্থি != ফাঁকা) && (sys_sfnode_flags_get(গ্রন্থি) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		কাঠামো alloc_গ্রন্থি *anode;

		anode = CONTAINER_OF(গ্রন্থি, কাঠামো alloc_গ্রন্থি, গ্রন্থি);
		ret = anode->তথ্য;
		যদি (needs_free) {
			k_free(anode);
		}
	} অন্যথা {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (শূন্য *)গ্রন্থি;
	}

	ফেরত ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(কাঠামো k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(কাঠামো k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, ফাঁকা,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

শূন্য z_impl_k_queue_init(কাঠামো k_queue *queue)
{
	sys_sflist_init(&queue->তথ্য_q);
	queue->lock = (কাঠামো k_spinlock) {};
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
স্থির অন্তঃরেখ শূন্য z_vrfy_k_queue_init(কাঠামো k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

স্থির অন্তঃরেখ bool queue_handle_poll_events(কাঠামো k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	ফেরত z_handle_obj_poll_events(&queue->poll_events, state);
#অন্যথা
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	ফেরত false;
#endif /* CONFIG_POLL */
}

শূন্য z_impl_k_queue_cancel_wait(কাঠামো k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	bool resched = false;

	যদি (z_sched_wake(&queue->wait_q, 0, ফাঁকা)) {
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
স্থির অন্তঃরেখ শূন্য z_vrfy_k_queue_cancel_wait(কাঠামো k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
স্থির int32_t queue_ঢোকাও(কাঠামো k_queue *queue, শূন্য *পূর্ব, শূন্য *তথ্য,
			    bool alloc, bool is_append, k_spinlock_চাবি_t চাবি)
{
	int32_t ফলাফল = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_ঢোকাও, queue, alloc);

	যদি (is_append) {
		পূর্ব = sys_sflist_peek_লেজ(&queue->তথ্য_q);
	}

	যদি (z_sched_wake(&queue->wait_q, 0, তথ্য)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_ঢোকাও, queue, alloc, K_FOREVER);
		resched = true;
		যাও out;
	}

	/* Only need to actually allocate if no threads are pending */
	যদি (alloc) {
		কাঠামো alloc_গ্রন্থি *anode;

		anode = z_thread_malloc(আকার(*anode));
		যদি (anode == ফাঁকা) {
			ফলাফল = -ENOMEM;
			যাও out;
		}
		anode->তথ্য = তথ্য;
		sys_sfnode_init(&anode->গ্রন্থি, 0x1);
		তথ্য = anode;
	} অন্যথা {
		sys_sfnode_init(তথ্য, 0x0);
	}

	sys_sflist_ঢোকাও(&queue->তথ্য_q, পূর্ব, তথ্য);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_তথ্য_AVAILABLE);

out:
	যদি (resched) {
		z_reschedule(&queue->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&queue->lock, চাবি);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_ঢোকাও, queue, alloc, ফলাফল);

	ফেরত ফলাফল;
}

শূন্য k_queue_ঢোকাও(কাঠামো k_queue *queue, শূন্য *পূর্ব, শূন্য *তথ্য)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, ঢোকাও, queue);

	(শূন্য)queue_ঢোকাও(queue, পূর্ব, তথ্য, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, ঢোকাও, queue);
}

শূন্য k_queue_append(কাঠামো k_queue *queue, শূন্য *তথ্য)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(শূন্য)queue_ঢোকাও(queue, ফাঁকা, তথ্য, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

শূন্য k_queue_prepend(কাঠামো k_queue *queue, শূন্য *তথ্য)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(শূন্য)queue_ঢোকাও(queue, ফাঁকা, তথ্য, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(কাঠামো k_queue *queue, শূন্য *তথ্য)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_ঢোকাও(queue, ফাঁকা, তথ্য, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	ফেরত ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ int32_t z_vrfy_k_queue_alloc_append(কাঠামো k_queue *queue,
						  শূন্য *তথ্য)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ফেরত z_impl_k_queue_alloc_append(queue, তথ্য);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(কাঠামো k_queue *queue, শূন্য *তথ্য)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_ঢোকাও(queue, ফাঁকা, তথ্য, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	ফেরত ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ int32_t z_vrfy_k_queue_alloc_prepend(কাঠামো k_queue *queue,
						   শূন্য *তথ্য)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ফেরত z_impl_k_queue_alloc_prepend(queue, তথ্য);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা k_queue_append_list(কাঠামো k_queue *queue, শূন্য *শীর্ষ, শূন্য *লেজ)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((শীর্ষ == ফাঁকা) || (লেজ == ফাঁকা)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		ফেরত -EINVAL;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);

	যখন ((শীর্ষ != ফাঁকা) && z_sched_wake(&queue->wait_q, 0, শীর্ষ)) {
		resched = true;
		শীর্ষ = *(শূন্য **)শীর্ষ;
	}

	যদি (শীর্ষ != ফাঁকা) {
		sys_sflist_append_list(&queue->তথ্য_q, শীর্ষ, লেজ);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_তথ্য_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	যদি (resched) {
		z_reschedule(&queue->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&queue->lock, চাবি);
	}

	ফেরত 0;
}

পূর্ণসংখ্যা k_queue_merge_slist(কাঠামো k_queue *queue, sys_slist_t *list)
{
	পূর্ণসংখ্যা ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		ফেরত -EINVAL;
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
	ret = k_queue_append_list(queue, list->শীর্ষ, list->লেজ);
	CHECKIF(ret != 0) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, ret);

		ফেরত ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	ফেরত 0;
}

শূন্য *z_impl_k_queue_get(কাঠামো k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	শূন্য *তথ্য;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	যদি (likely(!sys_sflist_is_empty(&queue->তথ্য_q))) {
		sys_sfnode_t *গ্রন্থি;

		গ্রন্থি = sys_sflist_get_not_empty(&queue->তথ্য_q);
		তথ্য = z_queue_গ্রন্থি_peek(গ্রন্থি, true);
		k_spin_unlock(&queue->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, তথ্য);

		ফেরত তথ্য;
	}

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, ফাঁকা);

		ফেরত ফাঁকা;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	পূর্ণসংখ্যা ret = z_pend_curr(&queue->lock, চাবি, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? ফাঁকা : _current->base.swap_তথ্য);

	ফেরত (ret != 0) ? ফাঁকা : _current->base.swap_তথ্য;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(কাঠামো k_queue *queue, শূন্য *তথ্য)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	sys_sfnode_t *পূর্ব = ফাঁকা;
	sys_sfnode_t *গ্রন্থি = sys_sflist_peek_শীর্ষ(&queue->তথ্য_q);
	bool ret = false;

	যখন (গ্রন্থি != ফাঁকা) {
		শূন্য *peeked = z_queue_গ্রন্থি_peek(গ্রন্থি, false);

		যদি (peeked == তথ্য) {
			sys_sflist_remove(&queue->তথ্য_q, পূর্ব, গ্রন্থি);
			(শূন্য)z_queue_গ্রন্থি_peek(গ্রন্থি, true);
			ret = true;
			বিরতি;
		}
		পূর্ব = গ্রন্থি;
		গ্রন্থি = sys_sflist_peek_পরবর্তী(গ্রন্থি);
	}

	k_spin_unlock(&queue->lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	ফেরত ret;
}

bool k_queue_unique_append(কাঠামো k_queue *queue, শূন্য *তথ্য)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_গ্রন্থি(&queue->তথ্য_q, test) {
		যদি (test == (sys_sfnode_t *) তথ্য) {
			k_spin_unlock(&queue->lock, চাবি);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			ফেরত false;
		}
	}

	(শূন্য)queue_ঢোকাও(queue, ফাঁকা, তথ্য, false, true, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	ফেরত true;
}

শূন্য *z_impl_k_queue_peek_শীর্ষ(কাঠামো k_queue *queue)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	শূন্য *ret = z_queue_গ্রন্থি_peek(sys_sflist_peek_শীর্ষ(&queue->তথ্য_q), false);

	k_spin_unlock(&queue->lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_শীর্ষ, queue, ret);

	ফেরত ret;
}

শূন্য *z_impl_k_queue_peek_লেজ(কাঠামো k_queue *queue)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&queue->lock);
	শূন্য *ret = z_queue_গ্রন্থি_peek(sys_sflist_peek_লেজ(&queue->তথ্য_q), false);

	k_spin_unlock(&queue->lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_লেজ, queue, ret);

	ফেরত ret;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ শূন্য *z_vrfy_k_queue_get(কাঠামো k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ফেরত z_impl_k_queue_get(queue, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_get_mrsh.c>

স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_queue_is_empty(কাঠামো k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ফেরত z_impl_k_queue_is_empty(queue);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_is_empty_mrsh.c>

স্থির অন্তঃরেখ শূন্য *z_vrfy_k_queue_peek_শীর্ষ(কাঠামো k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ফেরত z_impl_k_queue_peek_শীর্ষ(queue);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_peek_head_mrsh.c>

স্থির অন্তঃরেখ শূন্য *z_vrfy_k_queue_peek_লেজ(কাঠামো k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	ফেরত z_impl_k_queue_peek_লেজ(queue);
}
#অন্তর্ভাব <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ফাঁকা, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ফাঁকা, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
