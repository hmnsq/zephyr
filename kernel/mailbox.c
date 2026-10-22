/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <zephyr/linker/sections.h>
#شامل <string.h>
#شامل <zephyr/sys/dlist.h>
/* private kernel APIs */
#شامل <ksched.h>
#شامل <kthread.h>
#شامل <scheduler.h>
#شامل <kernel_internal.h>
#شامل <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

ساختار mbox_walk_data {
	ساختار k_thread *thread;
	ساختار k_mbox_پیام *پیام;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#اگر (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
ساختار k_mbox_async {
	ساختار _thread_base thread;		/* dummy thread object */
	ساختار k_mbox_پیام tx_پیام;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_پشته_DEFINE(async_پیام_free, CONFIG_رقم_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ایستا درونخطی تهی mbox_async_alloc(ساختار k_mbox_async **async)
{
	(تهی)k_پشته_برداشت(&async_پیام_free, (پشته_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ایستا درونخطی تهی mbox_async_free(ساختار k_mbox_async *async)
{
	k_پشته_افزودن(&async_پیام_free, (پشته_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ایستا تهی init_mbox_module(تهی)
{
	/* array of asynchronous message descriptors */
	ایستا ساختار k_mbox_async __noinit async_پیام[CONFIG_رقم_MBOX_ASYNC_MSGS];

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

	صحیح i;

	برای (i = 0; i < CONFIG_رقم_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_پیام[i].thread, 0, _THREAD_DUMMY, 0);
		k_پشته_افزودن(&async_پیام_free, (پشته_data_t)&async_پیام[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

تهی k_mbox_init(ساختار k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_پیام_queue);
	z_waitq_init(&mbox->rx_پیام_queue);
	mbox->lock = (ساختار k_spinlock) {};

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
ایستا صحیح mbox_message_match(ساختار k_mbox_پیام *tx_پیام,
			       ساختار k_mbox_پیام *rx_پیام)
{
	uint32_t temp_info;

	اگر (((tx_پیام->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_پیام->tx_target_thread == rx_پیام->tx_target_thread)) &&
	    ((rx_پیام->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_پیام->rx_source_thread == tx_پیام->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_پیام->rx_source_thread = tx_پیام->rx_source_thread;
		tx_پیام->tx_target_thread = rx_پیام->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_پیام->info;
		rx_پیام->info = tx_پیام->info;
		tx_پیام->info = temp_info;

		/* update data size field for receiver only */
		اگر (rx_پیام->size > tx_پیام->size) {
			rx_پیام->size = tx_پیام->size;
		}

		/* update data location fields for receiver only */
		rx_پیام->tx_data = tx_پیام->tx_data;

		/* update syncing thread field for receiver only */
		rx_پیام->_syncing_thread = tx_پیام->_syncing_thread;

		بازگشت 0;
	}

	بازگشت -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
ایستا تهی mbox_message_dispose(ساختار k_mbox_پیام *rx_پیام)
{
	ساختار k_thread *sending_thread;
	ساختار k_mbox_پیام *tx_پیام;

	/* do nothing if message was disposed of when it was received */
	اگر (rx_پیام->_syncing_thread == پوچ) {
		بازگشت;
	}

	/* recover sender info */
	sending_thread = rx_پیام->_syncing_thread;
	rx_پیام->_syncing_thread = پوچ;
	tx_پیام = (ساختار k_mbox_پیام *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_پیام->size = rx_پیام->size;

#اگر (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	اگر (is_thread_dummy(sending_thread)) {
		ساختار k_sem *async_sem = tx_پیام->_async_sem;

		mbox_async_free((ساختار k_mbox_async *)sending_thread);
		اگر (async_sem != پوچ) {
			k_sem_give(async_sem);
		}
		بازگشت;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_کلید_t کلید = k_spin_lock(&_sched_spinlock);

	arch_thread_return_مقدار_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, کلید);
}

ایستا صحیح mbox_message_put_walk_op(ساختار k_thread *thread, تهی *data)
{
	ساختار mbox_walk_data *walk_data = (ساختار mbox_walk_data *)data;
	ساختار k_mbox_پیام *rx_پیام;

	rx_پیام = (ساختار k_mbox_پیام *)thread->base.swap_data;

	اگر (mbox_message_match(walk_data->پیام, rx_پیام) != 0) {
		/* No match. Proceed to next in wait queue */
		بازگشت 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(تهی)z_try_abort_thread_timeout(thread);
	arch_thread_return_مقدار_set(thread, 0);
	z_sched_ready_locked(thread);

#اگر (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	اگر (is_thread_dummy(walk_data->thread)) {
		بازگشت ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	بازگشت SYNCHRONOUS_MATCH_FOUND;
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
ایستا صحیح mbox_message_put(ساختار k_mbox *mbox, ساختار k_mbox_پیام *tx_پیام,
			     k_timeout_t timeout)
{
	ساختار k_thread *sending_thread;
	k_spinlock_کلید_t کلید;

	/* save sender id so it can be used during message matching */
	tx_پیام->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_پیام->_syncing_thread;
	sending_thread->base.swap_data = tx_پیام;

	/* search mailbox's rx queue for a compatible receiver */
	کلید = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	ساختار mbox_walk_data walk_data;
	صحیح status;

	walk_data.thread = sending_thread;
	walk_data.پیام = tx_پیام;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_پیام_queue,
				    mbox_message_put_walk_op, پوچ,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#اگر (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	اگر (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, کلید);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		بازگشت 0;
	}
#endif

	اگر (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		صحیح ret = z_pend_curr(&mbox->lock, کلید, پوچ, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		بازگشت ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, کلید);
		بازگشت -ENOMSG;
	}

#اگر (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	اگر (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_پیام_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, کلید);
		بازگشت 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	صحیح ret = z_pend_curr(&mbox->lock, کلید, &mbox->tx_پیام_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	بازگشت ret;
}

صحیح k_mbox_put(ساختار k_mbox *mbox, ساختار k_mbox_پیام *tx_پیام,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_پیام->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	صحیح ret = mbox_message_put(mbox, tx_پیام, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	بازگشت ret;
}

#اگر (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
تهی k_mbox_async_put(ساختار k_mbox *mbox, ساختار k_mbox_پیام *tx_پیام,
		      ساختار k_sem *sem)
{
	ساختار k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_پیام = *tx_پیام;
	async->tx_پیام._syncing_thread = (ساختار k_thread *)&async->thread;
	async->tx_پیام._async_sem = sem;

	(تهی)mbox_message_put(mbox, &async->tx_پیام, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

تهی k_mbox_data_get(ساختار k_mbox_پیام *rx_پیام, تهی *buffer)
{
	/* handle case where data is to be discarded */
	اگر (buffer == پوچ) {
		rx_پیام->size = 0;
		mbox_message_dispose(rx_پیام);
		بازگشت;
	}

	/* copy message data to buffer, then dispose of message */
	اگر ((rx_پیام->tx_data != پوچ) && (rx_پیام->size > 0U)) {
		(تهی)memcpy(buffer, rx_پیام->tx_data, rx_پیام->size);
	}
	mbox_message_dispose(rx_پیام);
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
ایستا صحیح mbox_message_data_check(ساختار k_mbox_پیام *rx_پیام, تهی *buffer)
{
	اگر (buffer != پوچ) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_پیام, buffer);
	} وگرنه اگر (rx_پیام->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_پیام);
	} وگرنه {
		/* keep message around for later data retrieval */
	}

	بازگشت 0;
}

ایستا صحیح mbox_get_walk_op(ساختار k_thread *thread, تهی *data)
{
	ساختار mbox_walk_data *walk_data = (ساختار mbox_walk_data *)data;
	ساختار k_mbox_پیام *tx_پیام;

	tx_پیام = (ساختار k_mbox_پیام *)thread->base.swap_data;

	اگر (mbox_message_match(tx_پیام, walk_data->پیام) != 0) {
		/* No match. Proceed to next in wait queue */
		بازگشت 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(تهی)z_try_abort_thread_timeout(thread);

	بازگشت 1;
}

صحیح k_mbox_get(ساختار k_mbox *mbox, ساختار k_mbox_پیام *rx_پیام, تهی *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحیح result;

	/* save receiver id so it can be used during message matching */
	rx_پیام->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	کلید = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	ساختار mbox_walk_data walk_data;
	صحیح status;

	walk_data.thread = پوچ;   /* Unused by walk operation */
	walk_data.پیام = rx_پیام;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_پیام_queue,
				    mbox_get_walk_op, پوچ, &walk_data);

	اگر (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, کلید);
		result = 0;

		برو out;
	}

	/* didn't find a matching sender */

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, کلید);

		result = -ENOMSG;
		برو out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_پیام;
	result = z_pend_curr(&mbox->lock, کلید, &mbox->rx_پیام_queue, timeout);

out:
	/* consume message data immediately, if needed */
	اگر (result == 0) {
		result = mbox_message_data_check(rx_پیام, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	بازگشت result;
}
