/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/linker/sections.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>
/* private kernel APIs */
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <scheduler.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

સંરચના mbox_walk_data {
	સંરચના k_thread *thread;
	સંરચના k_mbox_સંદેશ *સંદેશ;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#જો (CONFIG_અંક_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
સંરચના k_mbox_async {
	સંરચના _thread_base thread;		/* dummy thread object */
	સંરચના k_mbox_સંદેશ tx_સંદેશ;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ઢગલો_DEFINE(async_સંદેશ_free, CONFIG_અંક_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
સ્થિર અંતઃરેખ શૂન્ય mbox_async_alloc(સંરચના k_mbox_async **async)
{
	(શૂન્ય)k_ઢગલો_કાઢો(&async_સંદેશ_free, (ઢગલો_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
સ્થિર અંતઃરેખ શૂન્ય mbox_async_free(સંરચના k_mbox_async *async)
{
	k_ઢગલો_ધકેલો(&async_સંદેશ_free, (ઢગલો_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
સ્થિર શૂન્ય init_mbox_module(શૂન્ય)
{
	/* array of asynchronous message descriptors */
	સ્થિર સંરચના k_mbox_async __noinit async_સંદેશ[CONFIG_અંક_MBOX_ASYNC_MSGS];

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

	પૂર્ણાંક i;

	માટે (i = 0; i < CONFIG_અંક_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_સંદેશ[i].thread, 0, _THREAD_DUMMY, 0);
		k_ઢગલો_ધકેલો(&async_સંદેશ_free, (ઢગલો_data_t)&async_સંદેશ[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

શૂન્ય k_mbox_init(સંરચના k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_સંદેશ_queue);
	z_waitq_init(&mbox->rx_સંદેશ_queue);
	mbox->lock = (સંરચના k_spinlock) {};

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
સ્થિર પૂર્ણાંક mbox_message_match(સંરચના k_mbox_સંદેશ *tx_સંદેશ,
			       સંરચના k_mbox_સંદેશ *rx_સંદેશ)
{
	uint32_t temp_info;

	જો (((tx_સંદેશ->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_સંદેશ->tx_target_thread == rx_સંદેશ->tx_target_thread)) &&
	    ((rx_સંદેશ->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_સંદેશ->rx_source_thread == tx_સંદેશ->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_સંદેશ->rx_source_thread = tx_સંદેશ->rx_source_thread;
		tx_સંદેશ->tx_target_thread = rx_સંદેશ->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_સંદેશ->info;
		rx_સંદેશ->info = tx_સંદેશ->info;
		tx_સંદેશ->info = temp_info;

		/* update data size field for receiver only */
		જો (rx_સંદેશ->size > tx_સંદેશ->size) {
			rx_સંદેશ->size = tx_સંદેશ->size;
		}

		/* update data location fields for receiver only */
		rx_સંદેશ->tx_data = tx_સંદેશ->tx_data;

		/* update syncing thread field for receiver only */
		rx_સંદેશ->_syncing_thread = tx_સંદેશ->_syncing_thread;

		પાછા 0;
	}

	પાછા -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
સ્થિર શૂન્ય mbox_message_dispose(સંરચના k_mbox_સંદેશ *rx_સંદેશ)
{
	સંરચના k_thread *sending_thread;
	સંરચના k_mbox_સંદેશ *tx_સંદેશ;

	/* do nothing if message was disposed of when it was received */
	જો (rx_સંદેશ->_syncing_thread == ખાલી) {
		પાછા;
	}

	/* recover sender info */
	sending_thread = rx_સંદેશ->_syncing_thread;
	rx_સંદેશ->_syncing_thread = ખાલી;
	tx_સંદેશ = (સંરચના k_mbox_સંદેશ *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_સંદેશ->size = rx_સંદેશ->size;

#જો (CONFIG_અંક_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	જો (is_thread_dummy(sending_thread)) {
		સંરચના k_sem *async_sem = tx_સંદેશ->_async_sem;

		mbox_async_free((સંરચના k_mbox_async *)sending_thread);
		જો (async_sem != ખાલી) {
			k_sem_give(async_sem);
		}
		પાછા;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&_sched_spinlock);

	arch_thread_return_કિંમત_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, ચાવી);
}

સ્થિર પૂર્ણાંક mbox_message_put_walk_op(સંરચના k_thread *thread, શૂન્ય *data)
{
	સંરચના mbox_walk_data *walk_data = (સંરચના mbox_walk_data *)data;
	સંરચના k_mbox_સંદેશ *rx_સંદેશ;

	rx_સંદેશ = (સંરચના k_mbox_સંદેશ *)thread->base.swap_data;

	જો (mbox_message_match(walk_data->સંદેશ, rx_સંદેશ) != 0) {
		/* No match. Proceed to next in wait queue */
		પાછા 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(શૂન્ય)z_try_abort_thread_timeout(thread);
	arch_thread_return_કિંમત_set(thread, 0);
	z_sched_ready_locked(thread);

#જો (CONFIG_અંક_MBOX_ASYNC_MSGS > 0)
	જો (is_thread_dummy(walk_data->thread)) {
		પાછા ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	પાછા SYNCHRONOUS_MATCH_FOUND;
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
સ્થિર પૂર્ણાંક mbox_message_put(સંરચના k_mbox *mbox, સંરચના k_mbox_સંદેશ *tx_સંદેશ,
			     k_timeout_t timeout)
{
	સંરચના k_thread *sending_thread;
	k_spinlock_ચાવી_t ચાવી;

	/* save sender id so it can be used during message matching */
	tx_સંદેશ->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_સંદેશ->_syncing_thread;
	sending_thread->base.swap_data = tx_સંદેશ;

	/* search mailbox's rx queue for a compatible receiver */
	ચાવી = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	સંરચના mbox_walk_data walk_data;
	પૂર્ણાંક status;

	walk_data.thread = sending_thread;
	walk_data.સંદેશ = tx_સંદેશ;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_સંદેશ_queue,
				    mbox_message_put_walk_op, ખાલી,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#જો (CONFIG_અંક_MBOX_ASYNC_MSGS > 0)
	જો (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, ચાવી);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		પાછા 0;
	}
#endif

	જો (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		પૂર્ણાંક ret = z_pend_curr(&mbox->lock, ચાવી, ખાલી, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		પાછા ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, ચાવી);
		પાછા -ENOMSG;
	}

#જો (CONFIG_અંક_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	જો (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_સંદેશ_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, ચાવી);
		પાછા 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	પૂર્ણાંક ret = z_pend_curr(&mbox->lock, ચાવી, &mbox->tx_સંદેશ_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	પાછા ret;
}

પૂર્ણાંક k_mbox_put(સંરચના k_mbox *mbox, સંરચના k_mbox_સંદેશ *tx_સંદેશ,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_સંદેશ->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	પૂર્ણાંક ret = mbox_message_put(mbox, tx_સંદેશ, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	પાછા ret;
}

#જો (CONFIG_અંક_MBOX_ASYNC_MSGS > 0)
શૂન્ય k_mbox_async_put(સંરચના k_mbox *mbox, સંરચના k_mbox_સંદેશ *tx_સંદેશ,
		      સંરચના k_sem *sem)
{
	સંરચના k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_સંદેશ = *tx_સંદેશ;
	async->tx_સંદેશ._syncing_thread = (સંરચના k_thread *)&async->thread;
	async->tx_સંદેશ._async_sem = sem;

	(શૂન્ય)mbox_message_put(mbox, &async->tx_સંદેશ, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

શૂન્ય k_mbox_data_get(સંરચના k_mbox_સંદેશ *rx_સંદેશ, શૂન્ય *buffer)
{
	/* handle case where data is to be discarded */
	જો (buffer == ખાલી) {
		rx_સંદેશ->size = 0;
		mbox_message_dispose(rx_સંદેશ);
		પાછા;
	}

	/* copy message data to buffer, then dispose of message */
	જો ((rx_સંદેશ->tx_data != ખાલી) && (rx_સંદેશ->size > 0U)) {
		(શૂન્ય)memcpy(buffer, rx_સંદેશ->tx_data, rx_સંદેશ->size);
	}
	mbox_message_dispose(rx_સંદેશ);
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
સ્થિર પૂર્ણાંક mbox_message_data_check(સંરચના k_mbox_સંદેશ *rx_સંદેશ, શૂન્ય *buffer)
{
	જો (buffer != ખાલી) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_સંદેશ, buffer);
	} નહીંતર જો (rx_સંદેશ->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_સંદેશ);
	} નહીંતર {
		/* keep message around for later data retrieval */
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક mbox_get_walk_op(સંરચના k_thread *thread, શૂન્ય *data)
{
	સંરચના mbox_walk_data *walk_data = (સંરચના mbox_walk_data *)data;
	સંરચના k_mbox_સંદેશ *tx_સંદેશ;

	tx_સંદેશ = (સંરચના k_mbox_સંદેશ *)thread->base.swap_data;

	જો (mbox_message_match(tx_સંદેશ, walk_data->સંદેશ) != 0) {
		/* No match. Proceed to next in wait queue */
		પાછા 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(શૂન્ય)z_try_abort_thread_timeout(thread);

	પાછા 1;
}

પૂર્ણાંક k_mbox_get(સંરચના k_mbox *mbox, સંરચના k_mbox_સંદેશ *rx_સંદેશ, શૂન્ય *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક result;

	/* save receiver id so it can be used during message matching */
	rx_સંદેશ->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	ચાવી = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	સંરચના mbox_walk_data walk_data;
	પૂર્ણાંક status;

	walk_data.thread = ખાલી;   /* Unused by walk operation */
	walk_data.સંદેશ = rx_સંદેશ;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_સંદેશ_queue,
				    mbox_get_walk_op, ખાલી, &walk_data);

	જો (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, ચાવી);
		result = 0;

		જાઓ out;
	}

	/* didn't find a matching sender */

	જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, ચાવી);

		result = -ENOMSG;
		જાઓ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_સંદેશ;
	result = z_pend_curr(&mbox->lock, ચાવી, &mbox->rx_સંદેશ_queue, timeout);

out:
	/* consume message data immediately, if needed */
	જો (result == 0) {
		result = mbox_message_data_check(rx_સંદેશ, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	પાછા result;
}
