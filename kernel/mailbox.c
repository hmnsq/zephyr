/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <zephyr/linker/sections.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <zephyr/sys/dlist.h>
/* private kernel APIs */
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <wait_q.h>

#define SYNCHRONOUS_MATCH_FOUND  1
#define ASYNCHRONOUS_MATCH_FOUND 2

संरचना mbox_walk_data {
	संरचना k_thread *thread;
	संरचना k_mbox_संदेश *संदेश;
};

#ifdef CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_MAILBOX */

#यदि (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
संरचना k_mbox_async {
	संरचना _thread_base thread;		/* dummy thread object */
	संरचना k_mbox_संदेश tx_संदेश;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ढेर_DEFINE(async_संदेश_free, CONFIG_अंक_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
स्थिर अन्तःस्थ शून्य mbox_async_alloc(संरचना k_mbox_async **async)
{
	(शून्य)k_ढेर_निकालो(&async_संदेश_free, (ढेर_data_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
स्थिर अन्तःस्थ शून्य mbox_async_free(संरचना k_mbox_async *async)
{
	k_ढेर_धकेलो(&async_संदेश_free, (ढेर_data_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
स्थिर शून्य init_mbox_module(शून्य)
{
	/* array of asynchronous message descriptors */
	स्थिर संरचना k_mbox_async __noinit async_संदेश[CONFIG_अंक_MBOX_ASYNC_MSGS];

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

	पूर्णाङ्क i;

	प्रति (i = 0; i < CONFIG_अंक_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_base(&async_संदेश[i].thread, 0, _THREAD_DUMMY, 0);
		k_ढेर_धकेलो(&async_संदेश_free, (ढेर_data_t)&async_संदेश[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

शून्य k_mbox_init(संरचना k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_संदेश_queue);
	z_waitq_init(&mbox->rx_संदेश_queue);
	mbox->lock = (संरचना k_spinlock) {};

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
स्थिर पूर्णाङ्क mbox_message_match(संरचना k_mbox_संदेश *tx_संदेश,
			       संरचना k_mbox_संदेश *rx_संदेश)
{
	uint32_t temp_info;

	यदि (((tx_संदेश->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_संदेश->tx_target_thread == rx_संदेश->tx_target_thread)) &&
	    ((rx_संदेश->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_संदेश->rx_source_thread == tx_संदेश->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_संदेश->rx_source_thread = tx_संदेश->rx_source_thread;
		tx_संदेश->tx_target_thread = rx_संदेश->tx_target_thread;

		/* update application info fields for both descriptors */
		temp_info = rx_संदेश->info;
		rx_संदेश->info = tx_संदेश->info;
		tx_संदेश->info = temp_info;

		/* update data size field for receiver only */
		यदि (rx_संदेश->size > tx_संदेश->size) {
			rx_संदेश->size = tx_संदेश->size;
		}

		/* update data location fields for receiver only */
		rx_संदेश->tx_data = tx_संदेश->tx_data;

		/* update syncing thread field for receiver only */
		rx_संदेश->_syncing_thread = tx_संदेश->_syncing_thread;

		प्रत्यागम 0;
	}

	प्रत्यागम -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
स्थिर शून्य mbox_message_dispose(संरचना k_mbox_संदेश *rx_संदेश)
{
	संरचना k_thread *sending_thread;
	संरचना k_mbox_संदेश *tx_संदेश;

	/* do nothing if message was disposed of when it was received */
	यदि (rx_संदेश->_syncing_thread == रिक्त) {
		प्रत्यागम;
	}

	/* recover sender info */
	sending_thread = rx_संदेश->_syncing_thread;
	rx_संदेश->_syncing_thread = रिक्त;
	tx_संदेश = (संरचना k_mbox_संदेश *)sending_thread->base.swap_data;

	/* update data size field for sender */
	tx_संदेश->size = rx_संदेश->size;

#यदि (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	यदि (is_thread_dummy(sending_thread)) {
		संरचना k_sem *async_sem = tx_संदेश->_async_sem;

		mbox_async_free((संरचना k_mbox_async *)sending_thread);
		यदि (async_sem != रिक्त) {
			k_sem_give(async_sem);
		}
		प्रत्यागम;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

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

स्थिर पूर्णाङ्क mbox_message_put_walk_op(संरचना k_thread *thread, शून्य *data)
{
	संरचना mbox_walk_data *walk_data = (संरचना mbox_walk_data *)data;
	संरचना k_mbox_संदेश *rx_संदेश;

	rx_संदेश = (संरचना k_mbox_संदेश *)thread->base.swap_data;

	यदि (mbox_message_match(walk_data->संदेश, rx_संदेश) != 0) {
		/* No match. Proceed to next in wait queue */
		प्रत्यागम 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(शून्य)z_try_abort_thread_timeout(thread);
	arch_thread_return_मूल्य_set(thread, 0);
	z_sched_ready_locked(thread);

#यदि (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	यदि (is_thread_dummy(walk_data->thread)) {
		प्रत्यागम ASYNCHRONOUS_MATCH_FOUND;
	}
#endif

	प्रत्यागम SYNCHRONOUS_MATCH_FOUND;
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
स्थिर पूर्णाङ्क mbox_message_put(संरचना k_mbox *mbox, संरचना k_mbox_संदेश *tx_संदेश,
			     k_timeout_t timeout)
{
	संरचना k_thread *sending_thread;
	k_spinlock_कुंजी_t कुंजी;

	/* save sender id so it can be used during message matching */
	tx_संदेश->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_संदेश->_syncing_thread;
	sending_thread->base.swap_data = tx_संदेश;

	/* search mailbox's rx queue for a compatible receiver */
	कुंजी = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, message_put, mbox, timeout);

	संरचना mbox_walk_data walk_data;
	पूर्णाङ्क status;

	walk_data.thread = sending_thread;
	walk_data.संदेश = tx_संदेश;

	/* Search the wait queue for the first matching receiver */

	status = z_sched_waitq_walk(&mbox->rx_संदेश_queue,
				    mbox_message_put_walk_op, रिक्त,
				    &walk_data);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#यदि (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	यदि (status == ASYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->lock, कुंजी);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, 0);
		प्रत्यागम 0;
	}
#endif

	यदि (status == SYNCHRONOUS_MATCH_FOUND) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox,
						   timeout);

		पूर्णाङ्क ret = z_pend_curr(&mbox->lock, कुंजी, रिक्त, K_FOREVER);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox,
					       timeout, ret);

		प्रत्यागम ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->lock, कुंजी);
		प्रत्यागम -ENOMSG;
	}

#यदि (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	यदि (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_संदेश_queue, K_FOREVER);
		k_spin_unlock(&mbox->lock, कुंजी);
		प्रत्यागम 0;
	}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, message_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	पूर्णाङ्क ret = z_pend_curr(&mbox->lock, कुंजी, &mbox->tx_संदेश_queue, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, message_put, mbox, timeout, ret);

	प्रत्यागम ret;
}

पूर्णाङ्क k_mbox_put(संरचना k_mbox *mbox, संरचना k_mbox_संदेश *tx_संदेश,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_संदेश->_syncing_thread = _current;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	पूर्णाङ्क ret = mbox_message_put(mbox, tx_संदेश, timeout);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	प्रत्यागम ret;
}

#यदि (CONFIG_अंक_MBOX_ASYNC_MSGS > 0)
शून्य k_mbox_async_put(संरचना k_mbox *mbox, संरचना k_mbox_संदेश *tx_संदेश,
		      संरचना k_sem *sem)
{
	संरचना k_mbox_async *async;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->base.prio;

	async->tx_संदेश = *tx_संदेश;
	async->tx_संदेश._syncing_thread = (संरचना k_thread *)&async->thread;
	async->tx_संदेश._async_sem = sem;

	(शून्य)mbox_message_put(mbox, &async->tx_संदेश, K_FOREVER);
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#endif /* CONFIG_NUM_MBOX_ASYNC_MSGS */

शून्य k_mbox_data_get(संरचना k_mbox_संदेश *rx_संदेश, शून्य *buffer)
{
	/* handle case where data is to be discarded */
	यदि (buffer == रिक्त) {
		rx_संदेश->size = 0;
		mbox_message_dispose(rx_संदेश);
		प्रत्यागम;
	}

	/* copy message data to buffer, then dispose of message */
	यदि ((rx_संदेश->tx_data != रिक्त) && (rx_संदेश->size > 0U)) {
		(शून्य)स_नकल(buffer, rx_संदेश->tx_data, rx_संदेश->size);
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
स्थिर पूर्णाङ्क mbox_message_data_check(संरचना k_mbox_संदेश *rx_संदेश, शून्य *buffer)
{
	यदि (buffer != रिक्त) {
		/* retrieve data now, then dispose of message */
		k_mbox_data_get(rx_संदेश, buffer);
	} अन्यथा यदि (rx_संदेश->size == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_message_dispose(rx_संदेश);
	} अन्यथा {
		/* keep message around for later data retrieval */
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क mbox_get_walk_op(संरचना k_thread *thread, शून्य *data)
{
	संरचना mbox_walk_data *walk_data = (संरचना mbox_walk_data *)data;
	संरचना k_mbox_संदेश *tx_संदेश;

	tx_संदेश = (संरचना k_mbox_संदेश *)thread->base.swap_data;

	यदि (mbox_message_match(tx_संदेश, walk_data->संदेश) != 0) {
		/* No match. Proceed to next in wait queue */
		प्रत्यागम 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(शून्य)z_try_abort_thread_timeout(thread);

	प्रत्यागम 1;
}

पूर्णाङ्क k_mbox_get(संरचना k_mbox *mbox, संरचना k_mbox_संदेश *rx_संदेश, शून्य *buffer,
	       k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णाङ्क result;

	/* save receiver id so it can be used during message matching */
	rx_संदेश->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	कुंजी = k_spin_lock(&mbox->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	संरचना mbox_walk_data walk_data;
	पूर्णाङ्क status;

	walk_data.thread = रिक्त;   /* Unused by walk operation */
	walk_data.संदेश = rx_संदेश;

	/* Search the wait queue for the first matching sender */

	status = z_sched_waitq_walk(&mbox->tx_संदेश_queue,
				    mbox_get_walk_op, रिक्त, &walk_data);

	यदि (status != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->lock, कुंजी);
		result = 0;

		गच्छ out;
	}

	/* didn't find a matching sender */

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->lock, कुंजी);

		result = -ENOMSG;
		गच्छ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->base.swap_data = rx_संदेश;
	result = z_pend_curr(&mbox->lock, कुंजी, &mbox->rx_संदेश_queue, timeout);

out:
	/* consume message data immediately, if needed */
	यदि (result == 0) {
		result = mbox_message_data_check(rx_संदेश, buffer);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, result);

	प्रत्यागम result;
}
