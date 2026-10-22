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

#تعریف_کن SYNCHRONOUS_تطابق_یافت  1
#تعریف_کن ASYNCHRONOUS_تطابق_یافت 2

بنية mbox_walk_داده {
	بنية k_thread *thread;
	بنية k_mbox_پیام *پیام;
};

#اگر_تعریف_شده CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, عدم);
#پایان_اگر /* CONFIG_OBJ_CORE_MAILBOX */

#إذا (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
بنية k_mbox_async {
	بنية thread_پایه thread;		/* dummy thread object */
	بنية k_mbox_پیام tx_پیام;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_پشته_DEFINE(async_پیام_free, CONFIG_رقم_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_alloc(بنية k_mbox_async **async)
{
	(فراغ)k_پشته_برداشت(&async_پیام_free, (پشته_داده_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ساكن مضمن فراغ mbox_async_free(بنية k_mbox_async *async)
{
	k_پشته_افزودن(&async_پیام_free, (پشته_داده_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ساكن فراغ init_mbox_module(فراغ)
{
	/* array of asynchronous message descriptors */
	ساكن بنية k_mbox_async __noinit async_پیام[CONFIG_رقم_MBOX_ASYNC_MSGS];

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

	لكل (i = 0; i < CONFIG_رقم_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_پایه(&async_پیام[i].thread, 0, _THREAD_DUMMY, 0);
		k_پشته_افزودن(&async_پیام_free, (پشته_داده_t)&async_پیام[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#پایان_اگر /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

فراغ k_mbox_init(بنية k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_پیام_صف);
	z_waitq_init(&mbox->rx_پیام_صف);
	mbox->قفل = (بنية k_spinlock) {};

#اگر_تعریف_شده CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#پایان_اگر /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_پورت_TRACING_OBJ_INIT(k_mbox, mbox);
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
ساكن صحيح mbox_پیام_تطابق(بنية k_mbox_پیام *tx_پیام,
			       بنية k_mbox_پیام *rx_پیام)
{
	uint32_t موقت_info;

	إذا (((tx_پیام->tx_هدف_thread == (k_tid_t)K_ANY) ||
	     (tx_پیام->tx_هدف_thread == rx_پیام->tx_هدف_thread)) &&
	    ((rx_پیام->rx_منبع_thread == (k_tid_t)K_ANY) ||
	     (rx_پیام->rx_منبع_thread == tx_پیام->rx_منبع_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_پیام->rx_منبع_thread = tx_پیام->rx_منبع_thread;
		tx_پیام->tx_هدف_thread = rx_پیام->tx_هدف_thread;

		/* update application info fields for both descriptors */
		موقت_info = rx_پیام->info;
		rx_پیام->info = tx_پیام->info;
		tx_پیام->info = موقت_info;

		/* update data size field for receiver only */
		إذا (rx_پیام->اندازه > tx_پیام->اندازه) {
			rx_پیام->اندازه = tx_پیام->اندازه;
		}

		/* update data location fields for receiver only */
		rx_پیام->tx_داده = tx_پیام->tx_داده;

		/* update syncing thread field for receiver only */
		rx_پیام->_syncing_thread = tx_پیام->_syncing_thread;

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
ساكن فراغ mbox_پیام_dispose(بنية k_mbox_پیام *rx_پیام)
{
	بنية k_thread *sending_thread;
	بنية k_mbox_پیام *tx_پیام;

	/* do nothing if message was disposed of when it was received */
	إذا (rx_پیام->_syncing_thread == عدم) {
		أرجع;
	}

	/* recover sender info */
	sending_thread = rx_پیام->_syncing_thread;
	rx_پیام->_syncing_thread = عدم;
	tx_پیام = (بنية k_mbox_پیام *)sending_thread->پایه.جابجا_داده;

	/* update data size field for sender */
	tx_پیام->اندازه = rx_پیام->اندازه;

#إذا (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	إذا (is_thread_dummy(sending_thread)) {
		بنية k_sem *async_sem = tx_پیام->_async_sem;

		mbox_async_free((بنية k_mbox_async *)sending_thread);
		إذا (async_sem != عدم) {
			k_sem_give(async_sem);
		}
		أرجع;
	}
#پایان_اگر /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_کلید_t کلید = k_spin_قفل(&_sched_spinlock);

	arch_thread_return_مقدار_مجموعه(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, کلید);
}

ساكن صحيح mbox_پیام_put_walk_op(بنية k_thread *thread, فراغ *داده)
{
	بنية mbox_walk_داده *walk_داده = (بنية mbox_walk_داده *)داده;
	بنية k_mbox_پیام *rx_پیام;

	rx_پیام = (بنية k_mbox_پیام *)thread->پایه.جابجا_داده;

	إذا (mbox_پیام_تطابق(walk_داده->پیام, rx_پیام) != 0) {
		/* No match. Proceed to next in wait queue */
		أرجع 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(فراغ)z_try_abort_thread_timeout(thread);
	arch_thread_return_مقدار_مجموعه(thread, 0);
	z_sched_ready_locked(thread);

#إذا (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	إذا (is_thread_dummy(walk_داده->thread)) {
		أرجع ASYNCHRONOUS_تطابق_یافت;
	}
#پایان_اگر

	أرجع SYNCHRONOUS_تطابق_یافت;
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
ساكن صحيح mbox_پیام_put(بنية k_mbox *mbox, بنية k_mbox_پیام *tx_پیام,
			     k_timeout_t timeout)
{
	بنية k_thread *sending_thread;
	k_spinlock_کلید_t کلید;

	/* save sender id so it can be used during message matching */
	tx_پیام->rx_منبع_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_پیام->_syncing_thread;
	sending_thread->پایه.جابجا_داده = tx_پیام;

	/* search mailbox's rx queue for a compatible receiver */
	کلید = k_spin_قفل(&mbox->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_mbox, پیام_put, mbox, timeout);

	بنية mbox_walk_داده walk_داده;
	صحيح وضعیت;

	walk_داده.thread = sending_thread;
	walk_داده.پیام = tx_پیام;

	/* Search the wait queue for the first matching receiver */

	وضعیت = z_sched_waitq_walk(&mbox->rx_پیام_صف,
				    mbox_پیام_put_walk_op, عدم,
				    &walk_داده);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#إذا (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	إذا (وضعیت == ASYNCHRONOUS_تطابق_یافت) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->قفل, کلید);
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mbox, پیام_put, mbox,
					       timeout, 0);
		أرجع 0;
	}
#پایان_اگر

	إذا (وضعیت == SYNCHRONOUS_تطابق_یافت) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_mbox, پیام_put, mbox,
						   timeout);

		صحيح ret = z_pend_curr(&mbox->قفل, کلید, عدم, K_FOREVER);

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mbox, پیام_put, mbox,
					       timeout, ret);

		أرجع ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mbox, پیام_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->قفل, کلید);
		أرجع -ENOMSG;
	}

#إذا (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	إذا (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_پیام_صف, K_FOREVER);
		k_spin_unlock(&mbox->قفل, کلید);
		أرجع 0;
	}
#پایان_اگر /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_mbox, پیام_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	صحيح ret = z_pend_curr(&mbox->قفل, کلید, &mbox->tx_پیام_صف, timeout);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mbox, پیام_put, mbox, timeout, ret);

	أرجع ret;
}

صحيح k_mbox_put(بنية k_mbox *mbox, بنية k_mbox_پیام *tx_پیام,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_پیام->_syncing_thread = _current;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	صحيح ret = mbox_پیام_put(mbox, tx_پیام, timeout);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	أرجع ret;
}

#إذا (CONFIG_رقم_MBOX_ASYNC_MSGS > 0)
فراغ k_mbox_async_put(بنية k_mbox *mbox, بنية k_mbox_پیام *tx_پیام,
		      بنية k_sem *sem)
{
	بنية k_mbox_async *async;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->پایه.prio;

	async->tx_پیام = *tx_پیام;
	async->tx_پیام._syncing_thread = (بنية k_thread *)&async->thread;
	async->tx_پیام._async_sem = sem;

	(فراغ)mbox_پیام_put(mbox, &async->tx_پیام, K_FOREVER);
	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#پایان_اگر /* CONFIG_NUM_MBOX_ASYNC_MSGS */

فراغ k_mbox_داده_get(بنية k_mbox_پیام *rx_پیام, فراغ *میانگیر)
{
	/* handle case where data is to be discarded */
	إذا (میانگیر == عدم) {
		rx_پیام->اندازه = 0;
		mbox_پیام_dispose(rx_پیام);
		أرجع;
	}

	/* copy message data to buffer, then dispose of message */
	إذا ((rx_پیام->tx_داده != عدم) && (rx_پیام->اندازه > 0U)) {
		(فراغ)کپی_حافظه(میانگیر, rx_پیام->tx_داده, rx_پیام->اندازه);
	}
	mbox_پیام_dispose(rx_پیام);
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
ساكن صحيح mbox_پیام_داده_check(بنية k_mbox_پیام *rx_پیام, فراغ *میانگیر)
{
	إذا (میانگیر != عدم) {
		/* retrieve data now, then dispose of message */
		k_mbox_داده_get(rx_پیام, میانگیر);
	} وإلا إذا (rx_پیام->اندازه == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_پیام_dispose(rx_پیام);
	} وإلا {
		/* keep message around for later data retrieval */
	}

	أرجع 0;
}

ساكن صحيح mbox_get_walk_op(بنية k_thread *thread, فراغ *داده)
{
	بنية mbox_walk_داده *walk_داده = (بنية mbox_walk_داده *)داده;
	بنية k_mbox_پیام *tx_پیام;

	tx_پیام = (بنية k_mbox_پیام *)thread->پایه.جابجا_داده;

	إذا (mbox_پیام_تطابق(tx_پیام, walk_داده->پیام) != 0) {
		/* No match. Proceed to next in wait queue */
		أرجع 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(فراغ)z_try_abort_thread_timeout(thread);

	أرجع 1;
}

صحيح k_mbox_get(بنية k_mbox *mbox, بنية k_mbox_پیام *rx_پیام, فراغ *میانگیر,
	       k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحيح نتیجه;

	/* save receiver id so it can be used during message matching */
	rx_پیام->tx_هدف_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	کلید = k_spin_قفل(&mbox->قفل);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	بنية mbox_walk_داده walk_داده;
	صحيح وضعیت;

	walk_داده.thread = عدم;   /* Unused by walk operation */
	walk_داده.پیام = rx_پیام;

	/* Search the wait queue for the first matching sender */

	وضعیت = z_sched_waitq_walk(&mbox->tx_پیام_صف,
				    mbox_get_walk_op, عدم, &walk_داده);

	إذا (وضعیت != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->قفل, کلید);
		نتیجه = 0;

		اذهب بیرون;
	}

	/* didn't find a matching sender */

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->قفل, کلید);

		نتیجه = -ENOMSG;
		اذهب بیرون;
	}

	SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->پایه.جابجا_داده = rx_پیام;
	نتیجه = z_pend_curr(&mbox->قفل, کلید, &mbox->rx_پیام_صف, timeout);

بیرون:
	/* consume message data immediately, if needed */
	إذا (نتیجه == 0) {
		نتیجه = mbox_پیام_داده_check(rx_پیام, میانگیر);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, نتیجه);

	أرجع نتیجه;
}
