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


#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

संरचना alloc_गाँठ {
	sys_sfnode_t गाँठ;
	शून्य *data;
};

/* The queue must have its spinlock held before calling this function. */
स्थिर शून्य *z_queue_गाँठ_peek(sys_sfnode_t *गाँठ, bool needs_free)
{
	शून्य *ret;

	यदि ((गाँठ != रिक्त) && (sys_sfnode_flags_get(गाँठ) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		संरचना alloc_गाँठ *anode;

		anode = CONTAINER_OF(गाँठ, संरचना alloc_गाँठ, गाँठ);
		ret = anode->data;
		यदि (needs_free) {
			k_free(anode);
		}
	} अन्यथा {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (शून्य *)गाँठ;
	}

	प्रत्यागम ret;
}

#ifdef CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(दुरत्व(संरचना k_fifo, _queue) == 0);
BUILD_ASSERT(दुरत्व(संरचना k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, रिक्त,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_QUEUE */

शून्य z_impl_k_queue_init(संरचना k_queue *queue)
{
	sys_sflist_init(&queue->data_q);
	queue->lock = (संरचना k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#यदि defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#endif

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#ifdef CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#endif /* CONFIG_OBJ_CORE_QUEUE */
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_queue_init(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

स्थिर अन्तःस्थ bool queue_handle_poll_events(संरचना k_queue *queue, uint32_t state)
{
#ifdef CONFIG_POLL
	प्रत्यागम z_handle_obj_poll_events(&queue->poll_events, state);
#अन्यथा
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	प्रत्यागम false;
#endif /* CONFIG_POLL */
}

शून्य z_impl_k_queue_cancel_wait(संरचना k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	bool resched = false;

	यदि (z_sched_wake(&queue->wait_q, 0, रिक्त)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	यदि (resched) {
		z_reschedule(&queue->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&queue->lock, कुंजी);
	}
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_queue_cancel_wait(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
स्थिर int32_t queue_जोड़ो(संरचना k_queue *queue, शून्य *prev, शून्य *data,
			    bool alloc, bool is_append, k_spinlock_कुंजी_t कुंजी)
{
	int32_t result = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_जोड़ो, queue, alloc);

	यदि (is_append) {
		prev = sys_sflist_peek_tail(&queue->data_q);
	}

	यदि (z_sched_wake(&queue->wait_q, 0, data)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_जोड़ो, queue, alloc, K_FOREVER);
		resched = true;
		गच्छ out;
	}

	/* Only need to actually allocate if no threads are pending */
	यदि (alloc) {
		संरचना alloc_गाँठ *anode;

		anode = z_thread_malloc(परिमाण(*anode));
		यदि (anode == रिक्त) {
			result = -ENOMEM;
			गच्छ out;
		}
		anode->data = data;
		sys_sfnode_init(&anode->गाँठ, 0x1);
		data = anode;
	} अन्यथा {
		sys_sfnode_init(data, 0x0);
	}

	sys_sflist_जोड़ो(&queue->data_q, prev, data);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE);

out:
	यदि (resched) {
		z_reschedule(&queue->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&queue->lock, कुंजी);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_जोड़ो, queue, alloc, result);

	प्रत्यागम result;
}

शून्य k_queue_जोड़ो(संरचना k_queue *queue, शून्य *prev, शून्य *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, जोड़ो, queue);

	(शून्य)queue_जोड़ो(queue, prev, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, जोड़ो, queue);
}

शून्य k_queue_append(संरचना k_queue *queue, शून्य *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(शून्य)queue_जोड़ो(queue, रिक्त, data, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

शून्य k_queue_prepend(संरचना k_queue *queue, शून्य *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(शून्य)queue_जोड़ो(queue, रिक्त, data, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(संरचना k_queue *queue, शून्य *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_जोड़ो(queue, रिक्त, data, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	प्रत्यागम ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_queue_alloc_append(संरचना k_queue *queue,
						  शून्य *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_alloc_append(queue, data);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#endif /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(संरचना k_queue *queue, शून्य *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_जोड़ो(queue, रिक्त, data, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	प्रत्यागम ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_queue_alloc_prepend(संरचना k_queue *queue,
						   शून्य *data)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_alloc_prepend(queue, data);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क k_queue_append_list(संरचना k_queue *queue, शून्य *head, शून्य *tail)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((head == रिक्त) || (tail == रिक्त)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		प्रत्यागम -EINVAL;
	}

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);

	यावत् ((head != रिक्त) && z_sched_wake(&queue->wait_q, 0, head)) {
		resched = true;
		head = *(शून्य **)head;
	}

	यदि (head != रिक्त) {
		sys_sflist_append_list(&queue->data_q, head, tail);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_DATA_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	यदि (resched) {
		z_reschedule(&queue->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&queue->lock, कुंजी);
	}

	प्रत्यागम 0;
}

पूर्णाङ्क k_queue_merge_slist(संरचना k_queue *queue, sys_slist_t *list)
{
	पूर्णाङ्क ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		प्रत्यागम -EINVAL;
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

		प्रत्यागम ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	प्रत्यागम 0;
}

शून्य *z_impl_k_queue_get(संरचना k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	शून्य *data;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	यदि (likely(!sys_sflist_is_empty(&queue->data_q))) {
		sys_sfnode_t *गाँठ;

		गाँठ = sys_sflist_get_not_empty(&queue->data_q);
		data = z_queue_गाँठ_peek(गाँठ, true);
		k_spin_unlock(&queue->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, data);

		प्रत्यागम data;
	}

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, रिक्त);

		प्रत्यागम रिक्त;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	पूर्णाङ्क ret = z_pend_curr(&queue->lock, कुंजी, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? रिक्त : _current->base.swap_data);

	प्रत्यागम (ret != 0) ? रिक्त : _current->base.swap_data;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(संरचना k_queue *queue, शून्य *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, हटाओ, queue);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	sys_sfnode_t *prev = रिक्त;
	sys_sfnode_t *गाँठ = sys_sflist_peek_head(&queue->data_q);
	bool ret = false;

	यावत् (गाँठ != रिक्त) {
		शून्य *peeked = z_queue_गाँठ_peek(गाँठ, false);

		यदि (peeked == data) {
			sys_sflist_remove(&queue->data_q, prev, गाँठ);
			(शून्य)z_queue_गाँठ_peek(गाँठ, true);
			ret = true;
			भङ्ग;
		}
		prev = गाँठ;
		गाँठ = sys_sflist_peek_next(गाँठ);
	}

	k_spin_unlock(&queue->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, हटाओ, queue, ret);

	प्रत्यागम ret;
}

bool k_queue_unique_append(संरचना k_queue *queue, शून्य *data)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_गाँठ(&queue->data_q, test) {
		यदि (test == (sys_sfnode_t *) data) {
			k_spin_unlock(&queue->lock, कुंजी);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			प्रत्यागम false;
		}
	}

	(शून्य)queue_जोड़ो(queue, रिक्त, data, false, true, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	प्रत्यागम true;
}

शून्य *z_impl_k_queue_peek_head(संरचना k_queue *queue)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	शून्य *ret = z_queue_गाँठ_peek(sys_sflist_peek_head(&queue->data_q), false);

	k_spin_unlock(&queue->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_head, queue, ret);

	प्रत्यागम ret;
}

शून्य *z_impl_k_queue_peek_tail(संरचना k_queue *queue)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	शून्य *ret = z_queue_गाँठ_peek(sys_sflist_peek_tail(&queue->data_q), false);

	k_spin_unlock(&queue->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_tail, queue, ret);

	प्रत्यागम ret;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य *z_vrfy_k_queue_get(संरचना k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_get(queue, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_get_mrsh.c>

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_queue_is_empty(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_is_empty(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_is_empty_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_queue_peek_head(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_peek_head(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_peek_head_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_queue_peek_tail(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_peek_tail(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, रिक्त, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#endif /* CONFIG_OBJ_CORE_FIFO */

#ifdef CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, रिक्त, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#endif /* CONFIG_OBJ_CORE_LIFO */
