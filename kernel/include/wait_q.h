/* wait queue for multiple threads on kernel objects */

/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_

#અન્તર્ભાવ <zephyr/kernel_structs.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>
#અન્તર્ભાવ <zephyr/sys/rb.h>
#અન્તર્ભાવ <timeout_q.h>
#અન્તર્ભાવ <priority_q.h>
#અન્તર્ભાવ <kspinlock.h>

#ifdef __cplusplus
બાહ્ય "C" {
#endif

#ifdef CONFIG_WAITQ_SCALABLE

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 */
#define _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	RB_FOR_EACH_CONTAINER(&(wq)->waitq.વૃક્ષ, thread_ptr, base.qnode_rb)

/*
 * RB_FOR_EACH_SAFE() does not (and cannot) exist so we are not
 * able to provide _WAIT_Q_FOR_EACH_SAFE when CONFIG_WAITQ_SCALABLE=y.
 */

સ્થિર અંતઃરેખ શૂન્ય z_waitq_init(_wait_q_t *w)
{
	w->waitq = (સંરચના _priq_rb) {
		.વૃક્ષ = {
			.lessthan_fn = z_priq_rb_lessthan
		}
	};
}

સ્થિર અંતઃરેખ સંરચના k_thread *z_waitq_head_locked(_wait_q_t *w)
{
	પાછા (સંરચના k_thread *)rb_get_min(&w->waitq.વૃક્ષ);
}

#નહીંતર /* !CONFIG_WAITQ_SCALABLE: */

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 * Use @ref _WAIT_Q_FOR_EACH_SAFE instead if this is required.
 */
#define _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER(&((wq)->waitq), thread_ptr, \
				     base.qnode_dlist)

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
#define _WAIT_Q_FOR_EACH_SAFE(wq, thread_ptr, loop_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&((wq)->waitq), thread_ptr, \
					  loop_ptr, base.qnode_dlist)

સ્થિર અંતઃરેખ શૂન્ય z_waitq_init(_wait_q_t *w)
{
	sys_dlist_init(&w->waitq);
}

સ્થિર અંતઃરેખ સંરચના k_thread *z_waitq_head_locked(_wait_q_t *w)
{
	પાછા (સંરચના k_thread *)sys_dlist_peek_head(&w->waitq);
}

#endif /* !CONFIG_WAITQ_SCALABLE */

સ્થિર અંતઃરેખ સંરચના k_thread *z_waitq_head(_wait_q_t *w)
{
	સંરચના k_thread *thread = ખાલી;

	/*
	 * On a single core system, all callers of this function are already
	 * protected by a spinlock held by the caller. On those systems there
	 * is no need to lock the scheduler's spinlock. However, on SMP systems
	 * the caller controlled spinlock is insufficient as the thread timeout
	 * handler may be running on another CPU. Use LOCK_SCHED_SPINLOCK
	 * to protect the scope as necessary.
	 */

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(w);
	}

	પાછા thread;
}


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_ */
