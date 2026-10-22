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

BUILD_ASSERT(SYS_SFLIST_FLAG_بیتها >= 1, "k_queue needs one sflist flag bit");

بنية alloc_گره {
	sys_sfnode_t گره;
	فراغ *داده;
};

/* The queue must have its spinlock held before calling this function. */
ساكن فراغ *z_صف_گره_peek(sys_sfnode_t *گره, bool needs_free)
{
	فراغ *ret;

	إذا ((گره != عدم) && (sys_sfnode_flags_get(گره) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		بنية alloc_گره *anode;

		anode = CONTAINER_از(گره, بنية alloc_گره, گره);
		ret = anode->داده;
		إذا (needs_free) {
			k_free(anode);
		}
	} وإلا {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (فراغ *)گره;
	}

	أرجع ret;
}

#اگر_تعریف_شده CONFIG_OBJ_CORE_صف
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(بنية k_fifo, صف_2) == 0);
BUILD_ASSERT(offsetof(بنية k_lifo, صف_2) == 0);
STRUCT_SECTION_شروع_EXTERN(k_fifo);
STRUCT_SECTION_انتها_EXTERN(k_fifo);
STRUCT_SECTION_شروع_EXTERN(k_lifo);
STRUCT_SECTION_انتها_EXTERN(k_lifo);
STRUCT_SECTION_شروع_EXTERN(k_صف);
STRUCT_SECTION_انتها_EXTERN(k_صف);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_صف, k_صف, K_OBJ_TYPE_صف_ID, عدم,
			 K_OBJ_RANGE_SECTION(k_صف), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#پایان_اگر /* CONFIG_OBJ_CORE_QUEUE */

فراغ z_impl_k_صف_init(بنية k_صف *صف)
{
	sys_sflist_init(&صف->داده_q);
	صف->قفل = (بنية k_spinlock) {};
	z_waitq_init(&صف->wait_q);
#إذا defined(CONFIG_POLL)
	sys_dlist_init(&صف->poll_events);
#پایان_اگر

	SYS_پورت_TRACING_OBJ_INIT(k_صف, صف);

	k_object_init(صف);

#اگر_تعریف_شده CONFIG_OBJ_CORE_صف
	k_obj_core_init_and_link(K_OBJ_CORE(صف), &obj_type_صف);
#پایان_اگر /* CONFIG_OBJ_CORE_QUEUE */
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_صف_init(بنية k_صف *صف)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(صف, K_OBJ_صف));
	z_impl_k_صف_init(صف);
}
#ضمن <zephyr/syscalls/k_queue_init_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

ساكن مضمن bool صف_handle_poll_events(بنية k_صف *صف, uint32_t state)
{
#اگر_تعریف_شده CONFIG_POLL
	أرجع z_handle_obj_poll_events(&صف->poll_events, state);
#وإلا
	ARG_استفادهنشده(صف);
	ARG_استفادهنشده(state);

	أرجع false;
#پایان_اگر /* CONFIG_POLL */
}

فراغ z_impl_k_صف_cancel_wait(بنية k_صف *صف)
{
	SYS_پورت_TRACING_OBJ_FUNC(k_صف, cancel_wait, صف);

	k_spinlock_کلید_t کلید = k_spin_قفل(&صف->قفل);
	bool resched = false;

	إذا (z_sched_wake(&صف->wait_q, 0, عدم)) {
		resched = true;
	}

	resched = صف_handle_poll_events(صف, K_POLL_STATE_CANCELLED) || resched;

	إذا (resched) {
		z_reschedule(&صف->قفل, کلید);
	} وإلا {
		k_spin_unlock(&صف->قفل, کلید);
	}
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_صف_cancel_wait(بنية k_صف *صف)
{
	K_OOPS(K_SYSCALL_OBJ(صف, K_OBJ_صف));
	z_impl_k_صف_cancel_wait(صف);
}
#ضمن <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ساكن int32_t صف_درج(بنية k_صف *صف, فراغ *prev, فراغ *داده,
			    bool alloc, bool is_append, k_spinlock_کلید_t کلید)
{
	int32_t نتیجه = 0;
	bool resched = false;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, صف_درج, صف, alloc);

	إذا (is_append) {
		prev = sys_sflist_peek_دم(&صف->داده_q);
	}

	إذا (z_sched_wake(&صف->wait_q, 0, داده)) {
		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_صف, صف_درج, صف, alloc, K_FOREVER);
		resched = true;
		اذهب بیرون;
	}

	/* Only need to actually allocate if no threads are pending */
	إذا (alloc) {
		بنية alloc_گره *anode;

		anode = z_thread_malloc(حجم(*anode));
		إذا (anode == عدم) {
			نتیجه = -ENOMEM;
			اذهب بیرون;
		}
		anode->داده = داده;
		sys_sfnode_init(&anode->گره, 0x1);
		داده = anode;
	} وإلا {
		sys_sfnode_init(داده, 0x0);
	}

	sys_sflist_درج(&صف->داده_q, prev, داده);
	resched = صف_handle_poll_events(صف, K_POLL_STATE_داده_موجود);

بیرون:
	إذا (resched) {
		z_reschedule(&صف->قفل, کلید);
	} وإلا {
		k_spin_unlock(&صف->قفل, کلید);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, صف_درج, صف, alloc, نتیجه);

	أرجع نتیجه;
}

فراغ k_صف_درج(بنية k_صف *صف, فراغ *prev, فراغ *داده)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, درج, صف);

	(فراغ)صف_درج(صف, prev, داده, false, false,
			   k_spin_قفل(&صف->قفل));

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, درج, صف);
}

فراغ k_صف_append(بنية k_صف *صف, فراغ *داده)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, append, صف);

	(فراغ)صف_درج(صف, عدم, داده, false, true,
			   k_spin_قفل(&صف->قفل));

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, append, صف);
}

فراغ k_صف_prepend(بنية k_صف *صف, فراغ *داده)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, prepend, صف);

	(فراغ)صف_درج(صف, عدم, داده, false, false,
			   k_spin_قفل(&صف->قفل));

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, prepend, صف);
}

int32_t z_impl_k_صف_alloc_append(بنية k_صف *صف, فراغ *داده)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, alloc_append, صف);

	int32_t ret = صف_درج(صف, عدم, داده, true, true,
				   k_spin_قفل(&صف->قفل));

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, alloc_append, صف, ret);

	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_صف_alloc_append(بنية k_صف *صف,
						  فراغ *داده)
{
	K_OOPS(K_SYSCALL_OBJ(صف, K_OBJ_صف));
	أرجع z_impl_k_صف_alloc_append(صف, داده);
}
#ضمن <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

int32_t z_impl_k_صف_alloc_prepend(بنية k_صف *صف, فراغ *داده)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, alloc_prepend, صف);

	int32_t ret = صف_درج(صف, عدم, داده, true, false,
				   k_spin_قفل(&صف->قفل));

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, alloc_prepend, صف, ret);

	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_صف_alloc_prepend(بنية k_صف *صف,
						   فراغ *داده)
{
	K_OOPS(K_SYSCALL_OBJ(صف, K_OBJ_صف));
	أرجع z_impl_k_صف_alloc_prepend(صف, داده);
}
#ضمن <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

صحيح k_صف_append_list(بنية k_صف *صف, فراغ *سر, فراغ *دم)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, append_list, صف);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((سر == عدم) || (دم == عدم)) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, append_list, صف, -EINVAL);

		أرجع -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_قفل(&صف->قفل);

	طالما ((سر != عدم) && z_sched_wake(&صف->wait_q, 0, سر)) {
		resched = true;
		سر = *(فراغ **)سر;
	}

	إذا (سر != عدم) {
		sys_sflist_append_list(&صف->داده_q, سر, دم);

		resched = صف_handle_poll_events(صف, K_POLL_STATE_داده_موجود) || resched;
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, append_list, صف, 0);

	إذا (resched) {
		z_reschedule(&صف->قفل, کلید);
	} وإلا {
		k_spin_unlock(&صف->قفل, کلید);
	}

	أرجع 0;
}

صحيح k_صف_ادغام_slist(بنية k_صف *صف, sys_slist_t *list)
{
	صحيح ret;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, ادغام_slist, صف);

	/* list must not be empty */
	CHECKIF(sys_slist_is_خالی(list)) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, ادغام_slist, صف, -EINVAL);

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
	ret = k_صف_append_list(صف, list->سر, list->دم);
	CHECKIF(ret != 0) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, ادغام_slist, صف, ret);

		أرجع ret;
	}
	sys_slist_init(list);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, ادغام_slist, صف, 0);

	أرجع 0;
}

فراغ *z_impl_k_صف_get(بنية k_صف *صف, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_قفل(&صف->قفل);
	فراغ *داده;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, get, صف, timeout);

	إذا (likely(!sys_sflist_is_خالی(&صف->داده_q))) {
		sys_sfnode_t *گره;

		گره = sys_sflist_get_not_خالی(&صف->داده_q);
		داده = z_صف_گره_peek(گره, true);
		k_spin_unlock(&صف->قفل, کلید);

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, get, صف, timeout, داده);

		أرجع داده;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&صف->قفل, کلید);

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, get, صف, timeout, عدم);

		أرجع عدم;
	}

	SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_صف, get, صف, timeout);

	صحيح ret = z_pend_curr(&صف->قفل, کلید, &صف->wait_q, timeout);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, get, صف, timeout,
		(ret != 0) ? عدم : _current->پایه.جابجا_داده);

	أرجع (ret != 0) ? عدم : _current->پایه.جابجا_داده;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_صف_remove(بنية k_صف *صف, فراغ *داده)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, remove, صف);
	k_spinlock_کلید_t کلید = k_spin_قفل(&صف->قفل);
	sys_sfnode_t *prev = عدم;
	sys_sfnode_t *گره = sys_sflist_peek_سر(&صف->داده_q);
	bool ret = false;

	طالما (گره != عدم) {
		فراغ *peeked = z_صف_گره_peek(گره, false);

		إذا (peeked == داده) {
			sys_sflist_remove(&صف->داده_q, prev, گره);
			(فراغ)z_صف_گره_peek(گره, true);
			ret = true;
			اكسر;
		}
		prev = گره;
		گره = sys_sflist_peek_بعدی(گره);
	}

	k_spin_unlock(&صف->قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, remove, صف, ret);

	أرجع ret;
}

bool k_صف_unique_append(بنية k_صف *صف, فراغ *داده)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_صف, unique_append, صف);
	k_spinlock_کلید_t کلید = k_spin_قفل(&صف->قفل);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_گره(&صف->داده_q, test) {
		إذا (test == (sys_sfnode_t *) داده) {
			k_spin_unlock(&صف->قفل, کلید);
			SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, unique_append, صف, false);

			أرجع false;
		}
	}

	(فراغ)صف_درج(صف, عدم, داده, false, true, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_صف, unique_append, صف, true);

	أرجع true;
}

فراغ *z_impl_k_صف_peek_سر(بنية k_صف *صف)
{
	k_spinlock_کلید_t کلید = k_spin_قفل(&صف->قفل);
	فراغ *ret = z_صف_گره_peek(sys_sflist_peek_سر(&صف->داده_q), false);

	k_spin_unlock(&صف->قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC(k_صف, peek_سر, صف, ret);

	أرجع ret;
}

فراغ *z_impl_k_صف_peek_دم(بنية k_صف *صف)
{
	k_spinlock_کلید_t کلید = k_spin_قفل(&صف->قفل);
	فراغ *ret = z_صف_گره_peek(sys_sflist_peek_دم(&صف->داده_q), false);

	k_spin_unlock(&صف->قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC(k_صف, peek_دم, صف, ret);

	أرجع ret;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_صف_get(بنية k_صف *صف,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(صف, K_OBJ_صف));
	أرجع z_impl_k_صف_get(صف, timeout);
}
#ضمن <zephyr/syscalls/k_queue_get_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_صف_is_خالی(بنية k_صف *صف)
{
	K_OOPS(K_SYSCALL_OBJ(صف, K_OBJ_صف));
	أرجع z_impl_k_صف_is_خالی(صف);
}
#ضمن <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_صف_peek_سر(بنية k_صف *صف)
{
	K_OOPS(K_SYSCALL_OBJ(صف, K_OBJ_صف));
	أرجع z_impl_k_صف_peek_سر(صف);
}
#ضمن <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_صف_peek_دم(بنية k_صف *صف)
{
	K_OOPS(K_SYSCALL_OBJ(صف, K_OBJ_صف));
	أرجع z_impl_k_صف_peek_دم(صف);
}
#ضمن <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#پایان_اگر /* CONFIG_USERSPACE */

#اگر_تعریف_شده CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_شروع_EXTERN(k_fifo);
STRUCT_SECTION_انتها_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#پایان_اگر /* CONFIG_OBJ_CORE_FIFO */

#اگر_تعریف_شده CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_شروع_EXTERN(k_lifo);
STRUCT_SECTION_انتها_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#پایان_اگر /* CONFIG_OBJ_CORE_LIFO */
