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

#تعریف_کړه SYNCHRONOUS_سمون_وموندل  1
#تعریف_کړه ASYNCHRONOUS_سمون_وموندل 2

بنية mbox_walk_معلومات {
	بنية k_thread *thread;
	بنية k_mbox_پیغام *پیغام;
};

#که_تعریف_شوی CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_MAILBOX */

#إذا (CONFIG_عدد_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
بنية k_mbox_async {
	بنية thread_بنسټ thread;		/* dummy thread object */
	بنية k_mbox_پیغام tx_پیغام;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ډېری_DEFINE(async_پیغام_free, CONFIG_عدد_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_alloc(بنية k_mbox_async **async)
{
	(فراغ)k_ډېری_راوباسه(&async_پیغام_free, (ډېری_معلومات_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_free(بنية k_mbox_async *async)
{
	k_ډېری_ټېل(&async_پیغام_free, (ډېری_معلومات_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ساكن فراغ init_mbox_module(فراغ)
{
	/* array of asynchronous message descriptors */
	ساكن بنية k_mbox_async __noinit async_پیغام[CONFIG_عدد_MBOX_ASYNC_MSGS];

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

	لكل (i = 0; i < CONFIG_عدد_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_بنسټ(&async_پیغام[i].thread, 0, _THREAD_DUMMY, 0);
		k_ډېری_ټېل(&async_پیغام_free, (ډېری_معلومات_t)&async_پیغام[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#د_که_پای /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

فراغ k_mbox_init(بنية k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_پیغام_کتار);
	z_waitq_init(&mbox->rx_پیغام_کتار);
	mbox->قلف = (بنية k_spinlock) {};

#که_تعریف_شوی CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#د_که_پای /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_پورټ_TRACING_OBJ_INIT(k_mbox, mbox);
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
ساكن صحيح mbox_پیغام_سمون(بنية k_mbox_پیغام *tx_پیغام,
			       بنية k_mbox_پیغام *rx_پیغام)
{
	uint32_t لنډمهاله_info;

	إذا (((tx_پیغام->tx_موخه_thread == (k_tid_t)K_ANY) ||
	     (tx_پیغام->tx_موخه_thread == rx_پیغام->tx_موخه_thread)) &&
	    ((rx_پیغام->rx_سرچینه_thread == (k_tid_t)K_ANY) ||
	     (rx_پیغام->rx_سرچینه_thread == tx_پیغام->rx_سرچینه_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_پیغام->rx_سرچینه_thread = tx_پیغام->rx_سرچینه_thread;
		tx_پیغام->tx_موخه_thread = rx_پیغام->tx_موخه_thread;

		/* update application info fields for both descriptors */
		لنډمهاله_info = rx_پیغام->info;
		rx_پیغام->info = tx_پیغام->info;
		tx_پیغام->info = لنډمهاله_info;

		/* update data size field for receiver only */
		إذا (rx_پیغام->اندازه > tx_پیغام->اندازه) {
			rx_پیغام->اندازه = tx_پیغام->اندازه;
		}

		/* update data location fields for receiver only */
		rx_پیغام->tx_معلومات = tx_پیغام->tx_معلومات;

		/* update syncing thread field for receiver only */
		rx_پیغام->_syncing_thread = tx_پیغام->_syncing_thread;

		أرجع 0;
	}

	أرجع -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
ساكن فراغ mbox_پیغام_dispose(بنية k_mbox_پیغام *rx_پیغام)
{
	بنية k_thread *sending_thread;
	بنية k_mbox_پیغام *tx_پیغام;

	/* do nothing if message was disposed of when it was received */
	إذا (rx_پیغام->_syncing_thread == عدم) {
		أرجع;
	}

	/* recover sender info */
	sending_thread = rx_پیغام->_syncing_thread;
	rx_پیغام->_syncing_thread = عدم;
	tx_پیغام = (بنية k_mbox_پیغام *)sending_thread->بنسټ.بدلون_معلومات;

	/* update data size field for sender */
	tx_پیغام->اندازه = rx_پیغام->اندازه;

#إذا (CONFIG_عدد_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	إذا (is_thread_dummy(sending_thread)) {
		بنية k_sem *async_sem = tx_پیغام->_async_sem;

		mbox_async_free((بنية k_mbox_async *)sending_thread);
		إذا (async_sem != عدم) {
			k_sem_give(async_sem);
		}
		أرجع;
	}
#د_که_پای /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_کلی_t کلی = k_spin_قلف(&_sched_spinlock);

	arch_thread_return_ارزښت_سېټ(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, کلی);
}

ساكن صحيح mbox_پیغام_put_walk_op(بنية k_thread *thread, فراغ *معلومات)
{
	بنية mbox_walk_معلومات *walk_معلومات = (بنية mbox_walk_معلومات *)معلومات;
	بنية k_mbox_پیغام *rx_پیغام;

	rx_پیغام = (بنية k_mbox_پیغام *)thread->بنسټ.بدلون_معلومات;

	إذا (mbox_پیغام_سمون(walk_معلومات->پیغام, rx_پیغام) != 0) {
		/* No match. Proceed to next in wait queue */
		أرجع 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(فراغ)z_try_abort_thread_timeout(thread);
	arch_thread_return_ارزښت_سېټ(thread, 0);
	z_sched_ready_locked(thread);

#إذا (CONFIG_عدد_MBOX_ASYNC_MSGS > 0)
	إذا (is_thread_dummy(walk_معلومات->thread)) {
		أرجع ASYNCHRONOUS_سمون_وموندل;
	}
#د_که_پای

	أرجع SYNCHRONOUS_سمون_وموندل;
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
ساكن صحيح mbox_پیغام_put(بنية k_mbox *mbox, بنية k_mbox_پیغام *tx_پیغام,
			     k_timeout_t timeout)
{
	بنية k_thread *sending_thread;
	k_spinlock_کلی_t کلی;

	/* save sender id so it can be used during message matching */
	tx_پیغام->rx_سرچینه_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_پیغام->_syncing_thread;
	sending_thread->بنسټ.بدلون_معلومات = tx_پیغام;

	/* search mailbox's rx queue for a compatible receiver */
	کلی = k_spin_قلف(&mbox->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mbox, پیغام_put, mbox, timeout);

	بنية mbox_walk_معلومات walk_معلومات;
	صحيح حالت;

	walk_معلومات.thread = sending_thread;
	walk_معلومات.پیغام = tx_پیغام;

	/* Search the wait queue for the first matching receiver */

	حالت = z_sched_waitq_walk(&mbox->rx_پیغام_کتار,
				    mbox_پیغام_put_walk_op, عدم,
				    &walk_معلومات);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#إذا (CONFIG_عدد_MBOX_ASYNC_MSGS > 0)
	إذا (حالت == ASYNCHRONOUS_سمون_وموندل) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->قلف, کلی);
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mbox, پیغام_put, mbox,
					       timeout, 0);
		أرجع 0;
	}
#د_که_پای

	إذا (حالت == SYNCHRONOUS_سمون_وموندل) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, پیغام_put, mbox,
						   timeout);

		صحيح ret = z_pend_curr(&mbox->قلف, کلی, عدم, K_FOREVER);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mbox, پیغام_put, mbox,
					       timeout, ret);

		أرجع ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mbox, پیغام_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->قلف, کلی);
		أرجع -ENOMSG;
	}

#إذا (CONFIG_عدد_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	إذا (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_پیغام_کتار, K_FOREVER);
		k_spin_unlock(&mbox->قلف, کلی);
		أرجع 0;
	}
#د_که_پای /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, پیغام_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	صحيح ret = z_pend_curr(&mbox->قلف, کلی, &mbox->tx_پیغام_کتار, timeout);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mbox, پیغام_put, mbox, timeout, ret);

	أرجع ret;
}

صحيح k_mbox_put(بنية k_mbox *mbox, بنية k_mbox_پیغام *tx_پیغام,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_پیغام->_syncing_thread = _current;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	صحيح ret = mbox_پیغام_put(mbox, tx_پیغام, timeout);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	أرجع ret;
}

#إذا (CONFIG_عدد_MBOX_ASYNC_MSGS > 0)
فراغ k_mbox_async_put(بنية k_mbox *mbox, بنية k_mbox_پیغام *tx_پیغام,
		      بنية k_sem *sem)
{
	بنية k_mbox_async *async;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->بنسټ.prio;

	async->tx_پیغام = *tx_پیغام;
	async->tx_پیغام._syncing_thread = (بنية k_thread *)&async->thread;
	async->tx_پیغام._async_sem = sem;

	(فراغ)mbox_پیغام_put(mbox, &async->tx_پیغام, K_FOREVER);
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#د_که_پای /* CONFIG_NUM_MBOX_ASYNC_MSGS */

فراغ k_mbox_معلومات_get(بنية k_mbox_پیغام *rx_پیغام, فراغ *بفر)
{
	/* handle case where data is to be discarded */
	إذا (بفر == عدم) {
		rx_پیغام->اندازه = 0;
		mbox_پیغام_dispose(rx_پیغام);
		أرجع;
	}

	/* copy message data to buffer, then dispose of message */
	إذا ((rx_پیغام->tx_معلومات != عدم) && (rx_پیغام->اندازه > 0U)) {
		(فراغ)حافظه_نقل(بفر, rx_پیغام->tx_معلومات, rx_پیغام->اندازه);
	}
	mbox_پیغام_dispose(rx_پیغام);
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
ساكن صحيح mbox_پیغام_معلومات_check(بنية k_mbox_پیغام *rx_پیغام, فراغ *بفر)
{
	إذا (بفر != عدم) {
		/* retrieve data now, then dispose of message */
		k_mbox_معلومات_get(rx_پیغام, بفر);
	} وإلا إذا (rx_پیغام->اندازه == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_پیغام_dispose(rx_پیغام);
	} وإلا {
		/* keep message around for later data retrieval */
	}

	أرجع 0;
}

ساكن صحيح mbox_get_walk_op(بنية k_thread *thread, فراغ *معلومات)
{
	بنية mbox_walk_معلومات *walk_معلومات = (بنية mbox_walk_معلومات *)معلومات;
	بنية k_mbox_پیغام *tx_پیغام;

	tx_پیغام = (بنية k_mbox_پیغام *)thread->بنسټ.بدلون_معلومات;

	إذا (mbox_پیغام_سمون(tx_پیغام, walk_معلومات->پیغام) != 0) {
		/* No match. Proceed to next in wait queue */
		أرجع 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(فراغ)z_try_abort_thread_timeout(thread);

	أرجع 1;
}

صحيح k_mbox_get(بنية k_mbox *mbox, بنية k_mbox_پیغام *rx_پیغام, فراغ *بفر,
	       k_timeout_t timeout)
{
	k_spinlock_کلی_t کلی;
	صحيح پایله;

	/* save receiver id so it can be used during message matching */
	rx_پیغام->tx_موخه_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	کلی = k_spin_قلف(&mbox->قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	بنية mbox_walk_معلومات walk_معلومات;
	صحيح حالت;

	walk_معلومات.thread = عدم;   /* Unused by walk operation */
	walk_معلومات.پیغام = rx_پیغام;

	/* Search the wait queue for the first matching sender */

	حالت = z_sched_waitq_walk(&mbox->tx_پیغام_کتار,
				    mbox_get_walk_op, عدم, &walk_معلومات);

	إذا (حالت != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->قلف, کلی);
		پایله = 0;

		اذهب بهر;
	}

	/* didn't find a matching sender */

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->قلف, کلی);

		پایله = -ENOMSG;
		اذهب بهر;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->بنسټ.بدلون_معلومات = rx_پیغام;
	پایله = z_pend_curr(&mbox->قلف, کلی, &mbox->rx_پیغام_کتار, timeout);

بهر:
	/* consume message data immediately, if needed */
	إذا (پایله == 0) {
		پایله = mbox_پیغام_معلومات_check(rx_پیغام, بفر);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, پایله);

	أرجع پایله;
}
