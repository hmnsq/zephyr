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

BUILD_ASSERT(SYS_SFLIST_FLAG_بٹ >= 1, "k_queue needs one sflist flag bit");

ساخت alloc_گرہ {
	sys_sfnode_t گرہ;
	خالی *ڈیٹا;
};

/* The queue must have its spinlock held before calling this function. */
ساکن خالی *z_قطار_گرہ_peek(sys_sfnode_t *گرہ, bool needs_free)
{
	خالی *ret;

	اگر ((گرہ != تہی) && (sys_sfnode_flags_get(گرہ) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		ساخت alloc_گرہ *anode;

		anode = CONTAINER_کا(گرہ, ساخت alloc_گرہ, گرہ);
		ret = anode->ڈیٹا;
		اگر (needs_free) {
			k_free(anode);
		}
	} ورنہ {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (خالی *)گرہ;
	}

	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_قطار
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(ساخت k_fifo, قطار_2) == 0);
BUILD_ASSERT(offsetof(ساخت k_lifo, قطار_2) == 0);
STRUCT_SECTION_آغاز_EXTERN(k_fifo);
STRUCT_SECTION_انتہا_EXTERN(k_fifo);
STRUCT_SECTION_آغاز_EXTERN(k_lifo);
STRUCT_SECTION_انتہا_EXTERN(k_lifo);
STRUCT_SECTION_آغاز_EXTERN(k_قطار);
STRUCT_SECTION_انتہا_EXTERN(k_قطار);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_قطار, k_قطار, K_OBJ_TYPE_قطار_ID, تہی,
			 K_OBJ_RANGE_SECTION(k_قطار), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#اگر_ختم /* CONFIG_OBJ_CORE_QUEUE */

خالی z_impl_k_قطار_init(ساخت k_قطار *قطار)
{
	sys_sflist_init(&قطار->ڈیٹا_q);
	قطار->تالا = (ساخت k_spinlock) {};
	z_waitq_init(&قطار->wait_q);
#اگر defined(CONFIG_POLL)
	sys_dlist_init(&قطار->poll_events);
#اگر_ختم

	SYS_پورٹ_TRACING_OBJ_INIT(k_قطار, قطار);

	k_object_init(قطار);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_قطار
	k_obj_core_init_and_link(K_OBJ_CORE(قطار), &obj_type_قطار);
#اگر_ختم /* CONFIG_OBJ_CORE_QUEUE */
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_قطار_init(ساخت k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(قطار, K_OBJ_قطار));
	z_impl_k_قطار_init(قطار);
}
#شامل <zephyr/syscalls/k_queue_init_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

ساکن اندرونی bool قطار_handle_poll_events(ساخت k_قطار *قطار, uint32_t state)
{
#اگر_تعریف_شدہ CONFIG_POLL
	واپس z_handle_obj_poll_events(&قطار->poll_events, state);
#ورنہ
	ARG_غیر_مستعمل(قطار);
	ARG_غیر_مستعمل(state);

	واپس false;
#اگر_ختم /* CONFIG_POLL */
}

خالی z_impl_k_قطار_cancel_wait(ساخت k_قطار *قطار)
{
	SYS_پورٹ_TRACING_OBJ_FUNC(k_قطار, cancel_wait, قطار);

	k_spinlock_کلید_t کلید = k_spin_تالا(&قطار->تالا);
	bool resched = false;

	اگر (z_sched_wake(&قطار->wait_q, 0, تہی)) {
		resched = true;
	}

	resched = قطار_handle_poll_events(قطار, K_POLL_STATE_CANCELLED) || resched;

	اگر (resched) {
		z_reschedule(&قطار->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&قطار->تالا, کلید);
	}
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_قطار_cancel_wait(ساخت k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	z_impl_k_قطار_cancel_wait(قطار);
}
#شامل <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ساکن int32_t قطار_داخل_کرو(ساخت k_قطار *قطار, خالی *پچھلا, خالی *ڈیٹا,
			    bool alloc, bool is_append, k_spinlock_کلید_t کلید)
{
	int32_t نتیجہ = 0;
	bool resched = false;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, قطار_داخل_کرو, قطار, alloc);

	اگر (is_append) {
		پچھلا = sys_sflist_peek_دم(&قطار->ڈیٹا_q);
	}

	اگر (z_sched_wake(&قطار->wait_q, 0, ڈیٹا)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_قطار, قطار_داخل_کرو, قطار, alloc, K_FOREVER);
		resched = true;
		جاؤ برآمد;
	}

	/* Only need to actually allocate if no threads are pending */
	اگر (alloc) {
		ساخت alloc_گرہ *anode;

		anode = z_thread_malloc(حجم(*anode));
		اگر (anode == تہی) {
			نتیجہ = -ENOMEM;
			جاؤ برآمد;
		}
		anode->ڈیٹا = ڈیٹا;
		sys_sfnode_init(&anode->گرہ, 0x1);
		ڈیٹا = anode;
	} ورنہ {
		sys_sfnode_init(ڈیٹا, 0x0);
	}

	sys_sflist_داخل_کرو(&قطار->ڈیٹا_q, پچھلا, ڈیٹا);
	resched = قطار_handle_poll_events(قطار, K_POLL_STATE_ڈیٹا_دستیاب);

برآمد:
	اگر (resched) {
		z_reschedule(&قطار->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&قطار->تالا, کلید);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, قطار_داخل_کرو, قطار, alloc, نتیجہ);

	واپس نتیجہ;
}

خالی k_قطار_داخل_کرو(ساخت k_قطار *قطار, خالی *پچھلا, خالی *ڈیٹا)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, داخل_کرو, قطار);

	(خالی)قطار_داخل_کرو(قطار, پچھلا, ڈیٹا, false, false,
			   k_spin_تالا(&قطار->تالا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, داخل_کرو, قطار);
}

خالی k_قطار_append(ساخت k_قطار *قطار, خالی *ڈیٹا)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, append, قطار);

	(خالی)قطار_داخل_کرو(قطار, تہی, ڈیٹا, false, true,
			   k_spin_تالا(&قطار->تالا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, append, قطار);
}

خالی k_قطار_prepend(ساخت k_قطار *قطار, خالی *ڈیٹا)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, prepend, قطار);

	(خالی)قطار_داخل_کرو(قطار, تہی, ڈیٹا, false, false,
			   k_spin_تالا(&قطار->تالا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, prepend, قطار);
}

int32_t z_impl_k_قطار_alloc_append(ساخت k_قطار *قطار, خالی *ڈیٹا)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, alloc_append, قطار);

	int32_t ret = قطار_داخل_کرو(قطار, تہی, ڈیٹا, true, true,
				   k_spin_تالا(&قطار->تالا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, alloc_append, قطار, ret);

	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی int32_t z_vrfy_k_قطار_alloc_append(ساخت k_قطار *قطار,
						  خالی *ڈیٹا)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	واپس z_impl_k_قطار_alloc_append(قطار, ڈیٹا);
}
#شامل <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

int32_t z_impl_k_قطار_alloc_prepend(ساخت k_قطار *قطار, خالی *ڈیٹا)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, alloc_prepend, قطار);

	int32_t ret = قطار_داخل_کرو(قطار, تہی, ڈیٹا, true, false,
				   k_spin_تالا(&قطار->تالا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, alloc_prepend, قطار, ret);

	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی int32_t z_vrfy_k_قطار_alloc_prepend(ساخت k_قطار *قطار,
						   خالی *ڈیٹا)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	واپس z_impl_k_قطار_alloc_prepend(قطار, ڈیٹا);
}
#شامل <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح k_قطار_append_list(ساخت k_قطار *قطار, خالی *سر, خالی *دم)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, append_list, قطار);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((سر == تہی) || (دم == تہی)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, append_list, قطار, -EINVAL);

		واپس -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_تالا(&قطار->تالا);

	جب ((سر != تہی) && z_sched_wake(&قطار->wait_q, 0, سر)) {
		resched = true;
		سر = *(خالی **)سر;
	}

	اگر (سر != تہی) {
		sys_sflist_append_list(&قطار->ڈیٹا_q, سر, دم);

		resched = قطار_handle_poll_events(قطار, K_POLL_STATE_ڈیٹا_دستیاب) || resched;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, append_list, قطار, 0);

	اگر (resched) {
		z_reschedule(&قطار->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&قطار->تالا, کلید);
	}

	واپس 0;
}

صحیح k_قطار_ملاؤ_slist(ساخت k_قطار *قطار, sys_slist_t *list)
{
	صحیح ret;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, ملاؤ_slist, قطار);

	/* list must not be empty */
	CHECKIF(sys_slist_is_خالی_خانے(list)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملاؤ_slist, قطار, -EINVAL);

		واپس -EINVAL;
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
	ret = k_قطار_append_list(قطار, list->سر, list->دم);
	CHECKIF(ret != 0) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملاؤ_slist, قطار, ret);

		واپس ret;
	}
	sys_slist_init(list);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملاؤ_slist, قطار, 0);

	واپس 0;
}

خالی *z_impl_k_قطار_get(ساخت k_قطار *قطار, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&قطار->تالا);
	خالی *ڈیٹا;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, get, قطار, timeout);

	اگر (likely(!sys_sflist_is_خالی_خانے(&قطار->ڈیٹا_q))) {
		sys_sfnode_t *گرہ;

		گرہ = sys_sflist_get_not_خالی_خانے(&قطار->ڈیٹا_q);
		ڈیٹا = z_قطار_گرہ_peek(گرہ, true);
		k_spin_unlock(&قطار->تالا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout, ڈیٹا);

		واپس ڈیٹا;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&قطار->تالا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout, تہی);

		واپس تہی;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_قطار, get, قطار, timeout);

	صحیح ret = z_pend_curr(&قطار->تالا, کلید, &قطار->wait_q, timeout);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout,
		(ret != 0) ? تہی : _current->بنیاد.تبادلہ_ڈیٹا);

	واپس (ret != 0) ? تہی : _current->بنیاد.تبادلہ_ڈیٹا;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_قطار_remove(ساخت k_قطار *قطار, خالی *ڈیٹا)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, remove, قطار);
	k_spinlock_کلید_t کلید = k_spin_تالا(&قطار->تالا);
	sys_sfnode_t *پچھلا = تہی;
	sys_sfnode_t *گرہ = sys_sflist_peek_سر(&قطار->ڈیٹا_q);
	bool ret = false;

	جب (گرہ != تہی) {
		خالی *peeked = z_قطار_گرہ_peek(گرہ, false);

		اگر (peeked == ڈیٹا) {
			sys_sflist_remove(&قطار->ڈیٹا_q, پچھلا, گرہ);
			(خالی)z_قطار_گرہ_peek(گرہ, true);
			ret = true;
			توڑو;
		}
		پچھلا = گرہ;
		گرہ = sys_sflist_peek_اگلا(گرہ);
	}

	k_spin_unlock(&قطار->تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, remove, قطار, ret);

	واپس ret;
}

bool k_قطار_unique_append(ساخت k_قطار *قطار, خالی *ڈیٹا)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, unique_append, قطار);
	k_spinlock_کلید_t کلید = k_spin_تالا(&قطار->تالا);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_گرہ(&قطار->ڈیٹا_q, test) {
		اگر (test == (sys_sfnode_t *) ڈیٹا) {
			k_spin_unlock(&قطار->تالا, کلید);
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, unique_append, قطار, false);

			واپس false;
		}
	}

	(خالی)قطار_داخل_کرو(قطار, تہی, ڈیٹا, false, true, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, unique_append, قطار, true);

	واپس true;
}

خالی *z_impl_k_قطار_peek_سر(ساخت k_قطار *قطار)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&قطار->تالا);
	خالی *ret = z_قطار_گرہ_peek(sys_sflist_peek_سر(&قطار->ڈیٹا_q), false);

	k_spin_unlock(&قطار->تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_قطار, peek_سر, قطار, ret);

	واپس ret;
}

خالی *z_impl_k_قطار_peek_دم(ساخت k_قطار *قطار)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&قطار->تالا);
	خالی *ret = z_قطار_گرہ_peek(sys_sflist_peek_دم(&قطار->ڈیٹا_q), false);

	k_spin_unlock(&قطار->تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_قطار, peek_دم, قطار, ret);

	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی *z_vrfy_k_قطار_get(ساخت k_قطار *قطار,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	واپس z_impl_k_قطار_get(قطار, timeout);
}
#شامل <zephyr/syscalls/k_queue_get_mrsh.c>

ساکن اندرونی صحیح z_vrfy_k_قطار_is_خالی_خانے(ساخت k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	واپس z_impl_k_قطار_is_خالی_خانے(قطار);
}
#شامل <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ساکن اندرونی خالی *z_vrfy_k_قطار_peek_سر(ساخت k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	واپس z_impl_k_قطار_peek_سر(قطار);
}
#شامل <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ساکن اندرونی خالی *z_vrfy_k_قطار_peek_دم(ساخت k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	واپس z_impl_k_قطار_peek_دم(قطار);
}
#شامل <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#اگر_ختم /* CONFIG_USERSPACE */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_آغاز_EXTERN(k_fifo);
STRUCT_SECTION_انتہا_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, تہی, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#اگر_ختم /* CONFIG_OBJ_CORE_FIFO */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_آغاز_EXTERN(k_lifo);
STRUCT_SECTION_انتہا_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, تہی, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#اگر_ختم /* CONFIG_OBJ_CORE_LIFO */
