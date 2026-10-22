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

BUILD_ASSERT(SYS_SFLIST_FLAG_بٽ >= 1, "k_queue needs one sflist flag bit");

بنية alloc_ڳنڍ {
	sys_sfnode_t ڳنڍ;
	فراغ *معلومات;
};

/* The queue must have its spinlock held before calling this function. */
ساكن فراغ *z_قطار_ڳنڍ_peek(sys_sfnode_t *ڳنڍ, bool needs_free)
{
	فراغ *ret;

	إذا ((ڳنڍ != عدم) && (sys_sfnode_flags_get(ڳنڍ) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		بنية alloc_ڳنڍ *anode;

		anode = CONTAINER_جو(ڳنڍ, بنية alloc_ڳنڍ, ڳنڍ);
		ret = anode->معلومات;
		إذا (needs_free) {
			k_free(anode);
		}
	} وإلا {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (فراغ *)ڳنڍ;
	}

	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_قطار
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(بنية k_fifo, قطار_2) == 0);
BUILD_ASSERT(offsetof(بنية k_lifo, قطار_2) == 0);
STRUCT_SECTION_شروعات_EXTERN(k_fifo);
STRUCT_SECTION_پڇاڙي_EXTERN(k_fifo);
STRUCT_SECTION_شروعات_EXTERN(k_lifo);
STRUCT_SECTION_پڇاڙي_EXTERN(k_lifo);
STRUCT_SECTION_شروعات_EXTERN(k_قطار);
STRUCT_SECTION_پڇاڙي_EXTERN(k_قطار);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_قطار, k_قطار, K_OBJ_TYPE_قطار_ID, عدم,
			 K_OBJ_RANGE_SECTION(k_قطار), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_QUEUE */

فراغ z_impl_k_قطار_init(بنية k_قطار *قطار)
{
	sys_sflist_init(&قطار->معلومات_q);
	قطار->تالو = (بنية k_spinlock) {};
	z_waitq_init(&قطار->wait_q);
#إذا defined(CONFIG_POLL)
	sys_dlist_init(&قطار->poll_events);
#جيڪڏهن_پورو

	SYS_پورٽ_TRACING_OBJ_INIT(k_قطار, قطار);

	k_object_init(قطار);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_قطار
	k_obj_core_init_and_link(K_OBJ_CORE(قطار), &obj_type_قطار);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_QUEUE */
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_قطار_init(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(قطار, K_OBJ_قطار));
	z_impl_k_قطار_init(قطار);
}
#ضمن <zephyr/syscalls/k_queue_init_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

ساكن مضمن bool قطار_handle_poll_events(بنية k_قطار *قطار, uint32_t state)
{
#جيڪڏهن_وصف_ٿيل CONFIG_POLL
	أرجع z_handle_obj_poll_events(&قطار->poll_events, state);
#وإلا
	ARG_اڻ_استعمال_ٿيل(قطار);
	ARG_اڻ_استعمال_ٿيل(state);

	أرجع false;
#جيڪڏهن_پورو /* CONFIG_POLL */
}

فراغ z_impl_k_قطار_cancel_wait(بنية k_قطار *قطار)
{
	SYS_پورٽ_TRACING_OBJ_FUNC(k_قطار, cancel_wait, قطار);

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&قطار->تالو);
	bool resched = false;

	إذا (z_sched_wake(&قطار->wait_q, 0, عدم)) {
		resched = true;
	}

	resched = قطار_handle_poll_events(قطار, K_POLL_STATE_CANCELLED) || resched;

	إذا (resched) {
		z_reschedule(&قطار->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&قطار->تالو, ڪنجي);
	}
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_قطار_cancel_wait(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	z_impl_k_قطار_cancel_wait(قطار);
}
#ضمن <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ساكن int32_t قطار_وجهو(بنية k_قطار *قطار, فراغ *prev, فراغ *معلومات,
			    bool alloc, bool is_append, k_spinlock_ڪنجي_t ڪنجي)
{
	int32_t نتيجو = 0;
	bool resched = false;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, قطار_وجهو, قطار, alloc);

	إذا (is_append) {
		prev = sys_sflist_peek_پڇ(&قطار->معلومات_q);
	}

	إذا (z_sched_wake(&قطار->wait_q, 0, معلومات)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_قطار, قطار_وجهو, قطار, alloc, K_FOREVER);
		resched = true;
		اذهب ٻاهريون;
	}

	/* Only need to actually allocate if no threads are pending */
	إذا (alloc) {
		بنية alloc_ڳنڍ *anode;

		anode = z_thread_malloc(حجم(*anode));
		إذا (anode == عدم) {
			نتيجو = -ENOMEM;
			اذهب ٻاهريون;
		}
		anode->معلومات = معلومات;
		sys_sfnode_init(&anode->ڳنڍ, 0x1);
		معلومات = anode;
	} وإلا {
		sys_sfnode_init(معلومات, 0x0);
	}

	sys_sflist_وجهو(&قطار->معلومات_q, prev, معلومات);
	resched = قطار_handle_poll_events(قطار, K_POLL_STATE_معلومات_موجود);

ٻاهريون:
	إذا (resched) {
		z_reschedule(&قطار->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&قطار->تالو, ڪنجي);
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, قطار_وجهو, قطار, alloc, نتيجو);

	أرجع نتيجو;
}

فراغ k_قطار_وجهو(بنية k_قطار *قطار, فراغ *prev, فراغ *معلومات)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, وجهو, قطار);

	(فراغ)قطار_وجهو(قطار, prev, معلومات, false, false,
			   k_spin_تالو(&قطار->تالو));

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, وجهو, قطار);
}

فراغ k_قطار_append(بنية k_قطار *قطار, فراغ *معلومات)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, append, قطار);

	(فراغ)قطار_وجهو(قطار, عدم, معلومات, false, true,
			   k_spin_تالو(&قطار->تالو));

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, append, قطار);
}

فراغ k_قطار_prepend(بنية k_قطار *قطار, فراغ *معلومات)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, prepend, قطار);

	(فراغ)قطار_وجهو(قطار, عدم, معلومات, false, false,
			   k_spin_تالو(&قطار->تالو));

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, prepend, قطار);
}

int32_t z_impl_k_قطار_alloc_append(بنية k_قطار *قطار, فراغ *معلومات)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, alloc_append, قطار);

	int32_t ret = قطار_وجهو(قطار, عدم, معلومات, true, true,
				   k_spin_تالو(&قطار->تالو));

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, alloc_append, قطار, ret);

	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_قطار_alloc_append(بنية k_قطار *قطار,
						  فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_alloc_append(قطار, معلومات);
}
#ضمن <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

int32_t z_impl_k_قطار_alloc_prepend(بنية k_قطار *قطار, فراغ *معلومات)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, alloc_prepend, قطار);

	int32_t ret = قطار_وجهو(قطار, عدم, معلومات, true, false,
				   k_spin_تالو(&قطار->تالو));

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, alloc_prepend, قطار, ret);

	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_قطار_alloc_prepend(بنية k_قطار *قطار,
						   فراغ *معلومات)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_alloc_prepend(قطار, معلومات);
}
#ضمن <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح k_قطار_append_list(بنية k_قطار *قطار, فراغ *سر, فراغ *پڇ)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, append_list, قطار);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((سر == عدم) || (پڇ == عدم)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, append_list, قطار, -EINVAL);

		أرجع -EINVAL;
	}

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&قطار->تالو);

	طالما ((سر != عدم) && z_sched_wake(&قطار->wait_q, 0, سر)) {
		resched = true;
		سر = *(فراغ **)سر;
	}

	إذا (سر != عدم) {
		sys_sflist_append_list(&قطار->معلومات_q, سر, پڇ);

		resched = قطار_handle_poll_events(قطار, K_POLL_STATE_معلومات_موجود) || resched;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, append_list, قطار, 0);

	إذا (resched) {
		z_reschedule(&قطار->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&قطار->تالو, ڪنجي);
	}

	أرجع 0;
}

صحيح k_قطار_ملايو_slist(بنية k_قطار *قطار, sys_slist_t *list)
{
	صحيح ret;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, ملايو_slist, قطار);

	/* list must not be empty */
	CHECKIF(sys_slist_is_خالي_جايون(list)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملايو_slist, قطار, -EINVAL);

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
	ret = k_قطار_append_list(قطار, list->سر, list->پڇ);
	CHECKIF(ret != 0) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملايو_slist, قطار, ret);

		أرجع ret;
	}
	sys_slist_init(list);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملايو_slist, قطار, 0);

	أرجع 0;
}

فراغ *z_impl_k_قطار_get(بنية k_قطار *قطار, k_timeout_t timeout)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&قطار->تالو);
	فراغ *معلومات;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, get, قطار, timeout);

	إذا (likely(!sys_sflist_is_خالي_جايون(&قطار->معلومات_q))) {
		sys_sfnode_t *ڳنڍ;

		ڳنڍ = sys_sflist_get_not_خالي_جايون(&قطار->معلومات_q);
		معلومات = z_قطار_ڳنڍ_peek(ڳنڍ, true);
		k_spin_unlock(&قطار->تالو, ڪنجي);

		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout, معلومات);

		أرجع معلومات;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&قطار->تالو, ڪنجي);

		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout, عدم);

		أرجع عدم;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_قطار, get, قطار, timeout);

	صحيح ret = z_pend_curr(&قطار->تالو, ڪنجي, &قطار->wait_q, timeout);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout,
		(ret != 0) ? عدم : _current->بنياد.مٽا_سٽا_معلومات);

	أرجع (ret != 0) ? عدم : _current->بنياد.مٽا_سٽا_معلومات;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_قطار_remove(بنية k_قطار *قطار, فراغ *معلومات)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, remove, قطار);
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&قطار->تالو);
	sys_sfnode_t *prev = عدم;
	sys_sfnode_t *ڳنڍ = sys_sflist_peek_سر(&قطار->معلومات_q);
	bool ret = false;

	طالما (ڳنڍ != عدم) {
		فراغ *peeked = z_قطار_ڳنڍ_peek(ڳنڍ, false);

		إذا (peeked == معلومات) {
			sys_sflist_remove(&قطار->معلومات_q, prev, ڳنڍ);
			(فراغ)z_قطار_ڳنڍ_peek(ڳنڍ, true);
			ret = true;
			اكسر;
		}
		prev = ڳنڍ;
		ڳنڍ = sys_sflist_peek_ايندڙ(ڳنڍ);
	}

	k_spin_unlock(&قطار->تالو, ڪنجي);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, remove, قطار, ret);

	أرجع ret;
}

bool k_قطار_unique_append(بنية k_قطار *قطار, فراغ *معلومات)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_قطار, unique_append, قطار);
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&قطار->تالو);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_ڳنڍ(&قطار->معلومات_q, test) {
		إذا (test == (sys_sfnode_t *) معلومات) {
			k_spin_unlock(&قطار->تالو, ڪنجي);
			SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, unique_append, قطار, false);

			أرجع false;
		}
	}

	(فراغ)قطار_وجهو(قطار, عدم, معلومات, false, true, ڪنجي);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_قطار, unique_append, قطار, true);

	أرجع true;
}

فراغ *z_impl_k_قطار_peek_سر(بنية k_قطار *قطار)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&قطار->تالو);
	فراغ *ret = z_قطار_ڳنڍ_peek(sys_sflist_peek_سر(&قطار->معلومات_q), false);

	k_spin_unlock(&قطار->تالو, ڪنجي);

	SYS_پورٽ_TRACING_OBJ_FUNC(k_قطار, peek_سر, قطار, ret);

	أرجع ret;
}

فراغ *z_impl_k_قطار_peek_پڇ(بنية k_قطار *قطار)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&قطار->تالو);
	فراغ *ret = z_قطار_ڳنڍ_peek(sys_sflist_peek_پڇ(&قطار->معلومات_q), false);

	k_spin_unlock(&قطار->تالو, ڪنجي);

	SYS_پورٽ_TRACING_OBJ_FUNC(k_قطار, peek_پڇ, قطار, ret);

	أرجع ret;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_قطار_get(بنية k_قطار *قطار,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_get(قطار, timeout);
}
#ضمن <zephyr/syscalls/k_queue_get_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_قطار_is_خالي_جايون(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_is_خالي_جايون(قطار);
}
#ضمن <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_قطار_peek_سر(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_peek_سر(قطار);
}
#ضمن <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_قطار_peek_پڇ(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_peek_پڇ(قطار);
}
#ضمن <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#جيڪڏهن_پورو /* CONFIG_USERSPACE */

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_شروعات_EXTERN(k_fifo);
STRUCT_SECTION_پڇاڙي_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_FIFO */

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_شروعات_EXTERN(k_lifo);
STRUCT_SECTION_پڇاڙي_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_LIFO */
