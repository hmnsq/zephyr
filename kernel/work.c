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

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <zephyr/sys/printk.h>
#అన్తర్భావ <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

స్థిర అంతఃవరుస శూన్య flag_clear(uint32_t *flagp,
			      uint32_t బిట్టు_2)
{
	*flagp &= ~బిట్టు(బిట్టు_2);
}

స్థిర అంతఃవరుస శూన్య flag_సమితి(uint32_t *flagp,
			    uint32_t బిట్టు_2)
{
	*flagp |= బిట్టు(బిట్టు_2);
}

స్థిర అంతఃవరుస bool flag_test(స్థిరరాశి uint32_t *flagp,
			     uint32_t బిట్టు_2)
{
	తిరిగి (*flagp & బిట్టు(బిట్టు_2)) != 0U;
}

స్థిర అంతఃవరుస bool flag_test_and_clear(uint32_t *flagp,
				       పూర్ణాంకం బిట్టు_2)
{
	bool ret = flag_test(flagp, బిట్టు_2);

	flag_clear(flagp, బిట్టు_2);

	తిరిగి ret;
}

స్థిర అంతఃవరుస శూన్య flags_సమితి(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

స్థిర అంతఃవరుస uint32_t flags_get(స్థిరరాశి uint32_t *flagp)
{
	తిరిగి *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
స్థిర నిర్మాణం k_spinlock work_తాళం;

/* Invoked by work thread */
స్థిర శూన్య handle_flush(నిర్మాణం k_work *work) { }

స్థిర అంతఃవరుస శూన్య init_flusher(నిర్మాణం z_work_flusher *flusher)
{
	నిర్మాణం k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_సమితి(&work->flags, K_WORK_FLUSHING_బిట్టు);
}

/* List of pending cancellations. */
స్థిర sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
స్థిర అంతఃవరుస శూన్య init_work_cancel(నిర్మాణం z_work_canceller *canceler,
				    నిర్మాణం k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->కణుపు);
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
స్థిర శూన్య finalize_flush_locked(నిర్మాణం k_work *work)
{
	నిర్మాణం z_work_flusher *flusher
		= CONTAINER_యొక్క(work, నిర్మాణం z_work_flusher, work);

	flag_clear(&work->flags, K_WORK_FLUSHING_బిట్టు);

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
స్థిర శూన్య finalize_cancel_locked(నిర్మాణం k_work *work)
{
	నిర్మాణం z_work_canceller *wc, *tmp;
	sys_snode_t *prev = ఖాళీ;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_బిట్టు);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_సురక్షిత(&pending_cancels, wc, tmp, కణుపు) {
		ఒకవేళ (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->కణుపు);
			k_sem_give(&wc->sem);
		} లేకపోతే {
			prev = &wc->కణుపు;
		}
	}
}

శూన్య k_work_init(నిర్మాణం k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != ఖాళీ);
	__ASSERT_NO_MSG(handler != ఖాళీ);

	*work = (నిర్మాణం k_work)Z_WORK_INITIALIZER(handler);

	SYS_పోర్టు_TRACING_OBJ_INIT(k_work, work);
}

స్థిర అంతఃవరుస పూర్ణాంకం work_busy_get_locked(స్థిరరాశి నిర్మాణం k_work *work)
{
	తిరిగి flags_get(&work->flags) & K_WORK_ముసుగు;
}

పూర్ణాంకం k_work_busy_get(స్థిరరాశి నిర్మాణం k_work *work)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
	పూర్ణాంకం ret = work_busy_get_locked(work);

	k_spin_unlock(&work_తాళం, కీలకం);

	తిరిగి ret;
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
స్థిర శూన్య వరుస_flusher_locked(నిర్మాణం k_work_q *వరుస,
				 నిర్మాణం k_work *work,
				 నిర్మాణం z_work_flusher *flusher)
{
	init_flusher(flusher);

	ఒకవేళ ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_చొప్పించు(&వరుస->pending, &work->కణుపు,
				 &flusher->work.కణుపు);
	} లేకపోతే {
		sys_slist_prepend(&వరుస->pending, &flusher->work.కణుపు);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
స్థిర అంతఃవరుస శూన్య వరుస_remove_locked(నిర్మాణం k_work_q *వరుస,
				       నిర్మాణం k_work *work)
{
	ఒకవేళ (flag_test_and_clear(&work->flags, K_WORK_QUEUED_బిట్టు)) {
		(శూన్య)sys_slist_కనుగొను_and_remove(&వరుస->pending, &work->కణుపు);
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
స్థిర అంతఃవరుస bool notify_వరుస_locked(నిర్మాణం k_work_q *వరుస)
{
	bool rv = false;

	ఒకవేళ (వరుస != ఖాళీ) {
		rv = z_sched_wake(&వరుస->notifyq, 0, ఖాళీ);
	}

	తిరిగి rv;
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
స్థిర అంతఃవరుస పూర్ణాంకం వరుస_submit_locked(నిర్మాణం k_work_q *వరుస,
				      నిర్మాణం k_work *work)
{
	ఒకవేళ (వరుస == ఖాళీ) {
		తిరిగి -EINVAL;
	}

	పూర్ణాంకం ret;
	bool chained = (_current == వరుస->thread_id) && !k_is_లోపల_isr();
	bool draining = flag_test(&వరుస->flags, K_WORK_వరుస_DRAIN_బిట్టు);
	bool plugged = flag_test(&వరుస->flags, K_WORK_వరుస_PLUGGED_బిట్టు);

	/* Test for acceptability, in priority order:
	 *
	 * * -ENODEV if the queue isn't running.
	 * * -EBUSY if draining and not chained
	 * * -EBUSY if plugged and not draining
	 * * otherwise OK
	 */
	ఒకవేళ (!flag_test(&వరుస->flags, K_WORK_వరుస_STARTED_బిట్టు)) {
		ret = -ENODEV;
	} లేకపోతే ఒకవేళ (draining && !chained) {
		ret = -EBUSY;
	} లేకపోతే ఒకవేళ (plugged && !draining) {
		ret = -EBUSY;
	} లేకపోతే {
		sys_slist_append(&వరుస->pending, &work->కణుపు);
		ret = 1;
		(శూన్య)notify_వరుస_locked(వరుస);
	}

	తిరిగి ret;
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
స్థిర పూర్ణాంకం submit_గమ్యం_వరుస_locked(నిర్మాణం k_work *work,
				  నిర్మాణం k_work_q **queuep)
{
	పూర్ణాంకం ret = 0;

	ఒకవేళ (flag_test(&work->flags, K_WORK_CANCELING_బిట్టు)) {
		/* Disallowed */
		ret = -EBUSY;
	} లేకపోతే ఒకవేళ (!flag_test(&work->flags, K_WORK_QUEUED_బిట్టు)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		ఒకవేళ (*queuep == ఖాళీ) {
			*queuep = work->వరుస;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		ఒకవేళ (flag_test(&work->flags, K_WORK_RUNNING_బిట్టు)) {
			__ASSERT_NO_MSG(work->వరుస != ఖాళీ);
			*queuep = work->వరుస;
			ret = 2;
		}

		పూర్ణాంకం rc = వరుస_submit_locked(*queuep, work);

		ఒకవేళ (rc < 0) {
			ret = rc;
		} లేకపోతే {
			flag_సమితి(&work->flags, K_WORK_QUEUED_బిట్టు);
			work->వరుస = *queuep;
		}
	} లేకపోతే {
		/* Already queued, do nothing. */
	}

	ఒకవేళ (ret <= 0) {
		*queuep = ఖాళీ;
	}

	తిరిగి ret;
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
పూర్ణాంకం z_work_submit_గమ్యం_వరుస(నిర్మాణం k_work_q *వరుస,
		  నిర్మాణం k_work *work)
{
	__ASSERT_NO_MSG(work != ఖాళీ);
	__ASSERT_NO_MSG(work->handler != ఖాళీ);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	పూర్ణాంకం ret = submit_గమ్యం_వరుస_locked(work, &వరుస);

	/* No need to reschedule if the submission failed. */
	ఒకవేళ (ret > 0) {
		z_reschedule(&work_తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&work_తాళం, కీలకం);
	}

	తిరిగి ret;
}

పూర్ణాంకం k_work_submit_గమ్యం_వరుస(నిర్మాణం k_work_q *వరుస,
			    నిర్మాణం k_work *work)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, submit_గమ్యం_వరుస, వరుస, work);

	పూర్ణాంకం ret = z_work_submit_గమ్యం_వరుస(వరుస, work);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, submit_గమ్యం_వరుస, వరుస, work, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_submit(నిర్మాణం k_work *work)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	పూర్ణాంకం ret = z_work_submit_గమ్యం_వరుస(&k_sys_work_q, work);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	తిరిగి ret;
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
స్థిర bool work_flush_locked(నిర్మాణం k_work *work,
			      నిర్మాణం z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	ఒకవేళ (need_flush) {
		నిర్మాణం k_work_q *వరుస = work->వరుస;

		__ASSERT_NO_MSG(వరుస != ఖాళీ);

		వరుస_flusher_locked(వరుస, work, flusher);
		notify_వరుస_locked(వరుస);
	}

	తిరిగి need_flush;
}

bool k_work_flush(నిర్మాణం k_work *work,
		  నిర్మాణం k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ఖాళీ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_బిట్టు));
	__ASSERT_NO_MSG(!k_is_లోపల_isr());
	__ASSERT_NO_MSG(sync != ఖాళీ);
#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	నిర్మాణం z_work_flusher *flusher = &sync->flusher;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_తాళం, కీలకం);

	/* If necessary wait until the flusher item completes */
	ఒకవేళ (need_flush) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	తిరిగి need_flush;
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
స్థిర పూర్ణాంకం cancel_async_locked(నిర్మాణం k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	ఒకవేళ (!flag_test(&work->flags, K_WORK_CANCELING_బిట్టు)) {
		/* Remove it from the queue, if it's queued. */
		వరుస_remove_locked(work->వరుస, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	పూర్ణాంకం ret = work_busy_get_locked(work);

	ఒకవేళ (ret != 0) {
		flag_సమితి(&work->flags, K_WORK_CANCELING_బిట్టు);
		ret = work_busy_get_locked(work);
	}

	తిరిగి ret;
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
స్థిర bool cancel_sync_locked(నిర్మాణం k_work *work,
			       నిర్మాణం z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_బిట్టు);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	ఒకవేళ (ret) {
		init_work_cancel(canceller, work);
	}

	తిరిగి ret;
}

పూర్ణాంకం k_work_cancel(నిర్మాణం k_work *work)
{
	__ASSERT_NO_MSG(work != ఖాళీ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_బిట్టు));

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
	పూర్ణాంకం ret = cancel_async_locked(work);

	k_spin_unlock(&work_తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	తిరిగి ret;
}

bool k_work_cancel_sync(నిర్మాణం k_work *work,
			నిర్మాణం k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ఖాళీ);
	__ASSERT_NO_MSG(sync != ఖాళీ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_బిట్టు));
	__ASSERT_NO_MSG(!k_is_లోపల_isr());
#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	నిర్మాణం z_work_canceller *canceller = &sync->canceller;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	ఒకవేళ (pending) {
		(శూన్య)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_తాళం, కీలకం);

	ఒకవేళ (need_wait) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	తిరిగి pending;
}

#ఒకవేళ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
స్థిర శూన్య work_timeout_handler(నిర్మాణం _timeout *రికార్డు)
{
	నిర్మాణం k_work_q *వరుస = CONTAINER_యొక్క(రికార్డు, నిర్మాణం k_work_q, work_timeout_రికార్డు);
	నిర్మాణం k_work *work = ఖాళీ;
	k_work_handler_t handler = ఖాళీ;
	స్థిరరాశి అక్షరం *name;
	స్థిరరాశి అక్షరం *space = " ";
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	work = వరుస->work;
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
	ఒకవేళ (వరుస->ముగిసిన || z_timeout_inflight_superseded(రికార్డు)) {
		k_spin_unlock(&work_తాళం, కీలకం);
		తిరిగి;
	}
	వరుస->ముగిసిన = true;

	k_spin_unlock(&work_తాళం, కీలకం);

	name = k_thread_name_get(వరుస->thread_id);
	ఒకవేళ (name == ఖాళీ) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		వరుస, space, name, work, handler);

	k_thread_abort(వరుస->thread_id);
}

స్థిర శూన్య work_timeout_ప్రారంభం_locked(నిర్మాణం k_work_q *వరుస, నిర్మాణం k_work *work)
{
	వరుస->ముగిసిన = false;

	ఒకవేళ (K_TIMEOUT_EQ(వరుస->work_timeout, K_FOREVER)) {
		తిరిగి;
	}

	వరుస->work = work;
	z_కూడు_timeout(&వరుస->work_timeout_రికార్డు, work_timeout_handler, వరుస->work_timeout);
}

స్థిర శూన్య work_timeout_stop_locked(నిర్మాణం k_work_q *వరుస)
{
	ఒకవేళ (K_TIMEOUT_EQ(వరుస->work_timeout, K_FOREVER)) {
		తిరిగి;
	}

	(శూన్య)z_try_abort_timeout(&వరుస->work_timeout_రికార్డు);
}
#అయితే_ముగింపు /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
స్థిర శూన్య work_వరుస_main(శూన్య *workq_ptr, శూన్య *p2, శూన్య *p3)
{
	ARG_వాడని(p2);
	ARG_వాడని(p3);

	నిర్మాణం k_work_q *వరుస = (నిర్మాణం k_work_q *)workq_ptr;

	అయితే (true) {
		sys_snode_t *కణుపు;
		నిర్మాణం k_work *work = ఖాళీ;
		k_work_handler_t handler = ఖాళీ;
		k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
		bool yield;

		/* Check for and prepare any new work. */
		కణుపు = sys_slist_get(&వరుస->pending);
		ఒకవేళ (కణుపు != ఖాళీ) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_సమితి(&వరుస->flags, K_WORK_వరుస_BUSY_బిట్టు);
			work = CONTAINER_యొక్క(కణుపు, నిర్మాణం k_work, కణుపు);
			flag_సమితి(&work->flags, K_WORK_RUNNING_బిట్టు);
			flag_clear(&work->flags, K_WORK_QUEUED_బిట్టు);

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
		} లేకపోతే ఒకవేళ (flag_test_and_clear(&వరుస->flags,
					       K_WORK_వరుస_DRAIN_బిట్టు)) {
			/* Not busy and draining: move threads waiting for
			 * drain to ready state.  The held spinlock inhibits
			 * immediate reschedule; released threads get their
			 * chance when this invokes z_sched_wait() below.
			 *
			 * We don't touch K_WORK_QUEUE_PLUGGABLE, so getting
			 * here doesn't mean that the queue will allow new
			 * submissions.
			 */
			(శూన్య)z_sched_wake_all(&వరుస->drainq, 1, ఖాళీ);
		} లేకపోతే ఒకవేళ (flag_test(&వరుస->flags, K_WORK_వరుస_STOP_బిట్టు)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_సమితి(&వరుస->flags, 0);
			k_spin_unlock(&work_తాళం, కీలకం);
			తిరిగి;
		} లేకపోతే {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		ఒకవేళ (work == ఖాళీ) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(శూన్య)z_sched_wait(&work_తాళం, కీలకం, &వరుస->notifyq,
					   K_FOREVER, ఖాళీ);
			కొనసాగు;
		}

#ఒకవేళ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_ప్రారంభం_locked(వరుస, work);
#అయితే_ముగింపు /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_తాళం, కీలకం);

		__ASSERT_NO_MSG(handler != ఖాళీ);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		కీలకం = k_spin_తాళం(&work_తాళం);

#ఒకవేళ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		ఒకవేళ (వరుస->ముగిసిన) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_తాళం, కీలకం);
			అయితే (1) {
				k_sleep(K_FOREVER);
			}
			సంకేతం_UNREACHABLE;
		}
		work_timeout_stop_locked(వరుస);
		వరుస->ముగిసిన = true;
#అయితే_ముగింపు /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_బిట్టు);
		ఒకవేళ (flag_test(&work->flags, K_WORK_FLUSHING_బిట్టు)) {
			finalize_flush_locked(work);
		}
		ఒకవేళ (flag_test(&work->flags, K_WORK_CANCELING_బిట్టు)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&వరుస->flags, K_WORK_వరుస_BUSY_బిట్టు);
		yield = (!flag_test(&వరుస->flags, K_WORK_వరుస_NO_YIELD_బిట్టు) &&
			 !sys_slist_is_ఖాళీలు(&వరుస->pending));
		k_spin_unlock(&work_తాళం, కీలకం);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		ఒకవేళ (yield) {
			k_yield();
		}
	}
}

శూన్య k_work_వరుస_init(నిర్మాణం k_work_q *వరుస)
{
	__ASSERT_NO_MSG(వరుస != ఖాళీ);

	*వరుస = (నిర్మాణం k_work_q) {
		.flags = 0,
	};

	SYS_పోర్టు_TRACING_OBJ_INIT(k_work_వరుస, వరుస);
}

శూన్య k_work_వరుస_నిరంతరం(నిర్మాణం k_work_q *వరుస, స్థిరరాశి నిర్మాణం k_work_వరుస_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&వరుస->flags, K_WORK_వరుస_STARTED_బిట్టు));

	uint32_t flags = K_WORK_వరుస_STARTED;

	ఒకవేళ ((cfg != ఖాళీ) && cfg->no_yield) {
		flags |= K_WORK_వరుస_NO_YIELD;
	}

	ఒకవేళ ((cfg != ఖాళీ) && (cfg->name != ఖాళీ)) {
		k_thread_name_సమితి(_current, cfg->name);
	}

#ఒకవేళ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ఒకవేళ ((cfg != ఖాళీ) && (cfg->work_timeout_ms)) {
		వరుస->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} లేకపోతే {
		వరుస->work_timeout = K_FOREVER;
	}
#అయితే_ముగింపు /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&వరుస->pending);
	z_waitq_init(&వరుస->notifyq);
	z_waitq_init(&వరుస->drainq);
	వరుస->thread_id = _current;
	flags_సమితి(&వరుస->flags, flags);
	work_వరుస_main(వరుస, ఖాళీ, ఖాళీ);
}

శూన్య k_work_వరుస_ప్రారంభం(నిర్మాణం k_work_q *వరుస,
			k_thread_దొంతర_t *దొంతర,
			size_t దొంతర_కొలత,
			పూర్ణాంకం prio,
			స్థిరరాశి నిర్మాణం k_work_వరుస_config *cfg)
{
	__ASSERT_NO_MSG(వరుస);
	__ASSERT_NO_MSG(దొంతర);
	__ASSERT_NO_MSG(!flag_test(&వరుస->flags, K_WORK_వరుస_STARTED_బిట్టు));

	/* In future, this whole function will be deprecated, but for now, we
	 * have to use the `thread` field to create a new thread in it.
	 */
	TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);

	uint32_t flags = K_WORK_వరుస_STARTED;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work_వరుస, ప్రారంభం, వరుస);

	sys_slist_init(&వరుస->pending);
	z_waitq_init(&వరుస->notifyq);
	z_waitq_init(&వరుస->drainq);

	ఒకవేళ ((cfg != ఖాళీ) && cfg->no_yield) {
		flags |= K_WORK_వరుస_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_సమితి(&వరుస->flags, flags);

	(శూన్య)k_thread_create(&వరుస->thread, దొంతర, దొంతర_కొలత,
			      work_వరుస_main, వరుస, ఖాళీ, ఖాళీ,
			      prio, 0, K_FOREVER);

	ఒకవేళ ((cfg != ఖాళీ) && (cfg->name != ఖాళీ)) {
		k_thread_name_సమితి(&వరుస->thread, cfg->name);
	}

	ఒకవేళ ((cfg != ఖాళీ) && (cfg->essential)) {
		వరుస->thread.పునాది.user_options |= K_ESSENTIAL;
	}

#ఒకవేళ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ఒకవేళ ((cfg != ఖాళీ) && (cfg->work_timeout_ms)) {
		వరుస->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} లేకపోతే {
		వరుస->work_timeout = K_FOREVER;
	}
#అయితే_ముగింపు /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_ప్రారంభం(&వరుస->thread);
	వరుస->thread_id = &వరుస->thread;

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_వరుస, ప్రారంభం, వరుస);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

పూర్ణాంకం k_work_వరుస_drain(నిర్మాణం k_work_q *వరుస,
		       bool plug)
{
	__ASSERT_NO_MSG(వరుస);
	__ASSERT_NO_MSG(!k_is_లోపల_isr());

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work_వరుస, drain, వరుస);

	పూర్ణాంకం ret = 0;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	ఒకవేళ (((flags_get(&వరుస->flags)
	      & (K_WORK_వరుస_BUSY | K_WORK_వరుస_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_ఖాళీలు(&వరుస->pending)) {
		flag_సమితి(&వరుస->flags, K_WORK_వరుస_DRAIN_బిట్టు);
		ఒకవేళ (plug) {
			flag_సమితి(&వరుస->flags, K_WORK_వరుస_PLUGGED_బిట్టు);
		}

		notify_వరుస_locked(వరుస);
		ret = z_sched_wait(&work_తాళం, కీలకం, &వరుస->drainq,
				   K_FOREVER, ఖాళీ);
	} లేకపోతే {
		k_spin_unlock(&work_తాళం, కీలకం);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_వరుస, drain, వరుస, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_వరుస_unplug(నిర్మాణం k_work_q *వరుస)
{
	__ASSERT_NO_MSG(వరుస);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work_వరుస, unplug, వరుస);

	పూర్ణాంకం ret = -EALREADY;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	ఒకవేళ (flag_test_and_clear(&వరుస->flags, K_WORK_వరుస_PLUGGED_బిట్టు)) {
		ret = 0;
	}

	k_spin_unlock(&work_తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_వరుస, unplug, వరుస, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_వరుస_stop(నిర్మాణం k_work_q *వరుస, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(వరుస);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work_వరుస, stop, వరుస, timeout);

	ఒకవేళ (వరుస->thread_id) {
		ఒకవేళ (z_is_thread_essential(వరుస->thread_id)) {
			తిరిగి -ENOTSUP;
		}
	}

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	ఒకవేళ (!flag_test(&వరుస->flags, K_WORK_వరుస_STARTED_బిట్టు)) {
		k_spin_unlock(&work_తాళం, కీలకం);
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_వరుస, stop, వరుస, timeout, -EALREADY);
		తిరిగి -EALREADY;
	}

	ఒకవేళ (!flag_test(&వరుస->flags, K_WORK_వరుస_PLUGGED_బిట్టు)) {
		k_spin_unlock(&work_తాళం, కీలకం);
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_వరుస, stop, వరుస, timeout, -EBUSY);
		తిరిగి -EBUSY;
	}

	flag_సమితి(&వరుస->flags, K_WORK_వరుస_STOP_బిట్టు);
	notify_వరుస_locked(వరుస);
	k_spin_unlock(&work_తాళం, కీలకం);
	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_work_వరుస, stop, వరుస, timeout);
	ఒకవేళ (k_thread_join(వరుస->thread_id, timeout)) {
		కీలకం = k_spin_తాళం(&work_తాళం);
		flag_clear(&వరుస->flags, K_WORK_వరుస_STOP_బిట్టు);
		k_spin_unlock(&work_తాళం, కీలకం);
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_వరుస, stop, వరుస, timeout, -ETIMEDOUT);
		తిరిగి -ETIMEDOUT;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work_వరుస, stop, వరుస, timeout, 0);
	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
స్థిర శూన్య work_timeout(నిర్మాణం _timeout *గమ్యం)
{
	నిర్మాణం k_work_delayable *dw
		= CONTAINER_యొక్క(గమ్యం, నిర్మాణం k_work_delayable, timeout);
	నిర్మాణం k_work *wp = &dw->work;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
	నిర్మాణం k_work_q *వరుస = ఖాళీ;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	ఒకవేళ (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_బిట్టు)) {
		వరుస = dw->వరుస;
		(శూన్య)submit_గమ్యం_వరుస_locked(wp, &వరుస);
	}

	k_spin_unlock(&work_తాళం, కీలకం);
}

శూన్య k_work_init_delayable(నిర్మాణం k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != ఖాళీ);
	__ASSERT_NO_MSG(handler != ఖాళీ);

	*dwork = (నిర్మాణం k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_పోర్టు_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

స్థిర అంతఃవరుస పూర్ణాంకం work_delayable_busy_get_locked(స్థిరరాశి నిర్మాణం k_work_delayable *dwork)
{
	తిరిగి flags_get(&dwork->work.flags) & K_WORK_ముసుగు;
}

పూర్ణాంకం k_work_delayable_busy_get(స్థిరరాశి నిర్మాణం k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ఖాళీ);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
	పూర్ణాంకం ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_తాళం, కీలకం);
	తిరిగి ret;
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
స్థిర పూర్ణాంకం కాలపట్టిక_for_వరుస_locked(నిర్మాణం k_work_q **queuep,
				     నిర్మాణం k_work_delayable *dwork,
				     k_timeout_t delay)
{
	పూర్ణాంకం ret = 1;
	నిర్మాణం k_work *work = &dwork->work;

	ఒకవేళ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		తిరిగి submit_గమ్యం_వరుస_locked(work, queuep);
	}

	flag_సమితి(&work->flags, K_WORK_DELAYED_బిట్టు);
	dwork->వరుస = *queuep;

	/* Add timeout */
	z_కూడు_timeout(&dwork->timeout, work_timeout, delay);

	తిరిగి ret;
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
స్థిర అంతఃవరుస bool unschedule_locked(నిర్మాణం k_work_delayable *dwork,
				     k_spinlock_కీలకం_t *కీలకం)
{
	bool ret = false;
	నిర్మాణం k_work *work = &dwork->work;

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
	ఒకవేళ (flag_test_and_clear(&work->flags, K_WORK_DELAYED_బిట్టు)) {
		అయితే (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_తాళం, *కీలకం);
			*కీలకం = k_spin_తాళం(&work_తాళం);
		}
		ret = true;
	}

	తిరిగి ret;
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
స్థిర పూర్ణాంకం cancel_delayable_async_locked(నిర్మాణం k_work_delayable *dwork,
					 k_spinlock_కీలకం_t *కీలకం)
{
	(శూన్య)unschedule_locked(dwork, కీలకం);

	తిరిగి cancel_async_locked(&dwork->work);
}

పూర్ణాంకం k_work_కాలపట్టిక_for_వరుస(నిర్మాణం k_work_q *వరుస, నిర్మాణం k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(వరుస != ఖాళీ);
	__ASSERT_NO_MSG(dwork != ఖాళీ);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, కాలపట్టిక_for_వరుస, వరుస, dwork, delay);

	నిర్మాణం k_work *work = &dwork->work;
	పూర్ణాంకం ret = 0;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	/* Schedule the work item if it's idle or running. */
	ఒకవేళ ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = కాలపట్టిక_for_వరుస_locked(&వరుస, dwork, delay);
	}

	k_spin_unlock(&work_తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, కాలపట్టిక_for_వరుస, వరుస, dwork, delay, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_కాలపట్టిక(నిర్మాణం k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, కాలపట్టిక, dwork, delay);

	పూర్ణాంకం ret = k_work_కాలపట్టిక_for_వరుస(&k_sys_work_q, dwork, delay);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, కాలపట్టిక, dwork, delay, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_reschedule_for_వరుస(నిర్మాణం k_work_q *వరుస, నిర్మాణం k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(వరుస != ఖాళీ);
	__ASSERT_NO_MSG(dwork != ఖాళీ);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_వరుస, వరుస, dwork, delay);

	పూర్ణాంకం ret;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	/* Remove any active scheduling. */
	(శూన్య)unschedule_locked(dwork, &కీలకం);

	/* Schedule the work item with the new parameters. */
	ret = కాలపట్టిక_for_వరుస_locked(&వరుస, dwork, delay);

	k_spin_unlock(&work_తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_వరుస, వరుస, dwork, delay, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_reschedule(నిర్మాణం k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	పూర్ణాంకం ret = k_work_reschedule_for_వరుస(&k_sys_work_q, dwork, delay);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	తిరిగి ret;
}

పూర్ణాంకం k_work_cancel_delayable(నిర్మాణం k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ఖాళీ);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
	పూర్ణాంకం ret = cancel_delayable_async_locked(dwork, &కీలకం);

	k_spin_unlock(&work_తాళం, కీలకం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	తిరిగి ret;
}

bool k_work_cancel_delayable_sync(నిర్మాణం k_work_delayable *dwork,
				  నిర్మాణం k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ఖాళీ);
	__ASSERT_NO_MSG(sync != ఖాళీ);
	__ASSERT_NO_MSG(!k_is_లోపల_isr());
#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	నిర్మాణం z_work_canceller *canceller = &sync->canceller;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	ఒకవేళ (pending) {
		(శూన్య)cancel_delayable_async_locked(dwork, &కీలకం);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_తాళం, కీలకం);

	ఒకవేళ (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	తిరిగి pending;
}

bool k_work_flush_delayable(నిర్మాణం k_work_delayable *dwork,
			    నిర్మాణం k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ఖాళీ);
	__ASSERT_NO_MSG(sync != ఖాళీ);
	__ASSERT_NO_MSG(!k_is_లోపల_isr());
#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	నిర్మాణం k_work *work = &dwork->work;
	నిర్మాణం z_work_flusher *flusher = &sync->flusher;
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&work_తాళం);

	/* If it's idle release the lock and return immediately. */
	ఒకవేళ (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		తిరిగి false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	ఒకవేళ (unschedule_locked(dwork, &కీలకం)) {
		నిర్మాణం k_work_q *వరుస = dwork->వరుస;

		(శూన్య)submit_గమ్యం_వరుస_locked(work, &వరుస);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_తాళం, కీలకం);

	/* If necessary wait until the flusher item completes */
	ఒకవేళ (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	తిరిగి need_flush;
}

#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */
