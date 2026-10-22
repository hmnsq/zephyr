/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/linker/sections.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
/* private kernel APIs */
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <wait_q.h>

#నిర్వచించు SYNCHRONOUS_సరిపోలిక_దొరికింది  1
#నిర్వచించు ASYNCHRONOUS_సరిపోలిక_దొరికింది 2

నిర్మాణం mbox_walk_దత్తాంశం {
	నిర్మాణం k_thread *thread;
	నిర్మాణం k_mbox_సందేశం *సందేశం;
};

#నిర్వచితమైతే CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MAILBOX */

#ఒకవేళ (CONFIG_అంకె_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
నిర్మాణం k_mbox_async {
	నిర్మాణం thread_పునాది thread;		/* dummy thread object */
	నిర్మాణం k_mbox_సందేశం tx_సందేశం;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_దొంతర_DEFINE(async_సందేశం_free, CONFIG_అంకె_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
స్థిర అంతఃవరుస శూన్య mbox_async_alloc(నిర్మాణం k_mbox_async **async)
{
	(శూన్య)k_దొంతర_తీయి(&async_సందేశం_free, (దొంతర_దత్తాంశం_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
స్థిర అంతఃవరుస శూన్య mbox_async_free(నిర్మాణం k_mbox_async *async)
{
	k_దొంతర_నెట్టు(&async_సందేశం_free, (దొంతర_దత్తాంశం_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
స్థిర శూన్య init_mbox_module(శూన్య)
{
	/* array of asynchronous message descriptors */
	స్థిర నిర్మాణం k_mbox_async __noinit async_సందేశం[CONFIG_అంకె_MBOX_ASYNC_MSGS];

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

	పూర్ణాంకం i;

	కొరకు (i = 0; i < CONFIG_అంకె_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_పునాది(&async_సందేశం[i].thread, 0, _THREAD_DUMMY, 0);
		k_దొంతర_నెట్టు(&async_సందేశం_free, (దొంతర_దత్తాంశం_t)&async_సందేశం[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#అయితే_ముగింపు /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

శూన్య k_mbox_init(నిర్మాణం k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_సందేశం_వరుస);
	z_waitq_init(&mbox->rx_సందేశం_వరుస);
	mbox->తాళం = (నిర్మాణం k_spinlock) {};

#నిర్వచితమైతే CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_పోర్టు_TRACING_OBJ_INIT(k_mbox, mbox);
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
స్థిర పూర్ణాంకం mbox_సందేశం_సరిపోలిక(నిర్మాణం k_mbox_సందేశం *tx_సందేశం,
			       నిర్మాణం k_mbox_సందేశం *rx_సందేశం)
{
	uint32_t తాత్కాలిక_info;

	ఒకవేళ (((tx_సందేశం->tx_లక్ష్యం_thread == (k_tid_t)K_ANY) ||
	     (tx_సందేశం->tx_లక్ష్యం_thread == rx_సందేశం->tx_లక్ష్యం_thread)) &&
	    ((rx_సందేశం->rx_ఆధారం_thread == (k_tid_t)K_ANY) ||
	     (rx_సందేశం->rx_ఆధారం_thread == tx_సందేశం->rx_ఆధారం_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_సందేశం->rx_ఆధారం_thread = tx_సందేశం->rx_ఆధారం_thread;
		tx_సందేశం->tx_లక్ష్యం_thread = rx_సందేశం->tx_లక్ష్యం_thread;

		/* update application info fields for both descriptors */
		తాత్కాలిక_info = rx_సందేశం->info;
		rx_సందేశం->info = tx_సందేశం->info;
		tx_సందేశం->info = తాత్కాలిక_info;

		/* update data size field for receiver only */
		ఒకవేళ (rx_సందేశం->కొలత > tx_సందేశం->కొలత) {
			rx_సందేశం->కొలత = tx_సందేశం->కొలత;
		}

		/* update data location fields for receiver only */
		rx_సందేశం->tx_దత్తాంశం = tx_సందేశం->tx_దత్తాంశం;

		/* update syncing thread field for receiver only */
		rx_సందేశం->_syncing_thread = tx_సందేశం->_syncing_thread;

		తిరిగి 0;
	}

	తిరిగి -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
స్థిర శూన్య mbox_సందేశం_dispose(నిర్మాణం k_mbox_సందేశం *rx_సందేశం)
{
	నిర్మాణం k_thread *sending_thread;
	నిర్మాణం k_mbox_సందేశం *tx_సందేశం;

	/* do nothing if message was disposed of when it was received */
	ఒకవేళ (rx_సందేశం->_syncing_thread == ఖాళీ) {
		తిరిగి;
	}

	/* recover sender info */
	sending_thread = rx_సందేశం->_syncing_thread;
	rx_సందేశం->_syncing_thread = ఖాళీ;
	tx_సందేశం = (నిర్మాణం k_mbox_సందేశం *)sending_thread->పునాది.మార్పిడి_దత్తాంశం;

	/* update data size field for sender */
	tx_సందేశం->కొలత = rx_సందేశం->కొలత;

#ఒకవేళ (CONFIG_అంకె_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	ఒకవేళ (is_thread_dummy(sending_thread)) {
		నిర్మాణం k_sem *async_sem = tx_సందేశం->_async_sem;

		mbox_async_free((నిర్మాణం k_mbox_async *)sending_thread);
		ఒకవేళ (async_sem != ఖాళీ) {
			k_sem_give(async_sem);
		}
		తిరిగి;
	}
#అయితే_ముగింపు /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&_sched_spinlock);

	arch_thread_return_విలువ_సమితి(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, కీలకం);
}

స్థిర పూర్ణాంకం mbox_సందేశం_put_walk_op(నిర్మాణం k_thread *thread, శూన్య *దత్తాంశం)
{
	నిర్మాణం mbox_walk_దత్తాంశం *walk_దత్తాంశం = (నిర్మాణం mbox_walk_దత్తాంశం *)దత్తాంశం;
	నిర్మాణం k_mbox_సందేశం *rx_సందేశం;

	rx_సందేశం = (నిర్మాణం k_mbox_సందేశం *)thread->పునాది.మార్పిడి_దత్తాంశం;

	ఒకవేళ (mbox_సందేశం_సరిపోలిక(walk_దత్తాంశం->సందేశం, rx_సందేశం) != 0) {
		/* No match. Proceed to next in wait queue */
		తిరిగి 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(శూన్య)z_try_abort_thread_timeout(thread);
	arch_thread_return_విలువ_సమితి(thread, 0);
	z_sched_ready_locked(thread);

#ఒకవేళ (CONFIG_అంకె_MBOX_ASYNC_MSGS > 0)
	ఒకవేళ (is_thread_dummy(walk_దత్తాంశం->thread)) {
		తిరిగి ASYNCHRONOUS_సరిపోలిక_దొరికింది;
	}
#అయితే_ముగింపు

	తిరిగి SYNCHRONOUS_సరిపోలిక_దొరికింది;
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
స్థిర పూర్ణాంకం mbox_సందేశం_put(నిర్మాణం k_mbox *mbox, నిర్మాణం k_mbox_సందేశం *tx_సందేశం,
			     k_timeout_t timeout)
{
	నిర్మాణం k_thread *sending_thread;
	k_spinlock_కీలకం_t కీలకం;

	/* save sender id so it can be used during message matching */
	tx_సందేశం->rx_ఆధారం_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_సందేశం->_syncing_thread;
	sending_thread->పునాది.మార్పిడి_దత్తాంశం = tx_సందేశం;

	/* search mailbox's rx queue for a compatible receiver */
	కీలకం = k_spin_తాళం(&mbox->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mbox, సందేశం_put, mbox, timeout);

	నిర్మాణం mbox_walk_దత్తాంశం walk_దత్తాంశం;
	పూర్ణాంకం స్థితి_2;

	walk_దత్తాంశం.thread = sending_thread;
	walk_దత్తాంశం.సందేశం = tx_సందేశం;

	/* Search the wait queue for the first matching receiver */

	స్థితి_2 = z_sched_waitq_walk(&mbox->rx_సందేశం_వరుస,
				    mbox_సందేశం_put_walk_op, ఖాళీ,
				    &walk_దత్తాంశం);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#ఒకవేళ (CONFIG_అంకె_MBOX_ASYNC_MSGS > 0)
	ఒకవేళ (స్థితి_2 == ASYNCHRONOUS_సరిపోలిక_దొరికింది) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->తాళం, కీలకం);
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mbox, సందేశం_put, mbox,
					       timeout, 0);
		తిరిగి 0;
	}
#అయితే_ముగింపు

	ఒకవేళ (స్థితి_2 == SYNCHRONOUS_సరిపోలిక_దొరికింది) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_mbox, సందేశం_put, mbox,
						   timeout);

		పూర్ణాంకం ret = z_pend_curr(&mbox->తాళం, కీలకం, ఖాళీ, K_FOREVER);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mbox, సందేశం_put, mbox,
					       timeout, ret);

		తిరిగి ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mbox, సందేశం_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->తాళం, కీలకం);
		తిరిగి -ENOMSG;
	}

#ఒకవేళ (CONFIG_అంకె_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	ఒకవేళ (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_సందేశం_వరుస, K_FOREVER);
		k_spin_unlock(&mbox->తాళం, కీలకం);
		తిరిగి 0;
	}
#అయితే_ముగింపు /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_mbox, సందేశం_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	పూర్ణాంకం ret = z_pend_curr(&mbox->తాళం, కీలకం, &mbox->tx_సందేశం_వరుస, timeout);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mbox, సందేశం_put, mbox, timeout, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_mbox_put(నిర్మాణం k_mbox *mbox, నిర్మాణం k_mbox_సందేశం *tx_సందేశం,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_సందేశం->_syncing_thread = _current;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	పూర్ణాంకం ret = mbox_సందేశం_put(mbox, tx_సందేశం, timeout);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	తిరిగి ret;
}

#ఒకవేళ (CONFIG_అంకె_MBOX_ASYNC_MSGS > 0)
శూన్య k_mbox_async_put(నిర్మాణం k_mbox *mbox, నిర్మాణం k_mbox_సందేశం *tx_సందేశం,
		      నిర్మాణం k_sem *sem)
{
	నిర్మాణం k_mbox_async *async;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->పునాది.prio;

	async->tx_సందేశం = *tx_సందేశం;
	async->tx_సందేశం._syncing_thread = (నిర్మాణం k_thread *)&async->thread;
	async->tx_సందేశం._async_sem = sem;

	(శూన్య)mbox_సందేశం_put(mbox, &async->tx_సందేశం, K_FOREVER);
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#అయితే_ముగింపు /* CONFIG_NUM_MBOX_ASYNC_MSGS */

శూన్య k_mbox_దత్తాంశం_get(నిర్మాణం k_mbox_సందేశం *rx_సందేశం, శూన్య *బఫరు)
{
	/* handle case where data is to be discarded */
	ఒకవేళ (బఫరు == ఖాళీ) {
		rx_సందేశం->కొలత = 0;
		mbox_సందేశం_dispose(rx_సందేశం);
		తిరిగి;
	}

	/* copy message data to buffer, then dispose of message */
	ఒకవేళ ((rx_సందేశం->tx_దత్తాంశం != ఖాళీ) && (rx_సందేశం->కొలత > 0U)) {
		(శూన్య)జ్ఞాపకం_నకలు(బఫరు, rx_సందేశం->tx_దత్తాంశం, rx_సందేశం->కొలత);
	}
	mbox_సందేశం_dispose(rx_సందేశం);
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
స్థిర పూర్ణాంకం mbox_సందేశం_దత్తాంశం_check(నిర్మాణం k_mbox_సందేశం *rx_సందేశం, శూన్య *బఫరు)
{
	ఒకవేళ (బఫరు != ఖాళీ) {
		/* retrieve data now, then dispose of message */
		k_mbox_దత్తాంశం_get(rx_సందేశం, బఫరు);
	} లేకపోతే ఒకవేళ (rx_సందేశం->కొలత == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_సందేశం_dispose(rx_సందేశం);
	} లేకపోతే {
		/* keep message around for later data retrieval */
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం mbox_get_walk_op(నిర్మాణం k_thread *thread, శూన్య *దత్తాంశం)
{
	నిర్మాణం mbox_walk_దత్తాంశం *walk_దత్తాంశం = (నిర్మాణం mbox_walk_దత్తాంశం *)దత్తాంశం;
	నిర్మాణం k_mbox_సందేశం *tx_సందేశం;

	tx_సందేశం = (నిర్మాణం k_mbox_సందేశం *)thread->పునాది.మార్పిడి_దత్తాంశం;

	ఒకవేళ (mbox_సందేశం_సరిపోలిక(tx_సందేశం, walk_దత్తాంశం->సందేశం) != 0) {
		/* No match. Proceed to next in wait queue */
		తిరిగి 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(శూన్య)z_try_abort_thread_timeout(thread);

	తిరిగి 1;
}

పూర్ణాంకం k_mbox_get(నిర్మాణం k_mbox *mbox, నిర్మాణం k_mbox_సందేశం *rx_సందేశం, శూన్య *బఫరు,
	       k_timeout_t timeout)
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ఫలితం;

	/* save receiver id so it can be used during message matching */
	rx_సందేశం->tx_లక్ష్యం_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	కీలకం = k_spin_తాళం(&mbox->తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	నిర్మాణం mbox_walk_దత్తాంశం walk_దత్తాంశం;
	పూర్ణాంకం స్థితి_2;

	walk_దత్తాంశం.thread = ఖాళీ;   /* Unused by walk operation */
	walk_దత్తాంశం.సందేశం = rx_సందేశం;

	/* Search the wait queue for the first matching sender */

	స్థితి_2 = z_sched_waitq_walk(&mbox->tx_సందేశం_వరుస,
				    mbox_get_walk_op, ఖాళీ, &walk_దత్తాంశం);

	ఒకవేళ (స్థితి_2 != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->తాళం, కీలకం);
		ఫలితం = 0;

		వెళ్ళు బహిర్గతం;
	}

	/* didn't find a matching sender */

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->తాళం, కీలకం);

		ఫలితం = -ENOMSG;
		వెళ్ళు బహిర్గతం;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->పునాది.మార్పిడి_దత్తాంశం = rx_సందేశం;
	ఫలితం = z_pend_curr(&mbox->తాళం, కీలకం, &mbox->rx_సందేశం_వరుస, timeout);

బహిర్గతం:
	/* consume message data immediately, if needed */
	ఒకవేళ (ఫలితం == 0) {
		ఫలితం = mbox_సందేశం_దత్తాంశం_check(rx_సందేశం, బఫరు);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, ఫలితం);

	తిరిగి ఫలితం;
}
