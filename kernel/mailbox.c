/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/sections.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
/* private kernel APIs */
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

ಸಂರಚನಾ mbox_walk_data {
	ಸಂರಚನಾ k_thread *thread;
	ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *ಸಂದೇಶ;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, ರಿಕ್ತ);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#ಯದಿ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
ಸಂರಚನಾ k_mbox_async {
	ಸಂರಚನಾ _thread_base thread;		/* dummy thread object */
	ಸಂರಚನಾ k_mbox_ಸಂದೇಶ tx_ಸಂದೇಶ;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ರಾಶಿ_DEFINE(async_ಸಂದೇಶ_free, CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ mbox_async_alloc(ಸಂರಚನಾ k_mbox_async **async)
{
	(ಶೂನ್ಯ)k_ರಾಶಿ_ತೆಗೆ(&async_ಸಂದೇಶ_free, (ರಾಶಿ_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ mbox_async_free(ಸಂರಚನಾ k_mbox_async *async)
{
	k_ರಾಶಿ_ತಳ್ಳು(&async_ಸಂದೇಶ_free, (ರಾಶಿ_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ಸ್ಥಿರ ಶೂನ್ಯ init_mbox_module(ಶೂನ್ಯ)
{
	/* array of asynchronous message descriptors */
	ಸ್ಥಿರ ಸಂರಚನಾ k_mbox_async __noinit async_ಸಂದೇಶ[CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS];

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

	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_ಸಂದೇಶ[i].thread, 0, _THREAD_DUMMY, 0);
		k_ರಾಶಿ_ತಳ್ಳು(&async_ಸಂದೇಶ_free, (ರಾಶಿ_data_t)&async_ಸಂದೇಶ[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

ಶೂನ್ಯ k_mbox_init(ಸಂರಚನಾ k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_ಸಂದೇಶ_queue);
	z_waitq_init(&mbox->rx_ಸಂದೇಶ_queue);
	mbox->lock = (ಸಂರಚನಾ k_spinlock) {};

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mbox_message_match(ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
			       ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ)
{
	uint32_t temp_info;

	ಯದಿ (((tx_ಸಂದೇಶ->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_ಸಂದೇಶ->tx_target_thread == rx_ಸಂದೇಶ->tx_target_thread)) &&
	    ((rx_ಸಂದೇಶ->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_ಸಂದೇಶ->rx_source_thread == tx_ಸಂದೇಶ->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_ಸಂದೇಶ->rx_source_thread = tx_ಸಂದೇಶ->rx_source_thread;
		tx_ಸಂದೇಶ->tx_target_thread = rx_ಸಂದೇಶ->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_ಸಂದೇಶ->info;
		rx_ಸಂದೇಶ->info = tx_ಸಂದೇಶ->info;
		tx_ಸಂದೇಶ->info = temp_info;

		/* update data size field for receiver only */
		ಯದಿ (rx_ಸಂದೇಶ->size > tx_ಸಂದೇಶ->size) {
			rx_ಸಂದೇಶ->size = tx_ಸಂದೇಶ->size;
		}

		/* update data location fields for receiver only */
		rx_ಸಂದೇಶ->tx_data = tx_ಸಂದೇಶ->tx_data;

		/* update syncing thread field for receiver only */
		rx_ಸಂದೇಶ->_syncing_thread = tx_ಸಂದೇಶ->_syncing_thread;

		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
ಸ್ಥಿರ ಶೂನ್ಯ mbox_message_dispose(ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ)
{
	ಸಂರಚನಾ k_thread *sending_thread;
	ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ;

	/* do nothing if message was disposed of when it was received */
	ಯದಿ (rx_ಸಂದೇಶ->_syncing_thread == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* recover sender info */
	sending_thread = rx_ಸಂದೇಶ->_syncing_thread;
	rx_ಸಂದೇಶ->_syncing_thread = ರಿಕ್ತ;
	tx_ಸಂದೇಶ = (ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_ಸಂದೇಶ->size = rx_ಸಂದೇಶ->size;

#ಯದಿ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	ಯದಿ (is_thread_dummy(sending_thread)) {
		ಸಂರಚನಾ k_sem *async_sem = tx_ಸಂದೇಶ->_async_sem;

		mbox_async_free((ಸಂರಚನಾ k_mbox_async *)sending_thread);
		ಯದಿ (async_sem != ರಿಕ್ತ) {
			k_sem_give(async_sem);
		}
		ಪ್ರತ್ಯಾಗಮ;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&_sched_spinlock);

	arch_thread_return_ಮೌಲ್ಯ_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, ಕೀಲಿ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mbox_message_put_walk_op(ಸಂರಚನಾ k_thread *thread, ಶೂನ್ಯ *data)
{
	ಸಂರಚನಾ mbox_walk_data *walk_data = (ಸಂರಚನಾ mbox_walk_data *)data;
	ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ;

	rx_ಸಂದೇಶ = (ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *)thread->base.swap_data;

	ಯದಿ (mbox_message_match(walk_data->ಸಂದೇಶ, rx_ಸಂದೇಶ) != 0) {
		/* No match. Proceed to next in wait queue */
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);
	arch_thread_return_ಮೌಲ್ಯ_set(thread, 0);
	z_sched_ready_locked(thread);

#ಯದಿ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	ಯದಿ (is_thread_dummy(walk_data->thread)) {
		ಪ್ರತ್ಯಾಗಮ ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	ಪ್ರತ್ಯಾಗಮ SYNCHRONOUS_MATCH_FOUND;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mbox_message_put(ಸಂರಚನಾ k_mbox *mbox, ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
			     k_timeout_t timeout)
{
	ಸಂರಚನಾ k_thread *sending_thread;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	/* save sender id so it can be used during message matching */
	tx_ಸಂದೇಶ->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_ಸಂದೇಶ->_syncing_thread;
	sending_thread->base.swap_data = tx_ಸಂದೇಶ;

	/* search mailbox's rx queue for a compatible receiver */
	ಕೀಲಿ = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	ಸಂರಚನಾ mbox_walk_data walk_data;
	ಪೂರ್ಣಾಙ್ಕ status;

	walk_data.thread = sending_thread;
	walk_data.ಸಂದೇಶ = tx_ಸಂದೇಶ;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_ಸಂದೇಶ_queue,
				    mbox_message_put_walk_op, ರಿಕ್ತ,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#ಯದಿ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	ಯದಿ (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, ಕೀಲಿ);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#endif

	ಯದಿ (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		ಪೂರ್ಣಾಙ್ಕ ret = z_pend_curr(&mbox->lock, ಕೀಲಿ, ರಿಕ್ತ, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		ಪ್ರತ್ಯಾಗಮ ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ -ENOMSG;
	}

#ಯದಿ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	ಯದಿ (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_ಸಂದೇಶ_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	ಪೂರ್ಣಾಙ್ಕ ret = z_pend_curr(&mbox->lock, ಕೀಲಿ, &mbox->tx_ಸಂದೇಶ_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ k_mbox_put(ಸಂರಚನಾ k_mbox *mbox, ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_ಸಂದೇಶ->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	ಪೂರ್ಣಾಙ್ಕ ret = mbox_message_put(mbox, tx_ಸಂದೇಶ, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ಯದಿ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
ಶೂನ್ಯ k_mbox_async_put(ಸಂರಚನಾ k_mbox *mbox, ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
		      ಸಂರಚನಾ k_sem *sem)
{
	ಸಂರಚನಾ k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_ಸಂದೇಶ = *tx_ಸಂದೇಶ;
	async->tx_ಸಂದೇಶ._syncing_thread = (ಸಂರಚನಾ k_thread *)&async->thread;
	async->tx_ಸಂದೇಶ._async_sem = sem;

	(ಶೂನ್ಯ)mbox_message_put(mbox, &async->tx_ಸಂದೇಶ, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

ಶೂನ್ಯ k_mbox_data_get(ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ, ಶೂನ್ಯ *buffer)
{
	/* handle case where data is to be discarded */
	ಯದಿ (buffer == ರಿಕ್ತ) {
		rx_ಸಂದೇಶ->size = 0;
		mbox_message_dispose(rx_ಸಂದೇಶ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* copy message data to buffer, then dispose of message */
	ಯದಿ ((rx_ಸಂದೇಶ->tx_data != ರಿಕ್ತ) && (rx_ಸಂದೇಶ->size > 0U)) {
		(ಶೂನ್ಯ)memcpy(buffer, rx_ಸಂದೇಶ->tx_data, rx_ಸಂದೇಶ->size);
	}
	mbox_message_dispose(rx_ಸಂದೇಶ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mbox_message_data_check(ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ, ಶೂನ್ಯ *buffer)
{
	ಯದಿ (buffer != ರಿಕ್ತ) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_ಸಂದೇಶ, buffer);
	} ಅನ್ಯಥಾ ಯದಿ (rx_ಸಂದೇಶ->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_ಸಂದೇಶ);
	} ಅನ್ಯಥಾ {
		/* keep message around for later data retrieval */
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mbox_get_walk_op(ಸಂರಚನಾ k_thread *thread, ಶೂನ್ಯ *data)
{
	ಸಂರಚನಾ mbox_walk_data *walk_data = (ಸಂರಚನಾ mbox_walk_data *)data;
	ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ;

	tx_ಸಂದೇಶ = (ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *)thread->base.swap_data;

	ಯದಿ (mbox_message_match(tx_ಸಂದೇಶ, walk_data->ಸಂದೇಶ) != 0) {
		/* No match. Proceed to next in wait queue */
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);

	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ k_mbox_get(ಸಂರಚನಾ k_mbox *mbox, ಸಂರಚನಾ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ, ಶೂನ್ಯ *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ result;

	/* save receiver id so it can be used during message matching */
	rx_ಸಂದೇಶ->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	ಕೀಲಿ = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	ಸಂರಚನಾ mbox_walk_data walk_data;
	ಪೂರ್ಣಾಙ್ಕ status;

	walk_data.thread = ರಿಕ್ತ;   /* Unused by walk operation */
	walk_data.ಸಂದೇಶ = rx_ಸಂದೇಶ;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_ಸಂದೇಶ_queue,
				    mbox_get_walk_op, ರಿಕ್ತ, &walk_data);

	ಯದಿ (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, ಕೀಲಿ);
		result = 0;

		ಗಚ್ಛ out;
	}

	/* didn't find a matching sender */

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, ಕೀಲಿ);

		result = -ENOMSG;
		ಗಚ್ಛ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_ಸಂದೇಶ;
	result = z_pend_curr(&mbox->lock, ಕೀಲಿ, &mbox->rx_ಸಂದೇಶ_queue, timeout);

out:
	/* consume message data immediately, if needed */
	ಯದಿ (result == 0) {
		result = mbox_message_data_check(rx_ಸಂದೇಶ, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	ಪ್ರತ್ಯಾಗಮ result;
}
