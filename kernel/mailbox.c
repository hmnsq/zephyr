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

#وصف_ڪريو SYNCHRONOUS_ميلاپ_مليو  1
#وصف_ڪريو ASYNCHRONOUS_ميلاپ_مليو 2

بنية mbox_walk_معلومات {
	بنية k_thread *thread;
	بنية k_mbox_نياپو *نياپو;
};

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_MAILBOX */

#إذا (CONFIG_انگ_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
بنية k_mbox_async {
	بنية thread_بنياد thread;		/* dummy thread object */
	بنية k_mbox_نياپو tx_نياپو;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ڍير_DEFINE(async_نياپو_free, CONFIG_انگ_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_alloc(بنية k_mbox_async **async)
{
	(فراغ)k_ڍير_ڪڍو(&async_نياپو_free, (ڍير_معلومات_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_free(بنية k_mbox_async *async)
{
	k_ڍير_ڌڪو(&async_نياپو_free, (ڍير_معلومات_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ساكن فراغ init_mbox_module(فراغ)
{
	/* array of asynchronous message descriptors */
	ساكن بنية k_mbox_async __noinit async_نياپو[CONFIG_انگ_MBOX_ASYNC_MSGS];

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

	لكل (i = 0; i < CONFIG_انگ_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_بنياد(&async_نياپو[i].thread, 0, _THREAD_DUMMY, 0);
		k_ڍير_ڌڪو(&async_نياپو_free, (ڍير_معلومات_t)&async_نياپو[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#جيڪڏهن_پورو /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

فراغ k_mbox_init(بنية k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_نياپو_قطار);
	z_waitq_init(&mbox->rx_نياپو_قطار);
	mbox->تالو = (بنية k_spinlock) {};

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_پورٽ_TRACING_OBJ_INIT(k_mbox, mbox);
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
ساكن صحيح mbox_پيغام_ميلاپ(بنية k_mbox_نياپو *tx_نياپو,
			       بنية k_mbox_نياپو *rx_نياپو)
{
	uint32_t عارضي_info;

	إذا (((tx_نياپو->tx_هدف_thread == (k_tid_t)K_ANY) ||
	     (tx_نياپو->tx_هدف_thread == rx_نياپو->tx_هدف_thread)) &&
	    ((rx_نياپو->rx_ذريعو_thread == (k_tid_t)K_ANY) ||
	     (rx_نياپو->rx_ذريعو_thread == tx_نياپو->rx_ذريعو_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_نياپو->rx_ذريعو_thread = tx_نياپو->rx_ذريعو_thread;
		tx_نياپو->tx_هدف_thread = rx_نياپو->tx_هدف_thread;

		/* update application info fields for both descriptors */
		عارضي_info = rx_نياپو->info;
		rx_نياپو->info = tx_نياپو->info;
		tx_نياپو->info = عارضي_info;

		/* update data size field for receiver only */
		إذا (rx_نياپو->سائيز > tx_نياپو->سائيز) {
			rx_نياپو->سائيز = tx_نياپو->سائيز;
		}

		/* update data location fields for receiver only */
		rx_نياپو->tx_معلومات = tx_نياپو->tx_معلومات;

		/* update syncing thread field for receiver only */
		rx_نياپو->_syncing_thread = tx_نياپو->_syncing_thread;

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
ساكن فراغ mbox_پيغام_dispose(بنية k_mbox_نياپو *rx_نياپو)
{
	بنية k_thread *sending_thread;
	بنية k_mbox_نياپو *tx_نياپو;

	/* do nothing if message was disposed of when it was received */
	إذا (rx_نياپو->_syncing_thread == عدم) {
		أرجع;
	}

	/* recover sender info */
	sending_thread = rx_نياپو->_syncing_thread;
	rx_نياپو->_syncing_thread = عدم;
	tx_نياپو = (بنية k_mbox_نياپو *)sending_thread->بنياد.مٽا_سٽا_معلومات;

	/* update data size field for sender */
	tx_نياپو->سائيز = rx_نياپو->سائيز;

#إذا (CONFIG_انگ_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	إذا (is_thread_dummy(sending_thread)) {
		بنية k_sem *async_sem = tx_نياپو->_async_sem;

		mbox_async_free((بنية k_mbox_async *)sending_thread);
		إذا (async_sem != عدم) {
			k_sem_give(async_sem);
		}
		أرجع;
	}
#جيڪڏهن_پورو /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&_sched_spinlock);

	arch_thread_return_قدر_سيٽ(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, ڪنجي);
}

ساكن صحيح mbox_پيغام_put_walk_op(بنية k_thread *thread, فراغ *معلومات)
{
	بنية mbox_walk_معلومات *walk_معلومات = (بنية mbox_walk_معلومات *)معلومات;
	بنية k_mbox_نياپو *rx_نياپو;

	rx_نياپو = (بنية k_mbox_نياپو *)thread->بنياد.مٽا_سٽا_معلومات;

	إذا (mbox_پيغام_ميلاپ(walk_معلومات->نياپو, rx_نياپو) != 0) {
		/* No match. Proceed to next in wait queue */
		أرجع 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(فراغ)z_try_abort_thread_timeout(thread);
	arch_thread_return_قدر_سيٽ(thread, 0);
	z_sched_ready_locked(thread);

#إذا (CONFIG_انگ_MBOX_ASYNC_MSGS > 0)
	إذا (is_thread_dummy(walk_معلومات->thread)) {
		أرجع ASYNCHRONOUS_ميلاپ_مليو;
	}
#جيڪڏهن_پورو

	أرجع SYNCHRONOUS_ميلاپ_مليو;
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
ساكن صحيح mbox_پيغام_put(بنية k_mbox *mbox, بنية k_mbox_نياپو *tx_نياپو,
			     k_timeout_t timeout)
{
	بنية k_thread *sending_thread;
	k_spinlock_ڪنجي_t ڪنجي;

	/* save sender id so it can be used during message matching */
	tx_نياپو->rx_ذريعو_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_نياپو->_syncing_thread;
	sending_thread->بنياد.مٽا_سٽا_معلومات = tx_نياپو;

	/* search mailbox's rx queue for a compatible receiver */
	ڪنجي = k_spin_تالو(&mbox->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_mbox, پيغام_put, mbox, timeout);

	بنية mbox_walk_معلومات walk_معلومات;
	صحيح حالت;

	walk_معلومات.thread = sending_thread;
	walk_معلومات.نياپو = tx_نياپو;

	/* Search the wait queue for the first matching receiver */

	حالت = z_sched_waitq_walk(&mbox->rx_نياپو_قطار,
				    mbox_پيغام_put_walk_op, عدم,
				    &walk_معلومات);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#إذا (CONFIG_انگ_MBOX_ASYNC_MSGS > 0)
	إذا (حالت == ASYNCHRONOUS_ميلاپ_مليو) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->تالو, ڪنجي);
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mbox, پيغام_put, mbox,
					       timeout, 0);
		أرجع 0;
	}
#جيڪڏهن_پورو

	إذا (حالت == SYNCHRONOUS_ميلاپ_مليو) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, پيغام_put, mbox,
						   timeout);

		صحيح ret = z_pend_curr(&mbox->تالو, ڪنجي, عدم, K_FOREVER);

		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mbox, پيغام_put, mbox,
					       timeout, ret);

		أرجع ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mbox, پيغام_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->تالو, ڪنجي);
		أرجع -ENOMSG;
	}

#إذا (CONFIG_انگ_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	إذا (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_نياپو_قطار, K_FOREVER);
		k_spin_unlock(&mbox->تالو, ڪنجي);
		أرجع 0;
	}
#جيڪڏهن_پورو /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, پيغام_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	صحيح ret = z_pend_curr(&mbox->تالو, ڪنجي, &mbox->tx_نياپو_قطار, timeout);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mbox, پيغام_put, mbox, timeout, ret);

	أرجع ret;
}

صحيح k_mbox_put(بنية k_mbox *mbox, بنية k_mbox_نياپو *tx_نياپو,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_نياپو->_syncing_thread = _current;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	صحيح ret = mbox_پيغام_put(mbox, tx_نياپو, timeout);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	أرجع ret;
}

#إذا (CONFIG_انگ_MBOX_ASYNC_MSGS > 0)
فراغ k_mbox_async_put(بنية k_mbox *mbox, بنية k_mbox_نياپو *tx_نياپو,
		      بنية k_sem *sem)
{
	بنية k_mbox_async *async;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->بنياد.prio;

	async->tx_نياپو = *tx_نياپو;
	async->tx_نياپو._syncing_thread = (بنية k_thread *)&async->thread;
	async->tx_نياپو._async_sem = sem;

	(فراغ)mbox_پيغام_put(mbox, &async->tx_نياپو, K_FOREVER);
	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#جيڪڏهن_پورو /* CONFIG_NUM_MBOX_ASYNC_MSGS */

فراغ k_mbox_معلومات_get(بنية k_mbox_نياپو *rx_نياپو, فراغ *بفر)
{
	/* handle case where data is to be discarded */
	إذا (بفر == عدم) {
		rx_نياپو->سائيز = 0;
		mbox_پيغام_dispose(rx_نياپو);
		أرجع;
	}

	/* copy message data to buffer, then dispose of message */
	إذا ((rx_نياپو->tx_معلومات != عدم) && (rx_نياپو->سائيز > 0U)) {
		(فراغ)يادگيري_نقل(بفر, rx_نياپو->tx_معلومات, rx_نياپو->سائيز);
	}
	mbox_پيغام_dispose(rx_نياپو);
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
ساكن صحيح mbox_پيغام_معلومات_check(بنية k_mbox_نياپو *rx_نياپو, فراغ *بفر)
{
	إذا (بفر != عدم) {
		/* retrieve data now, then dispose of message */
		k_mbox_معلومات_get(rx_نياپو, بفر);
	} وإلا إذا (rx_نياپو->سائيز == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_پيغام_dispose(rx_نياپو);
	} وإلا {
		/* keep message around for later data retrieval */
	}

	أرجع 0;
}

ساكن صحيح mbox_get_walk_op(بنية k_thread *thread, فراغ *معلومات)
{
	بنية mbox_walk_معلومات *walk_معلومات = (بنية mbox_walk_معلومات *)معلومات;
	بنية k_mbox_نياپو *tx_نياپو;

	tx_نياپو = (بنية k_mbox_نياپو *)thread->بنياد.مٽا_سٽا_معلومات;

	إذا (mbox_پيغام_ميلاپ(tx_نياپو, walk_معلومات->نياپو) != 0) {
		/* No match. Proceed to next in wait queue */
		أرجع 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(فراغ)z_try_abort_thread_timeout(thread);

	أرجع 1;
}

صحيح k_mbox_get(بنية k_mbox *mbox, بنية k_mbox_نياپو *rx_نياپو, فراغ *بفر,
	       k_timeout_t timeout)
{
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح نتيجو;

	/* save receiver id so it can be used during message matching */
	rx_نياپو->tx_هدف_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	ڪنجي = k_spin_تالو(&mbox->تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	بنية mbox_walk_معلومات walk_معلومات;
	صحيح حالت;

	walk_معلومات.thread = عدم;   /* Unused by walk operation */
	walk_معلومات.نياپو = rx_نياپو;

	/* Search the wait queue for the first matching sender */

	حالت = z_sched_waitq_walk(&mbox->tx_نياپو_قطار,
				    mbox_get_walk_op, عدم, &walk_معلومات);

	إذا (حالت != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->تالو, ڪنجي);
		نتيجو = 0;

		اذهب ٻاهريون;
	}

	/* didn't find a matching sender */

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->تالو, ڪنجي);

		نتيجو = -ENOMSG;
		اذهب ٻاهريون;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->بنياد.مٽا_سٽا_معلومات = rx_نياپو;
	نتيجو = z_pend_curr(&mbox->تالو, ڪنجي, &mbox->rx_نياپو_قطار, timeout);

ٻاهريون:
	/* consume message data immediately, if needed */
	إذا (نتيجو == 0) {
		نتيجو = mbox_پيغام_معلومات_check(rx_نياپو, بفر);
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, نتيجو);

	أرجع نتيجو;
}
