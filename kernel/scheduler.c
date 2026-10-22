/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kspinlock.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/__assert.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <run_q.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>


ଶୂନ୍ୟ z_sched_init(ଶୂନ୍ୟ)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < CONFIG_MP_MAX_ଅଙ୍କ_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#ଅନ୍ୟଥା
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ଶୂନ୍ୟ k_sched_lock(ଶୂନ୍ୟ)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

ଶୂନ୍ୟ k_sched_unlock(ଶୂନ୍ୟ)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_ଚାବି_t ଚାବି = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(ଚାବି);
}

ଶୂନ୍ୟ z_impl_k_reschedule(ଶୂନ୍ୟ)
{
	k_spinlock_ଚାବି_t ଚାବି;

	ଚାବି = z_sched_spinlock_lock();

	z_sched_lock_reschedule(ଚାବି);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_reschedule(ଶୂନ୍ୟ)
{
	z_impl_k_reschedule();
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ _current;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_tid_t z_vrfy_k_sched_current_thread_query(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ z_impl_k_sched_current_thread_query();
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, ଶୂନ୍ୟ *data)
{
	ସଂରଚନା k_thread *thread;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		ସଂରଚନା k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#ଅନ୍ୟଥା /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			ଯଦି (status != 0) {
				ଭାଙ୍ଗ;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		ଯଦି (post_func != ଖାଲି) {
			post_func(status, data);
		}
	}

	ଫେରନ୍ତୁ status;
}


/*
 * future scheduler.h API implementations
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_sched_wait(ସଂରଚନା k_spinlock *lock, k_spinlock_ଚାବି_t ଚାବି,
		 _wait_q_t *wait_q, k_timeout_t timeout, ଶୂନ୍ୟ **data)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = z_pend_curr(lock, ଚାବି, wait_q, timeout);

	ଯଦି (data != ଖାଲି) {
		*data = _current->base.swap_data;
	}
	ଫେରନ୍ତୁ ret;
}
