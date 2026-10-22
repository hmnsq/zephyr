/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#অন্তর্ভাব <zephyr/kernel.h>

#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <zephyr/linker/sections.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>
/* private kernel APIs */
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <kthread.h>
#অন্তর্ভাব <scheduler.h>
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

সংরচনা mbox_walk_data {
	সংরচনা k_thread *thread;
	সংরচনা k_mbox_বার্তা *বার্তা;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, রিক্ত);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#যদি (CONFIG_অঙ্ক_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
সংরচনা k_mbox_async {
	সংরচনা _thread_base thread;		/* dummy thread object */
	সংরচনা k_mbox_বার্তা tx_বার্তা;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_স্তূপ_DEFINE(async_বার্তা_free, CONFIG_অঙ্ক_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
স্থির অন্তঃস্থ শূন্য mbox_async_alloc(সংরচনা k_mbox_async **async)
{
	(শূন্য)k_স্তূপ_বের_করো(&async_বার্তা_free, (স্তূপ_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
স্থির অন্তঃস্থ শূন্য mbox_async_free(সংরচনা k_mbox_async *async)
{
	k_স্তূপ_ঠেলো(&async_বার্তা_free, (স্তূপ_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
স্থির শূন্য init_mbox_module(শূন্য)
{
	/* array of asynchronous message descriptors */
	স্থির সংরচনা k_mbox_async __noinit async_বার্তা[CONFIG_অঙ্ক_MBOX_ASYNC_MSGS];

	/*
	 * Create pool of asynchronous message descriptors.
	 *
	 * A dummy thread requires minimal initialization, since it never gets
	 * to execute. The _THREAD_DUMMY flag is sufficient to distinguish a
	 * dummy thread from a real one. The threads are *not* added to the
	 * kernel's list of known threads.
	 *
	 * Once initialized, the address of each descriptor is added to a stack
	 * that governs access to them.
	 */

	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < CONFIG_অঙ্ক_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_বার্তা[i].thread, 0, _THREAD_DUMMY, 0);
		k_স্তূপ_ঠেলো(&async_বার্তা_free, (স্তূপ_data_t)&async_বার্তা[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

শূন্য k_mbox_init(সংরচনা k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_বার্তা_queue);
	z_waitq_init(&mbox->rx_বার্তা_queue);
	mbox->lock = (সংরচনা k_spinlock) {};

#ifdef CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_PORT_TRACING_OBJ_INIT(k_mbox, mbox);
}

/**
 * @brief Check compatibility of sender's and receiver's message descriptors.
 *
 * Compares sender's and receiver's message descriptors to see if they are
 * compatible. If so, the descriptor fields are updated to reflect that a
 * match has occurred.
 *
 * @param tx_msg Pointer to transmit message descriptor.
 * @param rx_msg Pointer to receive message descriptor.
 *
 * @retval 0 Success.
 * @retval -1 Not matched.
 */
স্থির পূর্ণাঙ্ক mbox_message_match(সংরচনা k_mbox_বার্তা *tx_বার্তা,
			       সংরচনা k_mbox_বার্তা *rx_বার্তা)
{
	uint32_t temp_info;

	যদি (((tx_বার্তা->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_বার্তা->tx_target_thread == rx_বার্তা->tx_target_thread)) &&
	    ((rx_বার্তা->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_বার্তা->rx_source_thread == tx_বার্তা->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_বার্তা->rx_source_thread = tx_বার্তা->rx_source_thread;
		tx_বার্তা->tx_target_thread = rx_বার্তা->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_বার্তা->info;
		rx_বার্তা->info = tx_বার্তা->info;
		tx_বার্তা->info = temp_info;

		/* update data size field for receiver only */
		যদি (rx_বার্তা->size > tx_বার্তা->size) {
			rx_বার্তা->size = tx_বার্তা->size;
		}

		/* update data location fields for receiver only */
		rx_বার্তা->tx_data = tx_বার্তা->tx_data;

		/* update syncing thread field for receiver only */
		rx_বার্তা->_syncing_thread = tx_বার্তা->_syncing_thread;

		প্রত্যাগম 0;
	}

	প্রত্যাগম -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
স্থির শূন্য mbox_message_dispose(সংরচনা k_mbox_বার্তা *rx_বার্তা)
{
	সংরচনা k_thread *sending_thread;
	সংরচনা k_mbox_বার্তা *tx_বার্তা;

	/* do nothing if message was disposed of when it was received */
	যদি (rx_বার্তা->_syncing_thread == রিক্ত) {
		প্রত্যাগম;
	}

	/* recover sender info */
	sending_thread = rx_বার্তা->_syncing_thread;
	rx_বার্তা->_syncing_thread = রিক্ত;
	tx_বার্তা = (সংরচনা k_mbox_বার্তা *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_বার্তা->size = rx_বার্তা->size;

#যদি (CONFIG_অঙ্ক_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	যদি (is_thread_dummy(sending_thread)) {
		সংরচনা k_sem *async_sem = tx_বার্তা->_async_sem;

		mbox_async_free((সংরচনা k_mbox_async *)sending_thread);
		যদি (async_sem != রিক্ত) {
			k_sem_give(async_sem);
		}
		প্রত্যাগম;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_চাবি_t চাবি = k_spin_lock(&_sched_spinlock);

	arch_thread_return_মান_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, চাবি);
}

স্থির পূর্ণাঙ্ক mbox_message_put_walk_op(সংরচনা k_thread *thread, শূন্য *data)
{
	সংরচনা mbox_walk_data *walk_data = (সংরচনা mbox_walk_data *)data;
	সংরচনা k_mbox_বার্তা *rx_বার্তা;

	rx_বার্তা = (সংরচনা k_mbox_বার্তা *)thread->base.swap_data;

	যদি (mbox_message_match(walk_data->বার্তা, rx_বার্তা) != 0) {
		/* No match. Proceed to next in wait queue */
		প্রত্যাগম 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(শূন্য)z_try_abort_thread_timeout(thread);
	arch_thread_return_মান_set(thread, 0);
	z_sched_ready_locked(thread);

#যদি (CONFIG_অঙ্ক_MBOX_ASYNC_MSGS > 0)
	যদি (is_thread_dummy(walk_data->thread)) {
		প্রত্যাগম ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	প্রত্যাগম SYNCHRONOUS_MATCH_FOUND;
}

/**
 * @brief Send a mailbox message.
 *
 * Helper routine that handles both synchronous and asynchronous sends.
 *
 * @param mbox Pointer to the mailbox object.
 * @param tx_msg Pointer to transmit message descriptor.
 * @param timeout Maximum time (milliseconds) to wait for the message to be
 *        received (although not necessarily completely processed).
 *        Use K_NO_WAIT to return immediately, or K_FOREVER to wait as long
 *        as necessary.
 *
 * @retval 0 Success.
 * @retval -ENOMSG Failed immediately.
 * @retval -EAGAIN Timed out.
 */
স্থির পূর্ণাঙ্ক mbox_message_put(সংরচনা k_mbox *mbox, সংরচনা k_mbox_বার্তা *tx_বার্তা,
			     k_timeout_t timeout)
{
	সংরচনা k_thread *sending_thread;
	k_spinlock_চাবি_t চাবি;

	/* save sender id so it can be used during message matching */
	tx_বার্তা->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_বার্তা->_syncing_thread;
	sending_thread->base.swap_data = tx_বার্তা;

	/* search mailbox's rx queue for a compatible receiver */
	চাবি = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	সংরচনা mbox_walk_data walk_data;
	পূর্ণাঙ্ক status;

	walk_data.thread = sending_thread;
	walk_data.বার্তা = tx_বার্তা;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_বার্তা_queue,
				    mbox_message_put_walk_op, রিক্ত,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#যদি (CONFIG_অঙ্ক_MBOX_ASYNC_MSGS > 0)
	যদি (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, চাবি);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		প্রত্যাগম 0;
	}
#endif

	যদি (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		পূর্ণাঙ্ক ret = z_pend_curr(&mbox->lock, চাবি, রিক্ত, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		প্রত্যাগম ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, চাবি);
		প্রত্যাগম -ENOMSG;
	}

#যদি (CONFIG_অঙ্ক_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	যদি (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_বার্তা_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, চাবি);
		প্রত্যাগম 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	পূর্ণাঙ্ক ret = z_pend_curr(&mbox->lock, চাবি, &mbox->tx_বার্তা_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	প্রত্যাগম ret;
}

পূর্ণাঙ্ক k_mbox_put(সংরচনা k_mbox *mbox, সংরচনা k_mbox_বার্তা *tx_বার্তা,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_বার্তা->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	পূর্ণাঙ্ক ret = mbox_message_put(mbox, tx_বার্তা, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	প্রত্যাগম ret;
}

#যদি (CONFIG_অঙ্ক_MBOX_ASYNC_MSGS > 0)
শূন্য k_mbox_async_put(সংরচনা k_mbox *mbox, সংরচনা k_mbox_বার্তা *tx_বার্তা,
		      সংরচনা k_sem *sem)
{
	সংরচনা k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_বার্তা = *tx_বার্তা;
	async->tx_বার্তা._syncing_thread = (সংরচনা k_thread *)&async->thread;
	async->tx_বার্তা._async_sem = sem;

	(শূন্য)mbox_message_put(mbox, &async->tx_বার্তা, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

শূন্য k_mbox_data_get(সংরচনা k_mbox_বার্তা *rx_বার্তা, শূন্য *buffer)
{
	/* handle case where data is to be discarded */
	যদি (buffer == রিক্ত) {
		rx_বার্তা->size = 0;
		mbox_message_dispose(rx_বার্তা);
		প্রত্যাগম;
	}

	/* copy message data to buffer, then dispose of message */
	যদি ((rx_বার্তা->tx_data != রিক্ত) && (rx_বার্তা->size > 0U)) {
		(শূন্য)memcpy(buffer, rx_বার্তা->tx_data, rx_বার্তা->size);
	}
	mbox_message_dispose(rx_বার্তা);
}

/**
 * @brief Handle immediate consumption of received mailbox message data.
 *
 * Checks to see if received message data should be kept for later retrieval,
 * or if the data should consumed immediately and the message disposed of.
 *
 * The data is consumed immediately in either of the following cases:
 *     1) The receiver requested immediate retrieval by supplying a buffer
 *        to receive the data.
 *     2) There is no data to be retrieved. (i.e. Data size is 0 bytes.)
 *
 * @param rx_msg Pointer to receive message descriptor.
 * @param buffer Pointer to buffer to receive data.
 *
 * @retval 0 Always returns 0.
 */
স্থির পূর্ণাঙ্ক mbox_message_data_check(সংরচনা k_mbox_বার্তা *rx_বার্তা, শূন্য *buffer)
{
	যদি (buffer != রিক্ত) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_বার্তা, buffer);
	} অন্যথা যদি (rx_বার্তা->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_বার্তা);
	} অন্যথা {
		/* keep message around for later data retrieval */
	}

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক mbox_get_walk_op(সংরচনা k_thread *thread, শূন্য *data)
{
	সংরচনা mbox_walk_data *walk_data = (সংরচনা mbox_walk_data *)data;
	সংরচনা k_mbox_বার্তা *tx_বার্তা;

	tx_বার্তা = (সংরচনা k_mbox_বার্তা *)thread->base.swap_data;

	যদি (mbox_message_match(tx_বার্তা, walk_data->বার্তা) != 0) {
		/* No match. Proceed to next in wait queue */
		প্রত্যাগম 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(শূন্য)z_try_abort_thread_timeout(thread);

	প্রত্যাগম 1;
}

পূর্ণাঙ্ক k_mbox_get(সংরচনা k_mbox *mbox, সংরচনা k_mbox_বার্তা *rx_বার্তা, শূন্য *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক result;

	/* save receiver id so it can be used during message matching */
	rx_বার্তা->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	চাবি = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	সংরচনা mbox_walk_data walk_data;
	পূর্ণাঙ্ক status;

	walk_data.thread = রিক্ত;   /* Unused by walk operation */
	walk_data.বার্তা = rx_বার্তা;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_বার্তা_queue,
				    mbox_get_walk_op, রিক্ত, &walk_data);

	যদি (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, চাবি);
		result = 0;

		গচ্ছ out;
	}

	/* didn't find a matching sender */

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, চাবি);

		result = -ENOMSG;
		গচ্ছ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_বার্তা;
	result = z_pend_curr(&mbox->lock, চাবি, &mbox->rx_বার্তা_queue, timeout);

out:
	/* consume message data immediately, if needed */
	যদি (result == 0) {
		result = mbox_message_data_check(rx_বার্তা, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	প্রত্যাগম result;
}
