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

#include <zephyr/kernel.h>
#include <wait_q.h>
#include <zephyr/spinlock.h>
#include <errno.h>
#include <ksched.h>
#include <scheduler.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

static anta_hstha void flag_clear(uint32_t *flagp,
			      uint32_t bit)
{
	*flagp &= ~BIT(bit);
}

static anta_hstha void flag_set(uint32_t *flagp,
			    uint32_t bit)
{
	*flagp |= BIT(bit);
}

static anta_hstha bool flag_test(const uint32_t *flagp,
			     uint32_t bit)
{
	ཕྱིར (*flagp & BIT(bit)) != 0U;
}

static anta_hstha bool flag_test_and_clear(uint32_t *flagp,
				       ཧྲིལ bit)
{
	bool ret = flag_test(flagp, bit);

	flag_clear(flagp, bit);

	ཕྱིར ret;
}

static anta_hstha void flags_set(uint32_t *flagp,
			     uint32_t flags)
{
	*flagp = flags;
}

static anta_hstha uint32_t flags_get(const uint32_t *flagp)
{
	ཕྱིར *flagp;
}

/* Lock to protect the internal state of all work items, work queues,
 * and pending_cancels.
 */
static sa_nracanaa k_spinlock work_lock;

/* Invoked by work thread */
static void handle_flush(sa_nracanaa k_work *work) { }

static anta_hstha void init_flusher(sa_nracanaa z_work_flusher *flusher)
{
	sa_nracanaa k_work *work = &flusher->work;
	k_sem_init(&flusher->sem, 0, 1);
	k_work_init(&flusher->work, handle_flush);
	flag_set(&work->flags, K_WORK_FLUSHING_BIT);
}

/* List of pending cancellations. */
static sys_slist_t pending_cancels;

/* Initialize a canceler record and add it to the list of pending
 * cancels.
 *
 * Invoked with work lock held.
 *
 * @param canceler the structure used to notify a waiting process.
 * @param work the work structure that is to be canceled
 */
static anta_hstha void init_work_cancel(sa_nracanaa z_work_canceller *canceler,
				    sa_nracanaa k_work *work)
{
	k_sem_init(&canceler->sem, 0, 1);
	canceler->work = work;
	sys_slist_append(&pending_cancels, &canceler->node);
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
static void finalize_flush_locked(sa_nracanaa k_work *work)
{
	sa_nracanaa z_work_flusher *flusher
		= CONTAINER_OF(work, sa_nracanaa z_work_flusher, work);

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
static void finalize_cancel_locked(sa_nracanaa k_work *work)
{
	sa_nracanaa z_work_canceller *wc, *tmp;
	sys_snode_t *prev = NULL;

	/* Clear this first, so released high-priority threads don't
	 * see it when doing things.
	 */
	flag_clear(&work->flags, K_WORK_CANCELING_BIT);

	/* Search for and remove matching containers, and release
	 * what's waiting for the completion.  The same work item can
	 * appear multiple times in the list if multiple threads
	 * attempt to cancel it; each waiter must be woken.
	 */
	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&pending_cancels, wc, tmp, node) {
		གལ་ཏེ (wc->work == work) {
			sys_slist_remove(&pending_cancels, prev, &wc->node);
			k_sem_give(&wc->sem);
		} དེ་མིན {
			prev = &wc->node;
		}
	}
}

void k_work_init(sa_nracanaa k_work *work,
		  k_work_handler_t handler)
{
	__ASSERT_NO_MSG(work != NULL);
	__ASSERT_NO_MSG(handler != NULL);

	*work = (sa_nracanaa k_work)Z_WORK_INITIALIZER(handler);

	SYS_PORT_TRACING_OBJ_INIT(k_work, work);
}

static anta_hstha ཧྲིལ work_busy_get_locked(const sa_nracanaa k_work *work)
{
	ཕྱིར flags_get(&work->flags) & K_WORK_MASK;
}

ཧྲིལ k_work_busy_get(const sa_nracanaa k_work *work)
{
	k_spinlock_key_t key = k_spin_lock(&work_lock);
	ཧྲིལ ret = work_busy_get_locked(work);

	k_spin_unlock(&work_lock, key);

	ཕྱིར ret;
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
static void queue_flusher_locked(sa_nracanaa k_work_q *queue,
				 sa_nracanaa k_work *work,
				 sa_nracanaa z_work_flusher *flusher)
{
	init_flusher(flusher);

	གལ་ཏེ ((flags_get(&work->flags) & K_WORK_QUEUED) != 0U) {
		sys_slist_insert(&queue->pending, &work->node,
				 &flusher->work.node);
	} དེ་མིན {
		sys_slist_prepend(&queue->pending, &flusher->work.node);
	}
}

/* Try to remove a work item from the given queue.
 *
 * Invoked with work lock held.
 *
 * @param queue the queue from which the work should be removed
 * @param work work that may be on the queue
 */
static anta_hstha void queue_remove_locked(sa_nracanaa k_work_q *queue,
				       sa_nracanaa k_work *work)
{
	གལ་ཏེ (flag_test_and_clear(&work->flags, K_WORK_QUEUED_BIT)) {
		(void)sys_slist_find_and_remove(&queue->pending, &work->node);
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
static anta_hstha bool notify_queue_locked(sa_nracanaa k_work_q *queue)
{
	bool rv = false;

	གལ་ཏེ (queue != NULL) {
		rv = z_sched_wake(&queue->notifyq, 0, NULL);
	}

	ཕྱིར rv;
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
static anta_hstha ཧྲིལ queue_submit_locked(sa_nracanaa k_work_q *queue,
				      sa_nracanaa k_work *work)
{
	གལ་ཏེ (queue == NULL) {
		ཕྱིར -EINVAL;
	}

	ཧྲིལ ret;
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
	གལ་ཏེ (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		ret = -ENODEV;
	} དེ་མིན གལ་ཏེ (draining && !chained) {
		ret = -EBUSY;
	} དེ་མིན གལ་ཏེ (plugged && !draining) {
		ret = -EBUSY;
	} དེ་མིན {
		sys_slist_append(&queue->pending, &work->node);
		ret = 1;
		(void)notify_queue_locked(queue);
	}

	ཕྱིར ret;
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
static ཧྲིལ submit_to_queue_locked(sa_nracanaa k_work *work,
				  sa_nracanaa k_work_q **queuep)
{
	ཧྲིལ ret = 0;

	གལ་ཏེ (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Disallowed */
		ret = -EBUSY;
	} དེ་མིན གལ་ཏེ (!flag_test(&work->flags, K_WORK_QUEUED_BIT)) {
		/* Not currently queued */
		ret = 1;

		/* If no queue specified resubmit to last queue.
		 */
		གལ་ཏེ (*queuep == NULL) {
			*queuep = work->queue;
		}

		/* If the work is currently running we have to use the
		 * queue it's running on to prevent handler
		 * re-entrancy.
		 */
		གལ་ཏེ (flag_test(&work->flags, K_WORK_RUNNING_BIT)) {
			__ASSERT_NO_MSG(work->queue != NULL);
			*queuep = work->queue;
			ret = 2;
		}

		ཧྲིལ rc = queue_submit_locked(*queuep, work);

		གལ་ཏེ (rc < 0) {
			ret = rc;
		} དེ་མིན {
			flag_set(&work->flags, K_WORK_QUEUED_BIT);
			work->queue = *queuep;
		}
	} དེ་མིན {
		/* Already queued, do nothing. */
	}

	གལ་ཏེ (ret <= 0) {
		*queuep = NULL;
	}

	ཕྱིར ret;
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
ཧྲིལ z_work_submit_to_queue(sa_nracanaa k_work_q *queue,
		  sa_nracanaa k_work *work)
{
	__ASSERT_NO_MSG(work != NULL);
	__ASSERT_NO_MSG(work->handler != NULL);

	k_spinlock_key_t key = k_spin_lock(&work_lock);

	ཧྲིལ ret = submit_to_queue_locked(work, &queue);

	/* No need to reschedule if the submission failed. */
	གལ་ཏེ (ret > 0) {
		z_reschedule(&work_lock, key);
	} དེ་མིན {
		k_spin_unlock(&work_lock, key);
	}

	ཕྱིར ret;
}

ཧྲིལ k_work_submit_to_queue(sa_nracanaa k_work_q *queue,
			    sa_nracanaa k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit_to_queue, queue, work);

	ཧྲིལ ret = z_work_submit_to_queue(queue, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit_to_queue, queue, work, ret);

	ཕྱིར ret;
}

ཧྲིལ k_work_submit(sa_nracanaa k_work *work)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, submit, work);

	ཧྲིལ ret = z_work_submit_to_queue(&k_sys_work_q, work);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, submit, work, ret);

	ཕྱིར ret;
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
static bool work_flush_locked(sa_nracanaa k_work *work,
			      sa_nracanaa z_work_flusher *flusher)
{
	bool need_flush = (flags_get(&work->flags)
			   & (K_WORK_QUEUED | K_WORK_RUNNING)) != 0U;

	གལ་ཏེ (need_flush) {
		sa_nracanaa k_work_q *queue = work->queue;

		__ASSERT_NO_MSG(queue != NULL);

		queue_flusher_locked(queue, work, flusher);
		notify_queue_locked(queue);
	}

	ཕྱིར need_flush;
}

bool k_work_flush(sa_nracanaa k_work *work,
		  sa_nracanaa k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != NULL);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
	__ASSERT_NO_MSG(sync != NULL);
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush, work);

	sa_nracanaa z_work_flusher *flusher = &sync->flusher;
	k_spinlock_key_t key = k_spin_lock(&work_lock);

	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, key);

	/* If necessary wait until the flusher item completes */
	གལ་ཏེ (need_flush) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, flush, work, K_FOREVER);

		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush, work, need_flush);

	ཕྱིར need_flush;
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
static ཧྲིལ cancel_async_locked(sa_nracanaa k_work *work)
{
	/* If we haven't already started canceling, do it now. */
	གལ་ཏེ (!flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
		/* Remove it from the queue, if it's queued. */
		queue_remove_locked(work->queue, work);
	}

	/* If it's still busy after it's been dequeued, then flag it
	 * as canceling.
	 */
	ཧྲིལ ret = work_busy_get_locked(work);

	གལ་ཏེ (ret != 0) {
		flag_set(&work->flags, K_WORK_CANCELING_BIT);
		ret = work_busy_get_locked(work);
	}

	ཕྱིར ret;
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
static bool cancel_sync_locked(sa_nracanaa k_work *work,
			       sa_nracanaa z_work_canceller *canceller)
{
	bool ret = flag_test(&work->flags, K_WORK_CANCELING_BIT);

	/* If something's still running then we have to wait for
	 * completion, which is indicated when finish_cancel() gets
	 * invoked.
	 */
	གལ་ཏེ (ret) {
		init_work_cancel(canceller, work);
	}

	ཕྱིར ret;
}

ཧྲིལ k_work_cancel(sa_nracanaa k_work *work)
{
	__ASSERT_NO_MSG(work != NULL);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel, work);

	k_spinlock_key_t key = k_spin_lock(&work_lock);
	ཧྲིལ ret = cancel_async_locked(work);

	k_spin_unlock(&work_lock, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel, work, ret);

	ཕྱིར ret;
}

bool k_work_cancel_sync(sa_nracanaa k_work *work,
			sa_nracanaa k_work_sync *sync)
{
	__ASSERT_NO_MSG(work != NULL);
	__ASSERT_NO_MSG(sync != NULL);
	__ASSERT_NO_MSG(!flag_test(&work->flags, K_WORK_DELAYABLE_BIT));
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_sync, work, sync);

	sa_nracanaa z_work_canceller *canceller = &sync->canceller;
	k_spinlock_key_t key = k_spin_lock(&work_lock);
	bool pending = (work_busy_get_locked(work) != 0U);
	bool need_wait = false;

	གལ་ཏེ (pending) {
		(void)cancel_async_locked(work);
		need_wait = cancel_sync_locked(work, canceller);
	}

	k_spin_unlock(&work_lock, key);

	གལ་ཏེ (need_wait) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work, cancel_sync, work, sync);

		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_sync, work, sync, pending);
	ཕྱིར pending;
}

#གལ་ཏེ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
static void work_timeout_handler(sa_nracanaa _timeout *record)
{
	sa_nracanaa k_work_q *queue = CONTAINER_OF(record, sa_nracanaa k_work_q, work_timeout_record);
	sa_nracanaa k_work *work = NULL;
	k_work_handler_t handler = NULL;
	const var_na *name;
	const var_na *space = " ";
	k_spinlock_key_t key = k_spin_lock(&work_lock);

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
	གལ་ཏེ (queue->finished || z_timeout_inflight_superseded(record)) {
		k_spin_unlock(&work_lock, key);
		ཕྱིར;
	}
	queue->finished = true;

	k_spin_unlock(&work_lock, key);

	name = k_thread_name_get(queue->thread_id);
	གལ་ཏེ (name == NULL) {
		name = "";
		space = "";
	}

	LOG_ERR("queue %p%s%s blocked by work %p with handler %p",
		queue, space, name, work, handler);

	k_thread_abort(queue->thread_id);
}

static void work_timeout_start_locked(sa_nracanaa k_work_q *queue, sa_nracanaa k_work *work)
{
	queue->finished = false;

	གལ་ཏེ (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		ཕྱིར;
	}

	queue->work = work;
	z_add_timeout(&queue->work_timeout_record, work_timeout_handler, queue->work_timeout);
}

static void work_timeout_stop_locked(sa_nracanaa k_work_q *queue)
{
	གལ་ཏེ (K_TIMEOUT_EQ(queue->work_timeout, K_FOREVER)) {
		ཕྱིར;
	}

	(void)z_try_abort_timeout(&queue->work_timeout_record);
}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

/* Loop executed by a work queue thread.
 *
 * @param workq_ptr pointer to the work queue structure
 */
static void work_queue_main(void *workq_ptr, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	sa_nracanaa k_work_q *queue = (sa_nracanaa k_work_q *)workq_ptr;

	སྐབས (true) {
		sys_snode_t *node;
		sa_nracanaa k_work *work = NULL;
		k_work_handler_t handler = NULL;
		k_spinlock_key_t key = k_spin_lock(&work_lock);
		bool yield;

		/* Check for and prepare any new work. */
		node = sys_slist_get(&queue->pending);
		གལ་ཏེ (node != NULL) {
			/* Mark that there's some work active that's
			 * not on the pending list.
			 */
			flag_set(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
			work = CONTAINER_OF(node, sa_nracanaa k_work, node);
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
		} དེ་མིན གལ་ཏེ (flag_test_and_clear(&queue->flags,
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
			(void)z_sched_wake_all(&queue->drainq, 1, NULL);
		} དེ་མིན གལ་ཏེ (flag_test(&queue->flags, K_WORK_QUEUE_STOP_BIT)) {
			/* User has requested that the queue stop. Clear the status flags and exit.
			 */
			flags_set(&queue->flags, 0);
			k_spin_unlock(&work_lock, key);
			ཕྱིར;
		} དེ་མིན {
			/* No work is available and no queue state requires
			 * special handling.
			 */
			;
		}

		གལ་ཏེ (work == NULL) {
			/* Nothing's had a chance to add work since we took
			 * the lock, and we didn't find work nor got asked to
			 * stop.  Just go to sleep: when something happens the
			 * work thread will be woken and we can check again.
			 */

			(void)z_sched_wait(&work_lock, key, &queue->notifyq,
					   K_FOREVER, NULL);
			continue;
		}

#གལ་ཏེ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		work_timeout_start_locked(queue, work);
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		k_spin_unlock(&work_lock, key);

		__ASSERT_NO_MSG(handler != NULL);
		handler(work);

		/* Mark the work item as no longer running and deal
		 * with any cancellation and flushing issued while it
		 * was running.  Clear the BUSY flag and optionally
		 * yield to prevent starving other threads.
		 */
		key = k_spin_lock(&work_lock);

#གལ་ཏེ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
		གལ་ཏེ (queue->finished) {
			/*
			 * The work item timeout handler has flagged this work
			 * thread for taking too long and is going to abort it.
			 * Do not proceed to the next work item.
			 */
			k_spin_unlock(&work_lock, key);
			སྐབས (1) {
				k_sleep(K_FOREVER);
			}
			CODE_UNREACHABLE;
		}
		work_timeout_stop_locked(queue);
		queue->finished = true;
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

		flag_clear(&work->flags, K_WORK_RUNNING_BIT);
		གལ་ཏེ (flag_test(&work->flags, K_WORK_FLUSHING_BIT)) {
			finalize_flush_locked(work);
		}
		གལ་ཏེ (flag_test(&work->flags, K_WORK_CANCELING_BIT)) {
			finalize_cancel_locked(work);
		}

		flag_clear(&queue->flags, K_WORK_QUEUE_BUSY_BIT);
		yield = (!flag_test(&queue->flags, K_WORK_QUEUE_NO_YIELD_BIT) &&
			 !sys_slist_is_empty(&queue->pending));
		k_spin_unlock(&work_lock, key);

		/* Optionally yield to prevent the work queue from
		 * starving other threads.
		 */
		གལ་ཏེ (yield) {
			k_yield();
		}
	}
}

void k_work_queue_init(sa_nracanaa k_work_q *queue)
{
	__ASSERT_NO_MSG(queue != NULL);

	*queue = (sa_nracanaa k_work_q) {
		.flags = 0,
	};

	SYS_PORT_TRACING_OBJ_INIT(k_work_queue, queue);
}

void k_work_queue_run(sa_nracanaa k_work_q *queue, const sa_nracanaa k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT));

	uint32_t flags = K_WORK_QUEUE_STARTED;

	གལ་ཏེ ((cfg != NULL) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	གལ་ཏེ ((cfg != NULL) && (cfg->name != NULL)) {
		k_thread_name_set(_current, cfg->name);
	}

#གལ་ཏེ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	གལ་ཏེ ((cfg != NULL) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} དེ་མིན {
		queue->work_timeout = K_FOREVER;
	}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	sys_slist_init(&queue->pending);
	z_waitq_init(&queue->notifyq);
	z_waitq_init(&queue->drainq);
	queue->thread_id = _current;
	flags_set(&queue->flags, flags);
	work_queue_main(queue, NULL, NULL);
}

void k_work_queue_start(sa_nracanaa k_work_q *queue,
			k_thread_stack_t *stack,
			size_t stack_size,
			ཧྲིལ prio,
			const sa_nracanaa k_work_queue_config *cfg)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(stack);
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

	གལ་ཏེ ((cfg != NULL) && cfg->no_yield) {
		flags |= K_WORK_QUEUE_NO_YIELD;
	}

	/* It hasn't actually been started yet, but all the state is in place
	 * so we can submit things and once the thread gets control it's ready
	 * to roll.
	 */
	flags_set(&queue->flags, flags);

	(void)k_thread_create(&queue->thread, stack, stack_size,
			      work_queue_main, queue, NULL, NULL,
			      prio, 0, K_FOREVER);

	གལ་ཏེ ((cfg != NULL) && (cfg->name != NULL)) {
		k_thread_name_set(&queue->thread, cfg->name);
	}

	གལ་ཏེ ((cfg != NULL) && (cfg->essential)) {
		queue->thread.base.user_options |= K_ESSENTIAL;
	}

#གལ་ཏེ defined(CONFIG_WORKQUEUE_WORK_TIMEOUT)
	གལ་ཏེ ((cfg != NULL) && (cfg->work_timeout_ms)) {
		queue->work_timeout = K_MSEC(cfg->work_timeout_ms);
	} དེ་མིན {
		queue->work_timeout = K_FOREVER;
	}
#endif /* defined(CONFIG_WORKQUEUE_WORK_TIMEOUT) */

	k_thread_start(&queue->thread);
	queue->thread_id = &queue->thread;

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, start, queue);

	TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_DEPRECATED_DECLARATIONS);
}

ཧྲིལ k_work_queue_drain(sa_nracanaa k_work_q *queue,
		       bool plug)
{
	__ASSERT_NO_MSG(queue);
	__ASSERT_NO_MSG(!k_is_in_isr());

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, drain, queue);

	ཧྲིལ ret = 0;
	k_spinlock_key_t key = k_spin_lock(&work_lock);

	གལ་ཏེ (((flags_get(&queue->flags)
	      & (K_WORK_QUEUE_BUSY | K_WORK_QUEUE_DRAIN)) != 0U)
	    || plug
	    || !sys_slist_is_empty(&queue->pending)) {
		flag_set(&queue->flags, K_WORK_QUEUE_DRAIN_BIT);
		གལ་ཏེ (plug) {
			flag_set(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT);
		}

		notify_queue_locked(queue);
		ret = z_sched_wait(&work_lock, key, &queue->drainq,
				   K_FOREVER, NULL);
	} དེ་མིན {
		k_spin_unlock(&work_lock, key);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, drain, queue, ret);

	ཕྱིར ret;
}

ཧྲིལ k_work_queue_unplug(sa_nracanaa k_work_q *queue)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, unplug, queue);

	ཧྲིལ ret = -EALREADY;
	k_spinlock_key_t key = k_spin_lock(&work_lock);

	གལ་ཏེ (flag_test_and_clear(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		ret = 0;
	}

	k_spin_unlock(&work_lock, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, unplug, queue, ret);

	ཕྱིར ret;
}

ཧྲིལ k_work_queue_stop(sa_nracanaa k_work_q *queue, k_timeout_t timeout)
{
	__ASSERT_NO_MSG(queue);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work_queue, stop, queue, timeout);

	གལ་ཏེ (queue->thread_id) {
		གལ་ཏེ (z_is_thread_essential(queue->thread_id)) {
			ཕྱིར -ENOTSUP;
		}
	}

	k_spinlock_key_t key = k_spin_lock(&work_lock);

	གལ་ཏེ (!flag_test(&queue->flags, K_WORK_QUEUE_STARTED_BIT)) {
		k_spin_unlock(&work_lock, key);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EALREADY);
		ཕྱིར -EALREADY;
	}

	གལ་ཏེ (!flag_test(&queue->flags, K_WORK_QUEUE_PLUGGED_BIT)) {
		k_spin_unlock(&work_lock, key);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -EBUSY);
		ཕྱིར -EBUSY;
	}

	flag_set(&queue->flags, K_WORK_QUEUE_STOP_BIT);
	notify_queue_locked(queue);
	k_spin_unlock(&work_lock, key);
	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_work_queue, stop, queue, timeout);
	གལ་ཏེ (k_thread_join(queue->thread_id, timeout)) {
		key = k_spin_lock(&work_lock);
		flag_clear(&queue->flags, K_WORK_QUEUE_STOP_BIT);
		k_spin_unlock(&work_lock, key);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, -ETIMEDOUT);
		ཕྱིར -ETIMEDOUT;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work_queue, stop, queue, timeout, 0);
	ཕྱིར 0;
}

#ifdef CONFIG_SYS_CLOCK_EXISTS

/* Timeout handler for delayable work.
 *
 * Invoked by timeout infrastructure.
 * Takes and releases work lock.
 * Conditionally reschedules.
 */
static void work_timeout(sa_nracanaa _timeout *to)
{
	sa_nracanaa k_work_delayable *dw
		= CONTAINER_OF(to, sa_nracanaa k_work_delayable, timeout);
	sa_nracanaa k_work *wp = &dw->work;
	k_spinlock_key_t key = k_spin_lock(&work_lock);
	sa_nracanaa k_work_q *queue = NULL;

	/* K_WORK_DELAYED_BIT is the wake-ownership flag: whichever side
	 * (this handler or unschedule_locked()) clears it via
	 * flag_test_and_clear() first owns the outcome. If unschedule_locked
	 * got here first, the bit is clear and we bail.
	 */
	གལ་ཏེ (flag_test_and_clear(&wp->flags, K_WORK_DELAYED_BIT)) {
		queue = dw->queue;
		(void)submit_to_queue_locked(wp, &queue);
	}

	k_spin_unlock(&work_lock, key);
}

void k_work_init_delayable(sa_nracanaa k_work_delayable *dwork,
			    k_work_handler_t handler)
{
	__ASSERT_NO_MSG(dwork != NULL);
	__ASSERT_NO_MSG(handler != NULL);

	*dwork = (sa_nracanaa k_work_delayable)Z_WORK_DELAYABLE_INITIALIZER(handler);

	z_init_timeout(&dwork->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_work_delayable, dwork);
}

static anta_hstha ཧྲིལ work_delayable_busy_get_locked(const sa_nracanaa k_work_delayable *dwork)
{
	ཕྱིར flags_get(&dwork->work.flags) & K_WORK_MASK;
}

ཧྲིལ k_work_delayable_busy_get(const sa_nracanaa k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != NULL);

	k_spinlock_key_t key = k_spin_lock(&work_lock);
	ཧྲིལ ret = work_delayable_busy_get_locked(dwork);

	k_spin_unlock(&work_lock, key);
	ཕྱིར ret;
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
static ཧྲིལ schedule_for_queue_locked(sa_nracanaa k_work_q **queuep,
				     sa_nracanaa k_work_delayable *dwork,
				     k_timeout_t delay)
{
	ཧྲིལ ret = 1;
	sa_nracanaa k_work *work = &dwork->work;

	གལ་ཏེ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		ཕྱིར submit_to_queue_locked(work, queuep);
	}

	flag_set(&work->flags, K_WORK_DELAYED_BIT);
	dwork->queue = *queuep;

	/* Add timeout */
	z_add_timeout(&dwork->timeout, work_timeout, delay);

	ཕྱིར ret;
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
static anta_hstha bool unschedule_locked(sa_nracanaa k_work_delayable *dwork,
				     k_spinlock_key_t *key)
{
	bool ret = false;
	sa_nracanaa k_work *work = &dwork->work;

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
	གལ་ཏེ (flag_test_and_clear(&work->flags, K_WORK_DELAYED_BIT)) {
		སྐབས (z_try_abort_timeout(&dwork->timeout) == -EAGAIN) {
			k_spin_unlock(&work_lock, *key);
			*key = k_spin_lock(&work_lock);
		}
		ret = true;
	}

	ཕྱིར ret;
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
static ཧྲིལ cancel_delayable_async_locked(sa_nracanaa k_work_delayable *dwork,
					 k_spinlock_key_t *key)
{
	(void)unschedule_locked(dwork, key);

	ཕྱིར cancel_async_locked(&dwork->work);
}

ཧྲིལ k_work_schedule_for_queue(sa_nracanaa k_work_q *queue, sa_nracanaa k_work_delayable *dwork,
			      k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != NULL);
	__ASSERT_NO_MSG(dwork != NULL);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule_for_queue, queue, dwork, delay);

	sa_nracanaa k_work *work = &dwork->work;
	ཧྲིལ ret = 0;
	k_spinlock_key_t key = k_spin_lock(&work_lock);

	/* Schedule the work item if it's idle or running. */
	གལ་ཏེ ((work_busy_get_locked(work) & ~K_WORK_RUNNING) == 0U) {
		ret = schedule_for_queue_locked(&queue, dwork, delay);
	}

	k_spin_unlock(&work_lock, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule_for_queue, queue, dwork, delay, ret);

	ཕྱིར ret;
}

ཧྲིལ k_work_schedule(sa_nracanaa k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, schedule, dwork, delay);

	ཧྲིལ ret = k_work_schedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, schedule, dwork, delay, ret);

	ཕྱིར ret;
}

ཧྲིལ k_work_reschedule_for_queue(sa_nracanaa k_work_q *queue, sa_nracanaa k_work_delayable *dwork,
				k_timeout_t delay)
{
	__ASSERT_NO_MSG(queue != NULL);
	__ASSERT_NO_MSG(dwork != NULL);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule_for_queue, queue, dwork, delay);

	ཧྲིལ ret;
	k_spinlock_key_t key = k_spin_lock(&work_lock);

	/* Remove any active scheduling. */
	(void)unschedule_locked(dwork, &key);

	/* Schedule the work item with the new parameters. */
	ret = schedule_for_queue_locked(&queue, dwork, delay);

	k_spin_unlock(&work_lock, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule_for_queue, queue, dwork, delay, ret);

	ཕྱིར ret;
}

ཧྲིལ k_work_reschedule(sa_nracanaa k_work_delayable *dwork, k_timeout_t delay)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, reschedule, dwork, delay);

	ཧྲིལ ret = k_work_reschedule_for_queue(&k_sys_work_q, dwork, delay);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, reschedule, dwork, delay, ret);

	ཕྱིར ret;
}

ཧྲིལ k_work_cancel_delayable(sa_nracanaa k_work_delayable *dwork)
{
	__ASSERT_NO_MSG(dwork != NULL);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable, dwork);

	k_spinlock_key_t key = k_spin_lock(&work_lock);
	ཧྲིལ ret = cancel_delayable_async_locked(dwork, &key);

	k_spin_unlock(&work_lock, key);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable, dwork, ret);

	ཕྱིར ret;
}

bool k_work_cancel_delayable_sync(sa_nracanaa k_work_delayable *dwork,
				  sa_nracanaa k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != NULL);
	__ASSERT_NO_MSG(sync != NULL);
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, cancel_delayable_sync, dwork, sync);

	sa_nracanaa z_work_canceller *canceller = &sync->canceller;
	k_spinlock_key_t key = k_spin_lock(&work_lock);
	bool pending = (work_delayable_busy_get_locked(dwork) != 0U);
	bool need_wait = false;

	གལ་ཏེ (pending) {
		(void)cancel_delayable_async_locked(dwork, &key);
		need_wait = cancel_sync_locked(&dwork->work, canceller);
	}

	k_spin_unlock(&work_lock, key);

	གལ་ཏེ (need_wait) {
		k_sem_take(&canceller->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, cancel_delayable_sync, dwork, sync, pending);
	ཕྱིར pending;
}

bool k_work_flush_delayable(sa_nracanaa k_work_delayable *dwork,
			    sa_nracanaa k_work_sync *sync)
{
	__ASSERT_NO_MSG(dwork != NULL);
	__ASSERT_NO_MSG(sync != NULL);
	__ASSERT_NO_MSG(!k_is_in_isr());
#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(sync));
#endif /* CONFIG_KERNEL_COHERENCE */

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_work, flush_delayable, dwork, sync);

	sa_nracanaa k_work *work = &dwork->work;
	sa_nracanaa z_work_flusher *flusher = &sync->flusher;
	k_spinlock_key_t key = k_spin_lock(&work_lock);

	/* If it's idle release the lock and return immediately. */
	གལ་ཏེ (work_busy_get_locked(work) == 0U) {
		k_spin_unlock(&work_lock, key);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, false);

		ཕྱིར false;
	}

	/* If unscheduling did something then submit it.  Ignore a
	 * failed submission (e.g. when cancelling).
	 */
	གལ་ཏེ (unschedule_locked(dwork, &key)) {
		sa_nracanaa k_work_q *queue = dwork->queue;

		(void)submit_to_queue_locked(work, &queue);
	}

	/* Wait for it to finish */
	bool need_flush = work_flush_locked(work, flusher);

	k_spin_unlock(&work_lock, key);

	/* If necessary wait until the flusher item completes */
	གལ་ཏེ (need_flush) {
		k_sem_take(&flusher->sem, K_FOREVER);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_work, flush_delayable, dwork, sync, need_flush);

	ཕྱིར need_flush;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */
