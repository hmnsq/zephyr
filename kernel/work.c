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

#شامل <zephyr/kernel.h>
#شامل <wait_q.h>
#شامل <zephyr/spinlock.h>
#شامل <errno.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <zephyr/sys/printk.h>
#شامل <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

ساکن اندرونی خالی flag_clear(uint32_t *flagp,
			      uint32_t ثنائی_ہندسہ_2)
{
	*flagp &= ~ثنائی_ہندسہ(ثنائی_ہندسہ_2);
}

ساکن اندرونی خالی flag_سیٹ(uint32_t *flagp,
			    uint32_t ثنائی_ہندسہ_2)
{
	*flagp |= ثنائی_ہندسہ(ثنائی_ہندسہ_2);
}

ساکن اندرونی bool flag_test(ثابت uint32_t *flagp,
			     uint32_t ثنائی_ہندسہ_2)
{
	واپس (*flagp & ثنائی_ہندسہ(ثنائی_ہندسہ_2)) != 0U;
}

ساکن اندرونی bool flag_test_and_clear(uint32_t *flagp,
				       صحیح ثنائی_ہندسہ_2)
{
	bool ret = flag_test(flagp, ثنائی_ہندسہ_2);

	flag_clear(flagp, ثنائی_ہندسہ_2);

	واپس ret;
}

ساکن اندرونی خالی flags_سیٹ(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

ساکن اندرونی uint32_t flags_get(ثابت uint32_t *flagp)
{
	واپس *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
ساکن ساخت k_spinlock work_تالا;

/* Invoked by work thread */
ساکن خالی handle_flush(ساخت k_work *work) { }

ساکن اندرونی خالی init_flusher(ساخت z_work_flusher *flusher)
{
	ساخت k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_سیٹ(&work->flags, K_WORK_FLUSHING_ثنائی_ہندسہ);
}

/* List of pending cancellations. */
ساکن sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
ساکن اندرونی خالی init_work_cancel(ساخت z_work_canceller *canceler,
				    ساخت k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->گرہ);
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
ساکن خالی finalize_flush_locked(ساخت k_work *work)
{
	ساخت z_work_flusher *flusher
		= CONTAINER_کا(work, ساخت z_work_flusher, work);

	flag_clear(&work->flags, K_WORK_FLUSHING_ثنائی_ہندسہ);

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
ساکن خالی finalize_cancel_locked(ساخت k_work *work)
{
	ساخت z_work_canceller *wc, *tmp;
	sys_snode_t *پچھلا = تہی;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_ثنائی_ہندسہ);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_محفوظ(&pending_cancels, wc, tmp, گرہ) {
		اگر (wc->work == work) {
			sys_slist_remove(&pending_cancels, پچھلا, &wc->گرہ);
			k_sem_give(&wc->sem);
		} ورنہ {
			پچھلا = &wc->گرہ;
		}
	}
}

خالی k_work_init(ساخت k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != تہی);
	__ASSERT_NO_MSG(handler != تہی);

	*work = (ساخت k_work)Z_WORK_INITIALIZER(handler);

	SYS_پورٹ_TRACING_OBJ_INIT(k_work, work);
}

ساکن اندرونی صحیح work_busy_get_locked(ثابت ساخت k_work *work)
{
	واپس flags_get(&work->flags) & K_WORK_نقاب;
}

صحیح k_work_busy_get(ثابت ساخت k_work *work)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
	صحیح ret = work_busy_get_locked(work);

	k_spin_unlock(&work_تالا, کلید);

	واپس ret;
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
ساکن خالی قطار_flusher_locked(ساخت k_work_q *قطار,
				 ساخت k_work *work,
				 ساخت z_work_flusher *flusher)
{
	init_flusher(flusher);

	اگر ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_داخل_کرو(&قطار->pending, &work->گرہ,
				 &flusher->work.گرہ);
	} ورنہ {
		sys_slist_prepend(&قطار->pending, &flusher->work.گرہ);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
ساکن اندرونی خالی قطار_remove_locked(ساخت k_work_q *قطار,
				       ساخت k_work *work)
{
	اگر (flag_test_and_clear(&work->flags, K_WORK_QUEUED_ثنائی_ہندسہ)) {
		(خالی)sys_slist_ڈھونڈو_and_remove(&قطار->pending, &work->گرہ);
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
ساکن اندرونی bool notify_قطار_locked(ساخت k_work_q *قطار)
{
	bool rv = false;

	اگر (قطار != تہی) {
		rv = z_sched_wake(&قطار->notifyq, 0, تہی);
	}

	واپس rv;
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
ساکن اندرونی صحیح قطار_submit_locked(ساخت k_work_q *قطار,
				      ساخت k_work *work)
{
	اگر (قطار == تہی) {
		واپس -EINVAL;
	}

	صحیح ret;
	bool chained = (_current == قطار->thread_id) && !k_is_اندر_isr();
	bool draining = flag_test(&قطار->flags, K_WORK_قطار_DRAIN_ثنائی_ہندسہ);
	bool plugged = flag_test(&قطار->flags, K_WORK_قطار_PLUGGED_ثنائی_ہندسہ);

	/* Test for acceptability, in priority order:
	 *
	 * * -ENODEV if the queue isn't running.
	 * * -EBUSY if draining and not chained
	 * * -EBUSY if plugged and not draining
	 * * otherwise OK
	 */
	اگر (!flag_test(&قطار->flags, K_WORK_قطار_STARTED_ثنائی_ہندسہ)) {
		ret = -ENODEV;
	} ورنہ اگر (draining && !chained) {
		ret = -EBUSY;
	} ورنہ اگر (plugged && !draining) {
		ret = -EBUSY;
	} ورنہ {
		sys_slist_append(&قطار->pending, &work->گرہ);
		ret = 1;
		(خالی)notify_قطار_locked(قطار);
	}

	واپس ret;
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
ساکن صحیح submit_مقصد_قطار_locked(ساخت k_work *work,
				  ساخت k_work_q **queuep)
{
	صحیح ret = 0;

	اگر (flag_test(&work->flags, K_WORK_CANCELING_ثنائی_ہندسہ)) {
		/* Disallowed */
		ret = -EBUSY;
	} ورنہ اگر (!flag_test(&work->flags, K_WORK_QUEUED_ثنائی_ہندسہ)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		اگر (*queuep == تہی) {
			*queuep = work->قطار;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		اگر (flag_test(&work->flags, K_WORK_RUNNING_ثنائی_ہندسہ)) {
			__ASSERT_NO_MSG(work->قطار != تہی);
			*queuep = work->قطار;
			ret = 2;
		}

		صحیح rc = قطار_submit_locked(*queuep, work);

		اگر (rc < 0) {
			ret = rc;
		} ورنہ {
			flag_سیٹ(&work->flags, K_WORK_QUEUED_ثنائی_ہندسہ);
			work->قطار = *queuep;
		}
	} ورنہ {
		/* Already queued, do nothing. */
	}

	اگر (ret <= 0) {
		*queuep = تہی;
	}

	واپس ret;
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
صحیح z_work_submit_مقصد_قطار(ساخت k_work_q *قطار,
		  ساخت k_work *work)
{
	__ASSERT_NO_MSG(work != تہی);
	__ASSERT_NO_MSG(work->handler != تہی);

	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	صحیح ret = submit_مقصد_قطار_locked(work, &قطار);

	/* No need to reschedule if the submission failed. */
	اگر (ret > 0) {
		z_reschedule(&work_تالا, کلید);
	} ورنہ {
		k_spin_unlock(&work_تالا, کلید);
	}

	واپس ret;
}

صحیح k_work_submit_مقصد_قطار(ساخت k_work_q *قطار,
			    ساخت k_work *work)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, submit_مقصد_قطار, قطار, work);

	صحیح ret = z_work_submit_مقصد_قطار(قطار, work);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, submit_مقصد_قطار, قطار, work, ret);

	واپس ret;
}

صحیح k_work_submit(ساخت k_work *work)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	صحیح ret = z_work_submit_مقصد_قطار(&k_sys_work_q, work);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	واپس ret;
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
ساکن bool work_flush_locked(ساخت k_work *work,
			      ساخت z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	اگر (need_flush) {
		ساخت k_work_q *قطار = work->قطار;

		__ASSERT_NO_MSG(قطار != تہی);

		قطار_flusher_locked(قطار, work, flusher);
		notify_قطار_locked(قطار);
	}

	واپس need_flush;
}

bool k_work_flush(ساخت k_work *work,
		  ساخت k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != تہی);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ثنائی_ہندسہ));
	__ASSERT_NO_MSG(!k_is_اندر_isr());
	__ASSERT_NO_MSG(sync != تہی);
#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	ساخت z_work_flusher *flusher = &sync->flusher;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_تالا, کلید);

	/* If necessary wait until the flusher item completes */
	اگر (need_flush) {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	واپس need_flush;
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
ساکن صحیح cancel_async_locked(ساخت k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	اگر (!flag_test(&work->flags, K_WORK_CANCELING_ثنائی_ہندسہ)) {
		/* Remove it from the queue, if it's queued. */
		قطار_remove_locked(work->قطار, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	صحیح ret = work_busy_get_locked(work);

	اگر (ret != 0) {
		flag_سیٹ(&work->flags, K_WORK_CANCELING_ثنائی_ہندسہ);
		ret = work_busy_get_locked(work);
	}

	واپس ret;
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
ساکن bool cancel_sync_locked(ساخت k_work *work,
			       ساخت z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_ثنائی_ہندسہ);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	اگر (ret) {
		init_work_cancel(canceller, work);
	}

	واپس ret;
}

صحیح k_work_cancel(ساخت k_work *work)
{
	__ASSERT_NO_MSG(work != تہی);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ثنائی_ہندسہ));

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
	صحیح ret = cancel_async_locked(work);

	k_spin_unlock(&work_تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	واپس ret;
}

bool k_work_cancel_sync(ساخت k_work *work,
			ساخت k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != تہی);
	__ASSERT_NO_MSG(sync != تہی);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_ثنائی_ہندسہ));
	__ASSERT_NO_MSG(!k_is_اندر_isr());
#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	ساخت z_work_canceller *canceller = &sync->canceller;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	اگر (pending) {
		(خالی)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_تالا, کلید);

	اگر (need_wait) {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	واپس pending;
}

#اگر defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
ساکن خالی work_timeout_handler(ساخت _timeout *ریکارڈ)
{
	ساخت k_work_q *قطار = CONTAINER_کا(ریکارڈ, ساخت k_work_q, work_timeout_ریکارڈ);
	ساخت k_work *work = تہی;
	k_work_handler_t handler = تہی;
	ثابت حرف *name;
	ثابت حرف *space = " ";
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	work = قطار->work;
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
	اگر (قطار->ختم_شدہ || z_timeout_inflight_superseded(ریکارڈ)) {
		k_spin_unlock(&work_تالا, کلید);
		واپس;
	}
	قطار->ختم_شدہ = true;

	k_spin_unlock(&work_تالا, کلید);

	name = k_thread_name_get(قطار->thread_id);
	اگر (name == تہی) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		قطار, space, name, work, handler);

	k_thread_abort(قطار->thread_id);
}

ساکن خالی work_timeout_آغاز_locked(ساخت k_work_q *قطار, ساخت k_work *work)
{
	قطار->ختم_شدہ = false;

	اگر (K_TIMEOUT_EQ(قطار->work_timeout, K_FOREVER)) {
		واپس;
	}

	قطار->work = work;
	z_جوڑو_timeout(&قطار->work_timeout_ریکارڈ, work_timeout_handler, قطار->work_timeout);
}

ساکن خالی work_timeout_stop_locked(ساخت k_work_q *قطار)
{
	اگر (K_TIMEOUT_EQ(قطار->work_timeout, K_FOREVER)) {
		واپس;
	}

	(خالی)z_try_abort_timeout(&قطار->work_timeout_ریکارڈ);
}
#اگر_ختم /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
ساکن خالی work_قطار_main(خالی *workq_ptr, خالی *p2, خالی *p3)
{
	ARG_غیر_مستعمل(p2);
	ARG_غیر_مستعمل(p3);

	ساخت k_work_q *قطار = (ساخت k_work_q *)workq_ptr;

	جب (true) {
		sys_snode_t *گرہ;
		ساخت k_work *work = تہی;
		k_work_handler_t handler = تہی;
		k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
		bool yield;

		/* Check for and prepare any new work. */
		گرہ = sys_slist_get(&قطار->pending);
		اگر (گرہ != تہی) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_سیٹ(&قطار->flags, K_WORK_قطار_BUSY_ثنائی_ہندسہ);
			work = CONTAINER_کا(گرہ, ساخت k_work, گرہ);
			flag_سیٹ(&work->flags, K_WORK_RUNNING_ثنائی_ہندسہ);
			flag_clear(&work->flags, K_WORK_QUEUED_ثنائی_ہندسہ);

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
		} ورنہ اگر (flag_test_and_clear(&قطار->flags,
					       K_WORK_قطار_DRAIN_ثنائی_ہندسہ)) {
			/* Not busy and draining: move threads waiting for
			 * drain to ready state.  The held spinlock inhibits
			 * immediate reschedule; released threads get their
			 * chance when this invokes z_sched_wait() below.
			 *
			 * We don't touch K_WORK_QUEUE_PLUGGABLE, so getting
			 * here doesn't mean that the queue will allow new
			 * submissions.
			 */
			(خالی)z_sched_wake_all(&قطار->drainq, 1, تہی);
		} ورنہ اگر (flag_test(&قطار->flags, K_WORK_قطار_STOP_ثنائی_ہندسہ)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_سیٹ(&قطار->flags, 0);
			k_spin_unlock(&work_تالا, کلید);
			واپس;
		} ورنہ {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		اگر (work == تہی) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(خالی)z_sched_wait(&work_تالا, کلید, &قطار->notifyq,
					   K_FOREVER, تہی);
			جاری;
		}

#اگر defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_آغاز_locked(قطار, work);
#اگر_ختم /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_تالا, کلید);

		__ASSERT_NO_MSG(handler != تہی);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		کلید = k_spin_تالا(&work_تالا);

#اگر defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		اگر (قطار->ختم_شدہ) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_تالا, کلید);
			جب (1) {
				k_sleep(K_FOREVER);
			}
			رمز_UNREACHABLE;
		}
		work_timeout_stop_locked(قطار);
		قطار->ختم_شدہ = true;
#اگر_ختم /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_ثنائی_ہندسہ);
		اگر (flag_test(&work->flags, K_WORK_FLUSHING_ثنائی_ہندسہ)) {
			finalize_flush_locked(work);
		}
		اگر (flag_test(&work->flags, K_WORK_CANCELING_ثنائی_ہندسہ)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&قطار->flags, K_WORK_قطار_BUSY_ثنائی_ہندسہ);
		yield = (!flag_test(&قطار->flags, K_WORK_قطار_NO_YIELD_ثنائی_ہندسہ) &&
			 !sys_slist_is_خالی_خانے(&قطار->pending));
		k_spin_unlock(&work_تالا, کلید);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		اگر (yield) {
			k_yield();
		}
	}
}

خالی k_work_قطار_init(ساخت k_work_q *قطار)
{
	__ASSERT_NO_MSG(قطار != تہی);

	*قطار = (ساخت k_work_q) {
		.flags = 0,
	};

	SYS_پورٹ_TRACING_OBJ_INIT(k_work_قطار, قطار);
}

خالی k_work_قطار_تسلسل(ساخت k_work_q *قطار, ثابت ساخت k_work_قطار_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&قطار->flags, K_WORK_قطار_STARTED_ثنائی_ہندسہ));

	uint32_t flags = K_WORK_قطار_STARTED;

	اگر ((cfg != تہی) && cfg->no_yield) {
		flags |= K_WORK_قطار_NO_YIELD;
	}

	اگر ((cfg != تہی) && (cfg->name != تہی)) {
		k_thread_name_سیٹ(_current, cfg->name);
	}

#اگر defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	اگر ((cfg != تہی) && (cfg->work_timeout_ms)) {
		قطار->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ورنہ {
		قطار->work_timeout = K_FOREVER;
	}
#اگر_ختم /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&قطار->pending);
	z_waitq_init(&قطار->notifyq);
	z_waitq_init(&قطار->drainq);
	قطار->thread_id = _current;
	flags_سیٹ(&قطار->flags, flags);
	work_قطار_main(قطار, تہی, تہی);
}

خالی k_work_قطار_آغاز(ساخت k_work_q *قطار,
			k_thread_ڈھیر_t *ڈھیر,
			size_t ڈھیر_سائز,
			صحیح prio,
			ثابت ساخت k_work_قطار_config *cfg)
{
	__ASSERT_NO_MSG(قطار);
	__ASSERT_NO_MSG(ڈھیر);
	__ASSERT_NO_MSG(!flag_test(&قطار->flags, K_WORK_قطار_STARTED_ثنائی_ہندسہ));

	/* In future, this whole function will be deprecated, but for now, we
	 * have to use the `thread` field to create a new thread in it.
	 */
	TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);

	uint32_t flags = K_WORK_قطار_STARTED;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work_قطار, آغاز, قطار);

	sys_slist_init(&قطار->pending);
	z_waitq_init(&قطار->notifyq);
	z_waitq_init(&قطار->drainq);

	اگر ((cfg != تہی) && cfg->no_yield) {
		flags |= K_WORK_قطار_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_سیٹ(&قطار->flags, flags);

	(خالی)k_thread_create(&قطار->thread, ڈھیر, ڈھیر_سائز,
			      work_قطار_main, قطار, تہی, تہی,
			      prio, 0, K_FOREVER);

	اگر ((cfg != تہی) && (cfg->name != تہی)) {
		k_thread_name_سیٹ(&قطار->thread, cfg->name);
	}

	اگر ((cfg != تہی) && (cfg->essential)) {
		قطار->thread.بنیاد.user_options |= K_ESSENTIAL;
	}

#اگر defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	اگر ((cfg != تہی) && (cfg->work_timeout_ms)) {
		قطار->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} ورنہ {
		قطار->work_timeout = K_FOREVER;
	}
#اگر_ختم /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_آغاز(&قطار->thread);
	قطار->thread_id = &قطار->thread;

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_قطار, آغاز, قطار);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

صحیح k_work_قطار_drain(ساخت k_work_q *قطار,
		       bool plug)
{
	__ASSERT_NO_MSG(قطار);
	__ASSERT_NO_MSG(!k_is_اندر_isr());

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work_قطار, drain, قطار);

	صحیح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	اگر (((flags_get(&قطار->flags)
	      & (K_WORK_قطار_BUSY | K_WORK_قطار_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_خالی_خانے(&قطار->pending)) {
		flag_سیٹ(&قطار->flags, K_WORK_قطار_DRAIN_ثنائی_ہندسہ);
		اگر (plug) {
			flag_سیٹ(&قطار->flags, K_WORK_قطار_PLUGGED_ثنائی_ہندسہ);
		}

		notify_قطار_locked(قطار);
		ret = z_sched_wait(&work_تالا, کلید, &قطار->drainq,
				   K_FOREVER, تہی);
	} ورنہ {
		k_spin_unlock(&work_تالا, کلید);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_قطار, drain, قطار, ret);

	واپس ret;
}

صحیح k_work_قطار_unplug(ساخت k_work_q *قطار)
{
	__ASSERT_NO_MSG(قطار);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work_قطار, unplug, قطار);

	صحیح ret = -EALREADY;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	اگر (flag_test_and_clear(&قطار->flags, K_WORK_قطار_PLUGGED_ثنائی_ہندسہ)) {
		ret = 0;
	}

	k_spin_unlock(&work_تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_قطار, unplug, قطار, ret);

	واپس ret;
}

صحیح k_work_قطار_stop(ساخت k_work_q *قطار, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(قطار);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work_قطار, stop, قطار, timeout);

	اگر (قطار->thread_id) {
		اگر (z_is_thread_essential(قطار->thread_id)) {
			واپس -ENOTSUP;
		}
	}

	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	اگر (!flag_test(&قطار->flags, K_WORK_قطار_STARTED_ثنائی_ہندسہ)) {
		k_spin_unlock(&work_تالا, کلید);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_قطار, stop, قطار, timeout, -EALREADY);
		واپس -EALREADY;
	}

	اگر (!flag_test(&قطار->flags, K_WORK_قطار_PLUGGED_ثنائی_ہندسہ)) {
		k_spin_unlock(&work_تالا, کلید);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_قطار, stop, قطار, timeout, -EBUSY);
		واپس -EBUSY;
	}

	flag_سیٹ(&قطار->flags, K_WORK_قطار_STOP_ثنائی_ہندسہ);
	notify_قطار_locked(قطار);
	k_spin_unlock(&work_تالا, کلید);
	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_work_قطار, stop, قطار, timeout);
	اگر (k_thread_join(قطار->thread_id, timeout)) {
		کلید = k_spin_تالا(&work_تالا);
		flag_clear(&قطار->flags, K_WORK_قطار_STOP_ثنائی_ہندسہ);
		k_spin_unlock(&work_تالا, کلید);
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_قطار, stop, قطار, timeout, -ETIMEDOUT);
		واپس -ETIMEDOUT;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work_قطار, stop, قطار, timeout, 0);
	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
ساکن خالی work_timeout(ساخت _timeout *مقصد)
{
	ساخت k_work_delayable *dw
		= CONTAINER_کا(مقصد, ساخت k_work_delayable, timeout);
	ساخت k_work *wp = &dw->work;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
	ساخت k_work_q *قطار = تہی;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	اگر (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_ثنائی_ہندسہ)) {
		قطار = dw->قطار;
		(خالی)submit_مقصد_قطار_locked(wp, &قطار);
	}

	k_spin_unlock(&work_تالا, کلید);
}

خالی k_work_init_delayable(ساخت k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != تہی);
	__ASSERT_NO_MSG(handler != تہی);

	*dwork = (ساخت k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_پورٹ_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

ساکن اندرونی صحیح work_delayable_busy_get_locked(ثابت ساخت k_work_delayable *dwork)
{
	واپس flags_get(&dwork->work.flags) & K_WORK_نقاب;
}

صحیح k_work_delayable_busy_get(ثابت ساخت k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != تہی);

	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
	صحیح ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_تالا, کلید);
	واپس ret;
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
ساکن صحیح جدول_وقت_for_قطار_locked(ساخت k_work_q **queuep,
				     ساخت k_work_delayable *dwork,
				     k_timeout_t delay)
{
	صحیح ret = 1;
	ساخت k_work *work = &dwork->work;

	اگر (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		واپس submit_مقصد_قطار_locked(work, queuep);
	}

	flag_سیٹ(&work->flags, K_WORK_DELAYED_ثنائی_ہندسہ);
	dwork->قطار = *queuep;

	/* Add timeout */
	z_جوڑو_timeout(&dwork->timeout, work_timeout, delay);

	واپس ret;
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
ساکن اندرونی bool unschedule_locked(ساخت k_work_delayable *dwork,
				     k_spinlock_کلید_t *کلید)
{
	bool ret = false;
	ساخت k_work *work = &dwork->work;

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
	اگر (flag_test_and_clear(&work->flags, K_WORK_DELAYED_ثنائی_ہندسہ)) {
		جب (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_تالا, *کلید);
			*کلید = k_spin_تالا(&work_تالا);
		}
		ret = true;
	}

	واپس ret;
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
ساکن صحیح cancel_delayable_async_locked(ساخت k_work_delayable *dwork,
					 k_spinlock_کلید_t *کلید)
{
	(خالی)unschedule_locked(dwork, کلید);

	واپس cancel_async_locked(&dwork->work);
}

صحیح k_work_جدول_وقت_for_قطار(ساخت k_work_q *قطار, ساخت k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(قطار != تہی);
	__ASSERT_NO_MSG(dwork != تہی);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, جدول_وقت_for_قطار, قطار, dwork, delay);

	ساخت k_work *work = &dwork->work;
	صحیح ret = 0;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	/* Schedule the work item if it's idle or running. */
	اگر ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = جدول_وقت_for_قطار_locked(&قطار, dwork, delay);
	}

	k_spin_unlock(&work_تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, جدول_وقت_for_قطار, قطار, dwork, delay, ret);

	واپس ret;
}

صحیح k_work_جدول_وقت(ساخت k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, جدول_وقت, dwork, delay);

	صحیح ret = k_work_جدول_وقت_for_قطار(&k_sys_work_q, dwork, delay);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, جدول_وقت, dwork, delay, ret);

	واپس ret;
}

صحیح k_work_reschedule_for_قطار(ساخت k_work_q *قطار, ساخت k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(قطار != تہی);
	__ASSERT_NO_MSG(dwork != تہی);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_قطار, قطار, dwork, delay);

	صحیح ret;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	/* Remove any active scheduling. */
	(خالی)unschedule_locked(dwork, &کلید);

	/* Schedule the work item with the new parameters. */
	ret = جدول_وقت_for_قطار_locked(&قطار, dwork, delay);

	k_spin_unlock(&work_تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_قطار, قطار, dwork, delay, ret);

	واپس ret;
}

صحیح k_work_reschedule(ساخت k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	صحیح ret = k_work_reschedule_for_قطار(&k_sys_work_q, dwork, delay);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	واپس ret;
}

صحیح k_work_cancel_delayable(ساخت k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != تہی);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
	صحیح ret = cancel_delayable_async_locked(dwork, &کلید);

	k_spin_unlock(&work_تالا, کلید);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	واپس ret;
}

bool k_work_cancel_delayable_sync(ساخت k_work_delayable *dwork,
				  ساخت k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != تہی);
	__ASSERT_NO_MSG(sync != تہی);
	__ASSERT_NO_MSG(!k_is_اندر_isr());
#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	ساخت z_work_canceller *canceller = &sync->canceller;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	اگر (pending) {
		(خالی)cancel_delayable_async_locked(dwork, &کلید);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_تالا, کلید);

	اگر (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	واپس pending;
}

bool k_work_flush_delayable(ساخت k_work_delayable *dwork,
			    ساخت k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != تہی);
	__ASSERT_NO_MSG(sync != تہی);
	__ASSERT_NO_MSG(!k_is_اندر_isr());
#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	ساخت k_work *work = &dwork->work;
	ساخت z_work_flusher *flusher = &sync->flusher;
	k_spinlock_کلید_t کلید = k_spin_تالا(&work_تالا);

	/* If it's idle release the lock and return immediately. */
	اگر (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_تالا, کلید);

		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		واپس false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	اگر (unschedule_locked(dwork, &کلید)) {
		ساخت k_work_q *قطار = dwork->قطار;

		(خالی)submit_مقصد_قطار_locked(work, &قطار);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_تالا, کلید);

	/* If necessary wait until the flusher item completes */
	اگر (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	واپس need_flush;
}

#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */
