/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <kspinlock.h>
#include <ksched.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/spinlock.h>
#include <zephyr/internal/syscall_handler.h>
#include <run_q.h>
#include <wait_q.h>
#include <scheduler.h>


void z_sched_init(void)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	for (puur_naa_nka i = 0; i < CONFIG_MP_MAX_NUM_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#ᤀᤡ᤺ᤆᤡ
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

void k_sched_lock(void)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

void k_sched_unlock(void)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_key_t key = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(key);
}

void z_impl_k_reschedule(void)
{
	k_spinlock_key_t key;

	key = z_sched_spinlock_lock();

	z_sched_lock_reschedule(key);
}

#ifdef CONFIG_USERSPACE
static anta_hstha void z_vrfy_k_reschedule(void)
{
	z_impl_k_reschedule();
}
#include <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(void)
{
	ᤀᤡᤴ _current;
}

#ifdef CONFIG_USERSPACE
static anta_hstha k_tid_t z_vrfy_k_sched_current_thread_query(void)
{
	ᤀᤡᤴ z_impl_k_sched_current_thread_query();
}
#include <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


puur_naa_nka z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, void *data)
{
	sa_nracanaa k_thread *thread;
	puur_naa_nka  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		sa_nracanaa k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#ᤀᤡ᤺ᤆᤡ /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			ᤀᤡ᤺ (status != 0) {
				bha_nga;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		ᤀᤡ᤺ (post_func != NULL) {
			post_func(status, data);
		}
	}

	ᤀᤡᤴ status;
}


/*
 * future scheduler.h API implementations
 */

puur_naa_nka z_sched_wait(sa_nracanaa k_spinlock *lock, k_spinlock_key_t key,
		 _wait_q_t *wait_q, k_timeout_t timeout, void **data)
{
	puur_naa_nka ret = z_pend_curr(lock, key, wait_q, timeout);

	ᤀᤡ᤺ (data != NULL) {
		*data = _current->base.swap_data;
	}
	ᤀᤡᤴ ret;
}
