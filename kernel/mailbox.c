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

#परिभाषय SYNCHRONOUS_मेलनम्_प्राप्तम्  1
#परिभाषय ASYNCHRONOUS_मेलनम्_प्राप्तम् 2

संरचना mbox_walk_दत्त {
	संरचना k_thread *thread;
	संरचना k_mbox_सन्देशः *सन्देशः;
};

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_MAILBOX */

#यदि (CONFIG_अङ्कः_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
संरचना k_mbox_async {
	संरचना thread_आधारः thread;		/* dummy thread object */
	संरचना k_mbox_सन्देशः tx_सन्देशः;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_राशिः_DEFINE(async_सन्देशः_free, CONFIG_अङ्कः_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
स्थिर अन्तःस्थ शून्य mbox_async_alloc(संरचना k_mbox_async **async)
{
	(शून्य)k_राशिः_निष्कासय(&async_सन्देशः_free, (राशिः_दत्त_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
स्थिर अन्तःस्थ शून्य mbox_async_free(संरचना k_mbox_async *async)
{
	k_राशिः_प्रक्षिप(&async_सन्देशः_free, (राशिः_दत्त_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
स्थिर शून्य init_mbox_module(शून्य)
{
	/* array of asynchronous message descriptors */
	स्थिर संरचना k_mbox_async __noinit async_सन्देशः[CONFIG_अङ्कः_MBOX_ASYNC_MSGS];

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

	प्रति (i = 0; i < CONFIG_अङ्कः_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_आधारः(&async_सन्देशः[i].thread, 0, _THREAD_DUMMY, 0);
		k_राशिः_प्रक्षिप(&async_सन्देशः_free, (राशिः_दत्त_t)&async_सन्देशः[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#यदि_अन्तः /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

शून्य k_mbox_init(संरचना k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_सन्देशः_queue);
	z_waitq_init(&mbox->rx_सन्देशः_queue);
	mbox->तालकम् = (संरचना k_spinlock) {};

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#यदि_अन्तः /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_पत्तनम्_TRACING_OBJ_INIT(k_mbox, mbox);
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
स्थिर पूर्णाङ्क mbox_सन्देशः_मेलनम्(संरचना k_mbox_सन्देशः *tx_सन्देशः,
			       संरचना k_mbox_सन्देशः *rx_सन्देशः)
{
	uint32_t अस्थायी_info;

	यदि (((tx_सन्देशः->tx_target_thread == (k_tid_t)K_ANY) ||
	     (tx_सन्देशः->tx_target_thread == rx_सन्देशः->tx_target_thread)) &&
	    ((rx_सन्देशः->rx_source_thread == (k_tid_t)K_ANY) ||
	     (rx_सन्देशः->rx_source_thread == tx_सन्देशः->rx_source_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_सन्देशः->rx_source_thread = tx_सन्देशः->rx_source_thread;
		tx_सन्देशः->tx_target_thread = rx_सन्देशः->tx_target_thread;

		/* update application info fields for both descriptors */
		अस्थायी_info = rx_सन्देशः->info;
		rx_सन्देशः->info = tx_सन्देशः->info;
		tx_सन्देशः->info = अस्थायी_info;

		/* update data size field for receiver only */
		यदि (rx_सन्देशः->आकार > tx_सन्देशः->आकार) {
			rx_सन्देशः->आकार = tx_सन्देशः->आकार;
		}

		/* update data location fields for receiver only */
		rx_सन्देशः->tx_दत्त = tx_सन्देशः->tx_दत्त;

		/* update syncing thread field for receiver only */
		rx_सन्देशः->_syncing_thread = tx_सन्देशः->_syncing_thread;

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
स्थिर शून्य mbox_सन्देशः_dispose(संरचना k_mbox_सन्देशः *rx_सन्देशः)
{
	संरचना k_thread *sending_thread;
	संरचना k_mbox_सन्देशः *tx_सन्देशः;

	/* do nothing if message was disposed of when it was received */
	यदि (rx_सन्देशः->_syncing_thread == रिक्त) {
		प्रत्यागम;
	}

	/* recover sender info */
	sending_thread = rx_सन्देशः->_syncing_thread;
	rx_सन्देशः->_syncing_thread = रिक्त;
	tx_सन्देशः = (संरचना k_mbox_सन्देशः *)sending_thread->आधारः.swap_दत्त;

	/* update data size field for sender */
	tx_सन्देशः->आकार = rx_सन्देशः->आकार;

#यदि (CONFIG_अङ्कः_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	यदि (is_thread_dummy(sending_thread)) {
		संरचना k_sem *async_sem = tx_सन्देशः->_async_sem;

		mbox_async_free((संरचना k_mbox_async *)sending_thread);
		यदि (async_sem != रिक्त) {
			k_sem_give(async_sem);
		}
		प्रत्यागम;
	}
#यदि_अन्तः /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&_sched_spinlock);

	arch_thread_return_मानम्_समुच्चयः(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, कुञ्चिका);
}

स्थिर पूर्णाङ्क mbox_सन्देशः_put_walk_op(संरचना k_thread *thread, शून्य *दत्त)
{
	संरचना mbox_walk_दत्त *walk_दत्त = (संरचना mbox_walk_दत्त *)दत्त;
	संरचना k_mbox_सन्देशः *rx_सन्देशः;

	rx_सन्देशः = (संरचना k_mbox_सन्देशः *)thread->आधारः.swap_दत्त;

	यदि (mbox_सन्देशः_मेलनम्(walk_दत्त->सन्देशः, rx_सन्देशः) != 0) {
		/* No match. Proceed to next in wait queue */
		प्रत्यागम 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(शून्य)z_try_abort_thread_timeout(thread);
	arch_thread_return_मानम्_समुच्चयः(thread, 0);
	z_sched_ready_locked(thread);

#यदि (CONFIG_अङ्कः_MBOX_ASYNC_MSGS > 0)
	यदि (is_thread_dummy(walk_दत्त->thread)) {
		प्रत्यागम ASYNCHRONOUS_मेलनम्_प्राप्तम्;
	}
#यदि_अन्तः

	प्रत्यागम SYNCHRONOUS_मेलनम्_प्राप्तम्;
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
स्थिर पूर्णाङ्क mbox_सन्देशः_put(संरचना k_mbox *mbox, संरचना k_mbox_सन्देशः *tx_सन्देशः,
			     k_timeout_t timeout)
{
	संरचना k_thread *sending_thread;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	/* save sender id so it can be used during message matching */
	tx_सन्देशः->rx_source_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_सन्देशः->_syncing_thread;
	sending_thread->आधारः.swap_दत्त = tx_सन्देशः;

	/* search mailbox's rx queue for a compatible receiver */
	कुञ्चिका = k_spin_तालकम्(&mbox->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mbox, सन्देशः_put, mbox, timeout);

	संरचना mbox_walk_दत्त walk_दत्त;
	पूर्णाङ्क स्थितिः;

	walk_दत्त.thread = sending_thread;
	walk_दत्त.सन्देशः = tx_सन्देशः;

	/* Search the wait queue for the first matching receiver */

	स्थितिः = z_sched_waitq_walk(&mbox->rx_सन्देशः_queue,
				    mbox_सन्देशः_put_walk_op, रिक्त,
				    &walk_दत्त);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#यदि (CONFIG_अङ्कः_MBOX_ASYNC_MSGS > 0)
	यदि (स्थितिः == ASYNCHRONOUS_मेलनम्_प्राप्तम्) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->तालकम्, कुञ्चिका);
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mbox, सन्देशः_put, mbox,
					       timeout, 0);
		प्रत्यागम 0;
	}
#यदि_अन्तः

	यदि (स्थितिः == SYNCHRONOUS_मेलनम्_प्राप्तम्) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_mbox, सन्देशः_put, mbox,
						   timeout);

		पूर्णाङ्क ret = z_pend_curr(&mbox->तालकम्, कुञ्चिका, रिक्त, K_FOREVER);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mbox, सन्देशः_put, mbox,
					       timeout, ret);

		प्रत्यागम ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mbox, सन्देशः_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->तालकम्, कुञ्चिका);
		प्रत्यागम -ENOMSG;
	}

#यदि (CONFIG_अङ्कः_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	यदि (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_सन्देशः_queue, K_FOREVER);
		k_spin_unlock(&mbox->तालकम्, कुञ्चिका);
		प्रत्यागम 0;
	}
#यदि_अन्तः /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_mbox, सन्देशः_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	पूर्णाङ्क ret = z_pend_curr(&mbox->तालकम्, कुञ्चिका, &mbox->tx_सन्देशः_queue, timeout);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mbox, सन्देशः_put, mbox, timeout, ret);

	प्रत्यागम ret;
}

पूर्णाङ्क k_mbox_put(संरचना k_mbox *mbox, संरचना k_mbox_सन्देशः *tx_सन्देशः,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_सन्देशः->_syncing_thread = _current;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	पूर्णाङ्क ret = mbox_सन्देशः_put(mbox, tx_सन्देशः, timeout);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	प्रत्यागम ret;
}

#यदि (CONFIG_अङ्कः_MBOX_ASYNC_MSGS > 0)
शून्य k_mbox_async_put(संरचना k_mbox *mbox, संरचना k_mbox_सन्देशः *tx_सन्देशः,
		      संरचना k_sem *sem)
{
	संरचना k_mbox_async *async;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->आधारः.prio;

	async->tx_सन्देशः = *tx_सन्देशः;
	async->tx_सन्देशः._syncing_thread = (संरचना k_thread *)&async->thread;
	async->tx_सन्देशः._async_sem = sem;

	(शून्य)mbox_सन्देशः_put(mbox, &async->tx_सन्देशः, K_FOREVER);
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#यदि_अन्तः /* CONFIG_NUM_MBOX_ASYNC_MSGS */

शून्य k_mbox_दत्त_get(संरचना k_mbox_सन्देशः *rx_सन्देशः, शून्य *बफर)
{
	/* handle case where data is to be discarded */
	यदि (बफर == रिक्त) {
		rx_सन्देशः->आकार = 0;
		mbox_सन्देशः_dispose(rx_सन्देशः);
		प्रत्यागम;
	}

	/* copy message data to buffer, then dispose of message */
	यदि ((rx_सन्देशः->tx_दत्त != रिक्त) && (rx_सन्देशः->आकार > 0U)) {
		(शून्य)स्मृतिं_प्रतिलिख(बफर, rx_सन्देशः->tx_दत्त, rx_सन्देशः->आकार);
	}
	mbox_सन्देशः_dispose(rx_सन्देशः);
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
स्थिर पूर्णाङ्क mbox_सन्देशः_दत्त_check(संरचना k_mbox_सन्देशः *rx_सन्देशः, शून्य *बफर)
{
	यदि (बफर != रिक्त) {
		/* retrieve data now, then dispose of message */
		k_mbox_दत्त_get(rx_सन्देशः, बफर);
	} अन्यथा यदि (rx_सन्देशः->आकार == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_सन्देशः_dispose(rx_सन्देशः);
	} अन्यथा {
		/* keep message around for later data retrieval */
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क mbox_get_walk_op(संरचना k_thread *thread, शून्य *दत्त)
{
	संरचना mbox_walk_दत्त *walk_दत्त = (संरचना mbox_walk_दत्त *)दत्त;
	संरचना k_mbox_सन्देशः *tx_सन्देशः;

	tx_सन्देशः = (संरचना k_mbox_सन्देशः *)thread->आधारः.swap_दत्त;

	यदि (mbox_सन्देशः_मेलनम्(tx_सन्देशः, walk_दत्त->सन्देशः) != 0) {
		/* No match. Proceed to next in wait queue */
		प्रत्यागम 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(शून्य)z_try_abort_thread_timeout(thread);

	प्रत्यागम 1;
}

पूर्णाङ्क k_mbox_get(संरचना k_mbox *mbox, संरचना k_mbox_सन्देशः *rx_सन्देशः, शून्य *बफर,
	       k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क परिणाम;

	/* save receiver id so it can be used during message matching */
	rx_सन्देशः->tx_target_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	कुञ्चिका = k_spin_तालकम्(&mbox->तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	संरचना mbox_walk_दत्त walk_दत्त;
	पूर्णाङ्क स्थितिः;

	walk_दत्त.thread = रिक्त;   /* Unused by walk operation */
	walk_दत्त.सन्देशः = rx_सन्देशः;

	/* Search the wait queue for the first matching sender */

	स्थितिः = z_sched_waitq_walk(&mbox->tx_सन्देशः_queue,
				    mbox_get_walk_op, रिक्त, &walk_दत्त);

	यदि (स्थितिः != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->तालकम्, कुञ्चिका);
		परिणाम = 0;

		गच्छ बहिर्गमनम्;
	}

	/* didn't find a matching sender */

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->तालकम्, कुञ्चिका);

		परिणाम = -ENOMSG;
		गच्छ बहिर्गमनम्;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->आधारः.swap_दत्त = rx_सन्देशः;
	परिणाम = z_pend_curr(&mbox->तालकम्, कुञ्चिका, &mbox->rx_सन्देशः_queue, timeout);

बहिर्गमनम्:
	/* consume message data immediately, if needed */
	यदि (परिणाम == 0) {
		परिणाम = mbox_सन्देशः_दत्त_check(rx_सन्देशः, बफर);
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, परिणाम);

	प्रत्यागम परिणाम;
}
