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

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/printk.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ flag_clear(uint32_t *flagp,
			      uint32_t bit)
{
	*flagp &= ~BIT(bit);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ flag_set(uint32_t *flagp,
			    uint32_t bit)
{
	*flagp |= BIT(bit);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool flag_test(ଅଚଳ uint32_t *flagp,
			     uint32_t bit)
{
	ଫେରନ୍ତୁ (*flagp & BIT(bit)) != 0U;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool flag_test_and_clear(uint32_t *flagp,
				       ପୂର୍ଣ୍ଣସଂଖ୍ୟା bit)
{
	bool ret = flag_test(flagp, bit);

	flag_clear(flagp, bit);

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ flags_set(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint32_t flags_get(ଅଚଳ uint32_t *flagp)
{
	ଫେରନ୍ତୁ *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
ସ୍ଥିର ସଂରଚନା k_spinlock work_lock;

/* Invoked by work thread */
ସ୍ଥିର ଶୂନ୍ୟ handle_flush(ସଂରଚନା k_work *work) { }

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_flusher(ସଂରଚନା z_work_flusher *flusher)
{
	ସଂରଚନା k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_set(&work->flags, K_WORK_FLUSHING_BIT);
}

/* List of pending cancellations. */
ସ୍ଥିର sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_work_cancel(ସଂରଚନା z_work_canceller *canceler,
				    ସଂରଚନା k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->ଗଣ୍ଠି);
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
ସ୍ଥିର ଶୂନ୍ୟ finalize_flush_locked(ସଂରଚନା k_work *work)
{
	ସଂରଚନା z_work_flusher *flusher
		= CONTAINER_OF(work, ସଂରଚନା z_work_flusher, work);

	flag_clear(&work->flags, K_WORK_FLUSHING_BIT);

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
ସ୍ଥିର ଶୂନ୍ୟ finalize_cancel_locked(ସଂରଚନା k_work *work)
{
	ସଂରଚନା z_work_canceller *wc, *tmp;
	sys_snode_t *prev = ଖାଲି;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_BIT);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pending_cancels, wc, tmp, ଗଣ୍ଠି) {
		ଯଦି (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->ଗଣ୍ଠି);
			k_sem_give(&wc->sem);
		} ଅନ୍ୟଥା {
			prev = &wc->ଗଣ୍ଠି;
		}
	}
}

ଶୂନ୍ୟ k_work_init(ସଂରଚନା k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != ଖାଲି);
	__ASSERT_NO_MSG(handler != ଖାଲି);

	*work = (ସଂରଚନା k_work)Z_WORK_INITIALIZER(handler);

	SYS_PORT_TRACING_OBJ_INIT(k_work, work);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା work_busy_get_locked(ଅଚଳ ସଂରଚନା k_work *work)
{
	ଫେରନ୍ତୁ flags_get(&work->flags) & K_WORK_MASK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_busy_get(ଅଚଳ ସଂରଚନା k_work *work)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = work_busy_get_locked(work);

	k_spin_unlock(&work_lock, ଚାବି);

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର ଶୂନ୍ୟ queue_flusher_locked(ସଂରଚନା k_work_q *queue,
				 ସଂରଚନା k_work *work,
				 ସଂରଚନା z_work_flusher *flusher)
{
	init_flusher(flusher);

	ଯଦି ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_ଭର୍ତ୍ତି(&queue->pending, &work->ଗଣ୍ଠି,
				 &flusher->work.ଗଣ୍ଠି);
	} ଅନ୍ୟଥା {
		sys_slist_prepend(&queue->pending, &flusher->work.ଗଣ୍ଠି);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ queue_remove_locked(ସଂରଚନା k_work_q *queue,
				       ସଂରଚନା k_work *work)
{
	ଯଦି (flag_test_and_clear(&work->flags, K_WORK_QUEUED_BIT)) {
		(ଶୂନ୍ୟ)sys_slist_find_and_remove(&queue->pending, &work->ଗଣ୍ଠି);
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool notify_queue_locked(ସଂରଚନା k_work_q *queue)
{
	bool rv = false;

	ଯଦି (queue != ଖାଲି) {
		rv = z_sched_wake(&queue->notifyq, 0, ଖାଲି);
	}

	ଫେରନ୍ତୁ rv;
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା queue_submit_locked(ସଂରଚନା k_work_q *queue,
				      ସଂରଚନା k_work *work)
{
	ଯଦି (queue == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	bool chained = (_current == queue->thread_id) && !k_is_in_isr();
	bool draining = flag_test(&queue->flags, K_WORK_QUEUE_DRAIN_BIT);
	bool plugged = flag_test(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT);

	/* Test for acceptability, in priority order:
	 *
	 * * -ENODEV if the queue isn't running.
	 * * -EBUSY if draining and not chained
	 * * -EBUSY if plugged and not draining
	 * * otherwise OK
	 */
	ଯଦି (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		ret = -ENODEV;
	} ଅନ୍ୟଥା ଯଦି (draining && !chained) {
		ret = -EBUSY;
	} ଅନ୍ୟଥା ଯଦି (plugged && !draining) {
		ret = -EBUSY;
	} ଅନ୍ୟଥା {
		sys_slist_append(&queue->pending, &work->ଗଣ୍ଠି);
		ret = 1;
		(ଶୂନ୍ୟ)notify_queue_locked(queue);
	}

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା submit_to_queue_locked(ସଂରଚନା k_work *work,
				  ସଂରଚନା k_work_q **queuep)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	ଯଦି (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Disallowed */
		ret = -EBUSY;
	} ଅନ୍ୟଥା ଯଦି (!flag_test(&work->flags, K_WORK_QUEUED_BIT)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		ଯଦି (*queuep == ଖାଲି) {
			*queuep = work->queue;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		ଯଦି (flag_test(&work->flags, K_WORK_RUNNING_BIT)) {
			__ASSERT_NO_MSG(work->queue != ଖାଲି);
			*queuep = work->queue;
			ret = 2;
		}

		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = queue_submit_locked(*queuep, work);

		ଯଦି (rc < 0) {
			ret = rc;
		} ଅନ୍ୟଥା {
			flag_set(&work->flags, K_WORK_QUEUED_BIT);
			work->queue = *queuep;
		}
	} ଅନ୍ୟଥା {
		/* Already queued, do nothing. */
	}

	ଯଦି (ret <= 0) {
		*queuep = ଖାଲି;
	}

	ଫେରନ୍ତୁ ret;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_work_submit_to_queue(ସଂରଚନା k_work_q *queue,
		  ସଂରଚନା k_work *work)
{
	__ASSERT_NO_MSG(work != ଖାଲି);
	__ASSERT_NO_MSG(work->handler != ଖାଲି);

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = submit_to_queue_locked(work, &queue);

	/* No need to reschedule if the submission failed. */
	ଯଦି (ret > 0) {
		z_reschedule(&work_lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&work_lock, ଚାବି);
	}

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_submit_to_queue(ସଂରଚନା k_work_q *queue,
			    ସଂରଚନା k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit_to_queue, queue, work);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = z_work_submit_to_queue(queue, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit_to_queue, queue, work, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_submit(ସଂରଚନା k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = z_work_submit_to_queue(&k_sys_work_q, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର bool work_flush_locked(ସଂରଚନା k_work *work,
			      ସଂରଚନା z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	ଯଦି (need_flush) {
		ସଂରଚନା k_work_q *queue = work->queue;

		__ASSERT_NO_MSG(queue != ଖାଲି);

		queue_flusher_locked(queue, work, flusher);
		notify_queue_locked(queue);
	}

	ଫେରନ୍ତୁ need_flush;
}

bool k_work_flush(ସଂରଚନା k_work *work,
		  ସଂରଚନା k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ଖାଲି);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
	__ASSERT_NO_MSG(sync != ଖାଲି);
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	ସଂରଚନା z_work_flusher *flusher = &sync->flusher;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, ଚାବି);

	/* If necessary wait until the flusher item completes */
	ଯଦି (need_flush) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	ଫେରନ୍ତୁ need_flush;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cancel_async_locked(ସଂରଚନା k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	ଯଦି (!flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Remove it from the queue, if it's queued. */
		queue_remove_locked(work->queue, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = work_busy_get_locked(work);

	ଯଦି (ret != 0) {
		flag_set(&work->flags, K_WORK_CANCELING_BIT);
		ret = work_busy_get_locked(work);
	}

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର bool cancel_sync_locked(ସଂରଚନା k_work *work,
			       ସଂରଚନା z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_BIT);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	ଯଦି (ret) {
		init_work_cancel(canceller, work);
	}

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_cancel(ସଂରଚନା k_work *work)
{
	__ASSERT_NO_MSG(work != ଖାଲି);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = cancel_async_locked(work);

	k_spin_unlock(&work_lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	ଫେରନ୍ତୁ ret;
}

bool k_work_cancel_sync(ସଂରଚନା k_work *work,
			ସଂରଚନା k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != ଖାଲି);
	__ASSERT_NO_MSG(sync != ଖାଲି);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	ସଂରଚନା z_work_canceller *canceller = &sync->canceller;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	ଯଦି (pending) {
		(ଶୂନ୍ୟ)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_lock, ଚାବି);

	ଯଦି (need_wait) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	ଫେରନ୍ତୁ pending;
}

#ଯଦି defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
ସ୍ଥିର ଶୂନ୍ୟ work_timeout_handler(ସଂରଚନା _timeout *record)
{
	ସଂରଚନା k_work_q *queue = CONTAINER_OF(record, ସଂରଚନା k_work_q, work_timeout_record);
	ସଂରଚନା k_work *work = ଖାଲି;
	k_work_handler_t handler = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *name;
	ଅଚଳ ଅକ୍ଷର *space = " ";
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	work = queue->work;
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
	ଯଦି (queue->finished || z_timeout_inflight_superseded(record)) {
		k_spin_unlock(&work_lock, ଚାବି);
		ଫେରନ୍ତୁ;
	}
	queue->finished = true;

	k_spin_unlock(&work_lock, ଚାବି);

	name = k_thread_name_get(queue->thread_id);
	ଯଦି (name == ଖାଲି) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		queue, space, name, work, handler);

	k_thread_abort(queue->thread_id);
}

ସ୍ଥିର ଶୂନ୍ୟ work_timeout_start_locked(ସଂରଚନା k_work_q *queue, ସଂରଚନା k_work *work)
{
	queue->finished = false;

	ଯଦି (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		ଫେରନ୍ତୁ;
	}

	queue->work = work;
	z_add_timeout(&queue->work_timeout_record, work_timeout_handler, queue->work_timeout);
}

ସ୍ଥିର ଶୂନ୍ୟ work_timeout_stop_locked(ସଂରଚନା k_work_q *queue)
{
	ଯଦି (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		ଫେରନ୍ତୁ;
	}

	(ଶୂନ୍ୟ)z_try_abort_timeout(&queue->work_timeout_record);
}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
ସ୍ଥିର ଶୂନ୍ୟ work_queue_main(ଶୂନ୍ୟ *workq_ptr, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	ସଂରଚନା k_work_q *queue = (ସଂରଚନା k_work_q *)workq_ptr;

	ଯେତେବେଳେ (true) {
		sys_snode_t *ଗଣ୍ଠି;
		ସଂରଚନା k_work *work = ଖାଲି;
		k_work_handler_t handler = ଖାଲି;
		k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
		bool yield;

		/* Check for and prepare any new work. */
		ଗଣ୍ଠି = sys_slist_get(&queue->pending);
		ଯଦି (ଗଣ୍ଠି != ଖାଲି) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_set(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
			work = CONTAINER_OF(ଗଣ୍ଠି, ସଂରଚନା k_work, ଗଣ୍ଠି);
			flag_set(&work->flags, K_WORK_RUNNING_BIT);
			flag_clear(&work->flags, K_WORK_QUEUED_BIT);

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
		} ଅନ୍ୟଥା ଯଦି (flag_test_and_clear(&queue->flags,
					       K_WORK_QUEUE_DRAIN_BIT)) {
			/* Not busy and draining: move threads waiting for
			 * drain to ready state.  The held spinlock inhibits
			 * immediate reschedule; released threads get their
			 * chance when this invokes z_sched_wait() below.
			 *
			 * We don't touch K_WORK_QUEUE_PLUGGABLE, so getting
			 * here doesn't mean that the queue will allow new
			 * submissions.
			 */
			(ଶୂନ୍ୟ)z_sched_wake_all(&queue->drainq, 1, ଖାଲି);
		} ଅନ୍ୟଥା ଯଦି (flag_test(&queue->flags, K_WORK_QUEUE_STOP_BIT)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_set(&queue->flags, 0);
			k_spin_unlock(&work_lock, ଚାବି);
			ଫେରନ୍ତୁ;
		} ଅନ୍ୟଥା {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		ଯଦି (work == ଖାଲି) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(ଶୂନ୍ୟ)z_sched_wait(&work_lock, ଚାବି, &queue->notifyq,
					   K_FOREVER, ଖାଲି);
			ଜାରି;
		}

#ଯଦି defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_start_locked(queue, work);
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_lock, ଚାବି);

		__ASSERT_NO_MSG(handler != ଖାଲି);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		ଚାବି = k_spin_lock(&work_lock);

#ଯଦି defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		ଯଦି (queue->finished) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_lock, ଚାବି);
			ଯେତେବେଳେ (1) {
				k_sleep(K_FOREVER);
			}
			CODE_UNREACHABLE;
		}
		work_timeout_stop_locked(queue);
		queue->finished = true;
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_BIT);
		ଯଦି (flag_test(&work->flags, K_WORK_FLUSHING_BIT)) {
			finalize_flush_locked(work);
		}
		ଯଦି (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
		yield = (!flag_test(&queue->flags, K_WORK_QUEUE_NO_YIELD_BIT) &&
			 !sys_slist_is_empty(&queue->pending));
		k_spin_unlock(&work_lock, ଚାବି);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		ଯଦି (yield) {
			k_yield();
		}
	}
}

ଶୂନ୍ୟ k_work_queue_init(ସଂରଚନା k_work_q *queue)
{
	__ASSERT_NO_MSG(queue != ଖାଲି);

	*queue = (ସଂରଚନା k_work_q) {
		.flags = 0,
	};

	SYS_PORT_TRACING_OBJ_INIT(k_work_queue, queue);
}

ଶୂନ୍ୟ k_work_queue_run(ସଂରଚନା k_work_q *queue, ଅଚଳ ସଂରଚନା k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT));

	uint32_t flags = K_WORK_QUEUE_STARTED;

	ଯଦି ((cfg != ଖାଲି) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	ଯଦି ((cfg != ଖାଲି) && (cfg->name != ଖାଲି)) {
		k_thread_name_set(_current, cfg->name);
	}

#ଯଦି defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ଯଦି ((cfg != ଖାଲି) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ଅନ୍ୟଥା {
		queue->work_timeout = K_FOREVER;
	}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&queue->pending);
	z_waitq_init(&queue->notifyq);
	z_waitq_init(&queue->drainq);
	queue->thread_id = _current;
	flags_set(&queue->flags, flags);
	work_queue_main(queue, ଖାଲି, ଖାଲି);
}

ଶୂନ୍ୟ k_work_queue_start(ସଂରଚନା k_work_q *queue,
			k_thread_ଗଦା_t *ଗଦା,
			size_t ଗଦା_size,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio,
			ଅଚଳ ସଂରଚନା k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(ଗଦା);
	__ASSERT_NO_MSG(!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT));

	/* In future, this whole function will be deprecated, but for now, we
	 * have to use the `thread` field to create a new thread in it.
	 */
	TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);

	uint32_t flags = K_WORK_QUEUE_STARTED;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, start, queue);

	sys_slist_init(&queue->pending);
	z_waitq_init(&queue->notifyq);
	z_waitq_init(&queue->drainq);

	ଯଦି ((cfg != ଖାଲି) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_set(&queue->flags, flags);

	(ଶୂନ୍ୟ)k_thread_create(&queue->thread, ଗଦା, ଗଦା_size,
			      work_queue_main, queue, ଖାଲି, ଖାଲି,
			      prio, 0, K_FOREVER);

	ଯଦି ((cfg != ଖାଲି) && (cfg->name != ଖାଲି)) {
		k_thread_name_set(&queue->thread, cfg->name);
	}

	ଯଦି ((cfg != ଖାଲି) && (cfg->essential)) {
		queue->thread.base.user_options |= K_ESSENTIAL;
	}

#ଯଦି defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	ଯଦି ((cfg != ଖାଲି) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ଅନ୍ୟଥା {
		queue->work_timeout = K_FOREVER;
	}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_start(&queue->thread);
	queue->thread_id = &queue->thread;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, start, queue);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_queue_drain(ସଂରଚନା k_work_q *queue,
		       bool plug)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(!k_is_in_isr());

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, drain, queue);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	ଯଦି (((flags_get(&queue->flags)
	      & (K_WORK_QUEUE_BUSY | K_WORK_QUEUE_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_empty(&queue->pending)) {
		flag_set(&queue->flags, K_WORK_QUEUE_DRAIN_BIT);
		ଯଦି (plug) {
			flag_set(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT);
		}

		notify_queue_locked(queue);
		ret = z_sched_wait(&work_lock, ଚାବି, &queue->drainq,
				   K_FOREVER, ଖାଲି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&work_lock, ଚାବି);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, drain, queue, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_queue_unplug(ସଂରଚନା k_work_q *queue)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, unplug, queue);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -EALREADY;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	ଯଦି (flag_test_and_clear(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		ret = 0;
	}

	k_spin_unlock(&work_lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, unplug, queue, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_queue_stop(ସଂରଚନା k_work_q *queue, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, stop, queue, timeout);

	ଯଦି (queue->thread_id) {
		ଯଦି (z_is_thread_essential(queue->thread_id)) {
			ଫେରନ୍ତୁ -ENOTSUP;
		}
	}

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	ଯଦି (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		k_spin_unlock(&work_lock, ଚାବି);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EALREADY);
		ଫେରନ୍ତୁ -EALREADY;
	}

	ଯଦି (!flag_test(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		k_spin_unlock(&work_lock, ଚାବି);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EBUSY);
		ଫେରନ୍ତୁ -EBUSY;
	}

	flag_set(&queue->flags, K_WORK_QUEUE_STOP_BIT);
	notify_queue_locked(queue);
	k_spin_unlock(&work_lock, ଚାବି);
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work_queue, stop, queue, timeout);
	ଯଦି (k_thread_join(queue->thread_id, timeout)) {
		ଚାବି = k_spin_lock(&work_lock);
		flag_clear(&queue->flags, K_WORK_QUEUE_STOP_BIT);
		k_spin_unlock(&work_lock, ଚାବି);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -ETIMEDOUT);
		ଫେରନ୍ତୁ -ETIMEDOUT;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, 0);
	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
ସ୍ଥିର ଶୂନ୍ୟ work_timeout(ସଂରଚନା _timeout *to)
{
	ସଂରଚନା k_work_delayable *dw
		= CONTAINER_OF(to, ସଂରଚନା k_work_delayable, timeout);
	ସଂରଚନା k_work *wp = &dw->work;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
	ସଂରଚନା k_work_q *queue = ଖାଲି;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	ଯଦି (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_BIT)) {
		queue = dw->queue;
		(ଶୂନ୍ୟ)submit_to_queue_locked(wp, &queue);
	}

	k_spin_unlock(&work_lock, ଚାବି);
}

ଶୂନ୍ୟ k_work_init_delayable(ସଂରଚନା k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != ଖାଲି);
	__ASSERT_NO_MSG(handler != ଖାଲି);

	*dwork = (ସଂରଚନା k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା work_delayable_busy_get_locked(ଅଚଳ ସଂରଚନା k_work_delayable *dwork)
{
	ଫେରନ୍ତୁ flags_get(&dwork->work.flags) & K_WORK_MASK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_delayable_busy_get(ଅଚଳ ସଂରଚନା k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ଖାଲି);

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_lock, ଚାବି);
	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schedule_for_queue_locked(ସଂରଚନା k_work_q **queuep,
				     ସଂରଚନା k_work_delayable *dwork,
				     k_timeout_t delay)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 1;
	ସଂରଚନା k_work *work = &dwork->work;

	ଯଦି (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		ଫେରନ୍ତୁ submit_to_queue_locked(work, queuep);
	}

	flag_set(&work->flags, K_WORK_DELAYED_BIT);
	dwork->queue = *queuep;

	/* Add timeout */
	z_add_timeout(&dwork->timeout, work_timeout, delay);

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool unschedule_locked(ସଂରଚନା k_work_delayable *dwork,
				     k_spinlock_ଚାବି_t *ଚାବି)
{
	bool ret = false;
	ସଂରଚନା k_work *work = &dwork->work;

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
	ଯଦି (flag_test_and_clear(&work->flags, K_WORK_DELAYED_BIT)) {
		ଯେତେବେଳେ (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_lock, *ଚାବି);
			*ଚାବି = k_spin_lock(&work_lock);
		}
		ret = true;
	}

	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cancel_delayable_async_locked(ସଂରଚନା k_work_delayable *dwork,
					 k_spinlock_ଚାବି_t *ଚାବି)
{
	(ଶୂନ୍ୟ)unschedule_locked(dwork, ଚାବି);

	ଫେରନ୍ତୁ cancel_async_locked(&dwork->work);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_schedule_for_queue(ସଂରଚନା k_work_q *queue, ସଂରଚନା k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != ଖାଲି);
	__ASSERT_NO_MSG(dwork != ଖାଲି);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule_for_queue, queue, dwork, delay);

	ସଂରଚନା k_work *work = &dwork->work;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	/* Schedule the work item if it's idle or running. */
	ଯଦି ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = schedule_for_queue_locked(&queue, dwork, delay);
	}

	k_spin_unlock(&work_lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule_for_queue, queue, dwork, delay, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_schedule(ସଂରଚନା k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule, dwork, delay);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = k_work_schedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule, dwork, delay, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_reschedule_for_queue(ସଂରଚନା k_work_q *queue, ସଂରଚନା k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != ଖାଲି);
	__ASSERT_NO_MSG(dwork != ଖାଲି);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_queue, queue, dwork, delay);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	/* Remove any active scheduling. */
	(ଶୂନ୍ୟ)unschedule_locked(dwork, &ଚାବି);

	/* Schedule the work item with the new parameters. */
	ret = schedule_for_queue_locked(&queue, dwork, delay);

	k_spin_unlock(&work_lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_queue, queue, dwork, delay, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_reschedule(ସଂରଚନା k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = k_work_reschedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_work_cancel_delayable(ସଂରଚନା k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != ଖାଲି);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = cancel_delayable_async_locked(dwork, &ଚାବି);

	k_spin_unlock(&work_lock, ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	ଫେରନ୍ତୁ ret;
}

bool k_work_cancel_delayable_sync(ସଂରଚନା k_work_delayable *dwork,
				  ସଂରଚନା k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ଖାଲି);
	__ASSERT_NO_MSG(sync != ଖାଲି);
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	ସଂରଚନା z_work_canceller *canceller = &sync->canceller;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	ଯଦି (pending) {
		(ଶୂନ୍ୟ)cancel_delayable_async_locked(dwork, &ଚାବି);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_lock, ଚାବି);

	ଯଦି (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	ଫେରନ୍ତୁ pending;
}

bool k_work_flush_delayable(ସଂରଚନା k_work_delayable *dwork,
			    ସଂରଚନା k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != ଖାଲି);
	__ASSERT_NO_MSG(sync != ଖାଲି);
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	ସଂରଚନା k_work *work = &dwork->work;
	ସଂରଚନା z_work_flusher *flusher = &sync->flusher;
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&work_lock);

	/* If it's idle release the lock and return immediately. */
	ଯଦି (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_lock, ଚାବି);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		ଫେରନ୍ତୁ false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	ଯଦି (unschedule_locked(dwork, &ଚାବି)) {
		ସଂରଚନା k_work_q *queue = dwork->queue;

		(ଶୂନ୍ୟ)submit_to_queue_locked(work, &queue);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, ଚାବି);

	/* If necessary wait until the flusher item completes */
	ଯଦି (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	ଫେରନ୍ତୁ need_flush;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */
