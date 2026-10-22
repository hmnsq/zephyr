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


#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/sys/check.h>

BUILD_ASSERT(SYS_SFLIST_FLAG_బిట్లు >= 1, "k_queue needs one sflist flag bit");

నిర్మాణం alloc_కణుపు {
	sys_sfnode_t కణుపు;
	శూన్య *దత్తాంశం;
};

/* The queue must have its spinlock held before calling this function. */
స్థిర శూన్య *z_వరుస_కణుపు_peek(sys_sfnode_t *కణుపు, bool needs_free)
{
	శూన్య *ret;

	ఒకవేళ ((కణుపు != ఖాళీ) && (sys_sfnode_flags_get(కణుపు) != (uint8_t)0)) {
		/* If the flag is set, then the enqueue operation for this item
		 * did a behind-the scenes memory allocation of an alloc_node
		 * struct, which is what got put in the queue. Free it and pass
		 * back the data pointer.
		 */
		నిర్మాణం alloc_కణుపు *anode;

		anode = CONTAINER_యొక్క(కణుపు, నిర్మాణం alloc_కణుపు, కణుపు);
		ret = anode->దత్తాంశం;
		ఒకవేళ (needs_free) {
			k_free(anode);
		}
	} లేకపోతే {
		/* Data was directly placed in the queue, the first word
		 * reserved for the linked list. User mode isn't allowed to
		 * do this, although it can get data sent this way.
		 */
		ret = (శూన్య *)కణుపు;
	}

	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_OBJ_CORE_వరుస
/* The queues embedded in statically defined FIFOs and LIFOs are permanent
 * objects of the queue type too.
 */
BUILD_ASSERT(offsetof(నిర్మాణం k_fifo, వరుస_2) == 0);
BUILD_ASSERT(offsetof(నిర్మాణం k_lifo, వరుస_2) == 0);
STRUCT_SECTION_ప్రారంభం_EXTERN(k_fifo);
STRUCT_SECTION_చివర_EXTERN(k_fifo);
STRUCT_SECTION_ప్రారంభం_EXTERN(k_lifo);
STRUCT_SECTION_చివర_EXTERN(k_lifo);
STRUCT_SECTION_ప్రారంభం_EXTERN(k_వరుస);
STRUCT_SECTION_చివర_EXTERN(k_వరుస);
K_OBJ_TYPE_DEFINE_RANGES(obj_type_వరుస, k_వరుస, K_OBJ_TYPE_వరుస_ID, ఖాళీ,
			 K_OBJ_RANGE_SECTION(k_వరుస), K_OBJ_RANGE_SECTION(k_fifo),
			 K_OBJ_RANGE_SECTION(k_lifo));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_QUEUE */

శూన్య z_impl_k_వరుస_init(నిర్మాణం k_వరుస *వరుస)
{
	sys_sflist_init(&వరుస->దత్తాంశం_q);
	వరుస->తాళం = (నిర్మాణం k_spinlock) {};
	z_waitq_init(&వరుస->wait_q);
#ఒకవేళ defined(CONFIG_POLL)
	sys_dlist_init(&వరుస->poll_events);
#అయితే_ముగింపు

	SYS_పోర్టు_TRACING_OBJ_INIT(k_వరుస, వరుస);

	k_object_init(వరుస);

#నిర్వచితమైతే CONFIG_OBJ_CORE_వరుస
	k_obj_core_init_and_link(K_OBJ_CORE(వరుస), &obj_type_వరుస);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_QUEUE */
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_వరుస_init(నిర్మాణం k_వరుస *వరుస)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(వరుస, K_OBJ_వరుస));
	z_impl_k_వరుస_init(వరుస);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

స్థిర అంతఃవరుస bool వరుస_handle_poll_events(నిర్మాణం k_వరుస *వరుస, uint32_t state)
{
#నిర్వచితమైతే CONFIG_POLL
	తిరిగి z_handle_obj_poll_events(&వరుస->poll_events, state);
#లేకపోతే
	ARG_వాడని(వరుస);
	ARG_వాడని(state);

	తిరిగి false;
#అయితే_ముగింపు /* CONFIG_POLL */
}

శూన్య z_impl_k_వరుస_cancel_wait(నిర్మాణం k_వరుస *వరుస)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC(k_వరుస, cancel_wait, వరుస);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&వరుస->తాళం);
	bool resched = false;

	ఒకవేళ (z_sched_wake(&వరుస->wait_q, 0, ఖాళీ)) {
		resched = true;
	}

	resched = వరుస_handle_poll_events(వరుస, K_POLL_STATE_CANCELLED) || resched;

	ఒకవేళ (resched) {
		z_reschedule(&వరుస->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&వరుస->తాళం, కీలకం);
	}
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_వరుస_cancel_wait(నిర్మాణం k_వరుస *వరుస)
{
	K_OOPS(K_SYSCALL_OBJ(వరుస, K_OBJ_వరుస));
	z_impl_k_వరుస_cancel_wait(వరుస);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_cancel_wait_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

/*
 * Common code for inserting a node into the queue.
 * The queue's spinlock must be held prior to entry.
 * However, that same spinlock will be unlocked before it returns.
 */
స్థిర int32_t వరుస_చొప్పించు(నిర్మాణం k_వరుస *వరుస, శూన్య *prev, శూన్య *దత్తాంశం,
			    bool alloc, bool is_append, k_spinlock_కీలకం_t కీలకం)
{
	int32_t ఫలితం = 0;
	bool resched = false;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, వరుస_చొప్పించు, వరుస, alloc);

	ఒకవేళ (is_append) {
		prev = sys_sflist_peek_తోక(&వరుస->దత్తాంశం_q);
	}

	ఒకవేళ (z_sched_wake(&వరుస->wait_q, 0, దత్తాంశం)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_వరుస, వరుస_చొప్పించు, వరుస, alloc, K_FOREVER);
		resched = true;
		వెళ్ళు బహిర్గతం;
	}

	/* Only need to actually allocate if no threads are pending */
	ఒకవేళ (alloc) {
		నిర్మాణం alloc_కణుపు *anode;

		anode = z_thread_malloc(పరిమాణం(*anode));
		ఒకవేళ (anode == ఖాళీ) {
			ఫలితం = -ENOMEM;
			వెళ్ళు బహిర్గతం;
		}
		anode->దత్తాంశం = దత్తాంశం;
		sys_sfnode_init(&anode->కణుపు, 0x1);
		దత్తాంశం = anode;
	} లేకపోతే {
		sys_sfnode_init(దత్తాంశం, 0x0);
	}

	sys_sflist_చొప్పించు(&వరుస->దత్తాంశం_q, prev, దత్తాంశం);
	resched = వరుస_handle_poll_events(వరుస, K_POLL_STATE_దత్తాంశం_అందుబాటు);

బహిర్గతం:
	ఒకవేళ (resched) {
		z_reschedule(&వరుస->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&వరుస->తాళం, కీలకం);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, వరుస_చొప్పించు, వరుస, alloc, ఫలితం);

	తిరిగి ఫలితం;
}

శూన్య k_వరుస_చొప్పించు(నిర్మాణం k_వరుస *వరుస, శూన్య *prev, శూన్య *దత్తాంశం)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, చొప్పించు, వరుస);

	(శూన్య)వరుస_చొప్పించు(వరుస, prev, దత్తాంశం, false, false,
			   k_spin_తాళం(&వరుస->తాళం));

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, చొప్పించు, వరుస);
}

శూన్య k_వరుస_append(నిర్మాణం k_వరుస *వరుస, శూన్య *దత్తాంశం)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, append, వరుస);

	(శూన్య)వరుస_చొప్పించు(వరుస, ఖాళీ, దత్తాంశం, false, true,
			   k_spin_తాళం(&వరుస->తాళం));

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, append, వరుస);
}

శూన్య k_వరుస_prepend(నిర్మాణం k_వరుస *వరుస, శూన్య *దత్తాంశం)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, prepend, వరుస);

	(శూన్య)వరుస_చొప్పించు(వరుస, ఖాళీ, దత్తాంశం, false, false,
			   k_spin_తాళం(&వరుస->తాళం));

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, prepend, వరుస);
}

int32_t z_impl_k_వరుస_alloc_append(నిర్మాణం k_వరుస *వరుస, శూన్య *దత్తాంశం)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, alloc_append, వరుస);

	int32_t ret = వరుస_చొప్పించు(వరుస, ఖాళీ, దత్తాంశం, true, true,
				   k_spin_తాళం(&వరుస->తాళం));

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, alloc_append, వరుస, ret);

	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస int32_t z_vrfy_k_వరుస_alloc_append(నిర్మాణం k_వరుస *వరుస,
						  శూన్య *దత్తాంశం)
{
	K_OOPS(K_SYSCALL_OBJ(వరుస, K_OBJ_వరుస));
	తిరిగి z_impl_k_వరుస_alloc_append(వరుస, దత్తాంశం);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_alloc_append_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

int32_t z_impl_k_వరుస_alloc_prepend(నిర్మాణం k_వరుస *వరుస, శూన్య *దత్తాంశం)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, alloc_prepend, వరుస);

	int32_t ret = వరుస_చొప్పించు(వరుస, ఖాళీ, దత్తాంశం, true, false,
				   k_spin_తాళం(&వరుస->తాళం));

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, alloc_prepend, వరుస, ret);

	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస int32_t z_vrfy_k_వరుస_alloc_prepend(నిర్మాణం k_వరుస *వరుస,
						   శూన్య *దత్తాంశం)
{
	K_OOPS(K_SYSCALL_OBJ(వరుస, K_OBJ_వరుస));
	తిరిగి z_impl_k_వరుస_alloc_prepend(వరుస, దత్తాంశం);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_alloc_prepend_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం k_వరుస_append_list(నిర్మాణం k_వరుస *వరుస, శూన్య *తల, శూన్య *తోక)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, append_list, వరుస);
	bool resched = false;

	/* invalid head or tail of list */
	CHECKIF((తల == ఖాళీ) || (తోక == ఖాళీ)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, append_list, వరుస, -EINVAL);

		తిరిగి -EINVAL;
	}

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&వరుస->తాళం);

	అయితే ((తల != ఖాళీ) && z_sched_wake(&వరుస->wait_q, 0, తల)) {
		resched = true;
		తల = *(శూన్య **)తల;
	}

	ఒకవేళ (తల != ఖాళీ) {
		sys_sflist_append_list(&వరుస->దత్తాంశం_q, తల, తోక);

		resched = వరుస_handle_poll_events(వరుస, K_POLL_STATE_దత్తాంశం_అందుబాటు) || resched;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, append_list, వరుస, 0);

	ఒకవేళ (resched) {
		z_reschedule(&వరుస->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&వరుస->తాళం, కీలకం);
	}

	తిరిగి 0;
}

పూర్ణాంకం k_వరుస_విలీనం_slist(నిర్మాణం k_వరుస *వరుస, sys_slist_t *list)
{
	పూర్ణాంకం ret;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, విలీనం_slist, వరుస);

	/* list must not be empty */
	CHECKIF(sys_slist_is_ఖాళీలు(list)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, విలీనం_slist, వరుస, -EINVAL);

		తిరిగి -EINVAL;
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
	ret = k_వరుస_append_list(వరుస, list->తల, list->తోక);
	CHECKIF(ret != 0) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, విలీనం_slist, వరుస, ret);

		తిరిగి ret;
	}
	sys_slist_init(list);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, విలీనం_slist, వరుస, 0);

	తిరిగి 0;
}

శూన్య *z_impl_k_వరుస_get(నిర్మాణం k_వరుస *వరుస, k_timeout_t timeout)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&వరుస->తాళం);
	శూన్య *దత్తాంశం;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, get, వరుస, timeout);

	ఒకవేళ (likely(!sys_sflist_is_ఖాళీలు(&వరుస->దత్తాంశం_q))) {
		sys_sfnode_t *కణుపు;

		కణుపు = sys_sflist_get_not_ఖాళీలు(&వరుస->దత్తాంశం_q);
		దత్తాంశం = z_వరుస_కణుపు_peek(కణుపు, true);
		k_spin_unlock(&వరుస->తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, get, వరుస, timeout, దత్తాంశం);

		తిరిగి దత్తాంశం;
	}

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(&వరుస->తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, get, వరుస, timeout, ఖాళీ);

		తిరిగి ఖాళీ;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_వరుస, get, వరుస, timeout);

	పూర్ణాంకం ret = z_pend_curr(&వరుస->తాళం, కీలకం, &వరుస->wait_q, timeout);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, get, వరుస, timeout,
		(ret != 0) ? ఖాళీ : _current->పునాది.మార్పిడి_దత్తాంశం);

	తిరిగి (ret != 0) ? ఖాళీ : _current->పునాది.మార్పిడి_దత్తాంశం;
}

/* Remove a queue item by its data pointer and free any wrapper node. */
bool k_వరుస_remove(నిర్మాణం k_వరుస *వరుస, శూన్య *దత్తాంశం)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, remove, వరుస);
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&వరుస->తాళం);
	sys_sfnode_t *prev = ఖాళీ;
	sys_sfnode_t *కణుపు = sys_sflist_peek_తల(&వరుస->దత్తాంశం_q);
	bool ret = false;

	అయితే (కణుపు != ఖాళీ) {
		శూన్య *peeked = z_వరుస_కణుపు_peek(కణుపు, false);

		ఒకవేళ (peeked == దత్తాంశం) {
			sys_sflist_remove(&వరుస->దత్తాంశం_q, prev, కణుపు);
			(శూన్య)z_వరుస_కణుపు_peek(కణుపు, true);
			ret = true;
			ఆపు;
		}
		prev = కణుపు;
		కణుపు = sys_sflist_peek_తదుపరి(కణుపు);
	}

	k_spin_unlock(&వరుస->తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, remove, వరుస, ret);

	తిరిగి ret;
}

bool k_వరుస_unique_append(నిర్మాణం k_వరుస *వరుస, శూన్య *దత్తాంశం)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_వరుస, unique_append, వరుస);
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&వరుస->తాళం);

	sys_sfnode_t *test;

	SYS_SFLIST_FOR_EACH_కణుపు(&వరుస->దత్తాంశం_q, test) {
		ఒకవేళ (test == (sys_sfnode_t *) దత్తాంశం) {
			k_spin_unlock(&వరుస->తాళం, కీలకం);
			SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, unique_append, వరుస, false);

			తిరిగి false;
		}
	}

	(శూన్య)వరుస_చొప్పించు(వరుస, ఖాళీ, దత్తాంశం, false, true, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_వరుస, unique_append, వరుస, true);

	తిరిగి true;
}

శూన్య *z_impl_k_వరుస_peek_తల(నిర్మాణం k_వరుస *వరుస)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&వరుస->తాళం);
	శూన్య *ret = z_వరుస_కణుపు_peek(sys_sflist_peek_తల(&వరుస->దత్తాంశం_q), false);

	k_spin_unlock(&వరుస->తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_వరుస, peek_తల, వరుస, ret);

	తిరిగి ret;
}

శూన్య *z_impl_k_వరుస_peek_తోక(నిర్మాణం k_వరుస *వరుస)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&వరుస->తాళం);
	శూన్య *ret = z_వరుస_కణుపు_peek(sys_sflist_peek_తోక(&వరుస->దత్తాంశం_q), false);

	k_spin_unlock(&వరుస->తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_వరుస, peek_తోక, వరుస, ret);

	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య *z_vrfy_k_వరుస_get(నిర్మాణం k_వరుస *వరుస,
				       k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(వరుస, K_OBJ_వరుస));
	తిరిగి z_impl_k_వరుస_get(వరుస, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_get_mrsh.c>

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_వరుస_is_ఖాళీలు(నిర్మాణం k_వరుస *వరుస)
{
	K_OOPS(K_SYSCALL_OBJ(వరుస, K_OBJ_వరుస));
	తిరిగి z_impl_k_వరుస_is_ఖాళీలు(వరుస);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_is_empty_mrsh.c>

స్థిర అంతఃవరుస శూన్య *z_vrfy_k_వరుస_peek_తల(నిర్మాణం k_వరుస *వరుస)
{
	K_OOPS(K_SYSCALL_OBJ(వరుస, K_OBJ_వరుస));
	తిరిగి z_impl_k_వరుస_peek_తల(వరుస);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_peek_head_mrsh.c>

స్థిర అంతఃవరుస శూన్య *z_vrfy_k_వరుస_peek_తోక(నిర్మాణం k_వరుస *వరుస)
{
	K_OOPS(K_SYSCALL_OBJ(వరుస, K_OBJ_వరుస));
	తిరిగి z_impl_k_వరుస_peek_తోక(వరుస);
}
#అన్తర్భావ <zephyr/syscalls/k_queue_peek_tail_mrsh.c>

#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_OBJ_CORE_FIFO
/* Referenced by k_fifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_ప్రారంభం_EXTERN(k_fifo);
STRUCT_SECTION_చివర_EXTERN(k_fifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_fifo) =
	K_OBJ_TYPE_INITIALIZER(k_fifo, K_OBJ_TYPE_FIFO_ID, ఖాళీ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_fifo));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_FIFO */

#నిర్వచితమైతే CONFIG_OBJ_CORE_LIFO
/* Referenced by k_lifo_init() in kernel.h, so not file-local */
STRUCT_SECTION_ప్రారంభం_EXTERN(k_lifo);
STRUCT_SECTION_చివర_EXTERN(k_lifo);
STRUCT_SECTION_ITERABLE(k_obj_type, _obj_type_lifo) =
	K_OBJ_TYPE_INITIALIZER(k_lifo, K_OBJ_TYPE_LIFO_ID, ఖాళీ, 0, 0,
			       K_OBJ_RANGE_SECTION(k_lifo));
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_LIFO */
