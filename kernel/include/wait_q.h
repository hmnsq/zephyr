/* wait queue for multiple threads on kernel objects */

/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel_structs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/rb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeout_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <priority_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	RB_FOR_EACH_CONTAINER(&(wq)->waitq.ਰੁੱਖ, thread_ptr, ਅਧਾਰ.qnode_rb)

/*
 * RB_FOR_EACH_SAFE() does not (and cannot) exist so we are not
 * able to provide _WAIT_Q_FOR_EACH_SAFE when CONFIG_WAITQ_SCALABLE=y.
 */

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_waitq_init(_wait_q_t *w)
{
	w->waitq = (ਬਣਤਰ _priq_rb) {
		.ਰੁੱਖ = {
			.lessthan_fn = z_priq_rb_lessthan
		}
	};
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ k_thread *z_waitq_ਸਿਰ_locked(_wait_q_t *w)
{
	ਵਾਪਸ (ਬਣਤਰ k_thread *)rb_get_min(&w->waitq.ਰੁੱਖ);
}

#ਨਹੀਂਤਾਂ /* !CONFIG_WAITQ_SCALABLE: */

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 * Use @ref _WAIT_Q_FOR_EACH_SAFE instead if this is required.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER(&((wq)->waitq), thread_ptr, \
				     ਅਧਾਰ.qnode_dlist)

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WAIT_Q_FOR_EACH_ਸੁਰੱਖਿਅਤ(wq, thread_ptr, loop_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER_ਸੁਰੱਖਿਅਤ(&((wq)->waitq), thread_ptr, \
					  loop_ptr, ਅਧਾਰ.qnode_dlist)

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_waitq_init(_wait_q_t *w)
{
	sys_dlist_init(&w->waitq);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ k_thread *z_waitq_ਸਿਰ_locked(_wait_q_t *w)
{
	ਵਾਪਸ (ਬਣਤਰ k_thread *)sys_dlist_peek_ਸਿਰ(&w->waitq);
}

#ਜੇ_ਅੰਤ /* !CONFIG_WAITQ_SCALABLE */

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ k_thread *z_waitq_ਸਿਰ(_wait_q_t *w)
{
	ਬਣਤਰ k_thread *thread = ਨਿਰਾ;

	/*
	 * On a single core system, all callers of this function are already
	 * protected by a spinlock held by the caller. On those systems there
	 * is no need to lock the scheduler's spinlock. However, on SMP systems
	 * the caller controlled spinlock is insufficient as the thread timeout
	 * handler may be running on another CPU. Use LOCK_SCHED_SPINLOCK
	 * to protect the scope as necessary.
	 */

	ਤਾਲਾ_SCHED_SPINLOCK {
		thread = z_waitq_ਸਿਰ_locked(w);
	}

	ਵਾਪਸ thread;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_ */
