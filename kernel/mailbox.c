/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/linker/sections.h>
#ضمن <string.h>
#ضمن <zephyr/sys/dlist.h>
/* private kernel APIs */
#ضمن <ksched.h>
#ضمن <kthread.h>
#ضمن <scheduler.h>
#ضمن <kernel_internal.h>
#ضمن <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

بنية mbox_walk_data {
	بنية k_thread *thread;
	بنية k_mbox_msg *msg;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, عدم);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#ܐܢ (CONFIG_NUM_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
بنية k_mbox_async {
	بنية _thread_base thread;		/* dummy thread object */
	بنية k_mbox_msg tx_msg;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_STACK_DEFINE(async_msg_free, CONFIG_NUM_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_alloc(بنية k_mbox_async **async)
{
	(فراغ)k_stack_pop(&async_msg_free, (stack_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_free(بنية k_mbox_async *async)
{
	k_stack_push(&async_msg_free, (stack_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ساكن فراغ init_mbox_module(فراغ)
{
	/* array of asynchronous message descriptors */
	ساكن بنية k_mbox_async __noinit async_msg[CONFIG_NUM_MBOX_ASYNC_MSGS];

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

	صحيح i;

	لكل (i = 0; i < CONFIG_NUM_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_msg[i].thread, 0, _THREAD_DUMMY, 0);
		k_stack_push(&async_msg_free, (stack_data_t)&async_msg[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

فراغ k_mbox_init(بنية k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_msg_queue);
	z_waitq_init(&mbox->rx_msg_queue);
	mbox->lock = (بنية k_spinlock) {};

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
ساكن صحيح mbox_message_match(بنية k_mbox_msg *tx_msg,
			       بنية k_mbox_msg *rx_msg)
{
	uint32_t temp_info;

	ܐܢ (((tx_msg->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_msg->tx_target_thread == rx_msg->tx_target_thread)) &&
	    ((rx_msg->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_msg->rx_source_thread == tx_msg->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_msg->rx_source_thread = tx_msg->rx_source_thread;
		tx_msg->tx_target_thread = rx_msg->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_msg->info;
		rx_msg->info = tx_msg->info;
		tx_msg->info = temp_info;

		/* update data size field for receiver only */
		ܐܢ (rx_msg->size > tx_msg->size) {
			rx_msg->size = tx_msg->size;
		}

		/* update data location fields for receiver only */
		rx_msg->tx_data = tx_msg->tx_data;

		/* update syncing thread field for receiver only */
		rx_msg->_syncing_thread = tx_msg->_syncing_thread;

		ܦܢܐ 0;
	}

	ܦܢܐ -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
ساكن فراغ mbox_message_dispose(بنية k_mbox_msg *rx_msg)
{
	بنية k_thread *sending_thread;
	بنية k_mbox_msg *tx_msg;

	/* do nothing if message was disposed of when it was received */
	ܐܢ (rx_msg->_syncing_thread == عدم) {
		ܦܢܐ;
	}

	/* recover sender info */
	sending_thread = rx_msg->_syncing_thread;
	rx_msg->_syncing_thread = عدم;
	tx_msg = (بنية k_mbox_msg *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_msg->size = rx_msg->size;

#ܐܢ (CONFIG_NUM_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	ܐܢ (is_thread_dummy(sending_thread)) {
		بنية k_sem *async_sem = tx_msg->_async_sem;

		mbox_async_free((بنية k_mbox_async *)sending_thread);
		ܐܢ (async_sem != عدم) {
			k_sem_give(async_sem);
		}
		ܦܢܐ;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_key_t key = k_spin_lock(&_sched_spinlock);

	arch_thread_return_value_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, key);
}

ساكن صحيح mbox_message_put_walk_op(بنية k_thread *thread, فراغ *data)
{
	بنية mbox_walk_data *walk_data = (بنية mbox_walk_data *)data;
	بنية k_mbox_msg *rx_msg;

	rx_msg = (بنية k_mbox_msg *)thread->base.swap_data;

	ܐܢ (mbox_message_match(walk_data->msg, rx_msg) != 0) {
		/* No match. Proceed to next in wait queue */
		ܦܢܐ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(فراغ)z_try_abort_thread_timeout(thread);
	arch_thread_return_value_set(thread, 0);
	z_sched_ready_locked(thread);

#ܐܢ (CONFIG_NUM_MBOX_ASYNC_MSGS > 0)
	ܐܢ (is_thread_dummy(walk_data->thread)) {
		ܦܢܐ ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	ܦܢܐ SYNCHRONOUS_MATCH_FOUND;
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
ساكن صحيح mbox_message_put(بنية k_mbox *mbox, بنية k_mbox_msg *tx_msg,
			     k_timeout_t timeout)
{
	بنية k_thread *sending_thread;
	k_spinlock_key_t key;

	/* save sender id so it can be used during message matching */
	tx_msg->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_msg->_syncing_thread;
	sending_thread->base.swap_data = tx_msg;

	/* search mailbox's rx queue for a compatible receiver */
	key = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	بنية mbox_walk_data walk_data;
	صحيح status;

	walk_data.thread = sending_thread;
	walk_data.msg = tx_msg;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_msg_queue,
				    mbox_message_put_walk_op, عدم,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#ܐܢ (CONFIG_NUM_MBOX_ASYNC_MSGS > 0)
	ܐܢ (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, key);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		ܦܢܐ 0;
	}
#endif

	ܐܢ (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		صحيح ret = z_pend_curr(&mbox->lock, key, عدم, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		ܦܢܐ ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	ܐܢ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, key);
		ܦܢܐ -ENOMSG;
	}

#ܐܢ (CONFIG_NUM_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	ܐܢ (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_msg_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, key);
		ܦܢܐ 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	صحيح ret = z_pend_curr(&mbox->lock, key, &mbox->tx_msg_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	ܦܢܐ ret;
}

صحيح k_mbox_put(بنية k_mbox *mbox, بنية k_mbox_msg *tx_msg,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_msg->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	صحيح ret = mbox_message_put(mbox, tx_msg, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	ܦܢܐ ret;
}

#ܐܢ (CONFIG_NUM_MBOX_ASYNC_MSGS > 0)
فراغ k_mbox_async_put(بنية k_mbox *mbox, بنية k_mbox_msg *tx_msg,
		      بنية k_sem *sem)
{
	بنية k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_msg = *tx_msg;
	async->tx_msg._syncing_thread = (بنية k_thread *)&async->thread;
	async->tx_msg._async_sem = sem;

	(فراغ)mbox_message_put(mbox, &async->tx_msg, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

فراغ k_mbox_data_get(بنية k_mbox_msg *rx_msg, فراغ *buffer)
{
	/* handle case where data is to be discarded */
	ܐܢ (buffer == عدم) {
		rx_msg->size = 0;
		mbox_message_dispose(rx_msg);
		ܦܢܐ;
	}

	/* copy message data to buffer, then dispose of message */
	ܐܢ ((rx_msg->tx_data != عدم) && (rx_msg->size > 0U)) {
		(فراغ)memcpy(buffer, rx_msg->tx_data, rx_msg->size);
	}
	mbox_message_dispose(rx_msg);
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
ساكن صحيح mbox_message_data_check(بنية k_mbox_msg *rx_msg, فراغ *buffer)
{
	ܐܢ (buffer != عدم) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_msg, buffer);
	} ܐܚܪܢܐ ܐܢ (rx_msg->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_msg);
	} ܐܚܪܢܐ {
		/* keep message around for later data retrieval */
	}

	ܦܢܐ 0;
}

ساكن صحيح mbox_get_walk_op(بنية k_thread *thread, فراغ *data)
{
	بنية mbox_walk_data *walk_data = (بنية mbox_walk_data *)data;
	بنية k_mbox_msg *tx_msg;

	tx_msg = (بنية k_mbox_msg *)thread->base.swap_data;

	ܐܢ (mbox_message_match(tx_msg, walk_data->msg) != 0) {
		/* No match. Proceed to next in wait queue */
		ܦܢܐ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(فراغ)z_try_abort_thread_timeout(thread);

	ܦܢܐ 1;
}

صحيح k_mbox_get(بنية k_mbox *mbox, بنية k_mbox_msg *rx_msg, فراغ *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_key_t key;
	صحيح result;

	/* save receiver id so it can be used during message matching */
	rx_msg->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	key = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	بنية mbox_walk_data walk_data;
	صحيح status;

	walk_data.thread = عدم;   /* Unused by walk operation */
	walk_data.msg = rx_msg;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_msg_queue,
				    mbox_get_walk_op, عدم, &walk_data);

	ܐܢ (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, key);
		result = 0;

		اذهب out;
	}

	/* didn't find a matching sender */

	ܐܢ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, key);

		result = -ENOMSG;
		اذهب out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_msg;
	result = z_pend_curr(&mbox->lock, key, &mbox->rx_msg_queue, timeout);

out:
	/* consume message data immediately, if needed */
	ܐܢ (result == 0) {
		result = mbox_message_data_check(rx_msg, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	ܦܢܐ result;
}
