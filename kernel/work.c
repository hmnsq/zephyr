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

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/printk.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ flag_clear(uint32_t *flagp,
			      uint32_t ಬಿಟ್_2)
{
	*flagp &= ~ಬಿಟ್(ಬಿಟ್_2);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ flag_ಗಣ(uint32_t *flagp,
			    uint32_t ಬಿಟ್_2)
{
	*flagp |= ಬಿಟ್(ಬಿಟ್_2);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool flag_test(ಸ್ಥಿರಾಂಕ uint32_t *flagp,
			     uint32_t ಬಿಟ್_2)
{
	ಹಿಂತಿರುಗಿ (*flagp & ಬಿಟ್(ಬಿಟ್_2)) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool flag_test_and_clear(uint32_t *flagp,
				       ಪೂರ್ಣಾಂಕ ಬಿಟ್_2)
{
	bool ret = flag_test(flagp, ಬಿಟ್_2);

	flag_clear(flagp, ಬಿಟ್_2);

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ flags_ಗಣ(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t flags_get(ಸ್ಥಿರಾಂಕ uint32_t *flagp)
{
	ಹಿಂತಿರುಗಿ *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
ಸ್ಥಿರ ರಚನೆ k_spinlock work_ಬೀಗ;

/* Invoked by work thread */
ಸ್ಥಿರ ಶೂನ್ಯ handle_flush(ರಚನೆ k_work *work) { }

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_flusher(ರಚನೆ z_work_flusher *flusher)
{
	ರಚನೆ k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_ಗಣ(&work->flags, K_WORK_FLUSHING_ಬಿಟ್);
}

/* List of pending cancellations. */
ಸ್ಥಿರ sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_work_cancel(ರಚನೆ z_work_canceller *canceler,
				    ರಚನೆ k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->ಗಂಟು);
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
ಸ್ಥಿರ ಶೂನ್ಯ finalize_flush_locked(ರಚನೆ k_work *work)
{
	ರಚನೆ z_work_flusher *flusher
		= CONTAINER_ದ(work, ರಚನೆ z_work_flusher, work);

	flag_clear(&work->flags, K_WORK_FLUSHING_ಬಿಟ್);

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
ಸ್ಥಿರ ಶೂನ್ಯ finalize_cancel_locked(ರಚನೆ k_work *work)
{
	ರಚನೆ z_work_canceller *wc, *tmp;
	sys_snode_t *prev = ಖಾಲಿ;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_ಬಿಟ್);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_ಸುರಕ್ಷಿತ(&pending_cancels, wc, tmp, ಗಂಟು) {
		ಆದರೆ (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->ಗಂಟು);
			k_sem_give(&wc->sem);
		} ಇಲ್ಲದಿದ್ದರೆ {
			prev = &wc->ಗಂಟು;
		}
	}
}

ಶೂನ್ಯ k_work_init(ರಚನೆ k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != ಖಾಲಿ);
	__ASSERT_NO_MSG(handler != ಖಾಲಿ);

	*work = (ರಚನೆ k_work)Z_WORK_INITIALIZER(handler);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_work, work);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ work_busy_get_locked(ಸ್ಥಿರಾಂಕ ರಚನೆ k_work *work)
{
	ಹಿಂತಿರುಗಿ flags_get(&work->flags) & K_WORK_ಮುಖವಾಡ;
}

ಪೂರ್ಣಾಂಕ k_work_busy_get(ಸ್ಥಿರಾಂಕ ರಚನೆ k_work *work)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
	ಪೂರ್ಣಾಂಕ ret = work_busy_get_locked(work);

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ ಶೂನ್ಯ ಸರತಿ_flusher_locked(ರಚನೆ k_work_q *ಸರತಿ,
				 ರಚನೆ k_work *work,
				 ರಚನೆ z_work_flusher *flusher)
{
	init_flusher(flusher);

	ಆದರೆ ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_ಸೇರಿಸು(&ಸರತಿ->pending, &work->ಗಂಟು,
				 &flusher->work.ಗಂಟು);
	} ಇಲ್ಲದಿದ್ದರೆ {
		sys_slist_prepend(&ಸರತಿ->pending, &flusher->work.ಗಂಟು);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಸರತಿ_remove_locked(ರಚನೆ k_work_q *ಸರತಿ,
				       ರಚನೆ k_work *work)
{
	ಆದರೆ (flag_test_and_clear(&work->flags, K_WORK_QUEUED_ಬಿಟ್)) {
		(ಶೂನ್ಯ)sys_slist_ಕಂಡುಹಿಡಿ_and_remove(&ಸರತಿ->pending, &work->ಗಂಟು);
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool notify_ಸರತಿ_locked(ರಚನೆ k_work_q *ಸರತಿ)
{
	bool rv = false;

	ಆದರೆ (ಸರತಿ != ಖಾಲಿ) {
		rv = z_sched_wake(&ಸರತಿ->notifyq, 0, ಖಾಲಿ);
	}

	ಹಿಂತಿರುಗಿ rv;
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ಸರತಿ_submit_locked(ರಚನೆ k_work_q *ಸರತಿ,
				      ರಚನೆ k_work *work)
{
	ಆದರೆ (ಸರತಿ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಪೂರ್ಣಾಂಕ ret;
	bool chained = (_current == ಸರತಿ->thread_id) && !k_is_ಒಳ_isr();
	bool draining = flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_DRAIN_ಬಿಟ್);
	bool plugged = flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_PLUGGED_ಬಿಟ್);

	/* Test for acceptability, in priority order:
	 *
	 * * -ENODEV if the queue isn't running.
	 * * -EBUSY if draining and not chained
	 * * -EBUSY if plugged and not draining
	 * * otherwise OK
	 */
	ಆದರೆ (!flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_STARTED_ಬಿಟ್)) {
		ret = -ENODEV;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (draining && !chained) {
		ret = -EBUSY;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (plugged && !draining) {
		ret = -EBUSY;
	} ಇಲ್ಲದಿದ್ದರೆ {
		sys_slist_append(&ಸರತಿ->pending, &work->ಗಂಟು);
		ret = 1;
		(ಶೂನ್ಯ)notify_ಸರತಿ_locked(ಸರತಿ);
	}

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ submit_ಗಮ್ಯ_ಸರತಿ_locked(ರಚನೆ k_work *work,
				  ರಚನೆ k_work_q **queuep)
{
	ಪೂರ್ಣಾಂಕ ret = 0;

	ಆದರೆ (flag_test(&work->flags, K_WORK_CANCELING_ಬಿಟ್)) {
		/* Disallowed */
		ret = -EBUSY;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!flag_test(&work->flags, K_WORK_QUEUED_ಬಿಟ್)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		ಆದರೆ (*queuep == ಖಾಲಿ) {
			*queuep = work->ಸರತಿ;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		ಆದರೆ (flag_test(&work->flags, K_WORK_RUNNING_ಬಿಟ್)) {
			__ASSERT_NO_MSG(work->ಸರತಿ != ಖಾಲಿ);
			*queuep = work->ಸರತಿ;
			ret = 2;
		}

		ಪೂರ್ಣಾಂಕ rc = ಸರತಿ_submit_locked(*queuep, work);

		ಆದರೆ (rc < 0) {
			ret = rc;
		} ಇಲ್ಲದಿದ್ದರೆ {
			flag_ಗಣ(&work->flags, K_WORK_QUEUED_ಬಿಟ್);
			work->ಸರತಿ = *queuep;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Already queued, do nothing. */
	}

	ಆದರೆ (ret <= 0) {
		*queuep = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ ret;
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
ಪೂರ್ಣಾಂಕ z_work_submit_ಗಮ್ಯ_ಸರತಿ(ರಚನೆ k_work_q *ಸರತಿ,
		  ರಚನೆ k_work *work)
{
	__ASSERT_NO_MSG(work != ಖಾಲಿ);
	__ASSERT_NO_MSG(work->handler != ಖಾಲಿ);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	ಪೂರ್ಣಾಂಕ ret = submit_ಗಮ್ಯ_ಸರತಿ_locked(work, &ಸರತಿ);

	/* No need to reschedule if the submission failed. */
	ಆದರೆ (ret > 0) {
		z_reschedule(&work_ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_submit_ಗಮ್ಯ_ಸರತಿ(ರಚನೆ k_work_q *ಸರತಿ,
			    ರಚನೆ k_work *work)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, submit_ಗಮ್ಯ_ಸರತಿ, ಸರತಿ, work);

	ಪೂರ್ಣಾಂಕ ret = z_work_submit_ಗಮ್ಯ_ಸರತಿ(ಸರತಿ, work);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, submit_ಗಮ್ಯ_ಸರತಿ, ಸರತಿ, work, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_submit(ರಚನೆ k_work *work)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	ಪೂರ್ಣಾಂಕ ret = z_work_submit_ಗಮ್ಯ_ಸರತಿ(&k_sys_work_q, work);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ bool work_flush_locked(ರಚನೆ k_work *work,
			      ರಚನೆ z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	ಆದರೆ (need_flush) {
		ರಚನೆ k_work_q *ಸರತಿ = work->ಸರತಿ;

		__ASSERT_NO_MSG(ಸರತಿ != ಖಾಲಿ);

		ಸರತಿ_flusher_locked(ಸರತಿ, work, flusher);
		notify_ಸರತಿ_locked(ಸರತಿ);
	}

	ಹಿಂತಿರುಗಿ need_flush;
}

bool k_work_flush(ರಚನೆ k_work *work,
		  ರಚನೆ k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ಖಾಲಿ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ಬಿಟ್));
	__ASSERT_NO_MSG(!k_is_ಒಳ_isr());
	__ASSERT_NO_MSG(sync != ಖಾಲಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	ರಚನೆ z_work_flusher *flusher = &sync->flusher;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	/* If necessary wait until the flusher item completes */
	ಆದರೆ (need_flush) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	ಹಿಂತಿರುಗಿ need_flush;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cancel_async_locked(ರಚನೆ k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	ಆದರೆ (!flag_test(&work->flags, K_WORK_CANCELING_ಬಿಟ್)) {
		/* Remove it from the queue, if it's queued. */
		ಸರತಿ_remove_locked(work->ಸರತಿ, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	ಪೂರ್ಣಾಂಕ ret = work_busy_get_locked(work);

	ಆದರೆ (ret != 0) {
		flag_ಗಣ(&work->flags, K_WORK_CANCELING_ಬಿಟ್);
		ret = work_busy_get_locked(work);
	}

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ bool cancel_sync_locked(ರಚನೆ k_work *work,
			       ರಚನೆ z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_ಬಿಟ್);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	ಆದರೆ (ret) {
		init_work_cancel(canceller, work);
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_cancel(ರಚನೆ k_work *work)
{
	__ASSERT_NO_MSG(work != ಖಾಲಿ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ಬಿಟ್));

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
	ಪೂರ್ಣಾಂಕ ret = cancel_async_locked(work);

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	ಹಿಂತಿರುಗಿ ret;
}

bool k_work_cancel_sync(ರಚನೆ k_work *work,
			ರಚನೆ k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ಖಾಲಿ);
	__ASSERT_NO_MSG(sync != ಖಾಲಿ);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ಬಿಟ್));
	__ASSERT_NO_MSG(!k_is_ಒಳ_isr());
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	ರಚನೆ z_work_canceller *canceller = &sync->canceller;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	ಆದರೆ (pending) {
		(ಶೂನ್ಯ)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	ಆದರೆ (need_wait) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	ಹಿಂತಿರುಗಿ pending;
}

#ಆದರೆ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
ಸ್ಥಿರ ಶೂನ್ಯ work_timeout_handler(ರಚನೆ _timeout *ದಾಖಲೆ)
{
	ರಚನೆ k_work_q *ಸರತಿ = CONTAINER_ದ(ದಾಖಲೆ, ರಚನೆ k_work_q, work_timeout_ದಾಖಲೆ);
	ರಚನೆ k_work *work = ಖಾಲಿ;
	k_work_handler_t handler = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *space = " ";
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	work = ಸರತಿ->work;
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
	ಆದರೆ (ಸರತಿ->ಪೂರ್ಣಗೊಂಡ || z_timeout_inflight_superseded(ದಾಖಲೆ)) {
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}
	ಸರತಿ->ಪೂರ್ಣಗೊಂಡ = true;

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	name = k_thread_name_get(ಸರತಿ->thread_id);
	ಆದರೆ (name == ಖಾಲಿ) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		ಸರತಿ, space, name, work, handler);

	k_thread_abort(ಸರತಿ->thread_id);
}

ಸ್ಥಿರ ಶೂನ್ಯ work_timeout_ಆರಂಭ_locked(ರಚನೆ k_work_q *ಸರತಿ, ರಚನೆ k_work *work)
{
	ಸರತಿ->ಪೂರ್ಣಗೊಂಡ = false;

	ಆದರೆ (K_TIMEOUT_EQ(ಸರತಿ->work_timeout, K_FOREVER)) {
		ಹಿಂತಿರುಗಿ;
	}

	ಸರತಿ->work = work;
	z_ಕೂಡಿಸು_timeout(&ಸರತಿ->work_timeout_ದಾಖಲೆ, work_timeout_handler, ಸರತಿ->work_timeout);
}

ಸ್ಥಿರ ಶೂನ್ಯ work_timeout_stop_locked(ರಚನೆ k_work_q *ಸರತಿ)
{
	ಆದರೆ (K_TIMEOUT_EQ(ಸರತಿ->work_timeout, K_FOREVER)) {
		ಹಿಂತಿರುಗಿ;
	}

	(ಶೂನ್ಯ)z_try_abort_timeout(&ಸರತಿ->work_timeout_ದಾಖಲೆ);
}
#ಆದರೆ_ಅಂತ್ಯ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
ಸ್ಥಿರ ಶೂನ್ಯ work_ಸರತಿ_main(ಶೂನ್ಯ *workq_ptr, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3)
{
	ARG_ಬಳಸದ(p2);
	ARG_ಬಳಸದ(p3);

	ರಚನೆ k_work_q *ಸರತಿ = (ರಚನೆ k_work_q *)workq_ptr;

	ಆಗ (true) {
		sys_snode_t *ಗಂಟು;
		ರಚನೆ k_work *work = ಖಾಲಿ;
		k_work_handler_t handler = ಖಾಲಿ;
		k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
		bool yield;

		/* Check for and prepare any new work. */
		ಗಂಟು = sys_slist_get(&ಸರತಿ->pending);
		ಆದರೆ (ಗಂಟು != ಖಾಲಿ) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_ಗಣ(&ಸರತಿ->flags, K_WORK_ಸರತಿ_BUSY_ಬಿಟ್);
			work = CONTAINER_ದ(ಗಂಟು, ರಚನೆ k_work, ಗಂಟು);
			flag_ಗಣ(&work->flags, K_WORK_RUNNING_ಬಿಟ್);
			flag_clear(&work->flags, K_WORK_QUEUED_ಬಿಟ್);

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
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flag_test_and_clear(&ಸರತಿ->flags,
					       K_WORK_ಸರತಿ_DRAIN_ಬಿಟ್)) {
			/* Not busy and draining: move threads waiting for
			 * drain to ready state.  The held spinlock inhibits
			 * immediate reschedule; released threads get their
			 * chance when this invokes z_sched_wait() below.
			 *
			 * We don't touch K_WORK_QUEUE_PLUGGABLE, so getting
			 * here doesn't mean that the queue will allow new
			 * submissions.
			 */
			(ಶೂನ್ಯ)z_sched_wake_all(&ಸರತಿ->drainq, 1, ಖಾಲಿ);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_STOP_ಬಿಟ್)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_ಗಣ(&ಸರತಿ->flags, 0);
			k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
			ಹಿಂತಿರುಗಿ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		ಆದರೆ (work == ಖಾಲಿ) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(ಶೂನ್ಯ)z_sched_wait(&work_ಬೀಗ, ಕೀಲಿ, &ಸರತಿ->notifyq,
					   K_FOREVER, ಖಾಲಿ);
			ಮುಂದುವರಿ;
		}

#ಆದರೆ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_ಆರಂಭ_locked(ಸರತಿ, work);
#ಆದರೆ_ಅಂತ್ಯ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

		__ASSERT_NO_MSG(handler != ಖಾಲಿ);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

#ಆದರೆ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		ಆದರೆ (ಸರತಿ->ಪೂರ್ಣಗೊಂಡ) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
			ಆಗ (1) {
				k_sleep(K_FOREVER);
			}
			ಸಂಕೇತ_UNREACHABLE;
		}
		work_timeout_stop_locked(ಸರತಿ);
		ಸರತಿ->ಪೂರ್ಣಗೊಂಡ = true;
#ಆದರೆ_ಅಂತ್ಯ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_ಬಿಟ್);
		ಆದರೆ (flag_test(&work->flags, K_WORK_FLUSHING_ಬಿಟ್)) {
			finalize_flush_locked(work);
		}
		ಆದರೆ (flag_test(&work->flags, K_WORK_CANCELING_ಬಿಟ್)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&ಸರತಿ->flags, K_WORK_ಸರತಿ_BUSY_ಬಿಟ್);
		yield = (!flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_NO_YIELD_ಬಿಟ್) &&
			 !sys_slist_is_ಖಾಲಿಗಳು(&ಸರತಿ->pending));
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		ಆದರೆ (yield) {
			k_yield();
		}
	}
}

ಶೂನ್ಯ k_work_ಸರತಿ_init(ರಚನೆ k_work_q *ಸರತಿ)
{
	__ASSERT_NO_MSG(ಸರತಿ != ಖಾಲಿ);

	*ಸರತಿ = (ರಚನೆ k_work_q) {
		.flags = 0,
	};

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_work_ಸರತಿ, ಸರತಿ);
}

ಶೂನ್ಯ k_work_ಸರತಿ_ಸತತ(ರಚನೆ k_work_q *ಸರತಿ, ಸ್ಥಿರಾಂಕ ರಚನೆ k_work_ಸರತಿ_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_STARTED_ಬಿಟ್));

	uint32_t flags = K_WORK_ಸರತಿ_STARTED;

	ಆದರೆ ((cfg != ಖಾಲಿ) && cfg->no_yield) {
		flags |= K_WORK_ಸರತಿ_NO_YIELD;
	}

	ಆದರೆ ((cfg != ಖಾಲಿ) && (cfg->name != ಖಾಲಿ)) {
		k_thread_name_ಗಣ(_current, cfg->name);
	}

#ಆದರೆ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ಆದರೆ ((cfg != ಖಾಲಿ) && (cfg->work_timeout_ms)) {
		ಸರತಿ->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಸರತಿ->work_timeout = K_FOREVER;
	}
#ಆದರೆ_ಅಂತ್ಯ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&ಸರತಿ->pending);
	z_waitq_init(&ಸರತಿ->notifyq);
	z_waitq_init(&ಸರತಿ->drainq);
	ಸರತಿ->thread_id = _current;
	flags_ಗಣ(&ಸರತಿ->flags, flags);
	work_ಸರತಿ_main(ಸರತಿ, ಖಾಲಿ, ಖಾಲಿ);
}

ಶೂನ್ಯ k_work_ಸರತಿ_ಆರಂಭ(ರಚನೆ k_work_q *ಸರತಿ,
			k_thread_ರಾಶಿ_t *ರಾಶಿ,
			size_t ರಾಶಿ_ಗಾತ್ರ,
			ಪೂರ್ಣಾಂಕ prio,
			ಸ್ಥಿರಾಂಕ ರಚನೆ k_work_ಸರತಿ_config *cfg)
{
	__ASSERT_NO_MSG(ಸರತಿ);
	__ASSERT_NO_MSG(ರಾಶಿ);
	__ASSERT_NO_MSG(!flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_STARTED_ಬಿಟ್));

	/* In future, this whole function will be deprecated, but for now, we
	 * have to use the `thread` field to create a new thread in it.
	 */
	TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);

	uint32_t flags = K_WORK_ಸರತಿ_STARTED;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work_ಸರತಿ, ಆರಂಭ, ಸರತಿ);

	sys_slist_init(&ಸರತಿ->pending);
	z_waitq_init(&ಸರತಿ->notifyq);
	z_waitq_init(&ಸರತಿ->drainq);

	ಆದರೆ ((cfg != ಖಾಲಿ) && cfg->no_yield) {
		flags |= K_WORK_ಸರತಿ_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_ಗಣ(&ಸರತಿ->flags, flags);

	(ಶೂನ್ಯ)k_thread_create(&ಸರತಿ->thread, ರಾಶಿ, ರಾಶಿ_ಗಾತ್ರ,
			      work_ಸರತಿ_main, ಸರತಿ, ಖಾಲಿ, ಖಾಲಿ,
			      prio, 0, K_FOREVER);

	ಆದರೆ ((cfg != ಖಾಲಿ) && (cfg->name != ಖಾಲಿ)) {
		k_thread_name_ಗಣ(&ಸರತಿ->thread, cfg->name);
	}

	ಆದರೆ ((cfg != ಖಾಲಿ) && (cfg->essential)) {
		ಸರತಿ->thread.ತಳ.user_options |= K_ESSENTIAL;
	}

#ಆದರೆ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ಆದರೆ ((cfg != ಖಾಲಿ) && (cfg->work_timeout_ms)) {
		ಸರತಿ->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಸರತಿ->work_timeout = K_FOREVER;
	}
#ಆದರೆ_ಅಂತ್ಯ /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_ಆರಂಭ(&ಸರತಿ->thread);
	ಸರತಿ->thread_id = &ಸರತಿ->thread;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_ಸರತಿ, ಆರಂಭ, ಸರತಿ);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

ಪೂರ್ಣಾಂಕ k_work_ಸರತಿ_drain(ರಚನೆ k_work_q *ಸರತಿ,
		       bool plug)
{
	__ASSERT_NO_MSG(ಸರತಿ);
	__ASSERT_NO_MSG(!k_is_ಒಳ_isr());

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work_ಸರತಿ, drain, ಸರತಿ);

	ಪೂರ್ಣಾಂಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	ಆದರೆ (((flags_get(&ಸರತಿ->flags)
	      & (K_WORK_ಸರತಿ_BUSY | K_WORK_ಸರತಿ_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_ಖಾಲಿಗಳು(&ಸರತಿ->pending)) {
		flag_ಗಣ(&ಸರತಿ->flags, K_WORK_ಸರತಿ_DRAIN_ಬಿಟ್);
		ಆದರೆ (plug) {
			flag_ಗಣ(&ಸರತಿ->flags, K_WORK_ಸರತಿ_PLUGGED_ಬಿಟ್);
		}

		notify_ಸರತಿ_locked(ಸರತಿ);
		ret = z_sched_wait(&work_ಬೀಗ, ಕೀಲಿ, &ಸರತಿ->drainq,
				   K_FOREVER, ಖಾಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_ಸರತಿ, drain, ಸರತಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_ಸರತಿ_unplug(ರಚನೆ k_work_q *ಸರತಿ)
{
	__ASSERT_NO_MSG(ಸರತಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work_ಸರತಿ, unplug, ಸರತಿ);

	ಪೂರ್ಣಾಂಕ ret = -EALREADY;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	ಆದರೆ (flag_test_and_clear(&ಸರತಿ->flags, K_WORK_ಸರತಿ_PLUGGED_ಬಿಟ್)) {
		ret = 0;
	}

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_ಸರತಿ, unplug, ಸರತಿ, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_ಸರತಿ_stop(ರಚನೆ k_work_q *ಸರತಿ, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(ಸರತಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work_ಸರತಿ, stop, ಸರತಿ, timeout);

	ಆದರೆ (ಸರತಿ->thread_id) {
		ಆದರೆ (z_is_thread_essential(ಸರತಿ->thread_id)) {
			ಹಿಂತಿರುಗಿ -ENOTSUP;
		}
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	ಆದರೆ (!flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_STARTED_ಬಿಟ್)) {
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_ಸರತಿ, stop, ಸರತಿ, timeout, -EALREADY);
		ಹಿಂತಿರುಗಿ -EALREADY;
	}

	ಆದರೆ (!flag_test(&ಸರತಿ->flags, K_WORK_ಸರತಿ_PLUGGED_ಬಿಟ್)) {
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_ಸರತಿ, stop, ಸರತಿ, timeout, -EBUSY);
		ಹಿಂತಿರುಗಿ -EBUSY;
	}

	flag_ಗಣ(&ಸರತಿ->flags, K_WORK_ಸರತಿ_STOP_ಬಿಟ್);
	notify_ಸರತಿ_locked(ಸರತಿ);
	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_work_ಸರತಿ, stop, ಸರತಿ, timeout);
	ಆದರೆ (k_thread_join(ಸರತಿ->thread_id, timeout)) {
		ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
		flag_clear(&ಸರತಿ->flags, K_WORK_ಸರತಿ_STOP_ಬಿಟ್);
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_ಸರತಿ, stop, ಸರತಿ, timeout, -ETIMEDOUT);
		ಹಿಂತಿರುಗಿ -ETIMEDOUT;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work_ಸರತಿ, stop, ಸರತಿ, timeout, 0);
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
ಸ್ಥಿರ ಶೂನ್ಯ work_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ರಚನೆ k_work_delayable *dw
		= CONTAINER_ದ(ಗಮ್ಯ, ರಚನೆ k_work_delayable, timeout);
	ರಚನೆ k_work *wp = &dw->work;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
	ರಚನೆ k_work_q *ಸರತಿ = ಖಾಲಿ;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	ಆದರೆ (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_ಬಿಟ್)) {
		ಸರತಿ = dw->ಸರತಿ;
		(ಶೂನ್ಯ)submit_ಗಮ್ಯ_ಸರತಿ_locked(wp, &ಸರತಿ);
	}

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
}

ಶೂನ್ಯ k_work_init_delayable(ರಚನೆ k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != ಖಾಲಿ);
	__ASSERT_NO_MSG(handler != ಖಾಲಿ);

	*dwork = (ರಚನೆ k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ work_delayable_busy_get_locked(ಸ್ಥಿರಾಂಕ ರಚನೆ k_work_delayable *dwork)
{
	ಹಿಂತಿರುಗಿ flags_get(&dwork->work.flags) & K_WORK_ಮುಖವಾಡ;
}

ಪೂರ್ಣಾಂಕ k_work_delayable_busy_get(ಸ್ಥಿರಾಂಕ ರಚನೆ k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ಖಾಲಿ);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
	ಪೂರ್ಣಾಂಕ ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ವೇಳಾಪಟ್ಟಿ_for_ಸರತಿ_locked(ರಚನೆ k_work_q **queuep,
				     ರಚನೆ k_work_delayable *dwork,
				     k_timeout_t delay)
{
	ಪೂರ್ಣಾಂಕ ret = 1;
	ರಚನೆ k_work *work = &dwork->work;

	ಆದರೆ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		ಹಿಂತಿರುಗಿ submit_ಗಮ್ಯ_ಸರತಿ_locked(work, queuep);
	}

	flag_ಗಣ(&work->flags, K_WORK_DELAYED_ಬಿಟ್);
	dwork->ಸರತಿ = *queuep;

	/* Add timeout */
	z_ಕೂಡಿಸು_timeout(&dwork->timeout, work_timeout, delay);

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool unschedule_locked(ರಚನೆ k_work_delayable *dwork,
				     k_spinlock_ಕೀಲಿ_t *ಕೀಲಿ)
{
	bool ret = false;
	ರಚನೆ k_work *work = &dwork->work;

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
	ಆದರೆ (flag_test_and_clear(&work->flags, K_WORK_DELAYED_ಬಿಟ್)) {
		ಆಗ (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_ಬೀಗ, *ಕೀಲಿ);
			*ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
		}
		ret = true;
	}

	ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cancel_delayable_async_locked(ರಚನೆ k_work_delayable *dwork,
					 k_spinlock_ಕೀಲಿ_t *ಕೀಲಿ)
{
	(ಶೂನ್ಯ)unschedule_locked(dwork, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ cancel_async_locked(&dwork->work);
}

ಪೂರ್ಣಾಂಕ k_work_ವೇಳಾಪಟ್ಟಿ_for_ಸರತಿ(ರಚನೆ k_work_q *ಸರತಿ, ರಚನೆ k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(ಸರತಿ != ಖಾಲಿ);
	__ASSERT_NO_MSG(dwork != ಖಾಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, ವೇಳಾಪಟ್ಟಿ_for_ಸರತಿ, ಸರತಿ, dwork, delay);

	ರಚನೆ k_work *work = &dwork->work;
	ಪೂರ್ಣಾಂಕ ret = 0;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	/* Schedule the work item if it's idle or running. */
	ಆದರೆ ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = ವೇಳಾಪಟ್ಟಿ_for_ಸರತಿ_locked(&ಸರತಿ, dwork, delay);
	}

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, ವೇಳಾಪಟ್ಟಿ_for_ಸರತಿ, ಸರತಿ, dwork, delay, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_ವೇಳಾಪಟ್ಟಿ(ರಚನೆ k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, ವೇಳಾಪಟ್ಟಿ, dwork, delay);

	ಪೂರ್ಣಾಂಕ ret = k_work_ವೇಳಾಪಟ್ಟಿ_for_ಸರತಿ(&k_sys_work_q, dwork, delay);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, ವೇಳಾಪಟ್ಟಿ, dwork, delay, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_reschedule_for_ಸರತಿ(ರಚನೆ k_work_q *ಸರತಿ, ರಚನೆ k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(ಸರತಿ != ಖಾಲಿ);
	__ASSERT_NO_MSG(dwork != ಖಾಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_ಸರತಿ, ಸರತಿ, dwork, delay);

	ಪೂರ್ಣಾಂಕ ret;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	/* Remove any active scheduling. */
	(ಶೂನ್ಯ)unschedule_locked(dwork, &ಕೀಲಿ);

	/* Schedule the work item with the new parameters. */
	ret = ವೇಳಾಪಟ್ಟಿ_for_ಸರತಿ_locked(&ಸರತಿ, dwork, delay);

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_ಸರತಿ, ಸರತಿ, dwork, delay, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_reschedule(ರಚನೆ k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	ಪೂರ್ಣಾಂಕ ret = k_work_reschedule_for_ಸರತಿ(&k_sys_work_q, dwork, delay);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ k_work_cancel_delayable(ರಚನೆ k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ಖಾಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
	ಪೂರ್ಣಾಂಕ ret = cancel_delayable_async_locked(dwork, &ಕೀಲಿ);

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	ಹಿಂತಿರುಗಿ ret;
}

bool k_work_cancel_delayable_sync(ರಚನೆ k_work_delayable *dwork,
				  ರಚನೆ k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ಖಾಲಿ);
	__ASSERT_NO_MSG(sync != ಖಾಲಿ);
	__ASSERT_NO_MSG(!k_is_ಒಳ_isr());
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	ರಚನೆ z_work_canceller *canceller = &sync->canceller;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	ಆದರೆ (pending) {
		(ಶೂನ್ಯ)cancel_delayable_async_locked(dwork, &ಕೀಲಿ);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	ಆದರೆ (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	ಹಿಂತಿರುಗಿ pending;
}

bool k_work_flush_delayable(ರಚನೆ k_work_delayable *dwork,
			    ರಚನೆ k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ಖಾಲಿ);
	__ASSERT_NO_MSG(sync != ಖಾಲಿ);
	__ASSERT_NO_MSG(!k_is_ಒಳ_isr());
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	ರಚನೆ k_work *work = &dwork->work;
	ರಚನೆ z_work_flusher *flusher = &sync->flusher;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&work_ಬೀಗ);

	/* If it's idle release the lock and return immediately. */
	ಆದರೆ (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		ಹಿಂತಿರುಗಿ false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	ಆದರೆ (unschedule_locked(dwork, &ಕೀಲಿ)) {
		ರಚನೆ k_work_q *ಸರತಿ = dwork->ಸರತಿ;

		(ಶೂನ್ಯ)submit_ಗಮ್ಯ_ಸರತಿ_locked(work, &ಸರತಿ);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_ಬೀಗ, ಕೀಲಿ);

	/* If necessary wait until the flusher item completes */
	ಆದರೆ (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	ಹಿಂತಿರುಗಿ need_flush;
}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */
