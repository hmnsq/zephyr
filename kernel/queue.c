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

BUILD_ASSERT(SYS_SFLIST_FLAG_بټونه >= 1, "k_queue needs one sflist flag bit");

بنية alloc_غوټه {
	sys_sfnode_t غوټه;
	فراغ *معلومات;
};

/* The queue must have its spinlock held before calling this function. */
ساكن فراغ *z_کتار_غوټه_peek(sys_sfnode_t *غوټه, bool needs_free)
{
	فراغ *ret;

	إذا ((غوټه != عدم) && (sys_sfnode_flags_get(غوټه) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		بنية alloc_غوټه *anode;

		anode = CONTAINER_د(غوټه, بنية alloc_غوټه, غوټه);
		ret = anode->معلومات;
		إذا (needs_free) {
			k_free(anode);
		}
	} وإلا {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (فراغ *)غوټه;
	}

	أرجع ret;
}

#که_تعریف_شوی CONFIG_OBJ_CORE_کتار
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(بنية k_fifo, کتار_2) == 0);
BUILD_ASSERT(offsetof(بنية k_lifo, کتار_2) == 0);
STRUCT_SECTION_پیل_EXTERN(k_fifo);
STRUCT_SECTION_پای_ټکی_EXTERN(k_fifo);
STRUCT_SECTION_پیل_EXTERN(k_lifo);
STRUCT_SECTION_پای_ټکی_EXTERN(k_lifo);
STRUCT_SECTION_پیل_EXTERN(k_کتار);
STRUCT_SECTION_پای_ټکی_EXTERN(k_کتار);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_کتار, k_کتار, K_OBJ_TYPE_کتار_ID, عدم,
			 K_OBJ_RANGE_SECTION(k_کتار), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#د_که_پای /* CONFIG_OBJ_CORE_QUEUE */

فراغ z_impl_k_کتار_init(بنية k_کتار *کتار)
{
	sys_sflist_init(&کتار->معلومات_q);
	کتار->قلف = (بنية k_spinlock) {};
	z_waitq_init(&کتار->wait_q);
#إذا defined(CONFIG_POLL)
	sys_dlist_init(&کتار->poll_events);
#د_که_پای

	SYS_پورټ_TRACING_OBJ_INIT(k_کتار, کتار);

	k_object_init(کتار);

#که_تعریف_شوی CONFIG_OBJ_CORE_کتار
	k_obj_core_init_and_link(K_OBJ_CORE(کتار), &obj_type_کتار);
#د_که_پای /* CONFIG_OBJ_CORE_QUEUE */
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_کتار_init(بنية k_کتار *کتار)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(کتار, K_OBJ_کتار));
	z_impl_k_کتار_init(کتار);
}
#ضمن <zephyr/syscalls/k_queue_init_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

ساكن مضمن bool کتار_handle_poll_events(بنية k_کتار *کتار, uint32_t state)
{
#که_تعریف_شوی CONFIG_POLL
	أرجع z_handle_obj_poll_events(&کتار->poll_events, state);
#وإلا
	ARG_ناکارول_شوی(کتار);
	ARG_ناکارول_شوی(state);

	أرجع false;
#د_که_پای /* CONFIG_POLL */
}

فراغ z_impl_k_کتار_cancel_wait(بنية k_کتار *کتار)
{
	SYS_پورټ_TRACING_OBJ_FUNC(k_کتار, cancel_wait, کتار);

	k_spinlock_کلی_t کلی = k_spin_قلف(&کتار->قلف);
	bool resched = false;

	إذا (z_sched_wake(&کتار->wait_q, 0, عدم)) {
		resched = true;
	}

	resched = کتار_handle_poll_events(کتار, K_POLL_STATE_CANCELLED) || resched;

	إذا (resched) {
		z_reschedule(&کتار->قلف, کلی);
	} وإلا {
		k_spin_unlock(&کتار->قلف, کلی);
	}
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_کتار_cancel_wait(بنية k_کتار *کتار)
{
	K_OOPS(K_SYSCALL_OBJ(کتار, K_OBJ_کتار));
	z_impl_k_کتار_cancel_wait(کتار);
}
#ضمن <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ساكن int32_t کتار_ورننباسه(بنية k_کتار *کتار, فراغ *prev, فراغ *معلومات,
			    bool alloc, bool is_append, k_spinlock_کلی_t کلی)
{
	int32_t پایله = 0;
	bool resched = false;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, کتار_ورننباسه, کتار, alloc);

	إذا (is_append) {
		prev = sys_sflist_peek_لکۍ(&کتار->معلومات_q);
	}

	إذا (z_sched_wake(&کتار->wait_q, 0, معلومات)) {
		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_کتار, کتار_ورننباسه, کتار, alloc, K_FOREVER);
		resched = true;
		اذهب بهر;
	}

	/* Only need to actually allocate if no threads are pending */
	إذا (alloc) {
		بنية alloc_غوټه *anode;

		anode = z_thread_malloc(حجم(*anode));
		إذا (anode == عدم) {
			پایله = -ENOMEM;
			اذهب بهر;
		}
		anode->معلومات = معلومات;
		sys_sfnode_init(&anode->غوټه, 0x1);
		معلومات = anode;
	} وإلا {
		sys_sfnode_init(معلومات, 0x0);
	}

	sys_sflist_ورننباسه(&کتار->معلومات_q, prev, معلومات);
	resched = کتار_handle_poll_events(کتار, K_POLL_STATE_معلومات_شته);

بهر:
	إذا (resched) {
		z_reschedule(&کتار->قلف, کلی);
	} وإلا {
		k_spin_unlock(&کتار->قلف, کلی);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, کتار_ورننباسه, کتار, alloc, پایله);

	أرجع پایله;
}

فراغ k_کتار_ورننباسه(بنية k_کتار *کتار, فراغ *prev, فراغ *معلومات)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, ورننباسه, کتار);

	(فراغ)کتار_ورننباسه(کتار, prev, معلومات, false, false,
			   k_spin_قلف(&کتار->قلف));

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, ورننباسه, کتار);
}

فراغ k_کتار_append(بنية k_کتار *کتار, فراغ *معلومات)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, append, کتار);

	(فراغ)کتار_ورننباسه(کتار, عدم, معلومات, false, true,
			   k_spin_قلف(&کتار->قلف));

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, append, کتار);
}

فراغ k_کتار_prepend(بنية k_کتار *کتار, فراغ *معلومات)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, prepend, کتار);

	(فراغ)کتار_ورننباسه(کتار, عدم, معلومات, false, false,
			   k_spin_قلف(&کتار->قلف));

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, prepend, کتار);
}

int32_t z_impl_k_کتار_alloc_append(بنية k_کتار *کتار, فراغ *معلومات)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, alloc_append, کتار);

	int32_t ret = کتار_ورننباسه(کتار, عدم, معلومات, true, true,
				   k_spin_قلف(&کتار->قلف));

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, alloc_append, کتار, ret);

	أرجع ret;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_کتار_alloc_append(بنية k_کتار *کتار,
						  فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(کتار, K_OBJ_کتار));
	أرجع z_impl_k_کتار_alloc_append(کتار, معلومات);
}
#ضمن <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

int32_t z_impl_k_کتار_alloc_prepend(بنية k_کتار *کتار, فراغ *معلومات)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, alloc_prepend, کتار);

	int32_t ret = کتار_ورننباسه(کتار, عدم, معلومات, true, false,
				   k_spin_قلف(&کتار->قلف));

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, alloc_prepend, کتار, ret);

	أرجع ret;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_کتار_alloc_prepend(بنية k_کتار *کتار,
						   فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(کتار, K_OBJ_کتار));
	أرجع z_impl_k_کتار_alloc_prepend(کتار, معلومات);
}
#ضمن <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح k_کتار_append_list(بنية k_کتار *کتار, فراغ *سر, فراغ *لکۍ)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, append_list, کتار);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((سر == عدم) || (لکۍ == عدم)) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, append_list, کتار, -EINVAL);

		أرجع -EINVAL;
	}

	k_spinlock_کلی_t کلی = k_spin_قلف(&کتار->قلف);

	طالما ((سر != عدم) && z_sched_wake(&کتار->wait_q, 0, سر)) {
		resched = true;
		سر = *(فراغ **)سر;
	}

	إذا (سر != عدم) {
		sys_sflist_append_list(&کتار->معلومات_q, سر, لکۍ);

		resched = کتار_handle_poll_events(کتار, K_POLL_STATE_معلومات_شته) || resched;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, append_list, کتار, 0);

	إذا (resched) {
		z_reschedule(&کتار->قلف, کلی);
	} وإلا {
		k_spin_unlock(&کتار->قلف, کلی);
	}

	أرجع 0;
}

صحيح k_کتار_یوځای_slist(بنية k_کتار *کتار, sys_slist_t *list)
{
	صحيح ret;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, یوځای_slist, کتار);

	/* list must not be empty */
	CHECKIF(sys_slist_is_تشه(list)) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, یوځای_slist, کتار, -EINVAL);

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
	ret = k_کتار_append_list(کتار, list->سر, list->لکۍ);
	CHECKIF(ret != 0) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, یوځای_slist, کتار, ret);

		أرجع ret;
	}
	sys_slist_init(list);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, یوځای_slist, کتار, 0);

	أرجع 0;
}

فراغ *z_impl_k_کتار_get(بنية k_کتار *کتار, k_timeout_t timeout)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&کتار->قلف);
	فراغ *معلومات;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, get, کتار, timeout);

	إذا (likely(!sys_sflist_is_تشه(&کتار->معلومات_q))) {
		sys_sfnode_t *غوټه;

		غوټه = sys_sflist_get_not_تشه(&کتار->معلومات_q);
		معلومات = z_کتار_غوټه_peek(غوټه, true);
		k_spin_unlock(&کتار->قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, get, کتار, timeout, معلومات);

		أرجع معلومات;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&کتار->قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, get, کتار, timeout, عدم);

		أرجع عدم;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_کتار, get, کتار, timeout);

	صحيح ret = z_pend_curr(&کتار->قلف, کلی, &کتار->wait_q, timeout);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, get, کتار, timeout,
		(ret != 0) ? عدم : _current->بنسټ.بدلون_معلومات);

	أرجع (ret != 0) ? عدم : _current->بنسټ.بدلون_معلومات;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_کتار_remove(بنية k_کتار *کتار, فراغ *معلومات)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, remove, کتار);
	k_spinlock_کلی_t کلی = k_spin_قلف(&کتار->قلف);
	sys_sfnode_t *prev = عدم;
	sys_sfnode_t *غوټه = sys_sflist_peek_سر(&کتار->معلومات_q);
	bool ret = false;

	طالما (غوټه != عدم) {
		فراغ *peeked = z_کتار_غوټه_peek(غوټه, false);

		إذا (peeked == معلومات) {
			sys_sflist_remove(&کتار->معلومات_q, prev, غوټه);
			(فراغ)z_کتار_غوټه_peek(غوټه, true);
			ret = true;
			اكسر;
		}
		prev = غوټه;
		غوټه = sys_sflist_peek_بل(غوټه);
	}

	k_spin_unlock(&کتار->قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, remove, کتار, ret);

	أرجع ret;
}

bool k_کتار_unique_append(بنية k_کتار *کتار, فراغ *معلومات)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_کتار, unique_append, کتار);
	k_spinlock_کلی_t کلی = k_spin_قلف(&کتار->قلف);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_غوټه(&کتار->معلومات_q, test) {
		إذا (test == (sys_sfnode_t *) معلومات) {
			k_spin_unlock(&کتار->قلف, کلی);
			SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, unique_append, کتار, false);

			أرجع false;
		}
	}

	(فراغ)کتار_ورننباسه(کتار, عدم, معلومات, false, true, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_کتار, unique_append, کتار, true);

	أرجع true;
}

فراغ *z_impl_k_کتار_peek_سر(بنية k_کتار *کتار)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&کتار->قلف);
	فراغ *ret = z_کتار_غوټه_peek(sys_sflist_peek_سر(&کتار->معلومات_q), false);

	k_spin_unlock(&کتار->قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC(k_کتار, peek_سر, کتار, ret);

	أرجع ret;
}

فراغ *z_impl_k_کتار_peek_لکۍ(بنية k_کتار *کتار)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&کتار->قلف);
	فراغ *ret = z_کتار_غوټه_peek(sys_sflist_peek_لکۍ(&کتار->معلومات_q), false);

	k_spin_unlock(&کتار->قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC(k_کتار, peek_لکۍ, کتار, ret);

	أرجع ret;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_کتار_get(بنية k_کتار *کتار,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(کتار, K_OBJ_کتار));
	أرجع z_impl_k_کتار_get(کتار, timeout);
}
#ضمن <zephyr/syscalls/k_queue_get_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_کتار_is_تشه(بنية k_کتار *کتار)
{
	K_OOPS(K_SYSCALL_OBJ(کتار, K_OBJ_کتار));
	أرجع z_impl_k_کتار_is_تشه(کتار);
}
#ضمن <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_کتار_peek_سر(بنية k_کتار *کتار)
{
	K_OOPS(K_SYSCALL_OBJ(کتار, K_OBJ_کتار));
	أرجع z_impl_k_کتار_peek_سر(کتار);
}
#ضمن <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_کتار_peek_لکۍ(بنية k_کتار *کتار)
{
	K_OOPS(K_SYSCALL_OBJ(کتار, K_OBJ_کتار));
	أرجع z_impl_k_کتار_peek_لکۍ(کتار);
}
#ضمن <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#د_که_پای /* CONFIG_USERSPACE */

#که_تعریف_شوی CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_پیل_EXTERN(k_fifo);
STRUCT_SECTION_پای_ټکی_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#د_که_پای /* CONFIG_OBJ_CORE_FIFO */

#که_تعریف_شوی CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_پیل_EXTERN(k_lifo);
STRUCT_SECTION_پای_ټکی_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#د_که_پای /* CONFIG_OBJ_CORE_LIFO */
