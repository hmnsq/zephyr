/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <zephyr/linker/sections.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>
/* private kernel APIs */
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <kthread.h>
#அந்தர்பாவ <scheduler.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

கட்டமைப்பு mbox_walk_data {
	கட்டமைப்பு k_thread *thread;
	கட்டமைப்பு k_mbox_செய்தி *செய்தி;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#எனில் (CONFIG_இலக்கம்_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
கட்டமைப்பு k_mbox_async {
	கட்டமைப்பு _thread_base thread;		/* dummy thread object */
	கட்டமைப்பு k_mbox_செய்தி tx_செய்தி;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_அடுக்கு_DEFINE(async_செய்தி_free, CONFIG_இலக்கம்_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
நிலையான உள்வரி வெறுமை mbox_async_alloc(கட்டமைப்பு k_mbox_async **async)
{
	(வெறுமை)k_அடுக்கு_எடு(&async_செய்தி_free, (அடுக்கு_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
நிலையான உள்வரி வெறுமை mbox_async_free(கட்டமைப்பு k_mbox_async *async)
{
	k_அடுக்கு_தள்ளு(&async_செய்தி_free, (அடுக்கு_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
நிலையான வெறுமை init_mbox_module(வெறுமை)
{
	/* array of asynchronous message descriptors */
	நிலையான கட்டமைப்பு k_mbox_async __noinit async_செய்தி[CONFIG_இலக்கம்_MBOX_ASYNC_MSGS];

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

	முழு i;

	க்கு (i = 0; i < CONFIG_இலக்கம்_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_செய்தி[i].thread, 0, _THREAD_DUMMY, 0);
		k_அடுக்கு_தள்ளு(&async_செய்தி_free, (அடுக்கு_data_t)&async_செய்தி[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

வெறுமை k_mbox_init(கட்டமைப்பு k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_செய்தி_queue);
	z_waitq_init(&mbox->rx_செய்தி_queue);
	mbox->lock = (கட்டமைப்பு k_spinlock) {};

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
நிலையான முழு mbox_message_match(கட்டமைப்பு k_mbox_செய்தி *tx_செய்தி,
			       கட்டமைப்பு k_mbox_செய்தி *rx_செய்தி)
{
	uint32_t temp_info;

	எனில் (((tx_செய்தி->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_செய்தி->tx_target_thread == rx_செய்தி->tx_target_thread)) &&
	    ((rx_செய்தி->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_செய்தி->rx_source_thread == tx_செய்தி->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_செய்தி->rx_source_thread = tx_செய்தி->rx_source_thread;
		tx_செய்தி->tx_target_thread = rx_செய்தி->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_செய்தி->info;
		rx_செய்தி->info = tx_செய்தி->info;
		tx_செய்தி->info = temp_info;

		/* update data size field for receiver only */
		எனில் (rx_செய்தி->size > tx_செய்தி->size) {
			rx_செய்தி->size = tx_செய்தி->size;
		}

		/* update data location fields for receiver only */
		rx_செய்தி->tx_data = tx_செய்தி->tx_data;

		/* update syncing thread field for receiver only */
		rx_செய்தி->_syncing_thread = tx_செய்தி->_syncing_thread;

		திரும்பு 0;
	}

	திரும்பு -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
நிலையான வெறுமை mbox_message_dispose(கட்டமைப்பு k_mbox_செய்தி *rx_செய்தி)
{
	கட்டமைப்பு k_thread *sending_thread;
	கட்டமைப்பு k_mbox_செய்தி *tx_செய்தி;

	/* do nothing if message was disposed of when it was received */
	எனில் (rx_செய்தி->_syncing_thread == வெற்று) {
		திரும்பு;
	}

	/* recover sender info */
	sending_thread = rx_செய்தி->_syncing_thread;
	rx_செய்தி->_syncing_thread = வெற்று;
	tx_செய்தி = (கட்டமைப்பு k_mbox_செய்தி *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_செய்தி->size = rx_செய்தி->size;

#எனில் (CONFIG_இலக்கம்_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	எனில் (is_thread_dummy(sending_thread)) {
		கட்டமைப்பு k_sem *async_sem = tx_செய்தி->_async_sem;

		mbox_async_free((கட்டமைப்பு k_mbox_async *)sending_thread);
		எனில் (async_sem != வெற்று) {
			k_sem_give(async_sem);
		}
		திரும்பு;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_சாவி_t சாவி = k_spin_lock(&_sched_spinlock);

	arch_thread_return_மதிப்பு_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, சாவி);
}

நிலையான முழு mbox_message_put_walk_op(கட்டமைப்பு k_thread *thread, வெறுமை *data)
{
	கட்டமைப்பு mbox_walk_data *walk_data = (கட்டமைப்பு mbox_walk_data *)data;
	கட்டமைப்பு k_mbox_செய்தி *rx_செய்தி;

	rx_செய்தி = (கட்டமைப்பு k_mbox_செய்தி *)thread->base.swap_data;

	எனில் (mbox_message_match(walk_data->செய்தி, rx_செய்தி) != 0) {
		/* No match. Proceed to next in wait queue */
		திரும்பு 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(வெறுமை)z_try_abort_thread_timeout(thread);
	arch_thread_return_மதிப்பு_set(thread, 0);
	z_sched_ready_locked(thread);

#எனில் (CONFIG_இலக்கம்_MBOX_ASYNC_MSGS > 0)
	எனில் (is_thread_dummy(walk_data->thread)) {
		திரும்பு ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	திரும்பு SYNCHRONOUS_MATCH_FOUND;
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
நிலையான முழு mbox_message_put(கட்டமைப்பு k_mbox *mbox, கட்டமைப்பு k_mbox_செய்தி *tx_செய்தி,
			     k_timeout_t timeout)
{
	கட்டமைப்பு k_thread *sending_thread;
	k_spinlock_சாவி_t சாவி;

	/* save sender id so it can be used during message matching */
	tx_செய்தி->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_செய்தி->_syncing_thread;
	sending_thread->base.swap_data = tx_செய்தி;

	/* search mailbox's rx queue for a compatible receiver */
	சாவி = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	கட்டமைப்பு mbox_walk_data walk_data;
	முழு status;

	walk_data.thread = sending_thread;
	walk_data.செய்தி = tx_செய்தி;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_செய்தி_queue,
				    mbox_message_put_walk_op, வெற்று,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#எனில் (CONFIG_இலக்கம்_MBOX_ASYNC_MSGS > 0)
	எனில் (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, சாவி);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		திரும்பு 0;
	}
#endif

	எனில் (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		முழு ret = z_pend_curr(&mbox->lock, சாவி, வெற்று, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		திரும்பு ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, சாவி);
		திரும்பு -ENOMSG;
	}

#எனில் (CONFIG_இலக்கம்_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	எனில் (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_செய்தி_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, சாவி);
		திரும்பு 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	முழு ret = z_pend_curr(&mbox->lock, சாவி, &mbox->tx_செய்தி_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	திரும்பு ret;
}

முழு k_mbox_put(கட்டமைப்பு k_mbox *mbox, கட்டமைப்பு k_mbox_செய்தி *tx_செய்தி,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_செய்தி->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	முழு ret = mbox_message_put(mbox, tx_செய்தி, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	திரும்பு ret;
}

#எனில் (CONFIG_இலக்கம்_MBOX_ASYNC_MSGS > 0)
வெறுமை k_mbox_async_put(கட்டமைப்பு k_mbox *mbox, கட்டமைப்பு k_mbox_செய்தி *tx_செய்தி,
		      கட்டமைப்பு k_sem *sem)
{
	கட்டமைப்பு k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_செய்தி = *tx_செய்தி;
	async->tx_செய்தி._syncing_thread = (கட்டமைப்பு k_thread *)&async->thread;
	async->tx_செய்தி._async_sem = sem;

	(வெறுமை)mbox_message_put(mbox, &async->tx_செய்தி, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

வெறுமை k_mbox_data_get(கட்டமைப்பு k_mbox_செய்தி *rx_செய்தி, வெறுமை *buffer)
{
	/* handle case where data is to be discarded */
	எனில் (buffer == வெற்று) {
		rx_செய்தி->size = 0;
		mbox_message_dispose(rx_செய்தி);
		திரும்பு;
	}

	/* copy message data to buffer, then dispose of message */
	எனில் ((rx_செய்தி->tx_data != வெற்று) && (rx_செய்தி->size > 0U)) {
		(வெறுமை)memcpy(buffer, rx_செய்தி->tx_data, rx_செய்தி->size);
	}
	mbox_message_dispose(rx_செய்தி);
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
நிலையான முழு mbox_message_data_check(கட்டமைப்பு k_mbox_செய்தி *rx_செய்தி, வெறுமை *buffer)
{
	எனில் (buffer != வெற்று) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_செய்தி, buffer);
	} இல்லையெனில் எனில் (rx_செய்தி->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_செய்தி);
	} இல்லையெனில் {
		/* keep message around for later data retrieval */
	}

	திரும்பு 0;
}

நிலையான முழு mbox_get_walk_op(கட்டமைப்பு k_thread *thread, வெறுமை *data)
{
	கட்டமைப்பு mbox_walk_data *walk_data = (கட்டமைப்பு mbox_walk_data *)data;
	கட்டமைப்பு k_mbox_செய்தி *tx_செய்தி;

	tx_செய்தி = (கட்டமைப்பு k_mbox_செய்தி *)thread->base.swap_data;

	எனில் (mbox_message_match(tx_செய்தி, walk_data->செய்தி) != 0) {
		/* No match. Proceed to next in wait queue */
		திரும்பு 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(வெறுமை)z_try_abort_thread_timeout(thread);

	திரும்பு 1;
}

முழு k_mbox_get(கட்டமைப்பு k_mbox *mbox, கட்டமைப்பு k_mbox_செய்தி *rx_செய்தி, வெறுமை *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_சாவி_t சாவி;
	முழு result;

	/* save receiver id so it can be used during message matching */
	rx_செய்தி->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	சாவி = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	கட்டமைப்பு mbox_walk_data walk_data;
	முழு status;

	walk_data.thread = வெற்று;   /* Unused by walk operation */
	walk_data.செய்தி = rx_செய்தி;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_செய்தி_queue,
				    mbox_get_walk_op, வெற்று, &walk_data);

	எனில் (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, சாவி);
		result = 0;

		செல் out;
	}

	/* didn't find a matching sender */

	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, சாவி);

		result = -ENOMSG;
		செல் out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_செய்தி;
	result = z_pend_curr(&mbox->lock, சாவி, &mbox->rx_செய்தி_queue, timeout);

out:
	/* consume message data immediately, if needed */
	எனில் (result == 0) {
		result = mbox_message_data_check(rx_செய்தி, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	திரும்பு result;
}
