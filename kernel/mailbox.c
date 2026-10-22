/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#समावेश <zephyr/kernel.h>

#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/linker/sections.h>
#समावेश <string.h>
#समावेश <zephyr/sys/dlist.h>
/* private kernel APIs */
#समावेश <ksched.h>
#समावेश <kthread.h>
#समावेश <scheduler.h>
#समावेश <kernel_internal.h>
#समावेश <wait_q.h>

#घोषणा SYNCHRONOUS_MATCH_FOUND  1
#घोषणा ASYNCHRONOUS_MATCH_FOUND 2

काष्ठा mbox_walk_तथ्य {
	काष्ठा k_thread *thread;
	काष्ठा k_mbox_संदेश *संदेश;
};

#अगर_घोषित CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_MAILBOX */

#अगर (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
काष्ठा k_mbox_async {
	काष्ठा _thread_base thread;		/* dummy thread object */
	काष्ठा k_mbox_संदेश tx_संदेश;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ढेर_DEFINE(async_संदेश_free, CONFIG_अंक_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
अटल अंतरभूत व्योम mbox_async_alloc(काष्ठा k_mbox_async **async)
{
	(व्योम)k_ढेर_निकालो(&async_संदेश_free, (ढेर_तथ्य_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
अटल अंतरभूत व्योम mbox_async_free(काष्ठा k_mbox_async *async)
{
	k_ढेर_धकेलो(&async_संदेश_free, (ढेर_तथ्य_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
अटल व्योम init_mbox_module(व्योम)
{
	/* array of asynchronous message descriptors */
	अटल काष्ठा k_mbox_async __noinit async_संदेश[CONFIG_अंक_MBOX_ASYNC_MSGS];

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

	पूर्णांक i;

	क्रम (i = 0; i < CONFIG_अंक_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_संदेश[i].thread, 0, _THREAD_DUMMY, 0);
		k_ढेर_धकेलो(&async_संदेश_free, (ढेर_तथ्य_t)&async_संदेश[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#पूर्ण_अगर /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

व्योम k_mbox_init(काष्ठा k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_संदेश_queue);
	z_waitq_init(&mbox->rx_संदेश_queue);
	mbox->lock = (काष्ठा k_spinlock) {};

#अगर_घोषित CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_MAILBOX */

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
अटल पूर्णांक mbox_message_match(काष्ठा k_mbox_संदेश *tx_संदेश,
			       काष्ठा k_mbox_संदेश *rx_संदेश)
{
	uint32_t अस्थायी_info;

	अगर (((tx_संदेश->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_संदेश->tx_target_thread == rx_संदेश->tx_target_thread)) &&
	    ((rx_संदेश->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_संदेश->rx_source_thread == tx_संदेश->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_संदेश->rx_source_thread = tx_संदेश->rx_source_thread;
		tx_संदेश->tx_target_thread = rx_संदेश->tx_target_thread;

		/* update application info fields for both descriptors */
		अस्थायी_info = rx_संदेश->info;
		rx_संदेश->info = tx_संदेश->info;
		tx_संदेश->info = अस्थायी_info;

		/* update data size field for receiver only */
		अगर (rx_संदेश->आकार > tx_संदेश->आकार) {
			rx_संदेश->आकार = tx_संदेश->आकार;
		}

		/* update data location fields for receiver only */
		rx_संदेश->tx_तथ्य = tx_संदेश->tx_तथ्य;

		/* update syncing thread field for receiver only */
		rx_संदेश->_syncing_thread = tx_संदेश->_syncing_thread;

		वापस 0;
	}

	वापस -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
अटल व्योम mbox_message_dispose(काष्ठा k_mbox_संदेश *rx_संदेश)
{
	काष्ठा k_thread *sending_thread;
	काष्ठा k_mbox_संदेश *tx_संदेश;

	/* do nothing if message was disposed of when it was received */
	अगर (rx_संदेश->_syncing_thread == शून्य) {
		वापस;
	}

	/* recover sender info */
	sending_thread = rx_संदेश->_syncing_thread;
	rx_संदेश->_syncing_thread = शून्य;
	tx_संदेश = (काष्ठा k_mbox_संदेश *)sending_thread->base.swap_तथ्य;

	/* update data size field for sender */
	tx_संदेश->आकार = rx_संदेश->आकार;

#अगर (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	अगर (is_thread_dummy(sending_thread)) {
		काष्ठा k_sem *async_sem = tx_संदेश->_async_sem;

		mbox_async_free((काष्ठा k_mbox_async *)sending_thread);
		अगर (async_sem != शून्य) {
			k_sem_give(async_sem);
		}
		वापस;
	}
#पूर्ण_अगर /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&_sched_spinlock);

	arch_thread_return_मूल्य_set(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, कुंजी);
}

अटल पूर्णांक mbox_message_put_walk_op(काष्ठा k_thread *thread, व्योम *तथ्य)
{
	काष्ठा mbox_walk_तथ्य *walk_तथ्य = (काष्ठा mbox_walk_तथ्य *)तथ्य;
	काष्ठा k_mbox_संदेश *rx_संदेश;

	rx_संदेश = (काष्ठा k_mbox_संदेश *)thread->base.swap_तथ्य;

	अगर (mbox_message_match(walk_तथ्य->संदेश, rx_संदेश) != 0) {
		/* No match. Proceed to next in wait queue */
		वापस 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(व्योम)z_try_abort_thread_timeout(thread);
	arch_thread_return_मूल्य_set(thread, 0);
	z_sched_ready_locked(thread);

#अगर (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	अगर (is_thread_dummy(walk_तथ्य->thread)) {
		वापस ASYNCHRONOUS_MATCH_FOUND;
	}
#पूर्ण_अगर

	वापस SYNCHRONOUS_MATCH_FOUND;
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
अटल पूर्णांक mbox_message_put(काष्ठा k_mbox *mbox, काष्ठा k_mbox_संदेश *tx_संदेश,
			     k_timeout_t timeout)
{
	काष्ठा k_thread *sending_thread;
	k_spinlock_कुंजी_t कुंजी;

	/* save sender id so it can be used during message matching */
	tx_संदेश->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_संदेश->_syncing_thread;
	sending_thread->base.swap_तथ्य = tx_संदेश;

	/* search mailbox's rx queue for a compatible receiver */
	कुंजी = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	काष्ठा mbox_walk_तथ्य walk_तथ्य;
	पूर्णांक status;

	walk_तथ्य.thread = sending_thread;
	walk_तथ्य.संदेश = tx_संदेश;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_संदेश_queue,
				    mbox_message_put_walk_op, शून्य,
				    &walk_तथ्य);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#अगर (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	अगर (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, कुंजी);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		वापस 0;
	}
#पूर्ण_अगर

	अगर (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		पूर्णांक ret = z_pend_curr(&mbox->lock, कुंजी, शून्य, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		वापस ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, कुंजी);
		वापस -ENOMSG;
	}

#अगर (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	अगर (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_संदेश_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, कुंजी);
		वापस 0;
	}
#पूर्ण_अगर /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	पूर्णांक ret = z_pend_curr(&mbox->lock, कुंजी, &mbox->tx_संदेश_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	वापस ret;
}

पूर्णांक k_mbox_put(काष्ठा k_mbox *mbox, काष्ठा k_mbox_संदेश *tx_संदेश,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_संदेश->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	पूर्णांक ret = mbox_message_put(mbox, tx_संदेश, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	वापस ret;
}

#अगर (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
व्योम k_mbox_async_put(काष्ठा k_mbox *mbox, काष्ठा k_mbox_संदेश *tx_संदेश,
		      काष्ठा k_sem *sem)
{
	काष्ठा k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_संदेश = *tx_संदेश;
	async->tx_संदेश._syncing_thread = (काष्ठा k_thread *)&async->thread;
	async->tx_संदेश._async_sem = sem;

	(व्योम)mbox_message_put(mbox, &async->tx_संदेश, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#पूर्ण_अगर /* CONFIG_NUM_MBOX_ASYNC_MSGS */

व्योम k_mbox_तथ्य_get(काष्ठा k_mbox_संदेश *rx_संदेश, व्योम *बफर)
{
	/* handle case where data is to be discarded */
	अगर (बफर == शून्य) {
		rx_संदेश->आकार = 0;
		mbox_message_dispose(rx_संदेश);
		वापस;
	}

	/* copy message data to buffer, then dispose of message */
	अगर ((rx_संदेश->tx_तथ्य != शून्य) && (rx_संदेश->आकार > 0U)) {
		(व्योम)स_नकल(बफर, rx_संदेश->tx_तथ्य, rx_संदेश->आकार);
	}
	mbox_message_dispose(rx_संदेश);
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
अटल पूर्णांक mbox_message_तथ्य_check(काष्ठा k_mbox_संदेश *rx_संदेश, व्योम *बफर)
{
	अगर (बफर != शून्य) {
		/* retrieve data now, then dispose of message */
		k_mbox_तथ्य_get(rx_संदेश, बफर);
	} अन्यथा अगर (rx_संदेश->आकार == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_संदेश);
	} अन्यथा {
		/* keep message around for later data retrieval */
	}

	वापस 0;
}

अटल पूर्णांक mbox_get_walk_op(काष्ठा k_thread *thread, व्योम *तथ्य)
{
	काष्ठा mbox_walk_तथ्य *walk_तथ्य = (काष्ठा mbox_walk_तथ्य *)तथ्य;
	काष्ठा k_mbox_संदेश *tx_संदेश;

	tx_संदेश = (काष्ठा k_mbox_संदेश *)thread->base.swap_तथ्य;

	अगर (mbox_message_match(tx_संदेश, walk_तथ्य->संदेश) != 0) {
		/* No match. Proceed to next in wait queue */
		वापस 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(व्योम)z_try_abort_thread_timeout(thread);

	वापस 1;
}

पूर्णांक k_mbox_get(काष्ठा k_mbox *mbox, काष्ठा k_mbox_संदेश *rx_संदेश, व्योम *बफर,
	       k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक परिणाम;

	/* save receiver id so it can be used during message matching */
	rx_संदेश->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	कुंजी = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	काष्ठा mbox_walk_तथ्य walk_तथ्य;
	पूर्णांक status;

	walk_तथ्य.thread = शून्य;   /* Unused by walk operation */
	walk_तथ्य.संदेश = rx_संदेश;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_संदेश_queue,
				    mbox_get_walk_op, शून्य, &walk_तथ्य);

	अगर (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, कुंजी);
		परिणाम = 0;

		जाओ out;
	}

	/* didn't find a matching sender */

	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, कुंजी);

		परिणाम = -ENOMSG;
		जाओ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_तथ्य = rx_संदेश;
	परिणाम = z_pend_curr(&mbox->lock, कुंजी, &mbox->rx_संदेश_queue, timeout);

out:
	/* consume message data immediately, if needed */
	अगर (परिणाम == 0) {
		परिणाम = mbox_message_तथ्य_check(rx_संदेश, बफर);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, परिणाम);

	वापस परिणाम;
}
