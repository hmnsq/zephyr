/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Mailboxes.
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/sections.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
/* private kernel APIs */
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>

#ವ್ಯಾಖ್ಯಾನಿಸು SYNCHRONOUS_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು  1
#ವ್ಯಾಖ್ಯಾನಿಸು ASYNCHRONOUS_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು 2

ರಚನೆ mbox_walk_ದತ್ತಾಂಶ {
	ರಚನೆ k_thread *thread;
	ರಚನೆ k_mbox_ಸಂದೇಶ *ಸಂದೇಶ;
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MAILBOX
K_OBJ_TYPE_DEFINE(obj_type_mailbox, k_mbox, K_OBJ_TYPE_MBOX_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MAILBOX */

#ಆದರೆ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)

/* asynchronous message descriptor type */
ರಚನೆ k_mbox_async {
	ರಚನೆ thread_ತಳ thread;		/* dummy thread object */
	ರಚನೆ k_mbox_ಸಂದೇಶ tx_ಸಂದೇಶ;	/* transmit message descriptor */
};

/* stack of unused asynchronous message descriptors */
K_ರಾಶಿ_DEFINE(async_ಸಂದೇಶ_free, CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS);

/* allocate an asynchronous message descriptor */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ mbox_async_alloc(ರಚನೆ k_mbox_async **async)
{
	(ಶೂನ್ಯ)k_ರಾಶಿ_ತೆಗೆ(&async_ಸಂದೇಶ_free, (ರಾಶಿ_ದತ್ತಾಂಶ_t *)async, K_FOREVER);
}

/* free an asynchronous message descriptor */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ mbox_async_free(ರಚನೆ k_mbox_async *async)
{
	k_ರಾಶಿ_ತಳ್ಳು(&async_ಸಂದೇಶ_free, (ರಾಶಿ_ದತ್ತಾಂಶ_t)async);
}

/*
 * Do run-time initialization of mailbox object subsystem.
 */
ಸ್ಥಿರ ಶೂನ್ಯ init_mbox_module(ಶೂನ್ಯ)
{
	/* array of asynchronous message descriptors */
	ಸ್ಥಿರ ರಚನೆ k_mbox_async __noinit async_ಸಂದೇಶ[CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS];

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

	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS; i++) {
		z_init_thread_ತಳ(&async_ಸಂದೇಶ[i].thread, 0, _THREAD_DUMMY, 0);
		k_ರಾಶಿ_ತಳ್ಳು(&async_ಸಂದೇಶ_free, (ರಾಶಿ_ದತ್ತಾಂಶ_t)&async_ಸಂದೇಶ[i]);
	}

	/* Complete initialization of statically defined mailboxes. */
}

K_KERNEL_INIT_PRE(init_mbox_module);

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_NUM_MBOX_ASYNC_MSGS > 0 */

ಶೂನ್ಯ k_mbox_init(ರಚನೆ k_mbox *mbox)
{
	z_waitq_init(&mbox->tx_ಸಂದೇಶ_ಸರತಿ);
	z_waitq_init(&mbox->rx_ಸಂದೇಶ_ಸರತಿ);
	mbox->ಬೀಗ = (ರಚನೆ k_spinlock) {};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MAILBOX
	k_obj_core_init_and_link(K_OBJ_CORE(mbox), &obj_type_mailbox);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MAILBOX */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_mbox, mbox);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mbox_ಸಂದೇಶ_ಹೊಂದಿಕೆ(ರಚನೆ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
			       ರಚನೆ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ)
{
	uint32_t ತಾತ್ಕಾಲಿಕ_info;

	ಆದರೆ (((tx_ಸಂದೇಶ->tx_ಗುರಿ_thread == (k_tid_t)K_ANY) ||
	     (tx_ಸಂದೇಶ->tx_ಗುರಿ_thread == rx_ಸಂದೇಶ->tx_ಗುರಿ_thread)) &&
	    ((rx_ಸಂದೇಶ->rx_ಮೂಲ_thread == (k_tid_t)K_ANY) ||
	     (rx_ಸಂದೇಶ->rx_ಮೂಲ_thread == tx_ಸಂದೇಶ->rx_ಮೂಲ_thread))) {

		/* update thread identifier fields for both descriptors */
		rx_ಸಂದೇಶ->rx_ಮೂಲ_thread = tx_ಸಂದೇಶ->rx_ಮೂಲ_thread;
		tx_ಸಂದೇಶ->tx_ಗುರಿ_thread = rx_ಸಂದೇಶ->tx_ಗುರಿ_thread;

		/* update application info fields for both descriptors */
		ತಾತ್ಕಾಲಿಕ_info = rx_ಸಂದೇಶ->info;
		rx_ಸಂದೇಶ->info = tx_ಸಂದೇಶ->info;
		tx_ಸಂದೇಶ->info = ತಾತ್ಕಾಲಿಕ_info;

		/* update data size field for receiver only */
		ಆದರೆ (rx_ಸಂದೇಶ->ಗಾತ್ರ_2 > tx_ಸಂದೇಶ->ಗಾತ್ರ_2) {
			rx_ಸಂದೇಶ->ಗಾತ್ರ_2 = tx_ಸಂದೇಶ->ಗಾತ್ರ_2;
		}

		/* update data location fields for receiver only */
		rx_ಸಂದೇಶ->tx_ದತ್ತಾಂಶ = tx_ಸಂದೇಶ->tx_ದತ್ತಾಂಶ;

		/* update syncing thread field for receiver only */
		rx_ಸಂದೇಶ->_syncing_thread = tx_ಸಂದೇಶ->_syncing_thread;

		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ -1;
}

/**
 * @brief Dispose of received message.
 *
 * Notifies the sender that message processing is complete.
 *
 * @param rx_msg Pointer to receive message descriptor.
 */
ಸ್ಥಿರ ಶೂನ್ಯ mbox_ಸಂದೇಶ_dispose(ರಚನೆ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ)
{
	ರಚನೆ k_thread *sending_thread;
	ರಚನೆ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ;

	/* do nothing if message was disposed of when it was received */
	ಆದರೆ (rx_ಸಂದೇಶ->_syncing_thread == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	/* recover sender info */
	sending_thread = rx_ಸಂದೇಶ->_syncing_thread;
	rx_ಸಂದೇಶ->_syncing_thread = ಖಾಲಿ;
	tx_ಸಂದೇಶ = (ರಚನೆ k_mbox_ಸಂದೇಶ *)sending_thread->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;

	/* update data size field for sender */
	tx_ಸಂದೇಶ->ಗಾತ್ರ_2 = rx_ಸಂದೇಶ->ಗಾತ್ರ_2;

#ಆದರೆ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	/*
	 * asynchronous send: free asynchronous message descriptor +
	 * dummy thread pair, then give semaphore (if needed)
	 */
	ಆದರೆ (is_thread_dummy(sending_thread)) {
		ರಚನೆ k_sem *async_sem = tx_ಸಂದೇಶ->_async_sem;

		mbox_async_free((ರಚನೆ k_mbox_async *)sending_thread);
		ಆದರೆ (async_sem != ಖಾಲಿ) {
			k_sem_give(async_sem);
		}
		ಹಿಂತಿರುಗಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_NUM_MBOX_ASYNC_MSGS */

	/*
	 * synchronous send: wake up sending thread
	 * Lock scheduler's spinlock as thread internals are being modified.
	 */

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&_sched_spinlock);

	arch_thread_return_ಮೌಲ್ಯ_ಗಣ(sending_thread, 0);
	z_mark_thread_as_not_pending(sending_thread);
	z_sched_ready_locked(sending_thread);

	z_reschedule(&_sched_spinlock, ಕೀಲಿ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mbox_ಸಂದೇಶ_put_walk_op(ರಚನೆ k_thread *thread, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ರಚನೆ mbox_walk_ದತ್ತಾಂಶ *walk_ದತ್ತಾಂಶ = (ರಚನೆ mbox_walk_ದತ್ತಾಂಶ *)ದತ್ತಾಂಶ;
	ರಚನೆ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ;

	rx_ಸಂದೇಶ = (ರಚನೆ k_mbox_ಸಂದೇಶ *)thread->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;

	ಆದರೆ (mbox_ಸಂದೇಶ_ಹೊಂದಿಕೆ(walk_ದತ್ತಾಂಶ->ಸಂದೇಶ, rx_ಸಂದೇಶ) != 0) {
		/* No match. Proceed to next in wait queue */
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);
	(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);
	arch_thread_return_ಮೌಲ್ಯ_ಗಣ(thread, 0);
	z_sched_ready_locked(thread);

#ಆದರೆ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	ಆದರೆ (is_thread_dummy(walk_ದತ್ತಾಂಶ->thread)) {
		ಹಿಂತಿರುಗಿ ASYNCHRONOUS_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ SYNCHRONOUS_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mbox_ಸಂದೇಶ_put(ರಚನೆ k_mbox *mbox, ರಚನೆ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
			     k_timeout_t timeout)
{
	ರಚನೆ k_thread *sending_thread;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	/* save sender id so it can be used during message matching */
	tx_ಸಂದೇಶ->rx_ಮೂಲ_thread = _current;

	/* finish readying sending thread (actual or dummy) for send */
	sending_thread = tx_ಸಂದೇಶ->_syncing_thread;
	sending_thread->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ = tx_ಸಂದೇಶ;

	/* search mailbox's rx queue for a compatible receiver */
	ಕೀಲಿ = k_spin_ಬೀಗ(&mbox->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mbox, ಸಂದೇಶ_put, mbox, timeout);

	ರಚನೆ mbox_walk_ದತ್ತಾಂಶ walk_ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;

	walk_ದತ್ತಾಂಶ.thread = sending_thread;
	walk_ದತ್ತಾಂಶ.ಸಂದೇಶ = tx_ಸಂದೇಶ;

	/* Search the wait queue for the first matching receiver */

	ಸ್ಥಿತಿ_2 = z_sched_waitq_walk(&mbox->rx_ಸಂದೇಶ_ಸರತಿ,
				    mbox_ಸಂದೇಶ_put_walk_op, ಖಾಲಿ,
				    &walk_ದತ್ತಾಂಶ);

	/*
	 * <status> will be one of the following:
	 *   ASYNCHRONOUS_MATCH_FOUND - match found for an asynchronous send
	 *   SYNCHRONOUS_MATCH_FOUND  - match found for a synchronous send
	 *   0                        - no match found
	 */

#ಆದರೆ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	ಆದರೆ (ಸ್ಥಿತಿ_2 == ASYNCHRONOUS_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು) {
		/*
		 * Swap out current thread if receiver has priority, otherwise
		 * let it continue. Note that the dummy sending thread sits
		 * (unqueued) until the receiver consumes the message.
		 */

		z_reschedule(&mbox->ಬೀಗ, ಕೀಲಿ);
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mbox, ಸಂದೇಶ_put, mbox,
					       timeout, 0);
		ಹಿಂತಿರುಗಿ 0;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (ಸ್ಥಿತಿ_2 == SYNCHRONOUS_ಹೊಂದಿಕೆ_ಸಿಕ್ಕಿತು) {
		/*
		 * Pend current thread until the receiver
		 * consumes the message.
		 */

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_mbox, ಸಂದೇಶ_put, mbox,
						   timeout);

		ಪೂರ್ಣಾಂಕ ret = z_pend_curr(&mbox->ಬೀಗ, ಕೀಲಿ, ಖಾಲಿ, K_FOREVER);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mbox, ಸಂದೇಶ_put, mbox,
					       timeout, ret);

		ಹಿಂತಿರುಗಿ ret;
	}

	/* didn't find a matching receiver: don't wait for one */
	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mbox, ಸಂದೇಶ_put, mbox, timeout, -ENOMSG);

		k_spin_unlock(&mbox->ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ -ENOMSG;
	}

#ಆದರೆ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
	/* asynchronous send: dummy thread waits on tx queue for receiver */
	ಆದರೆ (is_thread_dummy(sending_thread)) {
		z_pend_thread(sending_thread, &mbox->tx_ಸಂದೇಶ_ಸರತಿ, K_FOREVER);
		k_spin_unlock(&mbox->ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ 0;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_NUM_MBOX_ASYNC_MSGS */
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_mbox, ಸಂದೇಶ_put, mbox, timeout);

	/* synchronous send: sender waits on tx queue for receiver or timeout */
	ಪೂರ್ಣಾಂಕ ret = z_pend_curr(&mbox->ಬೀಗ, ಕೀಲಿ, &mbox->tx_ಸಂದೇಶ_ಸರತಿ, timeout);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mbox, ಸಂದೇಶ_put, mbox, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_mbox_put(ರಚನೆ k_mbox *mbox, ರಚನೆ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
	       k_timeout_t timeout)
{
	/* configure things for a synchronous send, then send the message */
	tx_ಸಂದೇಶ->_syncing_thread = _current;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mbox, put, mbox, timeout);

	ಪೂರ್ಣಾಂಕ ret = mbox_ಸಂದೇಶ_put(mbox, tx_ಸಂದೇಶ, timeout);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mbox, put, mbox, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

#ಆದರೆ (CONFIG_ಅಂಕೆ_MBOX_ASYNC_MSGS > 0)
ಶೂನ್ಯ k_mbox_async_put(ರಚನೆ k_mbox *mbox, ರಚನೆ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ,
		      ರಚನೆ k_sem *sem)
{
	ರಚನೆ k_mbox_async *async;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mbox, async_put, mbox, sem);

	/*
	 * allocate an asynchronous message descriptor, configure both parts,
	 * then send the message asynchronously
	 */
	mbox_async_alloc(&async);

	async->thread.prio = _current->ತಳ.prio;

	async->tx_ಸಂದೇಶ = *tx_ಸಂದೇಶ;
	async->tx_ಸಂದೇಶ._syncing_thread = (ರಚನೆ k_thread *)&async->thread;
	async->tx_ಸಂದೇಶ._async_sem = sem;

	(ಶೂನ್ಯ)mbox_ಸಂದೇಶ_put(mbox, &async->tx_ಸಂದೇಶ, K_FOREVER);
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mbox, async_put, mbox, sem);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_NUM_MBOX_ASYNC_MSGS */

ಶೂನ್ಯ k_mbox_ದತ್ತಾಂಶ_get(ರಚನೆ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ, ಶೂನ್ಯ *ಬಫರ್)
{
	/* handle case where data is to be discarded */
	ಆದರೆ (ಬಫರ್ == ಖಾಲಿ) {
		rx_ಸಂದೇಶ->ಗಾತ್ರ_2 = 0;
		mbox_ಸಂದೇಶ_dispose(rx_ಸಂದೇಶ);
		ಹಿಂತಿರುಗಿ;
	}

	/* copy message data to buffer, then dispose of message */
	ಆದರೆ ((rx_ಸಂದೇಶ->tx_ದತ್ತಾಂಶ != ಖಾಲಿ) && (rx_ಸಂದೇಶ->ಗಾತ್ರ_2 > 0U)) {
		(ಶೂನ್ಯ)ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್, rx_ಸಂದೇಶ->tx_ದತ್ತಾಂಶ, rx_ಸಂದೇಶ->ಗಾತ್ರ_2);
	}
	mbox_ಸಂದೇಶ_dispose(rx_ಸಂದೇಶ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mbox_ಸಂದೇಶ_ದತ್ತಾಂಶ_check(ರಚನೆ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ, ಶೂನ್ಯ *ಬಫರ್)
{
	ಆದರೆ (ಬಫರ್ != ಖಾಲಿ) {
		/* retrieve data now, then dispose of message */
		k_mbox_ದತ್ತಾಂಶ_get(rx_ಸಂದೇಶ, ಬಫರ್);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rx_ಸಂದೇಶ->ಗಾತ್ರ_2 == 0U) {
		/* there is no data to get, so just dispose of message */
		mbox_ಸಂದೇಶ_dispose(rx_ಸಂದೇಶ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* keep message around for later data retrieval */
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mbox_get_walk_op(ರಚನೆ k_thread *thread, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ರಚನೆ mbox_walk_ದತ್ತಾಂಶ *walk_ದತ್ತಾಂಶ = (ರಚನೆ mbox_walk_ದತ್ತಾಂಶ *)ದತ್ತಾಂಶ;
	ರಚನೆ k_mbox_ಸಂದೇಶ *tx_ಸಂದೇಶ;

	tx_ಸಂದೇಶ = (ರಚನೆ k_mbox_ಸಂದೇಶ *)thread->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;

	ಆದರೆ (mbox_ಸಂದೇಶ_ಹೊಂದಿಕೆ(tx_ಸಂದೇಶ, walk_ದತ್ತಾಂಶ->ಸಂದೇಶ) != 0) {
		/* No match. Proceed to next in wait queue */
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Final step in the walk. Remove thread from wait queue. */

	unpend_thread_no_timeout(thread);

	(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);

	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ k_mbox_get(ರಚನೆ k_mbox *mbox, ರಚನೆ k_mbox_ಸಂದೇಶ *rx_ಸಂದೇಶ, ಶೂನ್ಯ *ಬಫರ್,
	       k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	/* save receiver id so it can be used during message matching */
	rx_ಸಂದೇಶ->tx_ಗುರಿ_thread = _current;

	/* search mailbox's tx queue for a compatible sender */
	ಕೀಲಿ = k_spin_ಬೀಗ(&mbox->ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mbox, get, mbox, timeout);

	ರಚನೆ mbox_walk_ದತ್ತಾಂಶ walk_ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;

	walk_ದತ್ತಾಂಶ.thread = ಖಾಲಿ;   /* Unused by walk operation */
	walk_ದತ್ತಾಂಶ.ಸಂದೇಶ = rx_ಸಂದೇಶ;

	/* Search the wait queue for the first matching sender */

	ಸ್ಥಿತಿ_2 = z_sched_waitq_walk(&mbox->tx_ಸಂದೇಶ_ಸರತಿ,
				    mbox_get_walk_op, ಖಾಲಿ, &walk_ದತ್ತಾಂಶ);

	ಆದರೆ (ಸ್ಥಿತಿ_2 != 0) {
		/* Matching sender found */
		k_spin_unlock(&mbox->ಬೀಗ, ಕೀಲಿ);
		ಫಲಿತಾಂಶ = 0;

		ಹೋಗು ಹೊರಹೋಗು;
	}

	/* didn't find a matching sender */

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a matching sender to appear */
		k_spin_unlock(&mbox->ಬೀಗ, ಕೀಲಿ);

		ಫಲಿತಾಂಶ = -ENOMSG;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_mbox, get, mbox, timeout);

	/* wait until a matching sender appears or a timeout occurs */
	_current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ = rx_ಸಂದೇಶ;
	ಫಲಿತಾಂಶ = z_pend_curr(&mbox->ಬೀಗ, ಕೀಲಿ, &mbox->rx_ಸಂದೇಶ_ಸರತಿ, timeout);

ಹೊರಹೋಗು:
	/* consume message data immediately, if needed */
	ಆದರೆ (ಫಲಿತಾಂಶ == 0) {
		ಫಲಿತಾಂಶ = mbox_ಸಂದೇಶ_ದತ್ತಾಂಶ_check(rx_ಸಂದೇಶ, ಬಫರ್);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mbox, get, mbox, timeout, ಫಲಿತಾಂಶ);

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}
