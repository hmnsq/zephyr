/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
/* private kernel APIs */
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYNCHRONOUS_ਮੇਲ_ਮਿਲਿਆ  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASYNCHRONOUS_ਮੇਲ_ਮਿਲਿਆ 2

ਬਣਤਰ mbox_walk_ਅੰਕੜੇ {
	ਬਣਤਰ k_thread *thread;
	ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ;
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MAILBOX */

#ਜੇ (CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
ਬਣਤਰ k_mbox_async {
	ਬਣਤਰ thread_ਅਧਾਰ thread;		/* dummy thread object */
	ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ tx_ਸੁਨੇਹਾ;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ਢੇਰ_DEFINE(async_ਸੁਨੇਹਾ_free, CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ mbox_async_alloc(ਬਣਤਰ k_mbox_async **async)
{
	(ਖਾਲੀ)k_ਢੇਰ_ਕੱਢੋ(&async_ਸੁਨੇਹਾ_free, (ਢੇਰ_ਅੰਕੜੇ_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ mbox_async_free(ਬਣਤਰ k_mbox_async *async)
{
	k_ਢੇਰ_ਧੱਕੋ(&async_ਸੁਨੇਹਾ_free, (ਢੇਰ_ਅੰਕੜੇ_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ਸਥਿਰ ਖਾਲੀ init_mbox_module(ਖਾਲੀ)
{
	/* array of asynchronous message descriptors */
	ਸਥਿਰ ਬਣਤਰ k_mbox_async __noinit async_ਸੁਨੇਹਾ[CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS];

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

	ਪੂਰਨ i;

	ਲਈ (i = 0; i < CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_ਅਧਾਰ(&async_ਸੁਨੇਹਾ[i].thread, 0, _THREAD_DUMMY, 0);
		k_ਢੇਰ_ਧੱਕੋ(&async_ਸੁਨੇਹਾ_free, (ਢੇਰ_ਅੰਕੜੇ_t)&async_ਸੁਨੇਹਾ[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#ਜੇ_ਅੰਤ /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

ਖਾਲੀ k_mbox_init(ਬਣਤਰ k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_ਸੁਨੇਹਾ_ਕਤਾਰ);
	z_waitq_init(&mbox->rx_ਸੁਨੇਹਾ_ਕਤਾਰ);
	mbox->ਤਾਲਾ = (ਬਣਤਰ k_spinlock) {};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_mbox, mbox);
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
ਸਥਿਰ ਪੂਰਨ mbox_ਸੁਨੇਹਾ_ਮੇਲ(ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *tx_ਸੁਨੇਹਾ,
			       ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *rx_ਸੁਨੇਹਾ)
{
	uint32_t ਆਰਜ਼ੀ_info;

	ਜੇ (((tx_ਸੁਨੇਹਾ->tx_ਨਿਸ਼ਾਨਾ_thread == (k_tid_t)K_ANY) ||
	     (tx_ਸੁਨੇਹਾ->tx_ਨਿਸ਼ਾਨਾ_thread == rx_ਸੁਨੇਹਾ->tx_ਨਿਸ਼ਾਨਾ_thread)) &&
	    ((rx_ਸੁਨੇਹਾ->rx_ਸਰੋਤ_thread == (k_tid_t)K_ANY) ||
	     (rx_ਸੁਨੇਹਾ->rx_ਸਰੋਤ_thread == tx_ਸੁਨੇਹਾ->rx_ਸਰੋਤ_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_ਸੁਨੇਹਾ->rx_ਸਰੋਤ_thread = tx_ਸੁਨੇਹਾ->rx_ਸਰੋਤ_thread;
		tx_ਸੁਨੇਹਾ->tx_ਨਿਸ਼ਾਨਾ_thread = rx_ਸੁਨੇਹਾ->tx_ਨਿਸ਼ਾਨਾ_thread;

		/* update application info fields for both descriptors */
		ਆਰਜ਼ੀ_info = rx_ਸੁਨੇਹਾ->info;
		rx_ਸੁਨੇਹਾ->info = tx_ਸੁਨੇਹਾ->info;
		tx_ਸੁਨੇਹਾ->info = ਆਰਜ਼ੀ_info;

		/* update data size field for receiver only */
		ਜੇ (rx_ਸੁਨੇਹਾ->ਮਾਪ > tx_ਸੁਨੇਹਾ->ਮਾਪ) {
			rx_ਸੁਨੇਹਾ->ਮਾਪ = tx_ਸੁਨੇਹਾ->ਮਾਪ;
		}

		/* update data location fields for receiver only */
		rx_ਸੁਨੇਹਾ->tx_ਅੰਕੜੇ = tx_ਸੁਨੇਹਾ->tx_ਅੰਕੜੇ;

		/* update syncing thread field for receiver only */
		rx_ਸੁਨੇਹਾ->_syncing_thread = tx_ਸੁਨੇਹਾ->_syncing_thread;

		ਵਾਪਸ 0;
	}

	ਵਾਪਸ -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
ਸਥਿਰ ਖਾਲੀ mbox_ਸੁਨੇਹਾ_dispose(ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *rx_ਸੁਨੇਹਾ)
{
	ਬਣਤਰ k_thread *sending_thread;
	ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *tx_ਸੁਨੇਹਾ;

	/* do nothing if message was disposed of when it was received */
	ਜੇ (rx_ਸੁਨੇਹਾ->_syncing_thread == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	/* recover sender info */
	sending_thread = rx_ਸੁਨੇਹਾ->_syncing_thread;
	rx_ਸੁਨੇਹਾ->_syncing_thread = ਨਿਰਾ;
	tx_ਸੁਨੇਹਾ = (ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *)sending_thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;

	/* update data size field for sender */
	tx_ਸੁਨੇਹਾ->ਮਾਪ = rx_ਸੁਨੇਹਾ->ਮਾਪ;

#ਜੇ (CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	ਜੇ (is_thread_dummy(sending_thread)) {
		ਬਣਤਰ k_sem *async_sem = tx_ਸੁਨੇਹਾ->_async_sem;

		mbox_async_free((ਬਣਤਰ k_mbox_async *)sending_thread);
		ਜੇ (async_sem != ਨਿਰਾ) {
			k_sem_give(async_sem);
		}
		ਵਾਪਸ;
	}
#ਜੇ_ਅੰਤ /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&_sched_spinlock);

	arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, ਕੁੰਜੀ);
}

ਸਥਿਰ ਪੂਰਨ mbox_ਸੁਨੇਹਾ_put_walk_op(ਬਣਤਰ k_thread *thread, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਬਣਤਰ mbox_walk_ਅੰਕੜੇ *walk_ਅੰਕੜੇ = (ਬਣਤਰ mbox_walk_ਅੰਕੜੇ *)ਅੰਕੜੇ;
	ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *rx_ਸੁਨੇਹਾ;

	rx_ਸੁਨੇਹਾ = (ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *)thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;

	ਜੇ (mbox_ਸੁਨੇਹਾ_ਮੇਲ(walk_ਅੰਕੜੇ->ਸੁਨੇਹਾ, rx_ਸੁਨੇਹਾ) != 0) {
		/* No match. Proceed to next in wait queue */
		ਵਾਪਸ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(ਖਾਲੀ)z_try_abort_thread_timeout(thread);
	arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, 0);
	z_sched_ready_locked(thread);

#ਜੇ (CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS > 0)
	ਜੇ (is_thread_dummy(walk_ਅੰਕੜੇ->thread)) {
		ਵਾਪਸ ASYNCHRONOUS_ਮੇਲ_ਮਿਲਿਆ;
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ SYNCHRONOUS_ਮੇਲ_ਮਿਲਿਆ;
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
ਸਥਿਰ ਪੂਰਨ mbox_ਸੁਨੇਹਾ_put(ਬਣਤਰ k_mbox *mbox, ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *tx_ਸੁਨੇਹਾ,
			     k_timeout_t timeout)
{
	ਬਣਤਰ k_thread *sending_thread;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	/* save sender id so it can be used during message matching */
	tx_ਸੁਨੇਹਾ->rx_ਸਰੋਤ_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_ਸੁਨੇਹਾ->_syncing_thread;
	sending_thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ = tx_ਸੁਨੇਹਾ;

	/* search mailbox's rx queue for a compatible receiver */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&mbox->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mbox, ਸੁਨੇਹਾ_put, mbox, timeout);

	ਬਣਤਰ mbox_walk_ਅੰਕੜੇ walk_ਅੰਕੜੇ;
	ਪੂਰਨ ਸਥਿਤੀ;

	walk_ਅੰਕੜੇ.thread = sending_thread;
	walk_ਅੰਕੜੇ.ਸੁਨੇਹਾ = tx_ਸੁਨੇਹਾ;

	/* Search the wait queue for the first matching receiver */

	ਸਥਿਤੀ = z_sched_waitq_walk(&mbox->rx_ਸੁਨੇਹਾ_ਕਤਾਰ,
				    mbox_ਸੁਨੇਹਾ_put_walk_op, ਨਿਰਾ,
				    &walk_ਅੰਕੜੇ);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#ਜੇ (CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS > 0)
	ਜੇ (ਸਥਿਤੀ == ASYNCHRONOUS_ਮੇਲ_ਮਿਲਿਆ) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->ਤਾਲਾ, ਕੁੰਜੀ);
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mbox, ਸੁਨੇਹਾ_put, mbox,
					       timeout, 0);
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ

	ਜੇ (ਸਥਿਤੀ == SYNCHRONOUS_ਮੇਲ_ਮਿਲਿਆ) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, ਸੁਨੇਹਾ_put, mbox,
						   timeout);

		ਪੂਰਨ ret = z_pend_curr(&mbox->ਤਾਲਾ, ਕੁੰਜੀ, ਨਿਰਾ, K_FOREVER);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mbox, ਸੁਨੇਹਾ_put, mbox,
					       timeout, ret);

		ਵਾਪਸ ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mbox, ਸੁਨੇਹਾ_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ -ENOMSG;
	}

#ਜੇ (CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	ਜੇ (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_ਸੁਨੇਹਾ_ਕਤਾਰ, K_FOREVER);
		k_spin_unlock(&mbox->ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, ਸੁਨੇਹਾ_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	ਪੂਰਨ ret = z_pend_curr(&mbox->ਤਾਲਾ, ਕੁੰਜੀ, &mbox->tx_ਸੁਨੇਹਾ_ਕਤਾਰ, timeout);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mbox, ਸੁਨੇਹਾ_put, mbox, timeout, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_mbox_put(ਬਣਤਰ k_mbox *mbox, ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *tx_ਸੁਨੇਹਾ,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_ਸੁਨੇਹਾ->_syncing_thread = _current;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	ਪੂਰਨ ret = mbox_ਸੁਨੇਹਾ_put(mbox, tx_ਸੁਨੇਹਾ, timeout);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	ਵਾਪਸ ret;
}

#ਜੇ (CONFIG_ਅੰਕ_MBOX_ASYNC_MSGS > 0)
ਖਾਲੀ k_mbox_async_put(ਬਣਤਰ k_mbox *mbox, ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *tx_ਸੁਨੇਹਾ,
		      ਬਣਤਰ k_sem *sem)
{
	ਬਣਤਰ k_mbox_async *async;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->ਅਧਾਰ.prio;

	async->tx_ਸੁਨੇਹਾ = *tx_ਸੁਨੇਹਾ;
	async->tx_ਸੁਨੇਹਾ._syncing_thread = (ਬਣਤਰ k_thread *)&async->thread;
	async->tx_ਸੁਨੇਹਾ._async_sem = sem;

	(ਖਾਲੀ)mbox_ਸੁਨੇਹਾ_put(mbox, &async->tx_ਸੁਨੇਹਾ, K_FOREVER);
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#ਜੇ_ਅੰਤ /* CONFIG_NUM_MBOX_ASYNC_MSGS */

ਖਾਲੀ k_mbox_ਅੰਕੜੇ_get(ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *rx_ਸੁਨੇਹਾ, ਖਾਲੀ *ਬਫਰ)
{
	/* handle case where data is to be discarded */
	ਜੇ (ਬਫਰ == ਨਿਰਾ) {
		rx_ਸੁਨੇਹਾ->ਮਾਪ = 0;
		mbox_ਸੁਨੇਹਾ_dispose(rx_ਸੁਨੇਹਾ);
		ਵਾਪਸ;
	}

	/* copy message data to buffer, then dispose of message */
	ਜੇ ((rx_ਸੁਨੇਹਾ->tx_ਅੰਕੜੇ != ਨਿਰਾ) && (rx_ਸੁਨੇਹਾ->ਮਾਪ > 0U)) {
		(ਖਾਲੀ)ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ, rx_ਸੁਨੇਹਾ->tx_ਅੰਕੜੇ, rx_ਸੁਨੇਹਾ->ਮਾਪ);
	}
	mbox_ਸੁਨੇਹਾ_dispose(rx_ਸੁਨੇਹਾ);
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
ਸਥਿਰ ਪੂਰਨ mbox_ਸੁਨੇਹਾ_ਅੰਕੜੇ_check(ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *rx_ਸੁਨੇਹਾ, ਖਾਲੀ *ਬਫਰ)
{
	ਜੇ (ਬਫਰ != ਨਿਰਾ) {
		/* retrieve data now, then dispose of message */
		k_mbox_ਅੰਕੜੇ_get(rx_ਸੁਨੇਹਾ, ਬਫਰ);
	} ਨਹੀਂਤਾਂ ਜੇ (rx_ਸੁਨੇਹਾ->ਮਾਪ == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_ਸੁਨੇਹਾ_dispose(rx_ਸੁਨੇਹਾ);
	} ਨਹੀਂਤਾਂ {
		/* keep message around for later data retrieval */
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ mbox_get_walk_op(ਬਣਤਰ k_thread *thread, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਬਣਤਰ mbox_walk_ਅੰਕੜੇ *walk_ਅੰਕੜੇ = (ਬਣਤਰ mbox_walk_ਅੰਕੜੇ *)ਅੰਕੜੇ;
	ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *tx_ਸੁਨੇਹਾ;

	tx_ਸੁਨੇਹਾ = (ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *)thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;

	ਜੇ (mbox_ਸੁਨੇਹਾ_ਮੇਲ(tx_ਸੁਨੇਹਾ, walk_ਅੰਕੜੇ->ਸੁਨੇਹਾ) != 0) {
		/* No match. Proceed to next in wait queue */
		ਵਾਪਸ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(ਖਾਲੀ)z_try_abort_thread_timeout(thread);

	ਵਾਪਸ 1;
}

ਪੂਰਨ k_mbox_get(ਬਣਤਰ k_mbox *mbox, ਬਣਤਰ k_mbox_ਸੁਨੇਹਾ *rx_ਸੁਨੇਹਾ, ਖਾਲੀ *ਬਫਰ,
	       k_timeout_t timeout)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ਨਤੀਜਾ;

	/* save receiver id so it can be used during message matching */
	rx_ਸੁਨੇਹਾ->tx_ਨਿਸ਼ਾਨਾ_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&mbox->ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	ਬਣਤਰ mbox_walk_ਅੰਕੜੇ walk_ਅੰਕੜੇ;
	ਪੂਰਨ ਸਥਿਤੀ;

	walk_ਅੰਕੜੇ.thread = ਨਿਰਾ;   /* Unused by walk operation */
	walk_ਅੰਕੜੇ.ਸੁਨੇਹਾ = rx_ਸੁਨੇਹਾ;

	/* Search the wait queue for the first matching sender */

	ਸਥਿਤੀ = z_sched_waitq_walk(&mbox->tx_ਸੁਨੇਹਾ_ਕਤਾਰ,
				    mbox_get_walk_op, ਨਿਰਾ, &walk_ਅੰਕੜੇ);

	ਜੇ (ਸਥਿਤੀ != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->ਤਾਲਾ, ਕੁੰਜੀ);
		ਨਤੀਜਾ = 0;

		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/* didn't find a matching sender */

	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->ਤਾਲਾ, ਕੁੰਜੀ);

		ਨਤੀਜਾ = -ENOMSG;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ = rx_ਸੁਨੇਹਾ;
	ਨਤੀਜਾ = z_pend_curr(&mbox->ਤਾਲਾ, ਕੁੰਜੀ, &mbox->rx_ਸੁਨੇਹਾ_ਕਤਾਰ, timeout);

ਬਾਹਰਲਾ:
	/* consume message data immediately, if needed */
	ਜੇ (ਨਤੀਜਾ == 0) {
		ਨਤੀਜਾ = mbox_ਸੁਨੇਹਾ_ਅੰਕੜੇ_check(rx_ਸੁਨੇਹਾ, ਬਫਰ);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, ਨਤੀਜਾ);

	ਵਾਪਸ ਨਤੀਜਾ;
}
