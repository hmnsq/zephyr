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

#समावेश <zephyr/kernel.h>
#समावेश <wait_q.h>
#समावेश <zephyr/spinlock.h>
#समावेश <errno.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>
#समावेश <zephyr/sys/printk.h>
#समावेश <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

अटल अंतरभूत व्योम flag_clear(uint32_t *flagp,
			      uint32_t bit)
{
	*flagp &= ~BIT(bit);
}

अटल अंतरभूत व्योम flag_set(uint32_t *flagp,
			    uint32_t bit)
{
	*flagp |= BIT(bit);
}

अटल अंतरभूत bool flag_test(स्थिर uint32_t *flagp,
			     uint32_t bit)
{
	वापस (*flagp & BIT(bit)) != 0U;
}

अटल अंतरभूत bool flag_test_and_clear(uint32_t *flagp,
				       पूर्णांक bit)
{
	bool ret = flag_test(flagp, bit);

	flag_clear(flagp, bit);

	वापस ret;
}

अटल अंतरभूत व्योम flags_set(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

अटल अंतरभूत uint32_t flags_get(स्थिर uint32_t *flagp)
{
	वापस *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
अटल काष्ठा k_spinlock work_lock;

/* Invoked by work thread */
अटल व्योम handle_flush(काष्ठा k_work *work) { }

अटल अंतरभूत व्योम init_flusher(काष्ठा z_work_flusher *flusher)
{
	काष्ठा k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_set(&work->flags, K_WORK_FLUSHING_BIT);
}

/* List of pending cancellations. */
अटल sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
अटल अंतरभूत व्योम init_work_cancel(काष्ठा z_work_canceller *canceler,
				    काष्ठा k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->गाँठ);
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
अटल व्योम finalize_flush_locked(काष्ठा k_work *work)
{
	काष्ठा z_work_flusher *flusher
		= CONTAINER_OF(work, काष्ठा z_work_flusher, work);

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
अटल व्योम finalize_cancel_locked(काष्ठा k_work *work)
{
	काष्ठा z_work_canceller *wc, *tmp;
	sys_snode_t *पूर्व = शून्य;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_BIT);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pending_cancels, wc, tmp, गाँठ) {
		अगर (wc->work == work) {
			sys_slist_remove(&pending_cancels, पूर्व, &wc->गाँठ);
			k_sem_give(&wc->sem);
		} अन्यथा {
			पूर्व = &wc->गाँठ;
		}
	}
}

व्योम k_work_init(काष्ठा k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != शून्य);
	__ASSERT_NO_MSG(handler != शून्य);

	*work = (काष्ठा k_work)Z_WORK_INITIALIZER(handler);

	SYS_PORT_TRACING_OBJ_INIT(k_work, work);
}

अटल अंतरभूत पूर्णांक work_busy_get_locked(स्थिर काष्ठा k_work *work)
{
	वापस flags_get(&work->flags) & K_WORK_MASK;
}

पूर्णांक k_work_busy_get(स्थिर काष्ठा k_work *work)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
	पूर्णांक ret = work_busy_get_locked(work);

	k_spin_unlock(&work_lock, कुंजी);

	वापस ret;
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
अटल व्योम queue_flusher_locked(काष्ठा k_work_q *queue,
				 काष्ठा k_work *work,
				 काष्ठा z_work_flusher *flusher)
{
	init_flusher(flusher);

	अगर ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_जोड़ो(&queue->pending, &work->गाँठ,
				 &flusher->work.गाँठ);
	} अन्यथा {
		sys_slist_prepend(&queue->pending, &flusher->work.गाँठ);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
अटल अंतरभूत व्योम queue_remove_locked(काष्ठा k_work_q *queue,
				       काष्ठा k_work *work)
{
	अगर (flag_test_and_clear(&work->flags, K_WORK_QUEUED_BIT)) {
		(व्योम)sys_slist_find_and_remove(&queue->pending, &work->गाँठ);
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
अटल अंतरभूत bool notify_queue_locked(काष्ठा k_work_q *queue)
{
	bool rv = false;

	अगर (queue != शून्य) {
		rv = z_sched_wake(&queue->notifyq, 0, शून्य);
	}

	वापस rv;
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
अटल अंतरभूत पूर्णांक queue_submit_locked(काष्ठा k_work_q *queue,
				      काष्ठा k_work *work)
{
	अगर (queue == शून्य) {
		वापस -EINVAL;
	}

	पूर्णांक ret;
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
	अगर (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		ret = -ENODEV;
	} अन्यथा अगर (draining && !chained) {
		ret = -EBUSY;
	} अन्यथा अगर (plugged && !draining) {
		ret = -EBUSY;
	} अन्यथा {
		sys_slist_append(&queue->pending, &work->गाँठ);
		ret = 1;
		(व्योम)notify_queue_locked(queue);
	}

	वापस ret;
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
अटल पूर्णांक submit_to_queue_locked(काष्ठा k_work *work,
				  काष्ठा k_work_q **queuep)
{
	पूर्णांक ret = 0;

	अगर (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Disallowed */
		ret = -EBUSY;
	} अन्यथा अगर (!flag_test(&work->flags, K_WORK_QUEUED_BIT)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		अगर (*queuep == शून्य) {
			*queuep = work->queue;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		अगर (flag_test(&work->flags, K_WORK_RUNNING_BIT)) {
			__ASSERT_NO_MSG(work->queue != शून्य);
			*queuep = work->queue;
			ret = 2;
		}

		पूर्णांक rc = queue_submit_locked(*queuep, work);

		अगर (rc < 0) {
			ret = rc;
		} अन्यथा {
			flag_set(&work->flags, K_WORK_QUEUED_BIT);
			work->queue = *queuep;
		}
	} अन्यथा {
		/* Already queued, do nothing. */
	}

	अगर (ret <= 0) {
		*queuep = शून्य;
	}

	वापस ret;
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
पूर्णांक z_work_submit_to_queue(काष्ठा k_work_q *queue,
		  काष्ठा k_work *work)
{
	__ASSERT_NO_MSG(work != शून्य);
	__ASSERT_NO_MSG(work->handler != शून्य);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	पूर्णांक ret = submit_to_queue_locked(work, &queue);

	/* No need to reschedule if the submission failed. */
	अगर (ret > 0) {
		z_reschedule(&work_lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&work_lock, कुंजी);
	}

	वापस ret;
}

पूर्णांक k_work_submit_to_queue(काष्ठा k_work_q *queue,
			    काष्ठा k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit_to_queue, queue, work);

	पूर्णांक ret = z_work_submit_to_queue(queue, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit_to_queue, queue, work, ret);

	वापस ret;
}

पूर्णांक k_work_submit(काष्ठा k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	पूर्णांक ret = z_work_submit_to_queue(&k_sys_work_q, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	वापस ret;
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
अटल bool work_flush_locked(काष्ठा k_work *work,
			      काष्ठा z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	अगर (need_flush) {
		काष्ठा k_work_q *queue = work->queue;

		__ASSERT_NO_MSG(queue != शून्य);

		queue_flusher_locked(queue, work, flusher);
		notify_queue_locked(queue);
	}

	वापस need_flush;
}

bool k_work_flush(काष्ठा k_work *work,
		  काष्ठा k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != शून्य);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
	__ASSERT_NO_MSG(sync != शून्य);
#अगर_घोषित CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	काष्ठा z_work_flusher *flusher = &sync->flusher;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, कुंजी);

	/* If necessary wait until the flusher item completes */
	अगर (need_flush) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	वापस need_flush;
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
अटल पूर्णांक cancel_async_locked(काष्ठा k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	अगर (!flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Remove it from the queue, if it's queued. */
		queue_remove_locked(work->queue, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	पूर्णांक ret = work_busy_get_locked(work);

	अगर (ret != 0) {
		flag_set(&work->flags, K_WORK_CANCELING_BIT);
		ret = work_busy_get_locked(work);
	}

	वापस ret;
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
अटल bool cancel_sync_locked(काष्ठा k_work *work,
			       काष्ठा z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_BIT);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	अगर (ret) {
		init_work_cancel(canceller, work);
	}

	वापस ret;
}

पूर्णांक k_work_cancel(काष्ठा k_work *work)
{
	__ASSERT_NO_MSG(work != शून्य);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
	पूर्णांक ret = cancel_async_locked(work);

	k_spin_unlock(&work_lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	वापस ret;
}

bool k_work_cancel_sync(काष्ठा k_work *work,
			काष्ठा k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != शून्य);
	__ASSERT_NO_MSG(sync != शून्य);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
#अगर_घोषित CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	काष्ठा z_work_canceller *canceller = &sync->canceller;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	अगर (pending) {
		(व्योम)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_lock, कुंजी);

	अगर (need_wait) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	वापस pending;
}

#अगर defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
अटल व्योम work_timeout_handler(काष्ठा _timeout *record)
{
	काष्ठा k_work_q *queue = CONTAINER_OF(record, काष्ठा k_work_q, work_timeout_record);
	काष्ठा k_work *work = शून्य;
	k_work_handler_t handler = शून्य;
	स्थिर अक्षर *name;
	स्थिर अक्षर *space = " ";
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

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
	अगर (queue->finished || z_timeout_inflight_superseded(record)) {
		k_spin_unlock(&work_lock, कुंजी);
		वापस;
	}
	queue->finished = true;

	k_spin_unlock(&work_lock, कुंजी);

	name = k_thread_name_get(queue->thread_id);
	अगर (name == शून्य) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		queue, space, name, work, handler);

	k_thread_abort(queue->thread_id);
}

अटल व्योम work_timeout_start_locked(काष्ठा k_work_q *queue, काष्ठा k_work *work)
{
	queue->finished = false;

	अगर (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		वापस;
	}

	queue->work = work;
	z_add_timeout(&queue->work_timeout_record, work_timeout_handler, queue->work_timeout);
}

अटल व्योम work_timeout_stop_locked(काष्ठा k_work_q *queue)
{
	अगर (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		वापस;
	}

	(व्योम)z_try_abort_timeout(&queue->work_timeout_record);
}
#पूर्ण_अगर /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
अटल व्योम work_queue_main(व्योम *workq_ptr, व्योम *p2, व्योम *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	काष्ठा k_work_q *queue = (काष्ठा k_work_q *)workq_ptr;

	जबतक (true) {
		sys_snode_t *गाँठ;
		काष्ठा k_work *work = शून्य;
		k_work_handler_t handler = शून्य;
		k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
		bool yield;

		/* Check for and prepare any new work. */
		गाँठ = sys_slist_get(&queue->pending);
		अगर (गाँठ != शून्य) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_set(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
			work = CONTAINER_OF(गाँठ, काष्ठा k_work, गाँठ);
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
		} अन्यथा अगर (flag_test_and_clear(&queue->flags,
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
			(व्योम)z_sched_wake_all(&queue->drainq, 1, शून्य);
		} अन्यथा अगर (flag_test(&queue->flags, K_WORK_QUEUE_STOP_BIT)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_set(&queue->flags, 0);
			k_spin_unlock(&work_lock, कुंजी);
			वापस;
		} अन्यथा {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		अगर (work == शून्य) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(व्योम)z_sched_wait(&work_lock, कुंजी, &queue->notifyq,
					   K_FOREVER, शून्य);
			जारी;
		}

#अगर defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_start_locked(queue, work);
#पूर्ण_अगर /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_lock, कुंजी);

		__ASSERT_NO_MSG(handler != शून्य);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		कुंजी = k_spin_lock(&work_lock);

#अगर defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		अगर (queue->finished) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_lock, कुंजी);
			जबतक (1) {
				k_sleep(K_FOREVER);
			}
			CODE_UNREACHABLE;
		}
		work_timeout_stop_locked(queue);
		queue->finished = true;
#पूर्ण_अगर /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_BIT);
		अगर (flag_test(&work->flags, K_WORK_FLUSHING_BIT)) {
			finalize_flush_locked(work);
		}
		अगर (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
		yield = (!flag_test(&queue->flags, K_WORK_QUEUE_NO_YIELD_BIT) &&
			 !sys_slist_is_empty(&queue->pending));
		k_spin_unlock(&work_lock, कुंजी);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		अगर (yield) {
			k_yield();
		}
	}
}

व्योम k_work_queue_init(काष्ठा k_work_q *queue)
{
	__ASSERT_NO_MSG(queue != शून्य);

	*queue = (काष्ठा k_work_q) {
		.flags = 0,
	};

	SYS_PORT_TRACING_OBJ_INIT(k_work_queue, queue);
}

व्योम k_work_queue_run(काष्ठा k_work_q *queue, स्थिर काष्ठा k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT));

	uint32_t flags = K_WORK_QUEUE_STARTED;

	अगर ((cfg != शून्य) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	अगर ((cfg != शून्य) && (cfg->name != शून्य)) {
		k_thread_name_set(_current, cfg->name);
	}

#अगर defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	अगर ((cfg != शून्य) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} अन्यथा {
		queue->work_timeout = K_FOREVER;
	}
#पूर्ण_अगर /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&queue->pending);
	z_waitq_init(&queue->notifyq);
	z_waitq_init(&queue->drainq);
	queue->thread_id = _current;
	flags_set(&queue->flags, flags);
	work_queue_main(queue, शून्य, शून्य);
}

व्योम k_work_queue_start(काष्ठा k_work_q *queue,
			k_thread_ढेर_t *ढेर,
			माप_प्रकार ढेर_आकार,
			पूर्णांक prio,
			स्थिर काष्ठा k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(ढेर);
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

	अगर ((cfg != शून्य) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_set(&queue->flags, flags);

	(व्योम)k_thread_create(&queue->thread, ढेर, ढेर_आकार,
			      work_queue_main, queue, शून्य, शून्य,
			      prio, 0, K_FOREVER);

	अगर ((cfg != शून्य) && (cfg->name != शून्य)) {
		k_thread_name_set(&queue->thread, cfg->name);
	}

	अगर ((cfg != शून्य) && (cfg->essential)) {
		queue->thread.base.user_options |= K_ESSENTIAL;
	}

#अगर defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	अगर ((cfg != शून्य) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} अन्यथा {
		queue->work_timeout = K_FOREVER;
	}
#पूर्ण_अगर /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_start(&queue->thread);
	queue->thread_id = &queue->thread;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, start, queue);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

पूर्णांक k_work_queue_drain(काष्ठा k_work_q *queue,
		       bool plug)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(!k_is_in_isr());

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, drain, queue);

	पूर्णांक ret = 0;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	अगर (((flags_get(&queue->flags)
	      & (K_WORK_QUEUE_BUSY | K_WORK_QUEUE_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_empty(&queue->pending)) {
		flag_set(&queue->flags, K_WORK_QUEUE_DRAIN_BIT);
		अगर (plug) {
			flag_set(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT);
		}

		notify_queue_locked(queue);
		ret = z_sched_wait(&work_lock, कुंजी, &queue->drainq,
				   K_FOREVER, शून्य);
	} अन्यथा {
		k_spin_unlock(&work_lock, कुंजी);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, drain, queue, ret);

	वापस ret;
}

पूर्णांक k_work_queue_unplug(काष्ठा k_work_q *queue)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, unplug, queue);

	पूर्णांक ret = -EALREADY;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	अगर (flag_test_and_clear(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		ret = 0;
	}

	k_spin_unlock(&work_lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, unplug, queue, ret);

	वापस ret;
}

पूर्णांक k_work_queue_stop(काष्ठा k_work_q *queue, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, stop, queue, timeout);

	अगर (queue->thread_id) {
		अगर (z_is_thread_essential(queue->thread_id)) {
			वापस -ENOTSUP;
		}
	}

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	अगर (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		k_spin_unlock(&work_lock, कुंजी);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EALREADY);
		वापस -EALREADY;
	}

	अगर (!flag_test(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		k_spin_unlock(&work_lock, कुंजी);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EBUSY);
		वापस -EBUSY;
	}

	flag_set(&queue->flags, K_WORK_QUEUE_STOP_BIT);
	notify_queue_locked(queue);
	k_spin_unlock(&work_lock, कुंजी);
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work_queue, stop, queue, timeout);
	अगर (k_thread_join(queue->thread_id, timeout)) {
		कुंजी = k_spin_lock(&work_lock);
		flag_clear(&queue->flags, K_WORK_QUEUE_STOP_BIT);
		k_spin_unlock(&work_lock, कुंजी);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -ETIMEDOUT);
		वापस -ETIMEDOUT;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, 0);
	वापस 0;
}

#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
अटल व्योम work_timeout(काष्ठा _timeout *to)
{
	काष्ठा k_work_delayable *dw
		= CONTAINER_OF(to, काष्ठा k_work_delayable, timeout);
	काष्ठा k_work *wp = &dw->work;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
	काष्ठा k_work_q *queue = शून्य;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	अगर (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_BIT)) {
		queue = dw->queue;
		(व्योम)submit_to_queue_locked(wp, &queue);
	}

	k_spin_unlock(&work_lock, कुंजी);
}

व्योम k_work_init_delayable(काष्ठा k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != शून्य);
	__ASSERT_NO_MSG(handler != शून्य);

	*dwork = (काष्ठा k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

अटल अंतरभूत पूर्णांक work_delayable_busy_get_locked(स्थिर काष्ठा k_work_delayable *dwork)
{
	वापस flags_get(&dwork->work.flags) & K_WORK_MASK;
}

पूर्णांक k_work_delayable_busy_get(स्थिर काष्ठा k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != शून्य);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
	पूर्णांक ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_lock, कुंजी);
	वापस ret;
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
अटल पूर्णांक schedule_for_queue_locked(काष्ठा k_work_q **queuep,
				     काष्ठा k_work_delayable *dwork,
				     k_timeout_t delay)
{
	पूर्णांक ret = 1;
	काष्ठा k_work *work = &dwork->work;

	अगर (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		वापस submit_to_queue_locked(work, queuep);
	}

	flag_set(&work->flags, K_WORK_DELAYED_BIT);
	dwork->queue = *queuep;

	/* Add timeout */
	z_add_timeout(&dwork->timeout, work_timeout, delay);

	वापस ret;
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
अटल अंतरभूत bool unschedule_locked(काष्ठा k_work_delayable *dwork,
				     k_spinlock_कुंजी_t *कुंजी)
{
	bool ret = false;
	काष्ठा k_work *work = &dwork->work;

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
	अगर (flag_test_and_clear(&work->flags, K_WORK_DELAYED_BIT)) {
		जबतक (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_lock, *कुंजी);
			*कुंजी = k_spin_lock(&work_lock);
		}
		ret = true;
	}

	वापस ret;
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
अटल पूर्णांक cancel_delayable_async_locked(काष्ठा k_work_delayable *dwork,
					 k_spinlock_कुंजी_t *कुंजी)
{
	(व्योम)unschedule_locked(dwork, कुंजी);

	वापस cancel_async_locked(&dwork->work);
}

पूर्णांक k_work_schedule_for_queue(काष्ठा k_work_q *queue, काष्ठा k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != शून्य);
	__ASSERT_NO_MSG(dwork != शून्य);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule_for_queue, queue, dwork, delay);

	काष्ठा k_work *work = &dwork->work;
	पूर्णांक ret = 0;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	/* Schedule the work item if it's idle or running. */
	अगर ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = schedule_for_queue_locked(&queue, dwork, delay);
	}

	k_spin_unlock(&work_lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule_for_queue, queue, dwork, delay, ret);

	वापस ret;
}

पूर्णांक k_work_schedule(काष्ठा k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule, dwork, delay);

	पूर्णांक ret = k_work_schedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule, dwork, delay, ret);

	वापस ret;
}

पूर्णांक k_work_reschedule_for_queue(काष्ठा k_work_q *queue, काष्ठा k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != शून्य);
	__ASSERT_NO_MSG(dwork != शून्य);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_queue, queue, dwork, delay);

	पूर्णांक ret;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	/* Remove any active scheduling. */
	(व्योम)unschedule_locked(dwork, &कुंजी);

	/* Schedule the work item with the new parameters. */
	ret = schedule_for_queue_locked(&queue, dwork, delay);

	k_spin_unlock(&work_lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_queue, queue, dwork, delay, ret);

	वापस ret;
}

पूर्णांक k_work_reschedule(काष्ठा k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	पूर्णांक ret = k_work_reschedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	वापस ret;
}

पूर्णांक k_work_cancel_delayable(काष्ठा k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != शून्य);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
	पूर्णांक ret = cancel_delayable_async_locked(dwork, &कुंजी);

	k_spin_unlock(&work_lock, कुंजी);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	वापस ret;
}

bool k_work_cancel_delayable_sync(काष्ठा k_work_delayable *dwork,
				  काष्ठा k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != शून्य);
	__ASSERT_NO_MSG(sync != शून्य);
	__ASSERT_NO_MSG(!k_is_in_isr());
#अगर_घोषित CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	काष्ठा z_work_canceller *canceller = &sync->canceller;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	अगर (pending) {
		(व्योम)cancel_delayable_async_locked(dwork, &कुंजी);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_lock, कुंजी);

	अगर (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	वापस pending;
}

bool k_work_flush_delayable(काष्ठा k_work_delayable *dwork,
			    काष्ठा k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != शून्य);
	__ASSERT_NO_MSG(sync != शून्य);
	__ASSERT_NO_MSG(!k_is_in_isr());
#अगर_घोषित CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	काष्ठा k_work *work = &dwork->work;
	काष्ठा z_work_flusher *flusher = &sync->flusher;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&work_lock);

	/* If it's idle release the lock and return immediately. */
	अगर (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		वापस false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	अगर (unschedule_locked(dwork, &कुंजी)) {
		काष्ठा k_work_q *queue = dwork->queue;

		(व्योम)submit_to_queue_locked(work, &queue);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, कुंजी);

	/* If necessary wait until the flusher item completes */
	अगर (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	वापस need_flush;
}

#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */
