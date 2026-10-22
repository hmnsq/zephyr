/* wait queue for multiple threads on kernel objects */

/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_
#परिभाषय ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_

#अन्तर्भाव <zephyr/kernel_structs.h>
#अन्तर्भाव <zephyr/sys/dlist.h>
#अन्तर्भाव <zephyr/sys/rb.h>
#अन्तर्भाव <timeout_q.h>
#अन्तर्भाव <priority_q.h>
#अन्तर्भाव <kspinlock.h>

#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_WAITQ_SCALABLE

/**
 * Walk all threads pending on a wait queue.
 *
 * @param wq Wait queue
 * @param thread_ptr Thread pointer iterator variable
 *
 * @warning Do not remove thread from wait queue when using this macro.
 */
#परिभाषय _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	RB_FOR_EACH_CONTAINER(&(wq)->waitq.वृक्षः, thread_ptr, आधारः.qnode_rb)

/*
 * RB_FOR_EACH_SAFE() does not (and cannot) exist so we are not
 * able to provide _WAIT_Q_FOR_EACH_SAFE when CONFIG_WAITQ_SCALABLE=y.
 */

स्थिर अन्तःस्थ शून्य z_waitq_init(_wait_q_t *w)
{
	w->waitq = (संरचना _priq_rb) {
		.वृक्षः = {
			.lessthan_fn = z_priq_rb_lessthan
		}
	};
}

स्थिर अन्तःस्थ संरचना k_thread *z_waitq_शीर्ष_locked(_wait_q_t *w)
{
	प्रत्यागम (संरचना k_thread *)rb_get_min(&w->waitq.वृक्षः);
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
#परिभाषय _WAIT_Q_FOR_EACH(wq, thread_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER(&((wq)->waitq), thread_ptr, \
				     आधारः.qnode_dlist)

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
#परिभाषय _WAIT_Q_FOR_EACH_SAFE(wq, thread_ptr, loop_ptr) \
	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&((wq)->waitq), thread_ptr, \
					  loop_ptr, आधारः.qnode_dlist)

स्थिर अन्तःस्थ शून्य z_waitq_init(_wait_q_t *w)
{
	sys_dlist_init(&w->waitq);
}

स्थिर अन्तःस्थ संरचना k_thread *z_waitq_शीर्ष_locked(_wait_q_t *w)
{
	प्रत्यागम (संरचना k_thread *)sys_dlist_peek_शीर्ष(&w->waitq);
}

#यदि_अन्तः /* !CONFIG_WAITQ_SCALABLE */

स्थिर अन्तःस्थ संरचना k_thread *z_waitq_शीर्ष(_wait_q_t *w)
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

	तालकम्_SCHED_SPINLOCK {
		thread = z_waitq_शीर्ष_locked(w);
	}

	प्रत्यागम thread;
}


#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः

#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_WAIT_Q_H_ */
