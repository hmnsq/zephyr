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
			      uint32_t بیت_2)
{
	*flagp &= ~بیت(بیت_2);
}

ساكن مضمن فراغ flag_مجموعه(uint32_t *flagp,
			    uint32_t بیت_2)
{
	*flagp |= بیت(بیت_2);
}

ساكن مضمن bool flag_test(ثابت uint32_t *flagp,
			     uint32_t بیت_2)
{
	أرجع (*flagp & بیت(بیت_2)) != 0U;
}

ساكن مضمن bool flag_test_and_clear(uint32_t *flagp,
				       صحيح بیت_2)
{
	bool ret = flag_test(flagp, بیت_2);

	flag_clear(flagp, بیت_2);

	أرجع ret;
}

ساكن مضمن فراغ flags_مجموعه(uint32_t *flagp,
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
ساكن بنية k_spinlock work_قفل;

/* Invoked by work thread */
ساكن فراغ handle_flush(بنية k_work *work) { }

ساكن مضمن فراغ init_flusher(بنية z_work_flusher *flusher)
{
	بنية k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_مجموعه(&work->flags, K_WORK_FLUSHING_بیت);
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
	sys_slist_append(&pending_cancels, &canceler->گره);
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
		= CONTAINER_از(work, بنية z_work_flusher, work);

	flag_clear(&work->flags, K_WORK_FLUSHING_بیت);

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
	flag_clear(&work->flags, K_WORK_CANCELING_بیت);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_امن(&pending_cancels, wc, tmp, گره) {
		إذا (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->گره);
			k_sem_give(&wc->sem);
		} وإلا {
			prev = &wc->گره;
		}
	}
}

فراغ k_work_init(بنية k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(handler != عدم);

	*work = (بنية k_work)Z_WORK_INITIALIZER(handler);

	SYS_پورت_TRACING_OBJ_INIT(k_work, work);
}

ساكن مضمن صحيح work_busy_get_locked(ثابت بنية k_work *work)
{
	أرجع flags_get(&work->flags) & K_WORK_ماسک;
}

صحيح k_work_busy_get(ثابت بنية k_work *work)
{
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
	صحيح ret = work_busy_get_locked(work);

	k_spin_unlock(&work_قفل, کلید);

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
ساكن فراغ صف_flusher_locked(بنية k_work_q *صف,
				 بنية k_work *work,
				 بنية z_work_flusher *flusher)
{
	init_flusher(flusher);

	إذا ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_درج(&صف->pending, &work->گره,
				 &flusher->work.گره);
	} وإلا {
		sys_slist_prepend(&صف->pending, &flusher->work.گره);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
ساكن مضمن فراغ صف_remove_locked(بنية k_work_q *صف,
				       بنية k_work *work)
{
	إذا (flag_test_and_clear(&work->flags, K_WORK_QUEUED_بیت)) {
		(فراغ)sys_slist_پیدا_کن_and_remove(&صف->pending, &work->گره);
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
ساكن مضمن bool notify_صف_locked(بنية k_work_q *صف)
{
	bool rv = false;

	إذا (صف != عدم) {
		rv = z_sched_wake(&صف->notifyq, 0, عدم);
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
ساكن مضمن صحيح صف_submit_locked(بنية k_work_q *صف,
				      بنية k_work *work)
{
	إذا (صف == عدم) {
		أرجع -EINVAL;
	}

	صحيح ret;
	bool chained = (_current == صف->thread_id) && !k_is_درون_isr();
	bool draining = flag_test(&صف->flags, K_WORK_صف_DRAIN_بیت);
	bool plugged = flag_test(&صف->flags, K_WORK_صف_PLUGGED_بیت);

	/* Test for acceptability, in priority order:
	 *
	 * * -ENODEV if the queue isn't running.
	 * * -EBUSY if draining and not chained
	 * * -EBUSY if plugged and not draining
	 * * otherwise OK
	 */
	إذا (!flag_test(&صف->flags, K_WORK_صف_STARTED_بیت)) {
		ret = -ENODEV;
	} وإلا إذا (draining && !chained) {
		ret = -EBUSY;
	} وإلا إذا (plugged && !draining) {
		ret = -EBUSY;
	} وإلا {
		sys_slist_append(&صف->pending, &work->گره);
		ret = 1;
		(فراغ)notify_صف_locked(صف);
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
ساكن صحيح submit_مقصد_صف_locked(بنية k_work *work,
				  بنية k_work_q **queuep)
{
	صحيح ret = 0;

	إذا (flag_test(&work->flags, K_WORK_CANCELING_بیت)) {
		/* Disallowed */
		ret = -EBUSY;
	} وإلا إذا (!flag_test(&work->flags, K_WORK_QUEUED_بیت)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		إذا (*queuep == عدم) {
			*queuep = work->صف;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		إذا (flag_test(&work->flags, K_WORK_RUNNING_بیت)) {
			__ASSERT_NO_MSG(work->صف != عدم);
			*queuep = work->صف;
			ret = 2;
		}

		صحيح rc = صف_submit_locked(*queuep, work);

		إذا (rc < 0) {
			ret = rc;
		} وإلا {
			flag_مجموعه(&work->flags, K_WORK_QUEUED_بیت);
			work->صف = *queuep;
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
صحيح z_work_submit_مقصد_صف(بنية k_work_q *صف,
		  بنية k_work *work)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(work->handler != عدم);

	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	صحيح ret = submit_مقصد_صف_locked(work, &صف);

	/* No need to reschedule if the submission failed. */
	إذا (ret > 0) {
		z_reschedule(&work_قفل, کلید);
	} وإلا {
		k_spin_unlock(&work_قفل, کلید);
	}

	أرجع ret;
}

صحيح k_work_submit_مقصد_صف(بنية k_work_q *صف,
			    بنية k_work *work)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, submit_مقصد_صف, صف, work);

	صحيح ret = z_work_submit_مقصد_صف(صف, work);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, submit_مقصد_صف, صف, work, ret);

	أرجع ret;
}

صحيح k_work_submit(بنية k_work *work)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	صحيح ret = z_work_submit_مقصد_صف(&k_sys_work_q, work);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

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
		بنية k_work_q *صف = work->صف;

		__ASSERT_NO_MSG(صف != عدم);

		صف_flusher_locked(صف, work, flusher);
		notify_صف_locked(صف);
	}

	أرجع need_flush;
}

bool k_work_flush(بنية k_work *work,
		  بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_بیت));
	__ASSERT_NO_MSG(!k_is_درون_isr());
	__ASSERT_NO_MSG(sync != عدم);
#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	بنية z_work_flusher *flusher = &sync->flusher;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_قفل, کلید);

	/* If necessary wait until the flusher item completes */
	إذا (need_flush) {
		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

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
	إذا (!flag_test(&work->flags, K_WORK_CANCELING_بیت)) {
		/* Remove it from the queue, if it's queued. */
		صف_remove_locked(work->صف, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	صحيح ret = work_busy_get_locked(work);

	إذا (ret != 0) {
		flag_مجموعه(&work->flags, K_WORK_CANCELING_بیت);
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
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_بیت);

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
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_بیت));

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
	صحيح ret = cancel_async_locked(work);

	k_spin_unlock(&work_قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	أرجع ret;
}

bool k_work_cancel_sync(بنية k_work *work,
			بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != عدم);
	__ASSERT_NO_MSG(sync != عدم);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_بیت));
	__ASSERT_NO_MSG(!k_is_درون_isr());
#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	بنية z_work_canceller *canceller = &sync->canceller;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	إذا (pending) {
		(فراغ)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_قفل, کلید);

	إذا (need_wait) {
		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	أرجع pending;
}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
ساكن فراغ work_timeout_handler(بنية _timeout *رکورد)
{
	بنية k_work_q *صف = CONTAINER_از(رکورد, بنية k_work_q, work_timeout_رکورد);
	بنية k_work *work = عدم;
	k_work_handler_t handler = عدم;
	ثابت حرف *name;
	ثابت حرف *space = " ";
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	work = صف->work;
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
	إذا (صف->پایانیافته || z_timeout_inflight_superseded(رکورد)) {
		k_spin_unlock(&work_قفل, کلید);
		أرجع;
	}
	صف->پایانیافته = true;

	k_spin_unlock(&work_قفل, کلید);

	name = k_thread_name_get(صف->thread_id);
	إذا (name == عدم) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		صف, space, name, work, handler);

	k_thread_abort(صف->thread_id);
}

ساكن فراغ work_timeout_شروع_locked(بنية k_work_q *صف, بنية k_work *work)
{
	صف->پایانیافته = false;

	إذا (K_TIMEOUT_EQ(صف->work_timeout, K_FOREVER)) {
		أرجع;
	}

	صف->work = work;
	z_جمع_کن_timeout(&صف->work_timeout_رکورد, work_timeout_handler, صف->work_timeout);
}

ساكن فراغ work_timeout_stop_locked(بنية k_work_q *صف)
{
	إذا (K_TIMEOUT_EQ(صف->work_timeout, K_FOREVER)) {
		أرجع;
	}

	(فراغ)z_try_abort_timeout(&صف->work_timeout_رکورد);
}
#پایان_اگر /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
ساكن فراغ work_صف_main(فراغ *workq_ptr, فراغ *p2, فراغ *p3)
{
	ARG_استفادهنشده(p2);
	ARG_استفادهنشده(p3);

	بنية k_work_q *صف = (بنية k_work_q *)workq_ptr;

	طالما (true) {
		sys_snode_t *گره;
		بنية k_work *work = عدم;
		k_work_handler_t handler = عدم;
		k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
		bool yield;

		/* Check for and prepare any new work. */
		گره = sys_slist_get(&صف->pending);
		إذا (گره != عدم) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_مجموعه(&صف->flags, K_WORK_صف_BUSY_بیت);
			work = CONTAINER_از(گره, بنية k_work, گره);
			flag_مجموعه(&work->flags, K_WORK_RUNNING_بیت);
			flag_clear(&work->flags, K_WORK_QUEUED_بیت);

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
		} وإلا إذا (flag_test_and_clear(&صف->flags,
					       K_WORK_صف_DRAIN_بیت)) {
			/* Not busy and draining: move threads waiting for
			 * drain to ready state.  The held spinlock inhibits
			 * immediate reschedule; released threads get their
			 * chance when this invokes z_sched_wait() below.
			 *
			 * We don't touch K_WORK_QUEUE_PLUGGABLE, so getting
			 * here doesn't mean that the queue will allow new
			 * submissions.
			 */
			(فراغ)z_sched_wake_all(&صف->drainq, 1, عدم);
		} وإلا إذا (flag_test(&صف->flags, K_WORK_صف_STOP_بیت)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_مجموعه(&صف->flags, 0);
			k_spin_unlock(&work_قفل, کلید);
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

			(فراغ)z_sched_wait(&work_قفل, کلید, &صف->notifyq,
					   K_FOREVER, عدم);
			استمر;
		}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_شروع_locked(صف, work);
#پایان_اگر /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_قفل, کلید);

		__ASSERT_NO_MSG(handler != عدم);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		کلید = k_spin_قفل(&work_قفل);

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		إذا (صف->پایانیافته) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_قفل, کلید);
			طالما (1) {
				k_sleep(K_FOREVER);
			}
			کد_UNREACHABLE;
		}
		work_timeout_stop_locked(صف);
		صف->پایانیافته = true;
#پایان_اگر /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_بیت);
		إذا (flag_test(&work->flags, K_WORK_FLUSHING_بیت)) {
			finalize_flush_locked(work);
		}
		إذا (flag_test(&work->flags, K_WORK_CANCELING_بیت)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&صف->flags, K_WORK_صف_BUSY_بیت);
		yield = (!flag_test(&صف->flags, K_WORK_صف_NO_YIELD_بیت) &&
			 !sys_slist_is_خالی(&صف->pending));
		k_spin_unlock(&work_قفل, کلید);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		إذا (yield) {
			k_yield();
		}
	}
}

فراغ k_work_صف_init(بنية k_work_q *صف)
{
	__ASSERT_NO_MSG(صف != عدم);

	*صف = (بنية k_work_q) {
		.flags = 0,
	};

	SYS_پورت_TRACING_OBJ_INIT(k_work_صف, صف);
}

فراغ k_work_صف_پیاپی(بنية k_work_q *صف, ثابت بنية k_work_صف_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&صف->flags, K_WORK_صف_STARTED_بیت));

	uint32_t flags = K_WORK_صف_STARTED;

	إذا ((cfg != عدم) && cfg->no_yield) {
		flags |= K_WORK_صف_NO_YIELD;
	}

	إذا ((cfg != عدم) && (cfg->name != عدم)) {
		k_thread_name_مجموعه(_current, cfg->name);
	}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	إذا ((cfg != عدم) && (cfg->work_timeout_ms)) {
		صف->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} وإلا {
		صف->work_timeout = K_FOREVER;
	}
#پایان_اگر /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&صف->pending);
	z_waitq_init(&صف->notifyq);
	z_waitq_init(&صف->drainq);
	صف->thread_id = _current;
	flags_مجموعه(&صف->flags, flags);
	work_صف_main(صف, عدم, عدم);
}

فراغ k_work_صف_شروع(بنية k_work_q *صف,
			k_thread_پشته_t *پشته,
			size_t پشته_اندازه,
			صحيح prio,
			ثابت بنية k_work_صف_config *cfg)
{
	__ASSERT_NO_MSG(صف);
	__ASSERT_NO_MSG(پشته);
	__ASSERT_NO_MSG(!flag_test(&صف->flags, K_WORK_صف_STARTED_بیت));

	/* In future, this whole function will be deprecated, but for now, we
	 * have to use the `thread` field to create a new thread in it.
	 */
	TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);

	uint32_t flags = K_WORK_صف_STARTED;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work_صف, شروع, صف);

	sys_slist_init(&صف->pending);
	z_waitq_init(&صف->notifyq);
	z_waitq_init(&صف->drainq);

	إذا ((cfg != عدم) && cfg->no_yield) {
		flags |= K_WORK_صف_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_مجموعه(&صف->flags, flags);

	(فراغ)k_thread_create(&صف->thread, پشته, پشته_اندازه,
			      work_صف_main, صف, عدم, عدم,
			      prio, 0, K_FOREVER);

	إذا ((cfg != عدم) && (cfg->name != عدم)) {
		k_thread_name_مجموعه(&صف->thread, cfg->name);
	}

	إذا ((cfg != عدم) && (cfg->essential)) {
		صف->thread.پایه.user_options |= K_ESSENTIAL;
	}

#إذا defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	إذا ((cfg != عدم) && (cfg->work_timeout_ms)) {
		صف->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} وإلا {
		صف->work_timeout = K_FOREVER;
	}
#پایان_اگر /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_شروع(&صف->thread);
	صف->thread_id = &صف->thread;

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work_صف, شروع, صف);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

صحيح k_work_صف_drain(بنية k_work_q *صف,
		       bool plug)
{
	__ASSERT_NO_MSG(صف);
	__ASSERT_NO_MSG(!k_is_درون_isr());

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work_صف, drain, صف);

	صحيح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	إذا (((flags_get(&صف->flags)
	      & (K_WORK_صف_BUSY | K_WORK_صف_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_خالی(&صف->pending)) {
		flag_مجموعه(&صف->flags, K_WORK_صف_DRAIN_بیت);
		إذا (plug) {
			flag_مجموعه(&صف->flags, K_WORK_صف_PLUGGED_بیت);
		}

		notify_صف_locked(صف);
		ret = z_sched_wait(&work_قفل, کلید, &صف->drainq,
				   K_FOREVER, عدم);
	} وإلا {
		k_spin_unlock(&work_قفل, کلید);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work_صف, drain, صف, ret);

	أرجع ret;
}

صحيح k_work_صف_unplug(بنية k_work_q *صف)
{
	__ASSERT_NO_MSG(صف);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work_صف, unplug, صف);

	صحيح ret = -EALREADY;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	إذا (flag_test_and_clear(&صف->flags, K_WORK_صف_PLUGGED_بیت)) {
		ret = 0;
	}

	k_spin_unlock(&work_قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work_صف, unplug, صف, ret);

	أرجع ret;
}

صحيح k_work_صف_stop(بنية k_work_q *صف, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(صف);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work_صف, stop, صف, timeout);

	إذا (صف->thread_id) {
		إذا (z_is_thread_essential(صف->thread_id)) {
			أرجع -ENOTSUP;
		}
	}

	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	إذا (!flag_test(&صف->flags, K_WORK_صف_STARTED_بیت)) {
		k_spin_unlock(&work_قفل, کلید);
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work_صف, stop, صف, timeout, -EALREADY);
		أرجع -EALREADY;
	}

	إذا (!flag_test(&صف->flags, K_WORK_صف_PLUGGED_بیت)) {
		k_spin_unlock(&work_قفل, کلید);
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work_صف, stop, صف, timeout, -EBUSY);
		أرجع -EBUSY;
	}

	flag_مجموعه(&صف->flags, K_WORK_صف_STOP_بیت);
	notify_صف_locked(صف);
	k_spin_unlock(&work_قفل, کلید);
	SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_work_صف, stop, صف, timeout);
	إذا (k_thread_join(صف->thread_id, timeout)) {
		کلید = k_spin_قفل(&work_قفل);
		flag_clear(&صف->flags, K_WORK_صف_STOP_بیت);
		k_spin_unlock(&work_قفل, کلید);
		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work_صف, stop, صف, timeout, -ETIMEDOUT);
		أرجع -ETIMEDOUT;
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work_صف, stop, صف, timeout, 0);
	أرجع 0;
}

#اگر_تعریف_شده CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
ساكن فراغ work_timeout(بنية _timeout *مقصد)
{
	بنية k_work_delayable *dw
		= CONTAINER_از(مقصد, بنية k_work_delayable, timeout);
	بنية k_work *wp = &dw->work;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
	بنية k_work_q *صف = عدم;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	إذا (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_بیت)) {
		صف = dw->صف;
		(فراغ)submit_مقصد_صف_locked(wp, &صف);
	}

	k_spin_unlock(&work_قفل, کلید);
}

فراغ k_work_init_delayable(بنية k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != عدم);
	__ASSERT_NO_MSG(handler != عدم);

	*dwork = (بنية k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_پورت_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

ساكن مضمن صحيح work_delayable_busy_get_locked(ثابت بنية k_work_delayable *dwork)
{
	أرجع flags_get(&dwork->work.flags) & K_WORK_ماسک;
}

صحيح k_work_delayable_busy_get(ثابت بنية k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != عدم);

	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
	صحيح ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_قفل, کلید);
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
ساكن صحيح زمانبندی_for_صف_locked(بنية k_work_q **queuep,
				     بنية k_work_delayable *dwork,
				     k_timeout_t delay)
{
	صحيح ret = 1;
	بنية k_work *work = &dwork->work;

	إذا (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		أرجع submit_مقصد_صف_locked(work, queuep);
	}

	flag_مجموعه(&work->flags, K_WORK_DELAYED_بیت);
	dwork->صف = *queuep;

	/* Add timeout */
	z_جمع_کن_timeout(&dwork->timeout, work_timeout, delay);

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
				     k_spinlock_کلید_t *کلید)
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
	إذا (flag_test_and_clear(&work->flags, K_WORK_DELAYED_بیت)) {
		طالما (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_قفل, *کلید);
			*کلید = k_spin_قفل(&work_قفل);
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
					 k_spinlock_کلید_t *کلید)
{
	(فراغ)unschedule_locked(dwork, کلید);

	أرجع cancel_async_locked(&dwork->work);
}

صحيح k_work_زمانبندی_for_صف(بنية k_work_q *صف, بنية k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(صف != عدم);
	__ASSERT_NO_MSG(dwork != عدم);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, زمانبندی_for_صف, صف, dwork, delay);

	بنية k_work *work = &dwork->work;
	صحيح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	/* Schedule the work item if it's idle or running. */
	إذا ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = زمانبندی_for_صف_locked(&صف, dwork, delay);
	}

	k_spin_unlock(&work_قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, زمانبندی_for_صف, صف, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_زمانبندی(بنية k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, زمانبندی, dwork, delay);

	صحيح ret = k_work_زمانبندی_for_صف(&k_sys_work_q, dwork, delay);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, زمانبندی, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_reschedule_for_صف(بنية k_work_q *صف, بنية k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(صف != عدم);
	__ASSERT_NO_MSG(dwork != عدم);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_صف, صف, dwork, delay);

	صحيح ret;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	/* Remove any active scheduling. */
	(فراغ)unschedule_locked(dwork, &کلید);

	/* Schedule the work item with the new parameters. */
	ret = زمانبندی_for_صف_locked(&صف, dwork, delay);

	k_spin_unlock(&work_قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_صف, صف, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_reschedule(بنية k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	صحيح ret = k_work_reschedule_for_صف(&k_sys_work_q, dwork, delay);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	أرجع ret;
}

صحيح k_work_cancel_delayable(بنية k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != عدم);

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
	صحيح ret = cancel_delayable_async_locked(dwork, &کلید);

	k_spin_unlock(&work_قفل, کلید);

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	أرجع ret;
}

bool k_work_cancel_delayable_sync(بنية k_work_delayable *dwork,
				  بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != عدم);
	__ASSERT_NO_MSG(sync != عدم);
	__ASSERT_NO_MSG(!k_is_درون_isr());
#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	بنية z_work_canceller *canceller = &sync->canceller;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	إذا (pending) {
		(فراغ)cancel_delayable_async_locked(dwork, &کلید);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_قفل, کلید);

	إذا (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	أرجع pending;
}

bool k_work_flush_delayable(بنية k_work_delayable *dwork,
			    بنية k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != عدم);
	__ASSERT_NO_MSG(sync != عدم);
	__ASSERT_NO_MSG(!k_is_درون_isr());
#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	بنية k_work *work = &dwork->work;
	بنية z_work_flusher *flusher = &sync->flusher;
	k_spinlock_کلید_t کلید = k_spin_قفل(&work_قفل);

	/* If it's idle release the lock and return immediately. */
	إذا (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_قفل, کلید);

		SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		أرجع false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	إذا (unschedule_locked(dwork, &کلید)) {
		بنية k_work_q *صف = dwork->صف;

		(فراغ)submit_مقصد_صف_locked(work, &صف);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_قفل, کلید);

	/* If necessary wait until the flusher item completes */
	إذا (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	أرجع need_flush;
}

#پایان_اگر /* CONFIG_SYS_CLOCK_EXISTS */
