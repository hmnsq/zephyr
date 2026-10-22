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


#समावेश <zephyr/kernel.h>

#समावेश <zephyr/toolchain.h>
#समावेश <wait_q.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <kernel_internal.h>
#समावेश <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_BITS >= 1, "k_queue needs one sflist flag bit");

काष्ठा alloc_गाँठ {
	sys_sfnode_t गाँठ;
	व्योम *तथ्य;
};

/* The queue must have its spinlock held before calling this function. */
अटल व्योम *z_queue_गाँठ_peek(sys_sfnode_t *गाँठ, bool needs_free)
{
	व्योम *ret;

	अगर ((गाँठ != शून्य) && (sys_sfnode_flags_get(गाँठ) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		काष्ठा alloc_गाँठ *anode;

		anode = CONTAINER_OF(गाँठ, काष्ठा alloc_गाँठ, गाँठ);
		ret = anode->तथ्य;
		अगर (needs_free) {
			k_free(anode);
		}
	} अन्यथा {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (व्योम *)गाँठ;
	}

	वापस ret;
}

#अगर_घोषित CONFIG_OBJ_CORE_QUEUE
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(दुरत्व(काष्ठा k_fifo, _queue) == 0);
BUILD_ASSERT(दुरत्व(काष्ठा k_lifo, _queue) == 0);
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_START_EXTERN(k_queue);
STRUCT_SECTION_END_EXTERN(k_queue);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_queue, k_queue, K_OBJ_TYPE_QUEUE_ID, शून्य,
			 K_OBJ_RANGE_SECTION(k_queue), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#पूर्ण_अगर /* CONFIG_OBJ_CORE_QUEUE */

व्योम z_impl_k_queue_init(काष्ठा k_queue *queue)
{
	sys_sflist_init(&queue->तथ्य_q);
	queue->lock = (काष्ठा k_spinlock) {};
	z_waitq_init(&queue->wait_q);
#अगर defined(CONFIG_POLL)
	sys_dlist_init(&queue->poll_events);
#पूर्ण_अगर

	SYS_PORT_TRACING_OBJ_INIT(k_queue, queue);

	k_object_init(queue);

#अगर_घोषित CONFIG_OBJ_CORE_QUEUE
	k_obj_core_init_and_link(K_OBJ_CORE(queue), &obj_type_queue);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_QUEUE */
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_queue_init(काष्ठा k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(queue, K_OBJ_QUEUE));
	z_impl_k_queue_init(queue);
}
#समावेश <zephyr/syscalls/k_queue_init_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

अटल अंतरभूत bool queue_handle_poll_events(काष्ठा k_queue *queue, uint32_t state)
{
#अगर_घोषित CONFIG_POLL
	वापस z_handle_obj_poll_events(&queue->poll_events, state);
#अन्यथा
	ARG_UNUSED(queue);
	ARG_UNUSED(state);

	वापस false;
#पूर्ण_अगर /* CONFIG_POLL */
}

व्योम z_impl_k_queue_cancel_wait(काष्ठा k_queue *queue)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_queue, cancel_wait, queue);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	bool resched = false;

	अगर (z_sched_wake(&queue->wait_q, 0, शून्य)) {
		resched = true;
	}

	resched = queue_handle_poll_events(queue, K_POLL_STATE_CANCELLED) || resched;

	अगर (resched) {
		z_reschedule(&queue->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&queue->lock, कुंजी);
	}
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_queue_cancel_wait(काष्ठा k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	z_impl_k_queue_cancel_wait(queue);
}
#समावेश <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
अटल int32_t queue_जोड़ो(काष्ठा k_queue *queue, व्योम *पूर्व, व्योम *तथ्य,
			    bool alloc, bool is_append, k_spinlock_कुंजी_t कुंजी)
{
	int32_t परिणाम = 0;
	bool resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, queue_जोड़ो, queue, alloc);

	अगर (is_append) {
		पूर्व = sys_sflist_peek_पुच्छ(&queue->तथ्य_q);
	}

	अगर (z_sched_wake(&queue->wait_q, 0, तथ्य)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, queue_जोड़ो, queue, alloc, K_FOREVER);
		resched = true;
		जाओ out;
	}

	/* Only need to actually allocate if no threads are pending */
	अगर (alloc) {
		काष्ठा alloc_गाँठ *anode;

		anode = z_thread_malloc(माप(*anode));
		अगर (anode == शून्य) {
			परिणाम = -ENOMEM;
			जाओ out;
		}
		anode->तथ्य = तथ्य;
		sys_sfnode_init(&anode->गाँठ, 0x1);
		तथ्य = anode;
	} अन्यथा {
		sys_sfnode_init(तथ्य, 0x0);
	}

	sys_sflist_जोड़ो(&queue->तथ्य_q, पूर्व, तथ्य);
	resched = queue_handle_poll_events(queue, K_POLL_STATE_तथ्य_AVAILABLE);

out:
	अगर (resched) {
		z_reschedule(&queue->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&queue->lock, कुंजी);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, queue_जोड़ो, queue, alloc, परिणाम);

	वापस परिणाम;
}

व्योम k_queue_जोड़ो(काष्ठा k_queue *queue, व्योम *पूर्व, व्योम *तथ्य)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, जोड़ो, queue);

	(व्योम)queue_जोड़ो(queue, पूर्व, तथ्य, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, जोड़ो, queue);
}

व्योम k_queue_append(काष्ठा k_queue *queue, व्योम *तथ्य)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append, queue);

	(व्योम)queue_जोड़ो(queue, शून्य, तथ्य, false, true,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append, queue);
}

व्योम k_queue_prepend(काष्ठा k_queue *queue, व्योम *तथ्य)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, prepend, queue);

	(व्योम)queue_जोड़ो(queue, शून्य, तथ्य, false, false,
			   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, prepend, queue);
}

int32_t z_impl_k_queue_alloc_append(काष्ठा k_queue *queue, व्योम *तथ्य)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_append, queue);

	int32_t ret = queue_जोड़ो(queue, शून्य, तथ्य, true, true,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_append, queue, ret);

	वापस ret;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत int32_t z_vrfy_k_queue_alloc_append(काष्ठा k_queue *queue,
						  व्योम *तथ्य)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	वापस z_impl_k_queue_alloc_append(queue, तथ्य);
}
#समावेश <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

int32_t z_impl_k_queue_alloc_prepend(काष्ठा k_queue *queue, व्योम *तथ्य)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, alloc_prepend, queue);

	int32_t ret = queue_जोड़ो(queue, शून्य, तथ्य, true, false,
				   k_spin_lock(&queue->lock));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, alloc_prepend, queue, ret);

	वापस ret;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत int32_t z_vrfy_k_queue_alloc_prepend(काष्ठा k_queue *queue,
						   व्योम *तथ्य)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	वापस z_impl_k_queue_alloc_prepend(queue, तथ्य);
}
#समावेश <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक k_queue_append_list(काष्ठा k_queue *queue, व्योम *शीर्ष, व्योम *पुच्छ)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, append_list, queue);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((शीर्ष == शून्य) || (पुच्छ == शून्य)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, -EINVAL);

		वापस -EINVAL;
	}

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);

	जबतक ((शीर्ष != शून्य) && z_sched_wake(&queue->wait_q, 0, शीर्ष)) {
		resched = true;
		शीर्ष = *(व्योम **)शीर्ष;
	}

	अगर (शीर्ष != शून्य) {
		sys_sflist_append_list(&queue->तथ्य_q, शीर्ष, पुच्छ);

		resched = queue_handle_poll_events(queue, K_POLL_STATE_तथ्य_AVAILABLE) || resched;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, append_list, queue, 0);

	अगर (resched) {
		z_reschedule(&queue->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&queue->lock, कुंजी);
	}

	वापस 0;
}

पूर्णांक k_queue_merge_slist(काष्ठा k_queue *queue, sys_slist_t *list)
{
	पूर्णांक ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, merge_slist, queue);

	/* list must not be empty */
	CHECKIF(sys_slist_is_empty(list)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, -EINVAL);

		वापस -EINVAL;
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
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, ret);

		वापस ret;
	}
	sys_slist_init(list);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, merge_slist, queue, 0);

	वापस 0;
}

व्योम *z_impl_k_queue_get(काष्ठा k_queue *queue, k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	व्योम *तथ्य;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, get, queue, timeout);

	अगर (likely(!sys_sflist_is_empty(&queue->तथ्य_q))) {
		sys_sfnode_t *गाँठ;

		गाँठ = sys_sflist_get_not_empty(&queue->तथ्य_q);
		तथ्य = z_queue_गाँठ_peek(गाँठ, true);
		k_spin_unlock(&queue->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, तथ्य);

		वापस तथ्य;
	}

	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&queue->lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout, शून्य);

		वापस शून्य;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_queue, get, queue, timeout);

	पूर्णांक ret = z_pend_curr(&queue->lock, कुंजी, &queue->wait_q, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, get, queue, timeout,
		(ret != 0) ? शून्य : _current->base.swap_तथ्य);

	वापस (ret != 0) ? शून्य : _current->base.swap_तथ्य;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_queue_remove(काष्ठा k_queue *queue, व्योम *तथ्य)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, हटाओ, queue);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	sys_sfnode_t *पूर्व = शून्य;
	sys_sfnode_t *गाँठ = sys_sflist_peek_शीर्ष(&queue->तथ्य_q);
	bool ret = false;

	जबतक (गाँठ != शून्य) {
		व्योम *peeked = z_queue_गाँठ_peek(गाँठ, false);

		अगर (peeked == तथ्य) {
			sys_sflist_remove(&queue->तथ्य_q, पूर्व, गाँठ);
			(व्योम)z_queue_गाँठ_peek(गाँठ, true);
			ret = true;
			अवरोध;
		}
		पूर्व = गाँठ;
		गाँठ = sys_sflist_peek_अग्र(गाँठ);
	}

	k_spin_unlock(&queue->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, हटाओ, queue, ret);

	वापस ret;
}

bool k_queue_unique_append(काष्ठा k_queue *queue, व्योम *तथ्य)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_queue, unique_append, queue);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_गाँठ(&queue->तथ्य_q, test) {
		अगर (test == (sys_sfnode_t *) तथ्य) {
			k_spin_unlock(&queue->lock, कुंजी);
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, false);

			वापस false;
		}
	}

	(व्योम)queue_जोड़ो(queue, शून्य, तथ्य, false, true, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_queue, unique_append, queue, true);

	वापस true;
}

व्योम *z_impl_k_queue_peek_शीर्ष(काष्ठा k_queue *queue)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	व्योम *ret = z_queue_गाँठ_peek(sys_sflist_peek_शीर्ष(&queue->तथ्य_q), false);

	k_spin_unlock(&queue->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_शीर्ष, queue, ret);

	वापस ret;
}

व्योम *z_impl_k_queue_peek_पुच्छ(काष्ठा k_queue *queue)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&queue->lock);
	व्योम *ret = z_queue_गाँठ_peek(sys_sflist_peek_पुच्छ(&queue->तथ्य_q), false);

	k_spin_unlock(&queue->lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC(k_queue, peek_पुच्छ, queue, ret);

	वापस ret;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम *z_vrfy_k_queue_get(काष्ठा k_queue *queue,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	वापस z_impl_k_queue_get(queue, timeout);
}
#समावेश <zephyr/syscalls/k_queue_get_mrsh.c>

अटल अंतरभूत पूर्णांक z_vrfy_k_queue_is_empty(काष्ठा k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	वापस z_impl_k_queue_is_empty(queue);
}
#समावेश <zephyr/syscalls/k_queue_is_empty_mrsh.c>

अटल अंतरभूत व्योम *z_vrfy_k_queue_peek_शीर्ष(काष्ठा k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	वापस z_impl_k_queue_peek_शीर्ष(queue);
}
#समावेश <zephyr/syscalls/k_queue_peek_head_mrsh.c>

अटल अंतरभूत व्योम *z_vrfy_k_queue_peek_पुच्छ(काष्ठा k_queue *queue)
{
	K_OOPS(K_SYSCALL_OBJ(queue, K_OBJ_QUEUE));
	वापस z_impl_k_queue_peek_पुच्छ(queue);
}
#समावेश <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#पूर्ण_अगर /* CONFIG_USERSPACE */

#अगर_घोषित CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_fifo);
STRUCT_SECTION_END_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, शून्य, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#पूर्ण_अगर /* CONFIG_OBJ_CORE_FIFO */

#अगर_घोषित CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_START_EXTERN(k_lifo);
STRUCT_SECTION_END_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, शून्य, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#पूर्ण_अगर /* CONFIG_OBJ_CORE_LIFO */
