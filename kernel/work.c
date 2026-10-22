/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * Second generation work queue implementation
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/printk.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ flag_clear(uint32_t *flagp,
			      uint32_t ਦੋਆਧਾਰੀ_ਅੰਕ_2)
{
	*flagp &= ~ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_2);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ flag_ਸੈੱਟ(uint32_t *flagp,
			    uint32_t ਦੋਆਧਾਰੀ_ਅੰਕ_2)
{
	*flagp |= ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_2);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool flag_test(ਅਚਲ uint32_t *flagp,
			     uint32_t ਦੋਆਧਾਰੀ_ਅੰਕ_2)
{
	ਵਾਪਸ (*flagp & ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_2)) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool flag_test_and_clear(uint32_t *flagp,
				       ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ_2)
{
	bool ret = flag_test(flagp, ਦੋਆਧਾਰੀ_ਅੰਕ_2);

	flag_clear(flagp, ਦੋਆਧਾਰੀ_ਅੰਕ_2);

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ flags_ਸੈੱਟ(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t flags_get(ਅਚਲ uint32_t *flagp)
{
	ਵਾਪਸ *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
ਸਥਿਰ ਬਣਤਰ k_spinlock work_ਤਾਲਾ;

/* Invoked by work thread */
ਸਥਿਰ ਖਾਲੀ handle_flush(ਬਣਤਰ k_work *work) { }

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_flusher(ਬਣਤਰ z_work_flusher *flusher)
{
	ਬਣਤਰ k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_ਸੈੱਟ(&work->flags, K_WORK_FLUSHING_ਦੋਆਧਾਰੀ_ਅੰਕ);
}

/* List of pending cancellations. */
ਸਥਿਰ sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_work_cancel(ਬਣਤਰ z_work_canceller *canceler,
				    ਬਣਤਰ k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->ਗੰਢ);
}

/* Complete flushing of a work item.
 *
 * Invoked with work lock held.
 *
 * Invoked from a work queue thread.
 *
 * Reschedules.
 *
 * @param work the work structure that has completed flushing.
 */
ਸਥਿਰ ਖਾਲੀ finalize_flush_locked(ਬਣਤਰ k_work *work)
{
	ਬਣਤਰ z_work_flusher *flusher
		= CONTAINER_ਦਾ(work, ਬਣਤਰ z_work_flusher, work);

	flag_clear(&work->flags, K_WORK_FLUSHING_ਦੋਆਧਾਰੀ_ਅੰਕ);

	k_sem_give(&flusher->sem);
};

/* Complete cancellation of a work item and unlock held lock.
 *
 * Invoked with work lock held.
 *
 * Invoked from a work queue thread.
 *
 * Reschedules.
 *
 * @param work the work structure that has completed cancellation
 */
ਸਥਿਰ ਖਾਲੀ finalize_cancel_locked(ਬਣਤਰ k_work *work)
{
	ਬਣਤਰ z_work_canceller *wc, *tmp;
	sys_snode_t *prev = ਨਿਰਾ;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_ਦੋਆਧਾਰੀ_ਅੰਕ);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_ਸੁਰੱਖਿਅਤ(&pending_cancels, wc, tmp, ਗੰਢ) {
		ਜੇ (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->ਗੰਢ);
			k_sem_give(&wc->sem);
		} ਨਹੀਂਤਾਂ {
			prev = &wc->ਗੰਢ;
		}
	}
}

ਖਾਲੀ k_work_init(ਬਣਤਰ k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != ਨਿਰਾ);
	__ASSERT_NO_MSG(handler != ਨਿਰਾ);

	*work = (ਬਣਤਰ k_work)Z_WORK_INITIALIZER(handler);

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_work, work);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ work_busy_get_locked(ਅਚਲ ਬਣਤਰ k_work *work)
{
	ਵਾਪਸ flags_get(&work->flags) & K_WORK_ਨਕਾਬ;
}

ਪੂਰਨ k_work_busy_get(ਅਚਲ ਬਣਤਰ k_work *work)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
	ਪੂਰਨ ret = work_busy_get_locked(work);

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ ret;
}

/* Add a flusher work item to the queue.
 *
 * Invoked with work lock held.
 *
 * Caller must notify queue of pending work.
 *
 * @param queue queue on which a work item may appear.
 * @param work the work item that is either queued or running on @p
 * queue
 * @param flusher an uninitialized/unused flusher object
 */
ਸਥਿਰ ਖਾਲੀ ਕਤਾਰ_flusher_locked(ਬਣਤਰ k_work_q *ਕਤਾਰ,
				 ਬਣਤਰ k_work *work,
				 ਬਣਤਰ z_work_flusher *flusher)
{
	init_flusher(flusher);

	ਜੇ ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_ਪਾਓ(&ਕਤਾਰ->pending, &work->ਗੰਢ,
				 &flusher->work.ਗੰਢ);
	} ਨਹੀਂਤਾਂ {
		sys_slist_prepend(&ਕਤਾਰ->pending, &flusher->work.ਗੰਢ);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਕਤਾਰ_remove_locked(ਬਣਤਰ k_work_q *ਕਤਾਰ,
				       ਬਣਤਰ k_work *work)
{
	ਜੇ (flag_test_and_clear(&work->flags, K_WORK_QUEUED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		(ਖਾਲੀ)sys_slist_ਲੱਭੋ_and_remove(&ਕਤਾਰ->pending, &work->ਗੰਢ);
	}
}

/* Potentially notify a queue that it needs to look for pending work.
 *
 * This may make the work queue thread ready, but as the lock is held it
 * will not be a reschedule point.  Callers should yield after the lock is
 * released where appropriate (generally if this returns true).
 *
 * @param queue to be notified.  If this is null no notification is required.
 *
 * @return true if and only if the queue was notified and woken, i.e. a
 * reschedule is pending.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool notify_ਕਤਾਰ_locked(ਬਣਤਰ k_work_q *ਕਤਾਰ)
{
	bool rv = false;

	ਜੇ (ਕਤਾਰ != ਨਿਰਾ) {
		rv = z_sched_wake(&ਕਤਾਰ->notifyq, 0, ਨਿਰਾ);
	}

	ਵਾਪਸ rv;
}

/* Submit an work item to a queue if queue state allows new work.
 *
 * Submission is rejected if no queue is provided, or if the queue is
 * draining and the work isn't being submitted from the queue's
 * thread (chained submission).
 *
 * Invoked with work lock held.
 * Conditionally notifies queue.
 *
 * @param queue the queue to which work should be submitted.  This may
 * be null, in which case the submission will fail.
 *
 * @param work to be submitted
 *
 * @retval 1 if successfully queued
 * @retval -EINVAL if no queue is provided
 * @retval -ENODEV if the queue is not started
 * @retval -EBUSY if the submission was rejected (draining, plugged)
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ਕਤਾਰ_submit_locked(ਬਣਤਰ k_work_q *ਕਤਾਰ,
				      ਬਣਤਰ k_work *work)
{
	ਜੇ (ਕਤਾਰ == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}

	ਪੂਰਨ ret;
	bool chained = (_current == ਕਤਾਰ->thread_id) && !k_is_ਵਿੱਚ_isr();
	bool draining = flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_DRAIN_ਦੋਆਧਾਰੀ_ਅੰਕ);
	bool plugged = flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_PLUGGED_ਦੋਆਧਾਰੀ_ਅੰਕ);

	/* Test for acceptability, in priority order:
	 *
	 * * -ENODEV if the queue isn't running.
	 * * -EBUSY if draining and not chained
	 * * -EBUSY if plugged and not draining
	 * * otherwise OK
	 */
	ਜੇ (!flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_STARTED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		ret = -ENODEV;
	} ਨਹੀਂਤਾਂ ਜੇ (draining && !chained) {
		ret = -EBUSY;
	} ਨਹੀਂਤਾਂ ਜੇ (plugged && !draining) {
		ret = -EBUSY;
	} ਨਹੀਂਤਾਂ {
		sys_slist_append(&ਕਤਾਰ->pending, &work->ਗੰਢ);
		ret = 1;
		(ਖਾਲੀ)notify_ਕਤਾਰ_locked(ਕਤਾਰ);
	}

	ਵਾਪਸ ret;
}

/* Attempt to submit work to a queue.
 *
 * The submission can fail if:
 * * the work is cancelling,
 * * no candidate queue can be identified;
 * * the candidate queue rejects the submission.
 *
 * Invoked with work lock held.
 * Conditionally notifies queue.
 *
 * @param work the work structure to be submitted

 * @param queuep pointer to a queue reference.  On input this should
 * dereference to the proposed queue (which may be null); after completion it
 * will be null if the work was not submitted or if submitted will reference
 * the queue it was submitted to.  That may or may not be the queue provided
 * on input.
 *
 * @retval 0 if work was already submitted to a queue
 * @retval 1 if work was not submitted and has been queued to @p queue
 * @retval 2 if work was running and has been queued to the queue that was
 * running it
 * @retval -EBUSY if canceling or submission was rejected by queue
 * @retval -EINVAL if no queue is provided
 * @retval -ENODEV if the queue is not started
 */
ਸਥਿਰ ਪੂਰਨ submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ_locked(ਬਣਤਰ k_work *work,
				  ਬਣਤਰ k_work_q **queuep)
{
	ਪੂਰਨ ret = 0;

	ਜੇ (flag_test(&work->flags, K_WORK_CANCELING_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		/* Disallowed */
		ret = -EBUSY;
	} ਨਹੀਂਤਾਂ ਜੇ (!flag_test(&work->flags, K_WORK_QUEUED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		ਜੇ (*queuep == ਨਿਰਾ) {
			*queuep = work->ਕਤਾਰ;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		ਜੇ (flag_test(&work->flags, K_WORK_RUNNING_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
			__ASSERT_NO_MSG(work->ਕਤਾਰ != ਨਿਰਾ);
			*queuep = work->ਕਤਾਰ;
			ret = 2;
		}

		ਪੂਰਨ rc = ਕਤਾਰ_submit_locked(*queuep, work);

		ਜੇ (rc < 0) {
			ret = rc;
		} ਨਹੀਂਤਾਂ {
			flag_ਸੈੱਟ(&work->flags, K_WORK_QUEUED_ਦੋਆਧਾਰੀ_ਅੰਕ);
			work->ਕਤਾਰ = *queuep;
		}
	} ਨਹੀਂਤਾਂ {
		/* Already queued, do nothing. */
	}

	ਜੇ (ret <= 0) {
		*queuep = ਨਿਰਾ;
	}

	ਵਾਪਸ ret;
}

/* Submit work to a queue
 *
 * Intended for internal use.
 *
 * See also submit_to_queue_locked().
 *
 * @param queuep pointer to a queue reference.
 * @param work the work structure to be submitted
 *
 * @retval see submit_to_queue_locked()
 */
ਪੂਰਨ z_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(ਬਣਤਰ k_work_q *ਕਤਾਰ,
		  ਬਣਤਰ k_work *work)
{
	__ASSERT_NO_MSG(work != ਨਿਰਾ);
	__ASSERT_NO_MSG(work->handler != ਨਿਰਾ);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	ਪੂਰਨ ret = submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ_locked(work, &ਕਤਾਰ);

	/* No need to reschedule if the submission failed. */
	ਜੇ (ret > 0) {
		z_reschedule(&work_ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(ਬਣਤਰ k_work_q *ਕਤਾਰ,
			    ਬਣਤਰ k_work *work)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ, ਕਤਾਰ, work);

	ਪੂਰਨ ret = z_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(ਕਤਾਰ, work);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ, ਕਤਾਰ, work, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_submit(ਬਣਤਰ k_work *work)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	ਪੂਰਨ ret = z_work_submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ(&k_sys_work_q, work);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	ਵਾਪਸ ret;
}

/* Flush the work item if necessary.
 *
 * Flushing is necessary only if the work is either queued or running.
 *
 * Invoked with work lock held by key.
 * Sleeps.
 *
 * @param work the work item that is to be flushed
 * @param flusher state used to synchronize the flush
 *
 * @retval true if work is queued or running.  If this happens the
 * caller must take the flusher semaphore after releasing the lock.
 *
 * @retval false otherwise.  No wait required.
 */
ਸਥਿਰ bool work_flush_locked(ਬਣਤਰ k_work *work,
			      ਬਣਤਰ z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	ਜੇ (need_flush) {
		ਬਣਤਰ k_work_q *ਕਤਾਰ = work->ਕਤਾਰ;

		__ASSERT_NO_MSG(ਕਤਾਰ != ਨਿਰਾ);

		ਕਤਾਰ_flusher_locked(ਕਤਾਰ, work, flusher);
		notify_ਕਤਾਰ_locked(ਕਤਾਰ);
	}

	ਵਾਪਸ need_flush;
}

bool k_work_flush(ਬਣਤਰ k_work *work,
		  ਬਣਤਰ k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ਨਿਰਾ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ਦੋਆਧਾਰੀ_ਅੰਕ));
	__ASSERT_NO_MSG(!k_is_ਵਿੱਚ_isr());
	__ASSERT_NO_MSG(sync != ਨਿਰਾ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	ਬਣਤਰ z_work_flusher *flusher = &sync->flusher;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	/* If necessary wait until the flusher item completes */
	ਜੇ (need_flush) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	ਵਾਪਸ need_flush;
}

/* Execute the non-waiting steps necessary to cancel a work item.
 *
 * Invoked with work lock held.
 *
 * @param work the work item to be canceled.
 *
 * @retval true if we need to wait for the work item to finish canceling
 * @retval false if the work item is idle
 *
 * @return k_busy_wait() captured under lock
 */
ਸਥਿਰ ਪੂਰਨ cancel_async_locked(ਬਣਤਰ k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	ਜੇ (!flag_test(&work->flags, K_WORK_CANCELING_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		/* Remove it from the queue, if it's queued. */
		ਕਤਾਰ_remove_locked(work->ਕਤਾਰ, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	ਪੂਰਨ ret = work_busy_get_locked(work);

	ਜੇ (ret != 0) {
		flag_ਸੈੱਟ(&work->flags, K_WORK_CANCELING_ਦੋਆਧਾਰੀ_ਅੰਕ);
		ret = work_busy_get_locked(work);
	}

	ਵਾਪਸ ret;
}

/* Complete cancellation necessary, release work lock, and wait if
 * necessary.
 *
 * Invoked with work lock held by key.
 * Sleeps.
 *
 * @param work work that is being canceled
 * @param canceller state used to synchronize the cancellation
 * @param key used by work lock
 *
 * @retval true if and only if the work was still active on entry.  The caller
 * must wait on the canceller semaphore after releasing the lock.
 *
 * @retval false if work was idle on entry.  The caller need not wait.
 */
ਸਥਿਰ bool cancel_sync_locked(ਬਣਤਰ k_work *work,
			       ਬਣਤਰ z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_ਦੋਆਧਾਰੀ_ਅੰਕ);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	ਜੇ (ret) {
		init_work_cancel(canceller, work);
	}

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_cancel(ਬਣਤਰ k_work *work)
{
	__ASSERT_NO_MSG(work != ਨਿਰਾ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ਦੋਆਧਾਰੀ_ਅੰਕ));

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
	ਪੂਰਨ ret = cancel_async_locked(work);

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	ਵਾਪਸ ret;
}

bool k_work_cancel_sync(ਬਣਤਰ k_work *work,
			ਬਣਤਰ k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ਨਿਰਾ);
	__ASSERT_NO_MSG(sync != ਨਿਰਾ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ਦੋਆਧਾਰੀ_ਅੰਕ));
	__ASSERT_NO_MSG(!k_is_ਵਿੱਚ_isr());
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	ਬਣਤਰ z_work_canceller *canceller = &sync->canceller;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	ਜੇ (pending) {
		(ਖਾਲੀ)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	ਜੇ (need_wait) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	ਵਾਪਸ pending;
}

#ਜੇ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
ਸਥਿਰ ਖਾਲੀ work_timeout_handler(ਬਣਤਰ _timeout *ਰਿਕਾਰਡ)
{
	ਬਣਤਰ k_work_q *ਕਤਾਰ = CONTAINER_ਦਾ(ਰਿਕਾਰਡ, ਬਣਤਰ k_work_q, work_timeout_ਰਿਕਾਰਡ);
	ਬਣਤਰ k_work *work = ਨਿਰਾ;
	k_work_handler_t handler = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *name;
	ਅਚਲ ਅੱਖਰ *space = " ";
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	work = ਕਤਾਰ->work;
	handler = work->handler;

	/*
	 * The work item may be running on a different CPU than the timeout
	 * handler. Two conditions must be checked.
	 *  1. Did the work item finish before the timeout handler got to run?
	 *  2. Was the timeout aborted while the handler was in flight?
	 * queue->finished alone is insufficient for (2): if the work thread
	 * completes the item and starts a new one before this handler wins
	 * <lock>, finished is reset and the stale handler would abort the
	 * wrong worker. work_timeout_stop_locked() aborts the timeout, which
	 * flags it superseded; bail on that too.
	 */
	ਜੇ (ਕਤਾਰ->ਮੁਕੰਮਲ || z_timeout_inflight_superseded(ਰਿਕਾਰਡ)) {
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}
	ਕਤਾਰ->ਮੁਕੰਮਲ = true;

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	name = k_thread_name_get(ਕਤਾਰ->thread_id);
	ਜੇ (name == ਨਿਰਾ) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		ਕਤਾਰ, space, name, work, handler);

	k_thread_abort(ਕਤਾਰ->thread_id);
}

ਸਥਿਰ ਖਾਲੀ work_timeout_ਸ਼ੁਰੂ_locked(ਬਣਤਰ k_work_q *ਕਤਾਰ, ਬਣਤਰ k_work *work)
{
	ਕਤਾਰ->ਮੁਕੰਮਲ = false;

	ਜੇ (K_TIMEOUT_EQ(ਕਤਾਰ->work_timeout, K_FOREVER)) {
		ਵਾਪਸ;
	}

	ਕਤਾਰ->work = work;
	z_ਜੋੜੋ_timeout(&ਕਤਾਰ->work_timeout_ਰਿਕਾਰਡ, work_timeout_handler, ਕਤਾਰ->work_timeout);
}

ਸਥਿਰ ਖਾਲੀ work_timeout_stop_locked(ਬਣਤਰ k_work_q *ਕਤਾਰ)
{
	ਜੇ (K_TIMEOUT_EQ(ਕਤਾਰ->work_timeout, K_FOREVER)) {
		ਵਾਪਸ;
	}

	(ਖਾਲੀ)z_try_abort_timeout(&ਕਤਾਰ->work_timeout_ਰਿਕਾਰਡ);
}
#ਜੇ_ਅੰਤ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
ਸਥਿਰ ਖਾਲੀ work_ਕਤਾਰ_main(ਖਾਲੀ *workq_ptr, ਖਾਲੀ *p2, ਖਾਲੀ *p3)
{
	ARG_ਅਣਵਰਤਿਆ(p2);
	ARG_ਅਣਵਰਤਿਆ(p3);

	ਬਣਤਰ k_work_q *ਕਤਾਰ = (ਬਣਤਰ k_work_q *)workq_ptr;

	ਜਦੋਂ (true) {
		sys_snode_t *ਗੰਢ;
		ਬਣਤਰ k_work *work = ਨਿਰਾ;
		k_work_handler_t handler = ਨਿਰਾ;
		k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
		bool yield;

		/* Check for and prepare any new work. */
		ਗੰਢ = sys_slist_get(&ਕਤਾਰ->pending);
		ਜੇ (ਗੰਢ != ਨਿਰਾ) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_ਸੈੱਟ(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_BUSY_ਦੋਆਧਾਰੀ_ਅੰਕ);
			work = CONTAINER_ਦਾ(ਗੰਢ, ਬਣਤਰ k_work, ਗੰਢ);
			flag_ਸੈੱਟ(&work->flags, K_WORK_RUNNING_ਦੋਆਧਾਰੀ_ਅੰਕ);
			flag_clear(&work->flags, K_WORK_QUEUED_ਦੋਆਧਾਰੀ_ਅੰਕ);

			/* Static code analysis tool can raise a false-positive violation
			 * in the line below that 'work' is checked for null after being
			 * dereferenced.
			 *
			 * The work is figured out by CONTAINER_OF, as a container
			 * of type struct k_work that contains the node.
			 * The only way for it to be NULL is if node would be a member
			 * of struct k_work object that has been placed at address NULL,
			 * which should never happen, even line 'if (work != NULL)'
			 * ensures that.
			 * This means that if node is not NULL, then work will not be NULL.
			 */
			handler = work->handler;
		} ਨਹੀਂਤਾਂ ਜੇ (flag_test_and_clear(&ਕਤਾਰ->flags,
					       K_WORK_ਕਤਾਰ_DRAIN_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
			/* Not busy and draining: move threads waiting for
			 * drain to ready state.  The held spinlock inhibits
			 * immediate reschedule; released threads get their
			 * chance when this invokes z_sched_wait() below.
			 *
			 * We don't touch K_WORK_QUEUE_PLUGGABLE, so getting
			 * here doesn't mean that the queue will allow new
			 * submissions.
			 */
			(ਖਾਲੀ)z_sched_wake_all(&ਕਤਾਰ->drainq, 1, ਨਿਰਾ);
		} ਨਹੀਂਤਾਂ ਜੇ (flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_STOP_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_ਸੈੱਟ(&ਕਤਾਰ->flags, 0);
			k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
			ਵਾਪਸ;
		} ਨਹੀਂਤਾਂ {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		ਜੇ (work == ਨਿਰਾ) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(ਖਾਲੀ)z_sched_wait(&work_ਤਾਲਾ, ਕੁੰਜੀ, &ਕਤਾਰ->notifyq,
					   K_FOREVER, ਨਿਰਾ);
			ਜਾਰੀ;
		}

#ਜੇ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_ਸ਼ੁਰੂ_locked(ਕਤਾਰ, work);
#ਜੇ_ਅੰਤ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

		__ASSERT_NO_MSG(handler != ਨਿਰਾ);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

#ਜੇ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		ਜੇ (ਕਤਾਰ->ਮੁਕੰਮਲ) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
			ਜਦੋਂ (1) {
				k_sleep(K_FOREVER);
			}
			ਕੋਡ_UNREACHABLE;
		}
		work_timeout_stop_locked(ਕਤਾਰ);
		ਕਤਾਰ->ਮੁਕੰਮਲ = true;
#ਜੇ_ਅੰਤ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_ਦੋਆਧਾਰੀ_ਅੰਕ);
		ਜੇ (flag_test(&work->flags, K_WORK_FLUSHING_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
			finalize_flush_locked(work);
		}
		ਜੇ (flag_test(&work->flags, K_WORK_CANCELING_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_BUSY_ਦੋਆਧਾਰੀ_ਅੰਕ);
		yield = (!flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_NO_YIELD_ਦੋਆਧਾਰੀ_ਅੰਕ) &&
			 !sys_slist_is_ਖਾਲੀ_ਥਾਵਾਂ(&ਕਤਾਰ->pending));
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		ਜੇ (yield) {
			k_yield();
		}
	}
}

ਖਾਲੀ k_work_ਕਤਾਰ_init(ਬਣਤਰ k_work_q *ਕਤਾਰ)
{
	__ASSERT_NO_MSG(ਕਤਾਰ != ਨਿਰਾ);

	*ਕਤਾਰ = (ਬਣਤਰ k_work_q) {
		.flags = 0,
	};

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_work_ਕਤਾਰ, ਕਤਾਰ);
}

ਖਾਲੀ k_work_ਕਤਾਰ_ਲੜੀਵਾਰ(ਬਣਤਰ k_work_q *ਕਤਾਰ, ਅਚਲ ਬਣਤਰ k_work_ਕਤਾਰ_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_STARTED_ਦੋਆਧਾਰੀ_ਅੰਕ));

	uint32_t flags = K_WORK_ਕਤਾਰ_STARTED;

	ਜੇ ((cfg != ਨਿਰਾ) && cfg->no_yield) {
		flags |= K_WORK_ਕਤਾਰ_NO_YIELD;
	}

	ਜੇ ((cfg != ਨਿਰਾ) && (cfg->name != ਨਿਰਾ)) {
		k_thread_name_ਸੈੱਟ(_current, cfg->name);
	}

#ਜੇ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ਜੇ ((cfg != ਨਿਰਾ) && (cfg->work_timeout_ms)) {
		ਕਤਾਰ->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ਨਹੀਂਤਾਂ {
		ਕਤਾਰ->work_timeout = K_FOREVER;
	}
#ਜੇ_ਅੰਤ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&ਕਤਾਰ->pending);
	z_waitq_init(&ਕਤਾਰ->notifyq);
	z_waitq_init(&ਕਤਾਰ->drainq);
	ਕਤਾਰ->thread_id = _current;
	flags_ਸੈੱਟ(&ਕਤਾਰ->flags, flags);
	work_ਕਤਾਰ_main(ਕਤਾਰ, ਨਿਰਾ, ਨਿਰਾ);
}

ਖਾਲੀ k_work_ਕਤਾਰ_ਸ਼ੁਰੂ(ਬਣਤਰ k_work_q *ਕਤਾਰ,
			k_thread_ਢੇਰ_t *ਢੇਰ,
			size_t ਢੇਰ_ਮਾਪ,
			ਪੂਰਨ prio,
			ਅਚਲ ਬਣਤਰ k_work_ਕਤਾਰ_config *cfg)
{
	__ASSERT_NO_MSG(ਕਤਾਰ);
	__ASSERT_NO_MSG(ਢੇਰ);
	__ASSERT_NO_MSG(!flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_STARTED_ਦੋਆਧਾਰੀ_ਅੰਕ));

	/* In future, this whole function will be deprecated, but for now, we
	 * have to use the `thread` field to create a new thread in it.
	 */
	TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);

	uint32_t flags = K_WORK_ਕਤਾਰ_STARTED;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work_ਕਤਾਰ, ਸ਼ੁਰੂ, ਕਤਾਰ);

	sys_slist_init(&ਕਤਾਰ->pending);
	z_waitq_init(&ਕਤਾਰ->notifyq);
	z_waitq_init(&ਕਤਾਰ->drainq);

	ਜੇ ((cfg != ਨਿਰਾ) && cfg->no_yield) {
		flags |= K_WORK_ਕਤਾਰ_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_ਸੈੱਟ(&ਕਤਾਰ->flags, flags);

	(ਖਾਲੀ)k_thread_create(&ਕਤਾਰ->thread, ਢੇਰ, ਢੇਰ_ਮਾਪ,
			      work_ਕਤਾਰ_main, ਕਤਾਰ, ਨਿਰਾ, ਨਿਰਾ,
			      prio, 0, K_FOREVER);

	ਜੇ ((cfg != ਨਿਰਾ) && (cfg->name != ਨਿਰਾ)) {
		k_thread_name_ਸੈੱਟ(&ਕਤਾਰ->thread, cfg->name);
	}

	ਜੇ ((cfg != ਨਿਰਾ) && (cfg->essential)) {
		ਕਤਾਰ->thread.ਅਧਾਰ.user_options |= K_ESSENTIAL;
	}

#ਜੇ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ਜੇ ((cfg != ਨਿਰਾ) && (cfg->work_timeout_ms)) {
		ਕਤਾਰ->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ਨਹੀਂਤਾਂ {
		ਕਤਾਰ->work_timeout = K_FOREVER;
	}
#ਜੇ_ਅੰਤ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_ਸ਼ੁਰੂ(&ਕਤਾਰ->thread);
	ਕਤਾਰ->thread_id = &ਕਤਾਰ->thread;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_ਕਤਾਰ, ਸ਼ੁਰੂ, ਕਤਾਰ);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

ਪੂਰਨ k_work_ਕਤਾਰ_drain(ਬਣਤਰ k_work_q *ਕਤਾਰ,
		       bool plug)
{
	__ASSERT_NO_MSG(ਕਤਾਰ);
	__ASSERT_NO_MSG(!k_is_ਵਿੱਚ_isr());

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work_ਕਤਾਰ, drain, ਕਤਾਰ);

	ਪੂਰਨ ret = 0;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	ਜੇ (((flags_get(&ਕਤਾਰ->flags)
	      & (K_WORK_ਕਤਾਰ_BUSY | K_WORK_ਕਤਾਰ_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_ਖਾਲੀ_ਥਾਵਾਂ(&ਕਤਾਰ->pending)) {
		flag_ਸੈੱਟ(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_DRAIN_ਦੋਆਧਾਰੀ_ਅੰਕ);
		ਜੇ (plug) {
			flag_ਸੈੱਟ(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_PLUGGED_ਦੋਆਧਾਰੀ_ਅੰਕ);
		}

		notify_ਕਤਾਰ_locked(ਕਤਾਰ);
		ret = z_sched_wait(&work_ਤਾਲਾ, ਕੁੰਜੀ, &ਕਤਾਰ->drainq,
				   K_FOREVER, ਨਿਰਾ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_ਕਤਾਰ, drain, ਕਤਾਰ, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_ਕਤਾਰ_unplug(ਬਣਤਰ k_work_q *ਕਤਾਰ)
{
	__ASSERT_NO_MSG(ਕਤਾਰ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work_ਕਤਾਰ, unplug, ਕਤਾਰ);

	ਪੂਰਨ ret = -EALREADY;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	ਜੇ (flag_test_and_clear(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_PLUGGED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		ret = 0;
	}

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_ਕਤਾਰ, unplug, ਕਤਾਰ, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_ਕਤਾਰ_stop(ਬਣਤਰ k_work_q *ਕਤਾਰ, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(ਕਤਾਰ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work_ਕਤਾਰ, stop, ਕਤਾਰ, timeout);

	ਜੇ (ਕਤਾਰ->thread_id) {
		ਜੇ (z_is_thread_essential(ਕਤਾਰ->thread_id)) {
			ਵਾਪਸ -ENOTSUP;
		}
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	ਜੇ (!flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_STARTED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_ਕਤਾਰ, stop, ਕਤਾਰ, timeout, -EALREADY);
		ਵਾਪਸ -EALREADY;
	}

	ਜੇ (!flag_test(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_PLUGGED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_ਕਤਾਰ, stop, ਕਤਾਰ, timeout, -EBUSY);
		ਵਾਪਸ -EBUSY;
	}

	flag_ਸੈੱਟ(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_STOP_ਦੋਆਧਾਰੀ_ਅੰਕ);
	notify_ਕਤਾਰ_locked(ਕਤਾਰ);
	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_work_ਕਤਾਰ, stop, ਕਤਾਰ, timeout);
	ਜੇ (k_thread_join(ਕਤਾਰ->thread_id, timeout)) {
		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
		flag_clear(&ਕਤਾਰ->flags, K_WORK_ਕਤਾਰ_STOP_ਦੋਆਧਾਰੀ_ਅੰਕ);
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_ਕਤਾਰ, stop, ਕਤਾਰ, timeout, -ETIMEDOUT);
		ਵਾਪਸ -ETIMEDOUT;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work_ਕਤਾਰ, stop, ਕਤਾਰ, timeout, 0);
	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
ਸਥਿਰ ਖਾਲੀ work_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਬਣਤਰ k_work_delayable *dw
		= CONTAINER_ਦਾ(ਮੰਜ਼ਿਲ, ਬਣਤਰ k_work_delayable, timeout);
	ਬਣਤਰ k_work *wp = &dw->work;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
	ਬਣਤਰ k_work_q *ਕਤਾਰ = ਨਿਰਾ;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	ਜੇ (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		ਕਤਾਰ = dw->ਕਤਾਰ;
		(ਖਾਲੀ)submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ_locked(wp, &ਕਤਾਰ);
	}

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਖਾਲੀ k_work_init_delayable(ਬਣਤਰ k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != ਨਿਰਾ);
	__ASSERT_NO_MSG(handler != ਨਿਰਾ);

	*dwork = (ਬਣਤਰ k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ work_delayable_busy_get_locked(ਅਚਲ ਬਣਤਰ k_work_delayable *dwork)
{
	ਵਾਪਸ flags_get(&dwork->work.flags) & K_WORK_ਨਕਾਬ;
}

ਪੂਰਨ k_work_delayable_busy_get(ਅਚਲ ਬਣਤਰ k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ਨਿਰਾ);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
	ਪੂਰਨ ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);
	ਵਾਪਸ ret;
}

/* Attempt to schedule a work item for future (maybe immediate)
 * submission.
 *
 * Invoked with work lock held.
 *
 * See also submit_to_queue_locked(), which implements this for a no-wait
 * delay.
 *
 * Invoked with work lock held.
 *
 * @param queuep pointer to a pointer to a queue.  On input this
 * should dereference to the proposed queue (which may be null); after
 * completion it will be null if the work was not submitted or if
 * submitted will reference the queue it was submitted to.  That may
 * or may not be the queue provided on input.
 *
 * @param dwork the delayed work structure
 *
 * @param delay the delay to use before scheduling.
 *
 * @retval from submit_to_queue_locked() if delay is K_NO_WAIT; otherwise
 * @retval 1 to indicate successfully scheduled.
 */
ਸਥਿਰ ਪੂਰਨ ਸਮਾਂ_ਸਾਰਣੀ_for_ਕਤਾਰ_locked(ਬਣਤਰ k_work_q **queuep,
				     ਬਣਤਰ k_work_delayable *dwork,
				     k_timeout_t delay)
{
	ਪੂਰਨ ret = 1;
	ਬਣਤਰ k_work *work = &dwork->work;

	ਜੇ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		ਵਾਪਸ submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ_locked(work, queuep);
	}

	flag_ਸੈੱਟ(&work->flags, K_WORK_DELAYED_ਦੋਆਧਾਰੀ_ਅੰਕ);
	dwork->ਕਤਾਰ = *queuep;

	/* Add timeout */
	z_ਜੋੜੋ_timeout(&dwork->timeout, work_timeout, delay);

	ਵਾਪਸ ret;
}

/* Unschedule delayable work.
 *
 * If the work is delayed, cancel the timeout and clear the delayed
 * flag.
 *
 * Invoked with work lock held.
 *
 * @param dwork pointer to delayable work structure.
 *
 * @return true if and only if work had been delayed so the timeout
 * was cancelled.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool unschedule_locked(ਬਣਤਰ k_work_delayable *dwork,
				     k_spinlock_ਕੁੰਜੀ_t *ਕੁੰਜੀ)
{
	bool ret = false;
	ਬਣਤਰ k_work *work = &dwork->work;

	/* If scheduled, try to cancel.  If it fails, that means the
	 * callback has been dequeued and will inevitably run (or has
	 * already run), so treat that as "undelayed" and return
	 * false.
	 *
	 * Clearing K_WORK_DELAYED_BIT claims the wake from the timeout
	 * handler (see work_timeout()). Then wait for any in-flight
	 * handler to actually complete: callers may free dwork or
	 * re-arm the same delayable for a new schedule once we return
	 * true, and either is unsafe while the handler is still about
	 * to dereference dwork->work.flags.
	 */
	ਜੇ (flag_test_and_clear(&work->flags, K_WORK_DELAYED_ਦੋਆਧਾਰੀ_ਅੰਕ)) {
		ਜਦੋਂ (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_ਤਾਲਾ, *ਕੁੰਜੀ);
			*ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
		}
		ret = true;
	}

	ਵਾਪਸ ret;
}

/* Full cancellation of a delayable work item.
 *
 * Unschedules the delayed part then delegates to standard work
 * cancellation.
 *
 * Invoked with work lock held.
 *
 * @param dwork delayable work item
 *
 * @return k_work_busy_get() flags
 */
ਸਥਿਰ ਪੂਰਨ cancel_delayable_async_locked(ਬਣਤਰ k_work_delayable *dwork,
					 k_spinlock_ਕੁੰਜੀ_t *ਕੁੰਜੀ)
{
	(ਖਾਲੀ)unschedule_locked(dwork, ਕੁੰਜੀ);

	ਵਾਪਸ cancel_async_locked(&dwork->work);
}

ਪੂਰਨ k_work_ਸਮਾਂ_ਸਾਰਣੀ_for_ਕਤਾਰ(ਬਣਤਰ k_work_q *ਕਤਾਰ, ਬਣਤਰ k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(ਕਤਾਰ != ਨਿਰਾ);
	__ASSERT_NO_MSG(dwork != ਨਿਰਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, ਸਮਾਂ_ਸਾਰਣੀ_for_ਕਤਾਰ, ਕਤਾਰ, dwork, delay);

	ਬਣਤਰ k_work *work = &dwork->work;
	ਪੂਰਨ ret = 0;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	/* Schedule the work item if it's idle or running. */
	ਜੇ ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = ਸਮਾਂ_ਸਾਰਣੀ_for_ਕਤਾਰ_locked(&ਕਤਾਰ, dwork, delay);
	}

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, ਸਮਾਂ_ਸਾਰਣੀ_for_ਕਤਾਰ, ਕਤਾਰ, dwork, delay, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_ਸਮਾਂ_ਸਾਰਣੀ(ਬਣਤਰ k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, ਸਮਾਂ_ਸਾਰਣੀ, dwork, delay);

	ਪੂਰਨ ret = k_work_ਸਮਾਂ_ਸਾਰਣੀ_for_ਕਤਾਰ(&k_sys_work_q, dwork, delay);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, ਸਮਾਂ_ਸਾਰਣੀ, dwork, delay, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_reschedule_for_ਕਤਾਰ(ਬਣਤਰ k_work_q *ਕਤਾਰ, ਬਣਤਰ k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(ਕਤਾਰ != ਨਿਰਾ);
	__ASSERT_NO_MSG(dwork != ਨਿਰਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_ਕਤਾਰ, ਕਤਾਰ, dwork, delay);

	ਪੂਰਨ ret;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	/* Remove any active scheduling. */
	(ਖਾਲੀ)unschedule_locked(dwork, &ਕੁੰਜੀ);

	/* Schedule the work item with the new parameters. */
	ret = ਸਮਾਂ_ਸਾਰਣੀ_for_ਕਤਾਰ_locked(&ਕਤਾਰ, dwork, delay);

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_ਕਤਾਰ, ਕਤਾਰ, dwork, delay, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_reschedule(ਬਣਤਰ k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	ਪੂਰਨ ret = k_work_reschedule_for_ਕਤਾਰ(&k_sys_work_q, dwork, delay);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	ਵਾਪਸ ret;
}

ਪੂਰਨ k_work_cancel_delayable(ਬਣਤਰ k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ਨਿਰਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
	ਪੂਰਨ ret = cancel_delayable_async_locked(dwork, &ਕੁੰਜੀ);

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	ਵਾਪਸ ret;
}

bool k_work_cancel_delayable_sync(ਬਣਤਰ k_work_delayable *dwork,
				  ਬਣਤਰ k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ਨਿਰਾ);
	__ASSERT_NO_MSG(sync != ਨਿਰਾ);
	__ASSERT_NO_MSG(!k_is_ਵਿੱਚ_isr());
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	ਬਣਤਰ z_work_canceller *canceller = &sync->canceller;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	ਜੇ (pending) {
		(ਖਾਲੀ)cancel_delayable_async_locked(dwork, &ਕੁੰਜੀ);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	ਜੇ (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	ਵਾਪਸ pending;
}

bool k_work_flush_delayable(ਬਣਤਰ k_work_delayable *dwork,
			    ਬਣਤਰ k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ਨਿਰਾ);
	__ASSERT_NO_MSG(sync != ਨਿਰਾ);
	__ASSERT_NO_MSG(!k_is_ਵਿੱਚ_isr());
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	ਬਣਤਰ k_work *work = &dwork->work;
	ਬਣਤਰ z_work_flusher *flusher = &sync->flusher;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&work_ਤਾਲਾ);

	/* If it's idle release the lock and return immediately. */
	ਜੇ (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		ਵਾਪਸ false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	ਜੇ (unschedule_locked(dwork, &ਕੁੰਜੀ)) {
		ਬਣਤਰ k_work_q *ਕਤਾਰ = dwork->ਕਤਾਰ;

		(ਖਾਲੀ)submit_ਮੰਜ਼ਿਲ_ਕਤਾਰ_locked(work, &ਕਤਾਰ);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_ਤਾਲਾ, ਕੁੰਜੀ);

	/* If necessary wait until the flusher item completes */
	ਜੇ (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	ਵਾਪਸ need_flush;
}

#ਜੇ_ਅੰਤ /* CONFIG_SYS_CLOCK_EXISTS */
