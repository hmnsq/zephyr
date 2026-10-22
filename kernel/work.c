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

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <wait_q.h>
#অন্তর্ভাৱ <zephyr/spinlock.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <scheduler.h>
#অন্তর্ভাৱ <zephyr/sys/printk.h>
#অন্তর্ভাৱ <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

স্থির অন্তঃস্থ শূন্য flag_clear(uint32_t *flagp,
			      uint32_t bit)
{
	*flagp &= ~BIT(bit);
}

স্থির অন্তঃস্থ শূন্য flag_set(uint32_t *flagp,
			    uint32_t bit)
{
	*flagp |= BIT(bit);
}

স্থির অন্তঃস্থ bool flag_test(ধ্রুৱ uint32_t *flagp,
			     uint32_t bit)
{
	ঘূৰাই (*flagp & BIT(bit)) != 0U;
}

স্থির অন্তঃস্থ bool flag_test_and_clear(uint32_t *flagp,
				       পূর্ণাঙ্ক bit)
{
	bool ret = flag_test(flagp, bit);

	flag_clear(flagp, bit);

	ঘূৰাই ret;
}

স্থির অন্তঃস্থ শূন্য flags_set(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

স্থির অন্তঃস্থ uint32_t flags_get(ধ্রুৱ uint32_t *flagp)
{
	ঘূৰাই *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
স্থির সংরচনা k_spinlock work_lock;

/* Invoked by work thread */
স্থির শূন্য handle_flush(সংরচনা k_work *work) { }

স্থির অন্তঃস্থ শূন্য init_flusher(সংরচনা z_work_flusher *flusher)
{
	সংরচনা k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_set(&work->flags, K_WORK_FLUSHING_BIT);
}

/* List of pending cancellations. */
স্থির sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
স্থির অন্তঃস্থ শূন্য init_work_cancel(সংরচনা z_work_canceller *canceler,
				    সংরচনা k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->গ্ৰন্থি);
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
স্থির শূন্য finalize_flush_locked(সংরচনা k_work *work)
{
	সংরচনা z_work_flusher *flusher
		= CONTAINER_OF(work, সংরচনা z_work_flusher, work);

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
স্থির শূন্য finalize_cancel_locked(সংরচনা k_work *work)
{
	সংরচনা z_work_canceller *wc, *tmp;
	sys_snode_t *prev = রিক্ত;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_BIT);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pending_cancels, wc, tmp, গ্ৰন্থি) {
		যদি (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->গ্ৰন্থি);
			k_sem_give(&wc->sem);
		} অন্যথা {
			prev = &wc->গ্ৰন্থি;
		}
	}
}

শূন্য k_work_init(সংরচনা k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != রিক্ত);
	__ASSERT_NO_MSG(handler != রিক্ত);

	*work = (সংরচনা k_work)Z_WORK_INITIALIZER(handler);

	SYS_PORT_TRACING_OBJ_INIT(k_work, work);
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক work_busy_get_locked(ধ্রুৱ সংরচনা k_work *work)
{
	ঘূৰাই flags_get(&work->flags) & K_WORK_MASK;
}

পূর্ণাঙ্ক k_work_busy_get(ধ্রুৱ সংরচনা k_work *work)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
	পূর্ণাঙ্ক ret = work_busy_get_locked(work);

	k_spin_unlock(&work_lock, চাবি);

	ঘূৰাই ret;
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
স্থির শূন্য queue_flusher_locked(সংরচনা k_work_q *queue,
				 সংরচনা k_work *work,
				 সংরচনা z_work_flusher *flusher)
{
	init_flusher(flusher);

	যদি ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_ঢোকাও(&queue->pending, &work->গ্ৰন্থি,
				 &flusher->work.গ্ৰন্থি);
	} অন্যথা {
		sys_slist_prepend(&queue->pending, &flusher->work.গ্ৰন্থি);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
স্থির অন্তঃস্থ শূন্য queue_remove_locked(সংরচনা k_work_q *queue,
				       সংরচনা k_work *work)
{
	যদি (flag_test_and_clear(&work->flags, K_WORK_QUEUED_BIT)) {
		(শূন্য)sys_slist_find_and_remove(&queue->pending, &work->গ্ৰন্থি);
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
স্থির অন্তঃস্থ bool notify_queue_locked(সংরচনা k_work_q *queue)
{
	bool rv = false;

	যদি (queue != রিক্ত) {
		rv = z_sched_wake(&queue->notifyq, 0, রিক্ত);
	}

	ঘূৰাই rv;
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
স্থির অন্তঃস্থ পূর্ণাঙ্ক queue_submit_locked(সংরচনা k_work_q *queue,
				      সংরচনা k_work *work)
{
	যদি (queue == রিক্ত) {
		ঘূৰাই -EINVAL;
	}

	পূর্ণাঙ্ক ret;
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
	যদি (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		ret = -ENODEV;
	} অন্যথা যদি (draining && !chained) {
		ret = -EBUSY;
	} অন্যথা যদি (plugged && !draining) {
		ret = -EBUSY;
	} অন্যথা {
		sys_slist_append(&queue->pending, &work->গ্ৰন্থি);
		ret = 1;
		(শূন্য)notify_queue_locked(queue);
	}

	ঘূৰাই ret;
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
স্থির পূর্ণাঙ্ক submit_to_queue_locked(সংরচনা k_work *work,
				  সংরচনা k_work_q **queuep)
{
	পূর্ণাঙ্ক ret = 0;

	যদি (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Disallowed */
		ret = -EBUSY;
	} অন্যথা যদি (!flag_test(&work->flags, K_WORK_QUEUED_BIT)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		যদি (*queuep == রিক্ত) {
			*queuep = work->queue;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		যদি (flag_test(&work->flags, K_WORK_RUNNING_BIT)) {
			__ASSERT_NO_MSG(work->queue != রিক্ত);
			*queuep = work->queue;
			ret = 2;
		}

		পূর্ণাঙ্ক rc = queue_submit_locked(*queuep, work);

		যদি (rc < 0) {
			ret = rc;
		} অন্যথা {
			flag_set(&work->flags, K_WORK_QUEUED_BIT);
			work->queue = *queuep;
		}
	} অন্যথা {
		/* Already queued, do nothing. */
	}

	যদি (ret <= 0) {
		*queuep = রিক্ত;
	}

	ঘূৰাই ret;
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
পূর্ণাঙ্ক z_work_submit_to_queue(সংরচনা k_work_q *queue,
		  সংরচনা k_work *work)
{
	__ASSERT_NO_MSG(work != রিক্ত);
	__ASSERT_NO_MSG(work->handler != রিক্ত);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	পূর্ণাঙ্ক ret = submit_to_queue_locked(work, &queue);

	/* No need to reschedule if the submission failed. */
	যদি (ret > 0) {
		z_reschedule(&work_lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&work_lock, চাবি);
	}

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_submit_to_queue(সংরচনা k_work_q *queue,
			    সংরচনা k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit_to_queue, queue, work);

	পূর্ণাঙ্ক ret = z_work_submit_to_queue(queue, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit_to_queue, queue, work, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_submit(সংরচনা k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	পূর্ণাঙ্ক ret = z_work_submit_to_queue(&k_sys_work_q, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	ঘূৰাই ret;
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
স্থির bool work_flush_locked(সংরচনা k_work *work,
			      সংরচনা z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	যদি (need_flush) {
		সংরচনা k_work_q *queue = work->queue;

		__ASSERT_NO_MSG(queue != রিক্ত);

		queue_flusher_locked(queue, work, flusher);
		notify_queue_locked(queue);
	}

	ঘূৰাই need_flush;
}

bool k_work_flush(সংরচনা k_work *work,
		  সংরচনা k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != রিক্ত);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
	__ASSERT_NO_MSG(sync != রিক্ত);
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	সংরচনা z_work_flusher *flusher = &sync->flusher;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, চাবি);

	/* If necessary wait until the flusher item completes */
	যদি (need_flush) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	ঘূৰাই need_flush;
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
স্থির পূর্ণাঙ্ক cancel_async_locked(সংরচনা k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	যদি (!flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Remove it from the queue, if it's queued. */
		queue_remove_locked(work->queue, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	পূর্ণাঙ্ক ret = work_busy_get_locked(work);

	যদি (ret != 0) {
		flag_set(&work->flags, K_WORK_CANCELING_BIT);
		ret = work_busy_get_locked(work);
	}

	ঘূৰাই ret;
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
স্থির bool cancel_sync_locked(সংরচনা k_work *work,
			       সংরচনা z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_BIT);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	যদি (ret) {
		init_work_cancel(canceller, work);
	}

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_cancel(সংরচনা k_work *work)
{
	__ASSERT_NO_MSG(work != রিক্ত);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
	পূর্ণাঙ্ক ret = cancel_async_locked(work);

	k_spin_unlock(&work_lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	ঘূৰাই ret;
}

bool k_work_cancel_sync(সংরচনা k_work *work,
			সংরচনা k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != রিক্ত);
	__ASSERT_NO_MSG(sync != রিক্ত);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	সংরচনা z_work_canceller *canceller = &sync->canceller;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	যদি (pending) {
		(শূন্য)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_lock, চাবি);

	যদি (need_wait) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	ঘূৰাই pending;
}

#যদি defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
স্থির শূন্য work_timeout_handler(সংরচনা _timeout *record)
{
	সংরচনা k_work_q *queue = CONTAINER_OF(record, সংরচনা k_work_q, work_timeout_record);
	সংরচনা k_work *work = রিক্ত;
	k_work_handler_t handler = রিক্ত;
	ধ্রুৱ ৱর্ণ *name;
	ধ্রুৱ ৱর্ণ *space = " ";
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

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
	যদি (queue->finished || z_timeout_inflight_superseded(record)) {
		k_spin_unlock(&work_lock, চাবি);
		ঘূৰাই;
	}
	queue->finished = true;

	k_spin_unlock(&work_lock, চাবি);

	name = k_thread_name_get(queue->thread_id);
	যদি (name == রিক্ত) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		queue, space, name, work, handler);

	k_thread_abort(queue->thread_id);
}

স্থির শূন্য work_timeout_start_locked(সংরচনা k_work_q *queue, সংরচনা k_work *work)
{
	queue->finished = false;

	যদি (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		ঘূৰাই;
	}

	queue->work = work;
	z_add_timeout(&queue->work_timeout_record, work_timeout_handler, queue->work_timeout);
}

স্থির শূন্য work_timeout_stop_locked(সংরচনা k_work_q *queue)
{
	যদি (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		ঘূৰাই;
	}

	(শূন্য)z_try_abort_timeout(&queue->work_timeout_record);
}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
স্থির শূন্য work_queue_main(শূন্য *workq_ptr, শূন্য *p2, শূন্য *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	সংরচনা k_work_q *queue = (সংরচনা k_work_q *)workq_ptr;

	যেতিয়ালৈ (true) {
		sys_snode_t *গ্ৰন্থি;
		সংরচনা k_work *work = রিক্ত;
		k_work_handler_t handler = রিক্ত;
		k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
		bool yield;

		/* Check for and prepare any new work. */
		গ্ৰন্থি = sys_slist_get(&queue->pending);
		যদি (গ্ৰন্থি != রিক্ত) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_set(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
			work = CONTAINER_OF(গ্ৰন্থি, সংরচনা k_work, গ্ৰন্থি);
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
		} অন্যথা যদি (flag_test_and_clear(&queue->flags,
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
			(শূন্য)z_sched_wake_all(&queue->drainq, 1, রিক্ত);
		} অন্যথা যদি (flag_test(&queue->flags, K_WORK_QUEUE_STOP_BIT)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_set(&queue->flags, 0);
			k_spin_unlock(&work_lock, চাবি);
			ঘূৰাই;
		} অন্যথা {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		যদি (work == রিক্ত) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(শূন্য)z_sched_wait(&work_lock, চাবি, &queue->notifyq,
					   K_FOREVER, রিক্ত);
			অনুৱর্ত;
		}

#যদি defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_start_locked(queue, work);
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_lock, চাবি);

		__ASSERT_NO_MSG(handler != রিক্ত);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		চাবি = k_spin_lock(&work_lock);

#যদি defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		যদি (queue->finished) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_lock, চাবি);
			যেতিয়ালৈ (1) {
				k_sleep(K_FOREVER);
			}
			CODE_UNREACHABLE;
		}
		work_timeout_stop_locked(queue);
		queue->finished = true;
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_BIT);
		যদি (flag_test(&work->flags, K_WORK_FLUSHING_BIT)) {
			finalize_flush_locked(work);
		}
		যদি (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
		yield = (!flag_test(&queue->flags, K_WORK_QUEUE_NO_YIELD_BIT) &&
			 !sys_slist_is_empty(&queue->pending));
		k_spin_unlock(&work_lock, চাবি);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		যদি (yield) {
			k_yield();
		}
	}
}

শূন্য k_work_queue_init(সংরচনা k_work_q *queue)
{
	__ASSERT_NO_MSG(queue != রিক্ত);

	*queue = (সংরচনা k_work_q) {
		.flags = 0,
	};

	SYS_PORT_TRACING_OBJ_INIT(k_work_queue, queue);
}

শূন্য k_work_queue_run(সংরচনা k_work_q *queue, ধ্রুৱ সংরচনা k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT));

	uint32_t flags = K_WORK_QUEUE_STARTED;

	যদি ((cfg != রিক্ত) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	যদি ((cfg != রিক্ত) && (cfg->name != রিক্ত)) {
		k_thread_name_set(_current, cfg->name);
	}

#যদি defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	যদি ((cfg != রিক্ত) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} অন্যথা {
		queue->work_timeout = K_FOREVER;
	}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&queue->pending);
	z_waitq_init(&queue->notifyq);
	z_waitq_init(&queue->drainq);
	queue->thread_id = _current;
	flags_set(&queue->flags, flags);
	work_queue_main(queue, রিক্ত, রিক্ত);
}

শূন্য k_work_queue_start(সংরচনা k_work_q *queue,
			k_thread_স্তূপ_t *স্তূপ,
			size_t স্তূপ_size,
			পূর্ণাঙ্ক prio,
			ধ্রুৱ সংরচনা k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(স্তূপ);
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

	যদি ((cfg != রিক্ত) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_set(&queue->flags, flags);

	(শূন্য)k_thread_create(&queue->thread, স্তূপ, স্তূপ_size,
			      work_queue_main, queue, রিক্ত, রিক্ত,
			      prio, 0, K_FOREVER);

	যদি ((cfg != রিক্ত) && (cfg->name != রিক্ত)) {
		k_thread_name_set(&queue->thread, cfg->name);
	}

	যদি ((cfg != রিক্ত) && (cfg->essential)) {
		queue->thread.base.user_options |= K_ESSENTIAL;
	}

#যদি defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	যদি ((cfg != রিক্ত) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} অন্যথা {
		queue->work_timeout = K_FOREVER;
	}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_start(&queue->thread);
	queue->thread_id = &queue->thread;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, start, queue);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

পূর্ণাঙ্ক k_work_queue_drain(সংরচনা k_work_q *queue,
		       bool plug)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(!k_is_in_isr());

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, drain, queue);

	পূর্ণাঙ্ক ret = 0;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	যদি (((flags_get(&queue->flags)
	      & (K_WORK_QUEUE_BUSY | K_WORK_QUEUE_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_empty(&queue->pending)) {
		flag_set(&queue->flags, K_WORK_QUEUE_DRAIN_BIT);
		যদি (plug) {
			flag_set(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT);
		}

		notify_queue_locked(queue);
		ret = z_sched_wait(&work_lock, চাবি, &queue->drainq,
				   K_FOREVER, রিক্ত);
	} অন্যথা {
		k_spin_unlock(&work_lock, চাবি);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, drain, queue, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_queue_unplug(সংরচনা k_work_q *queue)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, unplug, queue);

	পূর্ণাঙ্ক ret = -EALREADY;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	যদি (flag_test_and_clear(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		ret = 0;
	}

	k_spin_unlock(&work_lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, unplug, queue, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_queue_stop(সংরচনা k_work_q *queue, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, stop, queue, timeout);

	যদি (queue->thread_id) {
		যদি (z_is_thread_essential(queue->thread_id)) {
			ঘূৰাই -ENOTSUP;
		}
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	যদি (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		k_spin_unlock(&work_lock, চাবি);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EALREADY);
		ঘূৰাই -EALREADY;
	}

	যদি (!flag_test(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		k_spin_unlock(&work_lock, চাবি);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EBUSY);
		ঘূৰাই -EBUSY;
	}

	flag_set(&queue->flags, K_WORK_QUEUE_STOP_BIT);
	notify_queue_locked(queue);
	k_spin_unlock(&work_lock, চাবি);
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work_queue, stop, queue, timeout);
	যদি (k_thread_join(queue->thread_id, timeout)) {
		চাবি = k_spin_lock(&work_lock);
		flag_clear(&queue->flags, K_WORK_QUEUE_STOP_BIT);
		k_spin_unlock(&work_lock, চাবি);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -ETIMEDOUT);
		ঘূৰাই -ETIMEDOUT;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, 0);
	ঘূৰাই 0;
}

#ifdef CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
স্থির শূন্য work_timeout(সংরচনা _timeout *to)
{
	সংরচনা k_work_delayable *dw
		= CONTAINER_OF(to, সংরচনা k_work_delayable, timeout);
	সংরচনা k_work *wp = &dw->work;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
	সংরচনা k_work_q *queue = রিক্ত;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	যদি (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_BIT)) {
		queue = dw->queue;
		(শূন্য)submit_to_queue_locked(wp, &queue);
	}

	k_spin_unlock(&work_lock, চাবি);
}

শূন্য k_work_init_delayable(সংরচনা k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != রিক্ত);
	__ASSERT_NO_MSG(handler != রিক্ত);

	*dwork = (সংরচনা k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক work_delayable_busy_get_locked(ধ্রুৱ সংরচনা k_work_delayable *dwork)
{
	ঘূৰাই flags_get(&dwork->work.flags) & K_WORK_MASK;
}

পূর্ণাঙ্ক k_work_delayable_busy_get(ধ্রুৱ সংরচনা k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != রিক্ত);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
	পূর্ণাঙ্ক ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_lock, চাবি);
	ঘূৰাই ret;
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
স্থির পূর্ণাঙ্ক schedule_for_queue_locked(সংরচনা k_work_q **queuep,
				     সংরচনা k_work_delayable *dwork,
				     k_timeout_t delay)
{
	পূর্ণাঙ্ক ret = 1;
	সংরচনা k_work *work = &dwork->work;

	যদি (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		ঘূৰাই submit_to_queue_locked(work, queuep);
	}

	flag_set(&work->flags, K_WORK_DELAYED_BIT);
	dwork->queue = *queuep;

	/* Add timeout */
	z_add_timeout(&dwork->timeout, work_timeout, delay);

	ঘূৰাই ret;
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
স্থির অন্তঃস্থ bool unschedule_locked(সংরচনা k_work_delayable *dwork,
				     k_spinlock_চাবি_t *চাবি)
{
	bool ret = false;
	সংরচনা k_work *work = &dwork->work;

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
	যদি (flag_test_and_clear(&work->flags, K_WORK_DELAYED_BIT)) {
		যেতিয়ালৈ (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_lock, *চাবি);
			*চাবি = k_spin_lock(&work_lock);
		}
		ret = true;
	}

	ঘূৰাই ret;
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
স্থির পূর্ণাঙ্ক cancel_delayable_async_locked(সংরচনা k_work_delayable *dwork,
					 k_spinlock_চাবি_t *চাবি)
{
	(শূন্য)unschedule_locked(dwork, চাবি);

	ঘূৰাই cancel_async_locked(&dwork->work);
}

পূর্ণাঙ্ক k_work_schedule_for_queue(সংরচনা k_work_q *queue, সংরচনা k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != রিক্ত);
	__ASSERT_NO_MSG(dwork != রিক্ত);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule_for_queue, queue, dwork, delay);

	সংরচনা k_work *work = &dwork->work;
	পূর্ণাঙ্ক ret = 0;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	/* Schedule the work item if it's idle or running. */
	যদি ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = schedule_for_queue_locked(&queue, dwork, delay);
	}

	k_spin_unlock(&work_lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule_for_queue, queue, dwork, delay, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_schedule(সংরচনা k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule, dwork, delay);

	পূর্ণাঙ্ক ret = k_work_schedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule, dwork, delay, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_reschedule_for_queue(সংরচনা k_work_q *queue, সংরচনা k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != রিক্ত);
	__ASSERT_NO_MSG(dwork != রিক্ত);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_queue, queue, dwork, delay);

	পূর্ণাঙ্ক ret;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	/* Remove any active scheduling. */
	(শূন্য)unschedule_locked(dwork, &চাবি);

	/* Schedule the work item with the new parameters. */
	ret = schedule_for_queue_locked(&queue, dwork, delay);

	k_spin_unlock(&work_lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_queue, queue, dwork, delay, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_reschedule(সংরচনা k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	পূর্ণাঙ্ক ret = k_work_reschedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	ঘূৰাই ret;
}

পূর্ণাঙ্ক k_work_cancel_delayable(সংরচনা k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != রিক্ত);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
	পূর্ণাঙ্ক ret = cancel_delayable_async_locked(dwork, &চাবি);

	k_spin_unlock(&work_lock, চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	ঘূৰাই ret;
}

bool k_work_cancel_delayable_sync(সংরচনা k_work_delayable *dwork,
				  সংরচনা k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != রিক্ত);
	__ASSERT_NO_MSG(sync != রিক্ত);
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	সংরচনা z_work_canceller *canceller = &sync->canceller;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	যদি (pending) {
		(শূন্য)cancel_delayable_async_locked(dwork, &চাবি);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_lock, চাবি);

	যদি (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	ঘূৰাই pending;
}

bool k_work_flush_delayable(সংরচনা k_work_delayable *dwork,
			    সংরচনা k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != রিক্ত);
	__ASSERT_NO_MSG(sync != রিক্ত);
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	সংরচনা k_work *work = &dwork->work;
	সংরচনা z_work_flusher *flusher = &sync->flusher;
	k_spinlock_চাবি_t চাবি = k_spin_lock(&work_lock);

	/* If it's idle release the lock and return immediately. */
	যদি (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		ঘূৰাই false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	যদি (unschedule_locked(dwork, &চাবি)) {
		সংরচনা k_work_q *queue = dwork->queue;

		(শূন্য)submit_to_queue_locked(work, &queue);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, চাবি);

	/* If necessary wait until the flusher item completes */
	যদি (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	ঘূৰাই need_flush;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */
