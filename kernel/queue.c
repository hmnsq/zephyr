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

BUILD_ASSERT(SYS_SFLIST_FLAG_بٹ >= 1, "k_queue needs one sflist flag bit");

بنية alloc_گرہ {
	sys_sfnode_t گرہ;
	فراغ *انکڑے;
};

/* The queue must have its spinlock held before calling this function. */
ساكن فراغ *z_قطار_گرہ_peek(sys_sfnode_t *گرہ, bool needs_free)
{
	فراغ *ret;

	إذا ((گرہ != عدم) && (sys_sfnode_flags_get(گرہ) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		بنية alloc_گرہ *anode;

		anode = CONTAINER_دا(گرہ, بنية alloc_گرہ, گرہ);
		ret = anode->انکڑے;
		إذا (needs_free) {
			k_free(anode);
		}
	} وإلا {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (فراغ *)گرہ;
	}

	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_OBJ_CORE_قطار
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(بنية k_fifo, قطار_2) == 0);
BUILD_ASSERT(offsetof(بنية k_lifo, قطار_2) == 0);
STRUCT_SECTION_شروع_EXTERN(k_fifo);
STRUCT_SECTION_سرا_EXTERN(k_fifo);
STRUCT_SECTION_شروع_EXTERN(k_lifo);
STRUCT_SECTION_سرا_EXTERN(k_lifo);
STRUCT_SECTION_شروع_EXTERN(k_قطار);
STRUCT_SECTION_سرا_EXTERN(k_قطار);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_قطار, k_قطار, K_OBJ_TYPE_قطار_ID, عدم,
			 K_OBJ_RANGE_SECTION(k_قطار), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#جے_انت /* CONFIG_OBJ_CORE_QUEUE */

فراغ z_impl_k_قطار_init(بنية k_قطار *قطار)
{
	sys_sflist_init(&قطار->انکڑے_q);
	قطار->جندرا = (بنية k_spinlock) {};
	z_waitq_init(&قطار->wait_q);
#إذا defined(CONFIG_POLL)
	sys_dlist_init(&قطار->poll_events);
#جے_انت

	SYS_پورٹ_TRACING_OBJ_INIT(k_قطار, قطار);

	k_object_init(قطار);

#جے_تعریف_شدہ CONFIG_OBJ_CORE_قطار
	k_obj_core_init_and_link(K_OBJ_CORE(قطار), &obj_type_قطار);
#جے_انت /* CONFIG_OBJ_CORE_QUEUE */
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_قطار_init(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(قطار, K_OBJ_قطار));
	z_impl_k_قطار_init(قطار);
}
#ضمن <zephyr/syscalls/k_queue_init_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

ساكن مضمن bool قطار_handle_poll_events(بنية k_قطار *قطار, uint32_t state)
{
#جے_تعریف_شدہ CONFIG_POLL
	أرجع z_handle_obj_poll_events(&قطار->poll_events, state);
#وإلا
	ARG_بن_ورتیا(قطار);
	ARG_بن_ورتیا(state);

	أرجع false;
#جے_انت /* CONFIG_POLL */
}

فراغ z_impl_k_قطار_cancel_wait(بنية k_قطار *قطار)
{
	SYS_پورٹ_TRACING_OBJ_FUNC(k_قطار, cancel_wait, قطار);

	k_spinlock_کلید_t کلید = k_spin_جندرا(&قطار->جندرا);
	bool resched = false;

	إذا (z_sched_wake(&قطار->wait_q, 0, عدم)) {
		resched = true;
	}

	resched = قطار_handle_poll_events(قطار, K_POLL_STATE_CANCELLED) || resched;

	إذا (resched) {
		z_reschedule(&قطار->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&قطار->جندرا, کلید);
	}
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_قطار_cancel_wait(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	z_impl_k_قطار_cancel_wait(قطار);
}
#ضمن <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
ساكن int32_t قطار_داخل_کرو(بنية k_قطار *قطار, فراغ *prev, فراغ *انکڑے,
			    bool alloc, bool is_append, k_spinlock_کلید_t کلید)
{
	int32_t نتیجہ = 0;
	bool resched = false;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, قطار_داخل_کرو, قطار, alloc);

	إذا (is_append) {
		prev = sys_sflist_peek_پوچھ(&قطار->انکڑے_q);
	}

	إذا (z_sched_wake(&قطار->wait_q, 0, انکڑے)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_قطار, قطار_داخل_کرو, قطار, alloc, K_FOREVER);
		resched = true;
		اذهب باہرلا;
	}

	/* Only need to actually allocate if no threads are pending */
	إذا (alloc) {
		بنية alloc_گرہ *anode;

		anode = z_thread_malloc(حجم(*anode));
		إذا (anode == عدم) {
			نتیجہ = -ENOMEM;
			اذهب باہرلا;
		}
		anode->انکڑے = انکڑے;
		sys_sfnode_init(&anode->گرہ, 0x1);
		انکڑے = anode;
	} وإلا {
		sys_sfnode_init(انکڑے, 0x0);
	}

	sys_sflist_داخل_کرو(&قطار->انکڑے_q, prev, انکڑے);
	resched = قطار_handle_poll_events(قطار, K_POLL_STATE_انکڑے_دستیاب);

باہرلا:
	إذا (resched) {
		z_reschedule(&قطار->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&قطار->جندرا, کلید);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, قطار_داخل_کرو, قطار, alloc, نتیجہ);

	أرجع نتیجہ;
}

فراغ k_قطار_داخل_کرو(بنية k_قطار *قطار, فراغ *prev, فراغ *انکڑے)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, داخل_کرو, قطار);

	(فراغ)قطار_داخل_کرو(قطار, prev, انکڑے, false, false,
			   k_spin_جندرا(&قطار->جندرا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, داخل_کرو, قطار);
}

فراغ k_قطار_append(بنية k_قطار *قطار, فراغ *انکڑے)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, append, قطار);

	(فراغ)قطار_داخل_کرو(قطار, عدم, انکڑے, false, true,
			   k_spin_جندرا(&قطار->جندرا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, append, قطار);
}

فراغ k_قطار_prepend(بنية k_قطار *قطار, فراغ *انکڑے)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, prepend, قطار);

	(فراغ)قطار_داخل_کرو(قطار, عدم, انکڑے, false, false,
			   k_spin_جندرا(&قطار->جندرا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, prepend, قطار);
}

int32_t z_impl_k_قطار_alloc_append(بنية k_قطار *قطار, فراغ *انکڑے)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, alloc_append, قطار);

	int32_t ret = قطار_داخل_کرو(قطار, عدم, انکڑے, true, true,
				   k_spin_جندرا(&قطار->جندرا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, alloc_append, قطار, ret);

	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_قطار_alloc_append(بنية k_قطار *قطار,
						  فراغ *انکڑے)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_alloc_append(قطار, انکڑے);
}
#ضمن <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

int32_t z_impl_k_قطار_alloc_prepend(بنية k_قطار *قطار, فراغ *انکڑے)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, alloc_prepend, قطار);

	int32_t ret = قطار_داخل_کرو(قطار, عدم, انکڑے, true, false,
				   k_spin_جندرا(&قطار->جندرا));

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, alloc_prepend, قطار, ret);

	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن int32_t z_vrfy_k_قطار_alloc_prepend(بنية k_قطار *قطار,
						   فراغ *انکڑے)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_alloc_prepend(قطار, انکڑے);
}
#ضمن <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

صحيح k_قطار_append_list(بنية k_قطار *قطار, فراغ *سر, فراغ *پوچھ)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, append_list, قطار);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((سر == عدم) || (پوچھ == عدم)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, append_list, قطار, -EINVAL);

		أرجع -EINVAL;
	}

	k_spinlock_کلید_t کلید = k_spin_جندرا(&قطار->جندرا);

	طالما ((سر != عدم) && z_sched_wake(&قطار->wait_q, 0, سر)) {
		resched = true;
		سر = *(فراغ **)سر;
	}

	إذا (سر != عدم) {
		sys_sflist_append_list(&قطار->انکڑے_q, سر, پوچھ);

		resched = قطار_handle_poll_events(قطار, K_POLL_STATE_انکڑے_دستیاب) || resched;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, append_list, قطار, 0);

	إذا (resched) {
		z_reschedule(&قطار->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&قطار->جندرا, کلید);
	}

	أرجع 0;
}

صحيح k_قطار_ملاؤ_slist(بنية k_قطار *قطار, sys_slist_t *list)
{
	صحيح ret;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, ملاؤ_slist, قطار);

	/* list must not be empty */
	CHECKIF(sys_slist_is_خالی_تھاواں(list)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملاؤ_slist, قطار, -EINVAL);

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
	ret = k_قطار_append_list(قطار, list->سر, list->پوچھ);
	CHECKIF(ret != 0) {
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملاؤ_slist, قطار, ret);

		أرجع ret;
	}
	sys_slist_init(list);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, ملاؤ_slist, قطار, 0);

	أرجع 0;
}

فراغ *z_impl_k_قطار_get(بنية k_قطار *قطار, k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&قطار->جندرا);
	فراغ *انکڑے;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, get, قطار, timeout);

	إذا (likely(!sys_sflist_is_خالی_تھاواں(&قطار->انکڑے_q))) {
		sys_sfnode_t *گرہ;

		گرہ = sys_sflist_get_not_خالی_تھاواں(&قطار->انکڑے_q);
		انکڑے = z_قطار_گرہ_peek(گرہ, true);
		k_spin_unlock(&قطار->جندرا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout, انکڑے);

		أرجع انکڑے;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&قطار->جندرا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout, عدم);

		أرجع عدم;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_قطار, get, قطار, timeout);

	صحيح ret = z_pend_curr(&قطار->جندرا, کلید, &قطار->wait_q, timeout);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, get, قطار, timeout,
		(ret != 0) ? عدم : _current->ادھار.وٹاندرا_انکڑے);

	أرجع (ret != 0) ? عدم : _current->ادھار.وٹاندرا_انکڑے;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_قطار_remove(بنية k_قطار *قطار, فراغ *انکڑے)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, remove, قطار);
	k_spinlock_کلید_t کلید = k_spin_جندرا(&قطار->جندرا);
	sys_sfnode_t *prev = عدم;
	sys_sfnode_t *گرہ = sys_sflist_peek_سر(&قطار->انکڑے_q);
	bool ret = false;

	طالما (گرہ != عدم) {
		فراغ *peeked = z_قطار_گرہ_peek(گرہ, false);

		إذا (peeked == انکڑے) {
			sys_sflist_remove(&قطار->انکڑے_q, prev, گرہ);
			(فراغ)z_قطار_گرہ_peek(گرہ, true);
			ret = true;
			اكسر;
		}
		prev = گرہ;
		گرہ = sys_sflist_peek_اگلا(گرہ);
	}

	k_spin_unlock(&قطار->جندرا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, remove, قطار, ret);

	أرجع ret;
}

bool k_قطار_unique_append(بنية k_قطار *قطار, فراغ *انکڑے)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_قطار, unique_append, قطار);
	k_spinlock_کلید_t کلید = k_spin_جندرا(&قطار->جندرا);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_گرہ(&قطار->انکڑے_q, test) {
		إذا (test == (sys_sfnode_t *) انکڑے) {
			k_spin_unlock(&قطار->جندرا, کلید);
			SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, unique_append, قطار, false);

			أرجع false;
		}
	}

	(فراغ)قطار_داخل_کرو(قطار, عدم, انکڑے, false, true, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_قطار, unique_append, قطار, true);

	أرجع true;
}

فراغ *z_impl_k_قطار_peek_سر(بنية k_قطار *قطار)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&قطار->جندرا);
	فراغ *ret = z_قطار_گرہ_peek(sys_sflist_peek_سر(&قطار->انکڑے_q), false);

	k_spin_unlock(&قطار->جندرا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_قطار, peek_سر, قطار, ret);

	أرجع ret;
}

فراغ *z_impl_k_قطار_peek_پوچھ(بنية k_قطار *قطار)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&قطار->جندرا);
	فراغ *ret = z_قطار_گرہ_peek(sys_sflist_peek_پوچھ(&قطار->انکڑے_q), false);

	k_spin_unlock(&قطار->جندرا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC(k_قطار, peek_پوچھ, قطار, ret);

	أرجع ret;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ *z_vrfy_k_قطار_get(بنية k_قطار *قطار,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_get(قطار, timeout);
}
#ضمن <zephyr/syscalls/k_queue_get_mrsh.c>

ساكن مضمن صحيح z_vrfy_k_قطار_is_خالی_تھاواں(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_is_خالی_تھاواں(قطار);
}
#ضمن <zephyr/syscalls/k_queue_is_empty_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_قطار_peek_سر(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_peek_سر(قطار);
}
#ضمن <zephyr/syscalls/k_queue_peek_head_mrsh.c>

ساكن مضمن فراغ *z_vrfy_k_قطار_peek_پوچھ(بنية k_قطار *قطار)
{
	K_OOPS(K_SYSCALL_OBJ(قطار, K_OBJ_قطار));
	أرجع z_impl_k_قطار_peek_پوچھ(قطار);
}
#ضمن <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#جے_انت /* CONFIG_USERSPACE */

#جے_تعریف_شدہ CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_شروع_EXTERN(k_fifo);
STRUCT_SECTION_سرا_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#جے_انت /* CONFIG_OBJ_CORE_FIFO */

#جے_تعریف_شدہ CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_شروع_EXTERN(k_lifo);
STRUCT_SECTION_سرا_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, عدم, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#جے_انت /* CONFIG_OBJ_CORE_LIFO */
