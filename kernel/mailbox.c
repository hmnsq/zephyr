/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <zephyr/linker/sections.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>
/* private kernel APIs */
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <kthread.h>
#അന്തര്ഭാവ <scheduler.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

ഘടന mbox_walk_data {
	ഘടന k_thread *thread;
	ഘടന k_mbox_സന്ദേശം *സന്ദേശം;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#എങ്കിൽ (CONFIG_അക്കം_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
ഘടന k_mbox_async {
	ഘടന _thread_base thread;		/* dummy thread object */
	ഘടന k_mbox_സന്ദേശം tx_സന്ദേശം;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_അട്ടി_DEFINE(async_സന്ദേശം_free, CONFIG_അക്കം_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
സ്ഥിതം വരിയിൽ ശൂന്യം mbox_async_alloc(ഘടന k_mbox_async **async)
{
	(ശൂന്യം)k_അട്ടി_എടുക്കുക(&async_സന്ദേശം_free, (അട്ടി_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
സ്ഥിതം വരിയിൽ ശൂന്യം mbox_async_free(ഘടന k_mbox_async *async)
{
	k_അട്ടി_തള്ളുക(&async_സന്ദേശം_free, (അട്ടി_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
സ്ഥിതം ശൂന്യം init_mbox_module(ശൂന്യം)
{
	/* array of asynchronous message descriptors */
	സ്ഥിതം ഘടന k_mbox_async __noinit async_സന്ദേശം[CONFIG_അക്കം_MBOX_ASYNC_MSGS];

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

	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < CONFIG_അക്കം_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_സന്ദേശം[i].thread, 0, _THREAD_DUMMY, 0);
		k_അട്ടി_തള്ളുക(&async_സന്ദേശം_free, (അട്ടി_data_t)&async_സന്ദേശം[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

ശൂന്യം k_mbox_init(ഘടന k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_സന്ദേശം_queue);
	z_waitq_init(&mbox->rx_സന്ദേശം_queue);
	mbox->lock = (ഘടന k_spinlock) {};

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
സ്ഥിതം പൂർണ്ണസംഖ്യ mbox_message_match(ഘടന k_mbox_സന്ദേശം *tx_സന്ദേശം,
			       ഘടന k_mbox_സന്ദേശം *rx_സന്ദേശം)
{
	uint32_t temp_info;

	എങ്കിൽ (((tx_സന്ദേശം->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_സന്ദേശം->tx_target_thread == rx_സന്ദേശം->tx_target_thread)) &&
	    ((rx_സന്ദേശം->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_സന്ദേശം->rx_source_thread == tx_സന്ദേശം->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_സന്ദേശം->rx_source_thread = tx_സന്ദേശം->rx_source_thread;
		tx_സന്ദേശം->tx_target_thread = rx_സന്ദേശം->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_സന്ദേശം->info;
		rx_സന്ദേശം->info = tx_സന്ദേശം->info;
		tx_സന്ദേശം->info = temp_info;

		/* update data size field for receiver only */
		എങ്കിൽ (rx_സന്ദേശം->size > tx_സന്ദേശം->size) {
			rx_സന്ദേശം->size = tx_സന്ദേശം->size;
		}

		/* update data location fields for receiver only */
		rx_സന്ദേശം->tx_data = tx_സന്ദേശം->tx_data;

		/* update syncing thread field for receiver only */
		rx_സന്ദേശം->_syncing_thread = tx_സന്ദേശം->_syncing_thread;

		തിരികെ 0;
	}

	തിരികെ -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
സ്ഥിതം ശൂന്യം mbox_message_dispose(ഘടന k_mbox_സന്ദേശം *rx_സന്ദേശം)
{
	ഘടന k_thread *sending_thread;
	ഘടന k_mbox_സന്ദേശം *tx_സന്ദേശം;

	/* do nothing if message was disposed of when it was received */
	എങ്കിൽ (rx_സന്ദേശം->_syncing_thread == ശൂന്യ) {
		തിരികെ;
	}

	/* recover sender info */
	sending_thread = rx_സന്ദേശം->_syncing_thread;
	rx_സന്ദേശം->_syncing_thread = ശൂന്യ;
	tx_സന്ദേശം = (ഘടന k_mbox_സന്ദേശം *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_സന്ദേശം->size = rx_സന്ദേശം->size;

#എങ്കിൽ (CONFIG_അക്കം_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	എങ്കിൽ (is_thread_dummy(sending_thread)) {
		ഘടന k_sem *async_sem = tx_സന്ദേശം->_async_sem;

		mbox_async_free((ഘടന k_mbox_async *)sending_thread);
		എങ്കിൽ (async_sem != ശൂന്യ) {
			k_sem_give(async_sem);
		}
		തിരികെ;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&_sched_spinlock);

	arch_thread_return_മൂല്യം_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, താക്കോൽ);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ mbox_message_put_walk_op(ഘടന k_thread *thread, ശൂന്യം *data)
{
	ഘടന mbox_walk_data *walk_data = (ഘടന mbox_walk_data *)data;
	ഘടന k_mbox_സന്ദേശം *rx_സന്ദേശം;

	rx_സന്ദേശം = (ഘടന k_mbox_സന്ദേശം *)thread->base.swap_data;

	എങ്കിൽ (mbox_message_match(walk_data->സന്ദേശം, rx_സന്ദേശം) != 0) {
		/* No match. Proceed to next in wait queue */
		തിരികെ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(ശൂന്യം)z_try_abort_thread_timeout(thread);
	arch_thread_return_മൂല്യം_set(thread, 0);
	z_sched_ready_locked(thread);

#എങ്കിൽ (CONFIG_അക്കം_MBOX_ASYNC_MSGS > 0)
	എങ്കിൽ (is_thread_dummy(walk_data->thread)) {
		തിരികെ ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	തിരികെ SYNCHRONOUS_MATCH_FOUND;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ mbox_message_put(ഘടന k_mbox *mbox, ഘടന k_mbox_സന്ദേശം *tx_സന്ദേശം,
			     k_timeout_t timeout)
{
	ഘടന k_thread *sending_thread;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	/* save sender id so it can be used during message matching */
	tx_സന്ദേശം->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_സന്ദേശം->_syncing_thread;
	sending_thread->base.swap_data = tx_സന്ദേശം;

	/* search mailbox's rx queue for a compatible receiver */
	താക്കോൽ = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	ഘടന mbox_walk_data walk_data;
	പൂർണ്ണസംഖ്യ status;

	walk_data.thread = sending_thread;
	walk_data.സന്ദേശം = tx_സന്ദേശം;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_സന്ദേശം_queue,
				    mbox_message_put_walk_op, ശൂന്യ,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#എങ്കിൽ (CONFIG_അക്കം_MBOX_ASYNC_MSGS > 0)
	എങ്കിൽ (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, താക്കോൽ);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		തിരികെ 0;
	}
#endif

	എങ്കിൽ (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		പൂർണ്ണസംഖ്യ ret = z_pend_curr(&mbox->lock, താക്കോൽ, ശൂന്യ, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		തിരികെ ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, താക്കോൽ);
		തിരികെ -ENOMSG;
	}

#എങ്കിൽ (CONFIG_അക്കം_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	എങ്കിൽ (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_സന്ദേശം_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, താക്കോൽ);
		തിരികെ 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	പൂർണ്ണസംഖ്യ ret = z_pend_curr(&mbox->lock, താക്കോൽ, &mbox->tx_സന്ദേശം_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ k_mbox_put(ഘടന k_mbox *mbox, ഘടന k_mbox_സന്ദേശം *tx_സന്ദേശം,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_സന്ദേശം->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	പൂർണ്ണസംഖ്യ ret = mbox_message_put(mbox, tx_സന്ദേശം, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	തിരികെ ret;
}

#എങ്കിൽ (CONFIG_അക്കം_MBOX_ASYNC_MSGS > 0)
ശൂന്യം k_mbox_async_put(ഘടന k_mbox *mbox, ഘടന k_mbox_സന്ദേശം *tx_സന്ദേശം,
		      ഘടന k_sem *sem)
{
	ഘടന k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_സന്ദേശം = *tx_സന്ദേശം;
	async->tx_സന്ദേശം._syncing_thread = (ഘടന k_thread *)&async->thread;
	async->tx_സന്ദേശം._async_sem = sem;

	(ശൂന്യം)mbox_message_put(mbox, &async->tx_സന്ദേശം, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

ശൂന്യം k_mbox_data_get(ഘടന k_mbox_സന്ദേശം *rx_സന്ദേശം, ശൂന്യം *buffer)
{
	/* handle case where data is to be discarded */
	എങ്കിൽ (buffer == ശൂന്യ) {
		rx_സന്ദേശം->size = 0;
		mbox_message_dispose(rx_സന്ദേശം);
		തിരികെ;
	}

	/* copy message data to buffer, then dispose of message */
	എങ്കിൽ ((rx_സന്ദേശം->tx_data != ശൂന്യ) && (rx_സന്ദേശം->size > 0U)) {
		(ശൂന്യം)memcpy(buffer, rx_സന്ദേശം->tx_data, rx_സന്ദേശം->size);
	}
	mbox_message_dispose(rx_സന്ദേശം);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ mbox_message_data_check(ഘടന k_mbox_സന്ദേശം *rx_സന്ദേശം, ശൂന്യം *buffer)
{
	എങ്കിൽ (buffer != ശൂന്യ) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_സന്ദേശം, buffer);
	} അല്ലെങ്കിൽ എങ്കിൽ (rx_സന്ദേശം->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_സന്ദേശം);
	} അല്ലെങ്കിൽ {
		/* keep message around for later data retrieval */
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ mbox_get_walk_op(ഘടന k_thread *thread, ശൂന്യം *data)
{
	ഘടന mbox_walk_data *walk_data = (ഘടന mbox_walk_data *)data;
	ഘടന k_mbox_സന്ദേശം *tx_സന്ദേശം;

	tx_സന്ദേശം = (ഘടന k_mbox_സന്ദേശം *)thread->base.swap_data;

	എങ്കിൽ (mbox_message_match(tx_സന്ദേശം, walk_data->സന്ദേശം) != 0) {
		/* No match. Proceed to next in wait queue */
		തിരികെ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(ശൂന്യം)z_try_abort_thread_timeout(thread);

	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ k_mbox_get(ഘടന k_mbox *mbox, ഘടന k_mbox_സന്ദേശം *rx_സന്ദേശം, ശൂന്യം *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ result;

	/* save receiver id so it can be used during message matching */
	rx_സന്ദേശം->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	താക്കോൽ = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	ഘടന mbox_walk_data walk_data;
	പൂർണ്ണസംഖ്യ status;

	walk_data.thread = ശൂന്യ;   /* Unused by walk operation */
	walk_data.സന്ദേശം = rx_സന്ദേശം;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_സന്ദേശം_queue,
				    mbox_get_walk_op, ശൂന്യ, &walk_data);

	എങ്കിൽ (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, താക്കോൽ);
		result = 0;

		പോകുക out;
	}

	/* didn't find a matching sender */

	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, താക്കോൽ);

		result = -ENOMSG;
		പോകുക out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_സന്ദേശം;
	result = z_pend_curr(&mbox->lock, താക്കോൽ, &mbox->rx_സന്ദേശം_queue, timeout);

out:
	/* consume message data immediately, if needed */
	എങ്കിൽ (result == 0) {
		result = mbox_message_data_check(rx_സന്ദേശം, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	തിരികെ result;
}
