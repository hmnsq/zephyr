/* wait queue for multiple threads on kernel objects */

/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_

#అన్తర్భావ <zephyr/kernel_structs.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
#అన్తర్భావ <zephyr/sys/rb.h>
#అన్తర్భావ <timeout_q.h>
#అన్తర్భావ <priority_q.h>
#అన్తర్భావ <kspinlock.h>

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_WAITQ_SCALABLE

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 */
#నిర్వచించు _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	RB_FOR_EACH_CONTAINER(&(wq)->waitq.వృక్షం, thread_ptr, పునాది.qnode_rb)

/*
 * RB_FOR_EACH_SAFE() does not (and cannot) exist so we are not
 * able to provide _WAIT_Q_FOR_EACH_SAFE when CONFIG_WAITQ_SCALABLE=y.
 */

స్థిర అంతఃవరుస శూన్య z_waitq_init(_wait_q_t *w)
{
	w->waitq = (నిర్మాణం _priq_rb) {
		.వృక్షం = {
			.lessthan_fn = z_priq_rb_lessthan
		}
	};
}

స్థిర అంతఃవరుస నిర్మాణం k_thread *z_waitq_తల_locked(_wait_q_t *w)
{
	తిరిగి (నిర్మాణం k_thread *)rb_get_min(&w->waitq.వృక్షం);
}

#లేకపోతే /* !CONFIG_WAITQ_SCALABLE: */

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 * Use @ref _WAIT_Q_FOR_EACH_SAFE instead if this is required.
 */
#నిర్వచించు _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER(&((wq)->waitq), thread_ptr, \
				     పునాది.qnode_dlist)

/**
 * Walk all threads pending on a wait queue.
 *
 * @note The iterator thread may be safely detached from the wait
 * queue as part of the loop when using this macro.
 *
 * @warning `CONFIG_WAITQ_SCALABLE=n` is required to use this macro.
 * Use it sparingly and make sure to have a fallback implementation
 * which works when `CONFIG_WAITQ_SCALABLE=y`.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 * @param loop_ptr Thread pointer variable used by loop
 */
#నిర్వచించు WAIT_Q_FOR_EACH_సురక్షిత(wq, thread_ptr, loop_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER_సురక్షిత(&((wq)->waitq), thread_ptr, \
					  loop_ptr, పునాది.qnode_dlist)

స్థిర అంతఃవరుస శూన్య z_waitq_init(_wait_q_t *w)
{
	sys_dlist_init(&w->waitq);
}

స్థిర అంతఃవరుస నిర్మాణం k_thread *z_waitq_తల_locked(_wait_q_t *w)
{
	తిరిగి (నిర్మాణం k_thread *)sys_dlist_peek_తల(&w->waitq);
}

#అయితే_ముగింపు /* !CONFIG_WAITQ_SCALABLE */

స్థిర అంతఃవరుస నిర్మాణం k_thread *z_waitq_తల(_wait_q_t *w)
{
	నిర్మాణం k_thread *thread = ఖాళీ;

	/*
	 * On a single core system, all callers of this function are already
	 * protected by a spinlock held by the caller. On those systems there
	 * is no need to lock the scheduler's spinlock. However, on SMP systems
	 * the caller controlled spinlock is insufficient as the thread timeout
	 * handler may be running on another CPU. Use LOCK_SCHED_SPINLOCK
	 * to protect the scope as necessary.
	 */

	తాళం_SCHED_SPINLOCK {
		thread = z_waitq_తల_locked(w);
	}

	తిరిగి thread;
}


#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_ */
