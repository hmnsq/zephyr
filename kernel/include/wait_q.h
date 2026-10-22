/* wait queue for multiple threads on kernel objects */

/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_

#अन्तर्भाव <zephyr/kernel_structs.h>
#अन्तर्भाव <zephyr/sys/dlist.h>
#अन्तर्भाव <zephyr/sys/rb.h>
#अन्तर्भाव <timeout_q.h>
#अन्तर्भाव <priority_q.h>
#अन्तर्भाव <kspinlock.h>

#ifdef __cplusplus
बाह्य "C" {
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
	RB_FOR_EACH_CONTAINER(&(wq)->waitq.वृक्ष, thread_ptr, base.qnode_rb)

/*
 * RB_FOR_EACH_SAFE() does not (and cannot) exist so we are not
 * able to provide _WAIT_Q_FOR_EACH_SAFE when CONFIG_WAITQ_SCALABLE=y.
 */

स्थिर ओळीत शून्य z_waitq_init(_wait_q_t *w)
{
	w->waitq = (संरचना _priq_rb) {
		.वृक्ष = {
			.lessthan_fn = z_priq_rb_lessthan
		}
	};
}

स्थिर ओळीत संरचना k_thread *z_waitq_head_locked(_wait_q_t *w)
{
	परत (संरचना k_thread *)rb_get_min(&w->waitq.वृक्ष);
}

#अन्यथा /* !CONFIG_WAITQ_SCALABLE: */

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

स्थिर ओळीत शून्य z_waitq_init(_wait_q_t *w)
{
	sys_dlist_init(&w->waitq);
}

स्थिर ओळीत संरचना k_thread *z_waitq_head_locked(_wait_q_t *w)
{
	परत (संरचना k_thread *)sys_dlist_peek_head(&w->waitq);
}

#endif /* !CONFIG_WAITQ_SCALABLE */

स्थिर ओळीत संरचना k_thread *z_waitq_head(_wait_q_t *w)
{
	संरचना k_thread *thread = रिक्त;

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

	परत thread;
}


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_ */
