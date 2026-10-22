/* wait queue for multiple threads on kernel objects */

/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جے_بنا_تعریف ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_

#ضمن <zephyr/kernel_structs.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <zephyr/sys/rb.h>
#ضمن <timeout_q.h>
#ضمن <priority_q.h>
#ضمن <kspinlock.h>

#جے_تعریف_شدہ __cplusplus
خارجي "C" {
#جے_انت

#جے_تعریف_شدہ CONFIG_WAITQ_SCALABLE

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 */
#تعریف_کرو _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	RB_FOR_EACH_CONTAINER(&(wq)->waitq.درخت, thread_ptr, ادھار.qnode_rb)

/*
 * RB_FOR_EACH_SAFE() does not (and cannot) exist so we are not
 * able to provide _WAIT_Q_FOR_EACH_SAFE when CONFIG_WAITQ_SCALABLE=y.
 */

ساكن مضمن فراغ z_waitq_init(_wait_q_t *w)
{
	w->waitq = (بنية _priq_rb) {
		.درخت = {
			.lessthan_fn = z_priq_rb_lessthan
		}
	};
}

ساكن مضمن بنية k_thread *z_waitq_سر_locked(_wait_q_t *w)
{
	أرجع (بنية k_thread *)rb_get_min(&w->waitq.درخت);
}

#وإلا /* !CONFIG_WAITQ_SCALABLE: */

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 * Use @ref _WAIT_Q_FOR_EACH_SAFE instead if this is required.
 */
#تعریف_کرو _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER(&((wq)->waitq), thread_ptr, \
				     ادھار.qnode_dlist)

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
#تعریف_کرو WAIT_Q_FOR_EACH_محفوظ(wq, thread_ptr, loop_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER_محفوظ(&((wq)->waitq), thread_ptr, \
					  loop_ptr, ادھار.qnode_dlist)

ساكن مضمن فراغ z_waitq_init(_wait_q_t *w)
{
	sys_dlist_init(&w->waitq);
}

ساكن مضمن بنية k_thread *z_waitq_سر_locked(_wait_q_t *w)
{
	أرجع (بنية k_thread *)sys_dlist_peek_سر(&w->waitq);
}

#جے_انت /* !CONFIG_WAITQ_SCALABLE */

ساكن مضمن بنية k_thread *z_waitq_سر(_wait_q_t *w)
{
	بنية k_thread *thread = عدم;

	/*
	 * On a single core system, all callers of this function are already
	 * protected by a spinlock held by the caller. On those systems there
	 * is no need to lock the scheduler's spinlock. However, on SMP systems
	 * the caller controlled spinlock is insufficient as the thread timeout
	 * handler may be running on another CPU. Use LOCK_SCHED_SPINLOCK
	 * to protect the scope as necessary.
	 */

	جندرا_SCHED_SPINLOCK {
		thread = z_waitq_سر_locked(w);
	}

	أرجع thread;
}


#جے_تعریف_شدہ __cplusplus
}
#جے_انت

#جے_انت /* ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_ */
