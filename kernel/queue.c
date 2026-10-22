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

BUILD_ASSERT(SYS_SFLIST_FLAG_द्वयङ्काः >= 1, "k_queue needs one sflist flag bit");

संरचना alloc_ग्रन्थिः {
	sys_sfnode_t ग्रन्थिः;
	शून्य *दत्त;
};

/* The queue must have its spinlock held before calling this function. */
स्थिर शून्य *z_queue_ग्रन्थिः_peek(sys_sfnode_t *ग्रन्थिः, bool needs_free)
{
	शून्य *ret;

	यदि ((ग्रन्थिः != रिक्त) && (sys_sfnode_flags_get(ग्रन्थिः) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		संरचना alloc_ग्रन्थिः *anode;

		anode = CONTAINER_सम्बन्धि(ग्रन्थिः, संरचना alloc_ग्रन्थिः, ग्रन्थिः);
		ret = anode->दत्त;
		यदि (needs_free) {
			k_free(anode);
		}
	} अन्यथा {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (शून्य *)ग्रन्थिः;
	}

	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(संरचना k_fifo, _queue) == 0);
BUILD_ASSERT(offsetof(संरचना k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_अन्तः_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_अन्तः_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_अन्तः_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, रिक्त,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#यदि_अन्तः /* CONFIG_OBJ_CORE_QUEUE */

शून्य z_impl_k_queue_init(संरचना k_queue *queue)
{
	sys_sflist_init(&queue->दत्त_q);
	queue->तालकम् = (संरचना k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#यदि defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#यदि_अन्तः

	SYS_पत्तनम्_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#यदि_अन्तः /* CONFIG_OBJ_CORE_QUEUE */
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_queue_init(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_init_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

स्थिर अन्तःस्थ bool queue_handle_poll_events(संरचना k_queue *queue, uint32_t state)
{
#यदि_परिभाषितम् CONFIG_POLL
	प्रत्यागम z_handle_obj_poll_events(&queue->poll_events, state);
#अन्यथा
	ARG_अप्रयुक्तम्(queue);
	ARG_अप्रयुक्तम्(state);

	प्रत्यागम false;
#यदि_अन्तः /* CONFIG_POLL */
}

शून्य z_impl_k_queue_cancel_wait(संरचना k_queue *queue)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&queue->तालकम्);
	bool resched = false;

	यदि (z_sched_wake(&queue->wait_q, 0, रिक्त)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	यदि (resched) {
		z_reschedule(&queue->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&queue->तालकम्, कुञ्चिका);
	}
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_queue_cancel_wait(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
स्थिर int32_t queue_निवेशय(संरचना k_queue *queue, शून्य *पूर्व, शून्य *दत्त,
			    bool alloc, bool is_append, k_spinlock_कुञ्चिका_t कुञ्चिका)
{
	int32_t परिणाम = 0;
	bool resched = false;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, queue_निवेशय, queue, alloc);

	यदि (is_append) {
		पूर्व = sys_sflist_peek_पुच्छ(&queue->दत्त_q);
	}

	यदि (z_sched_wake(&queue->wait_q, 0, दत्त)) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_निवेशय, queue, alloc, K_FOREVER);
		resched = true;
		गच्छ बहिर्गमनम्;
	}

	/* Only need to actually allocate if no threads are pending */
	यदि (alloc) {
		संरचना alloc_ग्रन्थिः *anode;

		anode = z_thread_malloc(परिमाण(*anode));
		यदि (anode == रिक्त) {
			परिणाम = -ENOMEM;
			गच्छ बहिर्गमनम्;
		}
		anode->दत्त = दत्त;
		sys_sfnode_init(&anode->ग्रन्थिः, 0x1);
		दत्त = anode;
	} अन्यथा {
		sys_sfnode_init(दत्त, 0x0);
	}

	sys_sflist_निवेशय(&queue->दत्त_q, पूर्व, दत्त);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_दत्त_उपलब्धम्);

बहिर्गमनम्:
	यदि (resched) {
		z_reschedule(&queue->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&queue->तालकम्, कुञ्चिका);
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, queue_निवेशय, queue, alloc, परिणाम);

	प्रत्यागम परिणाम;
}

शून्य k_queue_निवेशय(संरचना k_queue *queue, शून्य *पूर्व, शून्य *दत्त)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, निवेशय, queue);

	(शून्य)queue_निवेशय(queue, पूर्व, दत्त, false, false,
			   k_spin_तालकम्(&queue->तालकम्));

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, निवेशय, queue);
}

शून्य k_queue_append(संरचना k_queue *queue, शून्य *दत्त)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(शून्य)queue_निवेशय(queue, रिक्त, दत्त, false, true,
			   k_spin_तालकम्(&queue->तालकम्));

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

शून्य k_queue_prepend(संरचना k_queue *queue, शून्य *दत्त)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(शून्य)queue_निवेशय(queue, रिक्त, दत्त, false, false,
			   k_spin_तालकम्(&queue->तालकम्));

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(संरचना k_queue *queue, शून्य *दत्त)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_निवेशय(queue, रिक्त, दत्त, true, true,
				   k_spin_तालकम्(&queue->तालकम्));

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_queue_alloc_append(संरचना k_queue *queue,
						  शून्य *दत्त)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_alloc_append(queue, दत्त);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(संरचना k_queue *queue, शून्य *दत्त)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_निवेशय(queue, रिक्त, दत्त, true, false,
				   k_spin_तालकम्(&queue->तालकम्));

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ int32_t z_vrfy_k_queue_alloc_prepend(संरचना k_queue *queue,
						   शून्य *दत्त)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_alloc_prepend(queue, दत्त);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क k_queue_append_list(संरचना k_queue *queue, शून्य *शीर्ष, शून्य *पुच्छ)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((शीर्ष == रिक्त) || (पुच्छ == रिक्त)) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		प्रत्यागम -EINVAL;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&queue->तालकम्);

	यावत् ((शीर्ष != रिक्त) && z_sched_wake(&queue->wait_q, 0, शीर्ष)) {
		resched = true;
		शीर्ष = *(शून्य **)शीर्ष;
	}

	यदि (शीर्ष != रिक्त) {
		sys_sflist_append_list(&queue->दत्त_q, शीर्ष, पुच्छ);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_दत्त_उपलब्धम्) || resched;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	यदि (resched) {
		z_reschedule(&queue->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&queue->तालकम्, कुञ्चिका);
	}

	प्रत्यागम 0;
}

पूर्णाङ्क k_queue_merge_slist(संरचना k_queue *queue, sys_slist_t *list)
{
	पूर्णाङ्क ret;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_रिक्तानि(list)) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

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
	ret = k_queue_append_list(queue, list->शीर्ष, list->पुच्छ);
	CHECKIF(ret != 0) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, ret);

		प्रत्यागम ret;
	}
	sys_slist_init(list);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	प्रत्यागम 0;
}

शून्य *z_impl_k_queue_get(संरचना k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&queue->तालकम्);
	शून्य *दत्त;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	यदि (likely(!sys_sflist_is_रिक्तानि(&queue->दत्त_q))) {
		sys_sfnode_t *ग्रन्थिः;

		ग्रन्थिः = sys_sflist_get_not_रिक्तानि(&queue->दत्त_q);
		दत्त = z_queue_ग्रन्थिः_peek(ग्रन्थिः, true);
		k_spin_unlock(&queue->तालकम्, कुञ्चिका);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, दत्त);

		प्रत्यागम दत्त;
	}

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->तालकम्, कुञ्चिका);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, रिक्त);

		प्रत्यागम रिक्त;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	पूर्णाङ्क ret = z_pend_curr(&queue->तालकम्, कुञ्चिका, &queue->wait_q, timeout);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? रिक्त : _current->आधारः.swap_दत्त);

	प्रत्यागम (ret != 0) ? रिक्त : _current->आधारः.swap_दत्त;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(संरचना k_queue *queue, शून्य *दत्त)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, remove, queue);
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&queue->तालकम्);
	sys_sfnode_t *पूर्व = रिक्त;
	sys_sfnode_t *ग्रन्थिः = sys_sflist_peek_शीर्ष(&queue->दत्त_q);
	bool ret = false;

	यावत् (ग्रन्थिः != रिक्त) {
		शून्य *peeked = z_queue_ग्रन्थिः_peek(ग्रन्थिः, false);

		यदि (peeked == दत्त) {
			sys_sflist_remove(&queue->दत्त_q, पूर्व, ग्रन्थिः);
			(शून्य)z_queue_ग्रन्थिः_peek(ग्रन्थिः, true);
			ret = true;
			भङ्ग;
		}
		पूर्व = ग्रन्थिः;
		ग्रन्थिः = sys_sflist_peek_अग्र(ग्रन्थिः);
	}

	k_spin_unlock(&queue->तालकम्, कुञ्चिका);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, remove, queue, ret);

	प्रत्यागम ret;
}

bool k_queue_unique_append(संरचना k_queue *queue, शून्य *दत्त)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&queue->तालकम्);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_ग्रन्थिः(&queue->दत्त_q, test) {
		यदि (test == (sys_sfnode_t *) दत्त) {
			k_spin_unlock(&queue->तालकम्, कुञ्चिका);
			SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			प्रत्यागम false;
		}
	}

	(शून्य)queue_निवेशय(queue, रिक्त, दत्त, false, true, कुञ्चिका);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	प्रत्यागम true;
}

शून्य *z_impl_k_queue_peek_शीर्ष(संरचना k_queue *queue)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&queue->तालकम्);
	शून्य *ret = z_queue_ग्रन्थिः_peek(sys_sflist_peek_शीर्ष(&queue->दत्त_q), false);

	k_spin_unlock(&queue->तालकम्, कुञ्चिका);

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_queue, peek_शीर्ष, queue, ret);

	प्रत्यागम ret;
}

शून्य *z_impl_k_queue_peek_पुच्छ(संरचना k_queue *queue)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&queue->तालकम्);
	शून्य *ret = z_queue_ग्रन्थिः_peek(sys_sflist_peek_पुच्छ(&queue->दत्त_q), false);

	k_spin_unlock(&queue->तालकम्, कुञ्चिका);

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_queue, peek_पुच्छ, queue, ret);

	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य *z_vrfy_k_queue_get(संरचना k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_get(queue, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_get_mrsh.c>

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_queue_is_रिक्तानि(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_is_रिक्तानि(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_is_empty_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_queue_peek_शीर्ष(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_peek_शीर्ष(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_peek_head_mrsh.c>

स्थिर अन्तःस्थ शून्य *z_vrfy_k_queue_peek_पुच्छ(संरचना k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	प्रत्यागम z_impl_k_queue_peek_पुच्छ(queue);
}
#अन्तर्भाव <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_अन्तः_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, रिक्त, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#यदि_अन्तः /* CONFIG_OBJ_CORE_FIFO */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_अन्तः_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, रिक्त, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#यदि_अन्तः /* CONFIG_OBJ_CORE_LIFO */
