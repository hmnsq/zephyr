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

#ضمن <zephyr/kernel.h>
#ضمن <wait_q.h>
#ضمن <zephyr/spinlock.h>
#ضمن <errno.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <zephyr/sys/printk.h>
#ضمن <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ساكن مضمن فراغ flag_clear(uint32_t *flagp,
			      uint32_t بټ_2)
{
	*flagp &= ~بټ(بټ_2);
}

ساكن مضمن فراغ flag_سېټ(uint32_t *flagp,
			    uint32_t بټ_2)
{
	*flagp |= بټ(بټ_2);
}

ساكن مضمن bool flag_test(ثابت uint32_t *flagp,
			     uint32_t بټ_2)
{
	أرجع (*flagp & بټ(بټ_2)) != 0U;
}

ساكن مضمن bool flag_test_and_clear(uint32_t *flagp,
				       صحيح بټ_2)
{
	bool ret = flag_test(flagp, بټ_2);

	flag_clear(flagp, بټ_2);

	أرجع ret;
}

ساكن مضمن فراغ flags_سېټ(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

ساكن مضمن uint32_t flags_get(ثابت uint32_t *flagp)
{
	أرجع *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
ساكن بنية k_spinlock work_قلف;

/* Invoked by work thread */
ساكن فراغ handle_flush(بنية k_work *work) { }

ساكن مضمن فراغ init_flusher(بنية z_work_flusher *flusher)
{
	بنية k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_سېټ(&work->flags, K_WORK_FLUSHING_بټ);
}

/* List of pending cancellations. */
ساكن sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
ساكن مضمن فراغ init_work_cancel(بنية z_work_canceller *canceler,
				    بنية k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->غوټه);
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
ساكن فراغ finalize_flush_locked(بنية k_work *work)
{
	بنية z_work_flusher *flusher
		= CONTAINER_د(work, بنية z_work_flusher, work);

	flag_clear(&work->flags, K_WORK_FLUSHING_بټ);

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
ساكن فراغ finalize_cancel_locked(بنية k_work *work)
{
	بنية z_work_canceller *wc, *tmp;
	sys_snode_t *prev = عدم;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_بټ);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_خوندي(&pending_cancels, wc, tmp, غوټه) {
		إذا (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->غوټه);
			k_sem_give(&wc->sem);
		} وإلا {
			prev = &wc->غوټه;
		}
	}
}

فراغ k_work_init(بنية k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(handler != عدم);

	*work = (بنية k_work)Z_WORK_INITIALIZER(handler);

	SYS_پورټ_TRACING_OBJ_INIT(k_work, work);
}

ساكن مضمن صحيح work_busy_get_locked(ثابت بنية k_work *work)
{
	أرجع flags_get(&work->flags) & K_WORK_ماسک;
}

صحيح k_work_busy_get(ثابت بنية k_work *work)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
	صحيح ret = work_busy_get_locked(work);

	k_spin_unlock(&work_قلف, کلی);

	أرجع ret;
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
ساكن فراغ کتار_flusher_locked(بنية k_work_q *کتار,
				 بنية k_work *work,
				 بنية z_work_flusher *flusher)
{
	init_flusher(flusher);

	إذا ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_ورننباسه(&کتار->pending, &work->غوټه,
				 &flusher->work.غوټه);
	} وإلا {
		sys_slist_prepend(&کتار->pending, &flusher->work.غوټه);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
ساكن مضمن فراغ کتار_remove_locked(بنية k_work_q *کتار,
				       بنية k_work *work)
{
	إذا (flag_test_and_clear(&work->flags, K_WORK_QUEUED_بټ)) {
		(فراغ)sys_slist_پیدا_کړه_and_remove(&کتار->pending, &work->غوټه);
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
ساكن مضمن bool notify_کتار_locked(بنية k_work_q *کتار)
{
	bool rv = false;

	إذا (کتار != عدم) {
		rv = z_sched_wake(&کتار->notifyq, 0, عدم);
	}

	أرجع rv;
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
ساكن مضمن صحيح کتار_submit_locked(بنية k_work_q *کتار,
				      بنية k_work *work)
{
	إذا (کتار == عدم) {
		أرجع -EINVAL;
	}

	صحيح ret;
	bool chained = (_current == کتار->thread_id) && !k_is_دننه_isr();
	bool draining = flag_test(&کتار->flags, K_WORK_کتار_DRAIN_بټ);
	bool plugged = flag_test(&کتار->flags, K_WORK_کتار_PLUGGED_بټ);

	/* Test for acceptability, in priority order:
	 *
	 * * -ENODEV if the queue isn't running.
	 * * -EBUSY if draining and not chained
	 * * -EBUSY if plugged and not draining
	 * * otherwise OK
	 */
	إذا (!flag_test(&کتار->flags, K_WORK_کتار_STARTED_بټ)) {
		ret = -ENODEV;
	} وإلا إذا (draining && !chained) {
		ret = -EBUSY;
	} وإلا إذا (plugged && !draining) {
		ret = -EBUSY;
	} وإلا {
		sys_slist_append(&کتار->pending, &work->غوټه);
		ret = 1;
		(فراغ)notify_کتار_locked(کتار);
	}

	أرجع ret;
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
ساكن صحيح submit_منزل_کتار_locked(بنية k_work *work,
				  بنية k_work_q **queuep)
{
	صحيح ret = 0;

	إذا (flag_test(&work->flags, K_WORK_CANCELING_بټ)) {
		/* Disallowed */
		ret = -EBUSY;
	} وإلا إذا (!flag_test(&work->flags, K_WORK_QUEUED_بټ)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		إذا (*queuep == عدم) {
			*queuep = work->کتار;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		إذا (flag_test(&work->flags, K_WORK_RUNNING_بټ)) {
			__ASSERT_NO_MSG(work->کتار != عدم);
			*queuep = work->کتار;
			ret = 2;
		}

		صحيح rc = کتار_submit_locked(*queuep, work);

		إذا (rc < 0) {
			ret = rc;
		} وإلا {
			flag_سېټ(&work->flags, K_WORK_QUEUED_بټ);
			work->کتار = *queuep;
		}
	} وإلا {
		/* Already queued, do nothing. */
	}

	إذا (ret <= 0) {
		*queuep = عدم;
	}

	أرجع ret;
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
صحيح z_work_submit_منزل_کتار(بنية k_work_q *کتار,
		  بنية k_work *work)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(work->handler != عدم);

	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	صحيح ret = submit_منزل_کتار_locked(work, &کتار);

	/* No need to reschedule if the submission failed. */
	إذا (ret > 0) {
		z_reschedule(&work_قلف, کلی);
	} وإلا {
		k_spin_unlock(&work_قلف, کلی);
	}

	أرجع ret;
}

صحيح k_work_submit_منزل_کتار(بنية k_work_q *کتار,
			    بنية k_work *work)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, submit_منزل_کتار, کتار, work);

	صحيح ret = z_work_submit_منزل_کتار(کتار, work);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, submit_منزل_کتار, کتار, work, ret);

	أرجع ret;
}

صحيح k_work_submit(بنية k_work *work)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	صحيح ret = z_work_submit_منزل_کتار(&k_sys_work_q, work);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	أرجع ret;
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
ساكن bool work_flush_locked(بنية k_work *work,
			      بنية z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	إذا (need_flush) {
		بنية k_work_q *کتار = work->کتار;

		__ASSERT_NO_MSG(کتار != عدم);

		کتار_flusher_locked(کتار, work, flusher);
		notify_کتار_locked(کتار);
	}

	أرجع need_flush;
}

bool k_work_flush(بنية k_work *work,
		  بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_بټ));
	__ASSERT_NO_MSG(!k_is_دننه_isr());
	__ASSERT_NO_MSG(sync != عدم);
#که_تعریف_شوی CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#د_که_پای /* CONFIG_KERNEL_COHERENCE */

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	بنية z_work_flusher *flusher = &sync->flusher;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_قلف, کلی);

	/* If necessary wait until the flusher item completes */
	إذا (need_flush) {
		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	أرجع need_flush;
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
ساكن صحيح cancel_async_locked(بنية k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	إذا (!flag_test(&work->flags, K_WORK_CANCELING_بټ)) {
		/* Remove it from the queue, if it's queued. */
		کتار_remove_locked(work->کتار, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	صحيح ret = work_busy_get_locked(work);

	إذا (ret != 0) {
		flag_سېټ(&work->flags, K_WORK_CANCELING_بټ);
		ret = work_busy_get_locked(work);
	}

	أرجع ret;
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
ساكن bool cancel_sync_locked(بنية k_work *work,
			       بنية z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_بټ);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	إذا (ret) {
		init_work_cancel(canceller, work);
	}

	أرجع ret;
}

صحيح k_work_cancel(بنية k_work *work)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_بټ));

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
	صحيح ret = cancel_async_locked(work);

	k_spin_unlock(&work_قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	أرجع ret;
}

bool k_work_cancel_sync(بنية k_work *work,
			بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(sync != عدم);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_بټ));
	__ASSERT_NO_MSG(!k_is_دننه_isr());
#که_تعریف_شوی CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#د_که_پای /* CONFIG_KERNEL_COHERENCE */

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	بنية z_work_canceller *canceller = &sync->canceller;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	إذا (pending) {
		(فراغ)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_قلف, کلی);

	إذا (need_wait) {
		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	أرجع pending;
}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
ساكن فراغ work_timeout_handler(بنية _timeout *ریکارډ)
{
	بنية k_work_q *کتار = CONTAINER_د(ریکارډ, بنية k_work_q, work_timeout_ریکارډ);
	بنية k_work *work = عدم;
	k_work_handler_t handler = عدم;
	ثابت حرف *name;
	ثابت حرف *space = " ";
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	work = کتار->work;
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
	إذا (کتار->پای_ته_رسېدلی || z_timeout_inflight_superseded(ریکارډ)) {
		k_spin_unlock(&work_قلف, کلی);
		أرجع;
	}
	کتار->پای_ته_رسېدلی = true;

	k_spin_unlock(&work_قلف, کلی);

	name = k_thread_name_get(کتار->thread_id);
	إذا (name == عدم) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		کتار, space, name, work, handler);

	k_thread_abort(کتار->thread_id);
}

ساكن فراغ work_timeout_پیل_locked(بنية k_work_q *کتار, بنية k_work *work)
{
	کتار->پای_ته_رسېدلی = false;

	إذا (K_TIMEOUT_EQ(کتار->work_timeout, K_FOREVER)) {
		أرجع;
	}

	کتار->work = work;
	z_جمع_کړه_timeout(&کتار->work_timeout_ریکارډ, work_timeout_handler, کتار->work_timeout);
}

ساكن فراغ work_timeout_stop_locked(بنية k_work_q *کتار)
{
	إذا (K_TIMEOUT_EQ(کتار->work_timeout, K_FOREVER)) {
		أرجع;
	}

	(فراغ)z_try_abort_timeout(&کتار->work_timeout_ریکارډ);
}
#د_که_پای /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
ساكن فراغ work_کتار_main(فراغ *workq_ptr, فراغ *p2, فراغ *p3)
{
	ARG_ناکارول_شوی(p2);
	ARG_ناکارول_شوی(p3);

	بنية k_work_q *کتار = (بنية k_work_q *)workq_ptr;

	طالما (true) {
		sys_snode_t *غوټه;
		بنية k_work *work = عدم;
		k_work_handler_t handler = عدم;
		k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
		bool yield;

		/* Check for and prepare any new work. */
		غوټه = sys_slist_get(&کتار->pending);
		إذا (غوټه != عدم) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_سېټ(&کتار->flags, K_WORK_کتار_BUSY_بټ);
			work = CONTAINER_د(غوټه, بنية k_work, غوټه);
			flag_سېټ(&work->flags, K_WORK_RUNNING_بټ);
			flag_clear(&work->flags, K_WORK_QUEUED_بټ);

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
		} وإلا إذا (flag_test_and_clear(&کتار->flags,
					       K_WORK_کتار_DRAIN_بټ)) {
			/* Not busy and draining: move threads waiting for
			 * drain to ready state.  The held spinlock inhibits
			 * immediate reschedule; released threads get their
			 * chance when this invokes z_sched_wait() below.
			 *
			 * We don't touch K_WORK_QUEUE_PLUGGABLE, so getting
			 * here doesn't mean that the queue will allow new
			 * submissions.
			 */
			(فراغ)z_sched_wake_all(&کتار->drainq, 1, عدم);
		} وإلا إذا (flag_test(&کتار->flags, K_WORK_کتار_STOP_بټ)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_سېټ(&کتار->flags, 0);
			k_spin_unlock(&work_قلف, کلی);
			أرجع;
		} وإلا {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		إذا (work == عدم) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(فراغ)z_sched_wait(&work_قلف, کلی, &کتار->notifyq,
					   K_FOREVER, عدم);
			استمر;
		}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_پیل_locked(کتار, work);
#د_که_پای /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_قلف, کلی);

		__ASSERT_NO_MSG(handler != عدم);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		کلی = k_spin_قلف(&work_قلف);

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		إذا (کتار->پای_ته_رسېدلی) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_قلف, کلی);
			طالما (1) {
				k_sleep(K_FOREVER);
			}
			کوډ_UNREACHABLE;
		}
		work_timeout_stop_locked(کتار);
		کتار->پای_ته_رسېدلی = true;
#د_که_پای /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_بټ);
		إذا (flag_test(&work->flags, K_WORK_FLUSHING_بټ)) {
			finalize_flush_locked(work);
		}
		إذا (flag_test(&work->flags, K_WORK_CANCELING_بټ)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&کتار->flags, K_WORK_کتار_BUSY_بټ);
		yield = (!flag_test(&کتار->flags, K_WORK_کتار_NO_YIELD_بټ) &&
			 !sys_slist_is_تشه(&کتار->pending));
		k_spin_unlock(&work_قلف, کلی);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		إذا (yield) {
			k_yield();
		}
	}
}

فراغ k_work_کتار_init(بنية k_work_q *کتار)
{
	__ASSERT_NO_MSG(کتار != عدم);

	*کتار = (بنية k_work_q) {
		.flags = 0,
	};

	SYS_پورټ_TRACING_OBJ_INIT(k_work_کتار, کتار);
}

فراغ k_work_کتار_پرلپسې(بنية k_work_q *کتار, ثابت بنية k_work_کتار_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&کتار->flags, K_WORK_کتار_STARTED_بټ));

	uint32_t flags = K_WORK_کتار_STARTED;

	إذا ((cfg != عدم) && cfg->no_yield) {
		flags |= K_WORK_کتار_NO_YIELD;
	}

	إذا ((cfg != عدم) && (cfg->name != عدم)) {
		k_thread_name_سېټ(_current, cfg->name);
	}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	إذا ((cfg != عدم) && (cfg->work_timeout_ms)) {
		کتار->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} وإلا {
		کتار->work_timeout = K_FOREVER;
	}
#د_که_پای /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&کتار->pending);
	z_waitq_init(&کتار->notifyq);
	z_waitq_init(&کتار->drainq);
	کتار->thread_id = _current;
	flags_سېټ(&کتار->flags, flags);
	work_کتار_main(کتار, عدم, عدم);
}

فراغ k_work_کتار_پیل(بنية k_work_q *کتار,
			k_thread_ډېری_t *ډېری,
			size_t ډېری_اندازه,
			صحيح prio,
			ثابت بنية k_work_کتار_config *cfg)
{
	__ASSERT_NO_MSG(کتار);
	__ASSERT_NO_MSG(ډېری);
	__ASSERT_NO_MSG(!flag_test(&کتار->flags, K_WORK_کتار_STARTED_بټ));

	/* In future, this whole function will be deprecated, but for now, we
	 * have to use the `thread` field to create a new thread in it.
	 */
	TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);

	uint32_t flags = K_WORK_کتار_STARTED;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work_کتار, پیل, کتار);

	sys_slist_init(&کتار->pending);
	z_waitq_init(&کتار->notifyq);
	z_waitq_init(&کتار->drainq);

	إذا ((cfg != عدم) && cfg->no_yield) {
		flags |= K_WORK_کتار_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_سېټ(&کتار->flags, flags);

	(فراغ)k_thread_create(&کتار->thread, ډېری, ډېری_اندازه,
			      work_کتار_main, کتار, عدم, عدم,
			      prio, 0, K_FOREVER);

	إذا ((cfg != عدم) && (cfg->name != عدم)) {
		k_thread_name_سېټ(&کتار->thread, cfg->name);
	}

	إذا ((cfg != عدم) && (cfg->essential)) {
		کتار->thread.بنسټ.user_options |= K_ESSENTIAL;
	}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	إذا ((cfg != عدم) && (cfg->work_timeout_ms)) {
		کتار->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} وإلا {
		کتار->work_timeout = K_FOREVER;
	}
#د_که_پای /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_پیل(&کتار->thread);
	کتار->thread_id = &کتار->thread;

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work_کتار, پیل, کتار);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

صحيح k_work_کتار_drain(بنية k_work_q *کتار,
		       bool plug)
{
	__ASSERT_NO_MSG(کتار);
	__ASSERT_NO_MSG(!k_is_دننه_isr());

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work_کتار, drain, کتار);

	صحيح ret = 0;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	إذا (((flags_get(&کتار->flags)
	      & (K_WORK_کتار_BUSY | K_WORK_کتار_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_تشه(&کتار->pending)) {
		flag_سېټ(&کتار->flags, K_WORK_کتار_DRAIN_بټ);
		إذا (plug) {
			flag_سېټ(&کتار->flags, K_WORK_کتار_PLUGGED_بټ);
		}

		notify_کتار_locked(کتار);
		ret = z_sched_wait(&work_قلف, کلی, &کتار->drainq,
				   K_FOREVER, عدم);
	} وإلا {
		k_spin_unlock(&work_قلف, کلی);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work_کتار, drain, کتار, ret);

	أرجع ret;
}

صحيح k_work_کتار_unplug(بنية k_work_q *کتار)
{
	__ASSERT_NO_MSG(کتار);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work_کتار, unplug, کتار);

	صحيح ret = -EALREADY;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	إذا (flag_test_and_clear(&کتار->flags, K_WORK_کتار_PLUGGED_بټ)) {
		ret = 0;
	}

	k_spin_unlock(&work_قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work_کتار, unplug, کتار, ret);

	أرجع ret;
}

صحيح k_work_کتار_stop(بنية k_work_q *کتار, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(کتار);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work_کتار, stop, کتار, timeout);

	إذا (کتار->thread_id) {
		إذا (z_is_thread_essential(کتار->thread_id)) {
			أرجع -ENOTSUP;
		}
	}

	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	إذا (!flag_test(&کتار->flags, K_WORK_کتار_STARTED_بټ)) {
		k_spin_unlock(&work_قلف, کلی);
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work_کتار, stop, کتار, timeout, -EALREADY);
		أرجع -EALREADY;
	}

	إذا (!flag_test(&کتار->flags, K_WORK_کتار_PLUGGED_بټ)) {
		k_spin_unlock(&work_قلف, کلی);
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work_کتار, stop, کتار, timeout, -EBUSY);
		أرجع -EBUSY;
	}

	flag_سېټ(&کتار->flags, K_WORK_کتار_STOP_بټ);
	notify_کتار_locked(کتار);
	k_spin_unlock(&work_قلف, کلی);
	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_work_کتار, stop, کتار, timeout);
	إذا (k_thread_join(کتار->thread_id, timeout)) {
		کلی = k_spin_قلف(&work_قلف);
		flag_clear(&کتار->flags, K_WORK_کتار_STOP_بټ);
		k_spin_unlock(&work_قلف, کلی);
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work_کتار, stop, کتار, timeout, -ETIMEDOUT);
		أرجع -ETIMEDOUT;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work_کتار, stop, کتار, timeout, 0);
	أرجع 0;
}

#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
ساكن فراغ work_timeout(بنية _timeout *منزل)
{
	بنية k_work_delayable *dw
		= CONTAINER_د(منزل, بنية k_work_delayable, timeout);
	بنية k_work *wp = &dw->work;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
	بنية k_work_q *کتار = عدم;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	إذا (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_بټ)) {
		کتار = dw->کتار;
		(فراغ)submit_منزل_کتار_locked(wp, &کتار);
	}

	k_spin_unlock(&work_قلف, کلی);
}

فراغ k_work_init_delayable(بنية k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != عدم);
	__ASSERT_NO_MSG(handler != عدم);

	*dwork = (بنية k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_پورټ_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

ساكن مضمن صحيح work_delayable_busy_get_locked(ثابت بنية k_work_delayable *dwork)
{
	أرجع flags_get(&dwork->work.flags) & K_WORK_ماسک;
}

صحيح k_work_delayable_busy_get(ثابت بنية k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != عدم);

	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
	صحيح ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_قلف, کلی);
	أرجع ret;
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
ساكن صحيح مهالوېش_for_کتار_locked(بنية k_work_q **queuep,
				     بنية k_work_delayable *dwork,
				     k_timeout_t delay)
{
	صحيح ret = 1;
	بنية k_work *work = &dwork->work;

	إذا (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		أرجع submit_منزل_کتار_locked(work, queuep);
	}

	flag_سېټ(&work->flags, K_WORK_DELAYED_بټ);
	dwork->کتار = *queuep;

	/* Add timeout */
	z_جمع_کړه_timeout(&dwork->timeout, work_timeout, delay);

	أرجع ret;
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
ساكن مضمن bool unschedule_locked(بنية k_work_delayable *dwork,
				     k_spinlock_کلی_t *کلی)
{
	bool ret = false;
	بنية k_work *work = &dwork->work;

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
	إذا (flag_test_and_clear(&work->flags, K_WORK_DELAYED_بټ)) {
		طالما (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_قلف, *کلی);
			*کلی = k_spin_قلف(&work_قلف);
		}
		ret = true;
	}

	أرجع ret;
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
ساكن صحيح cancel_delayable_async_locked(بنية k_work_delayable *dwork,
					 k_spinlock_کلی_t *کلی)
{
	(فراغ)unschedule_locked(dwork, کلی);

	أرجع cancel_async_locked(&dwork->work);
}

صحيح k_work_مهالوېش_for_کتار(بنية k_work_q *کتار, بنية k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(کتار != عدم);
	__ASSERT_NO_MSG(dwork != عدم);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, مهالوېش_for_کتار, کتار, dwork, delay);

	بنية k_work *work = &dwork->work;
	صحيح ret = 0;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	/* Schedule the work item if it's idle or running. */
	إذا ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = مهالوېش_for_کتار_locked(&کتار, dwork, delay);
	}

	k_spin_unlock(&work_قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, مهالوېش_for_کتار, کتار, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_مهالوېش(بنية k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, مهالوېش, dwork, delay);

	صحيح ret = k_work_مهالوېش_for_کتار(&k_sys_work_q, dwork, delay);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, مهالوېش, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_reschedule_for_کتار(بنية k_work_q *کتار, بنية k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(کتار != عدم);
	__ASSERT_NO_MSG(dwork != عدم);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_کتار, کتار, dwork, delay);

	صحيح ret;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	/* Remove any active scheduling. */
	(فراغ)unschedule_locked(dwork, &کلی);

	/* Schedule the work item with the new parameters. */
	ret = مهالوېش_for_کتار_locked(&کتار, dwork, delay);

	k_spin_unlock(&work_قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_کتار, کتار, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_reschedule(بنية k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	صحيح ret = k_work_reschedule_for_کتار(&k_sys_work_q, dwork, delay);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_cancel_delayable(بنية k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != عدم);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
	صحيح ret = cancel_delayable_async_locked(dwork, &کلی);

	k_spin_unlock(&work_قلف, کلی);

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	أرجع ret;
}

bool k_work_cancel_delayable_sync(بنية k_work_delayable *dwork,
				  بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != عدم);
	__ASSERT_NO_MSG(sync != عدم);
	__ASSERT_NO_MSG(!k_is_دننه_isr());
#که_تعریف_شوی CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#د_که_پای /* CONFIG_KERNEL_COHERENCE */

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	بنية z_work_canceller *canceller = &sync->canceller;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	إذا (pending) {
		(فراغ)cancel_delayable_async_locked(dwork, &کلی);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_قلف, کلی);

	إذا (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	أرجع pending;
}

bool k_work_flush_delayable(بنية k_work_delayable *dwork,
			    بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != عدم);
	__ASSERT_NO_MSG(sync != عدم);
	__ASSERT_NO_MSG(!k_is_دننه_isr());
#که_تعریف_شوی CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#د_که_پای /* CONFIG_KERNEL_COHERENCE */

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	بنية k_work *work = &dwork->work;
	بنية z_work_flusher *flusher = &sync->flusher;
	k_spinlock_کلی_t کلی = k_spin_قلف(&work_قلف);

	/* If it's idle release the lock and return immediately. */
	إذا (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		أرجع false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	إذا (unschedule_locked(dwork, &کلی)) {
		بنية k_work_q *کتار = dwork->کتار;

		(فراغ)submit_منزل_کتار_locked(work, &کتار);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_قلف, کلی);

	/* If necessary wait until the flusher item completes */
	إذا (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	أرجع need_flush;
}

#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */
