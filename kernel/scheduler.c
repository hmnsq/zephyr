/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kspinlock.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/sys/__assert.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <run_q.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <scheduler.h>


શૂન્ય z_sched_init(શૂન્ય)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	માટે (પૂર્ણાંક i = 0; i < CONFIG_MP_MAX_અંક_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#નહીંતર
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

શૂન્ય k_sched_lock(શૂન્ય)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

શૂન્ય k_sched_unlock(શૂન્ય)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(ચાવી);
}

શૂન્ય z_impl_k_reschedule(શૂન્ય)
{
	k_spinlock_ચાવી_t ચાવી;

	ચાવી = z_sched_spinlock_lock();

	z_sched_lock_reschedule(ચાવી);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_reschedule(શૂન્ય)
{
	z_impl_k_reschedule();
}
#અન્તર્ભાવ <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(શૂન્ય)
{
	પાછા _current;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ k_tid_t z_vrfy_k_sched_current_thread_query(શૂન્ય)
{
	પાછા z_impl_k_sched_current_thread_query();
}
#અન્તર્ભાવ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


પૂર્ણાંક z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, શૂન્ય *data)
{
	સંરચના k_thread *thread;
	પૂર્ણાંક  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		સંરચના k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#નહીંતર /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			જો (status != 0) {
				તોડો;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		જો (post_func != ખાલી) {
			post_func(status, data);
		}
	}

	પાછા status;
}


/*
 * future scheduler.h API implementations
 */

પૂર્ણાંક z_sched_wait(સંરચના k_spinlock *lock, k_spinlock_ચાવી_t ચાવી,
		 _wait_q_t *wait_q, k_timeout_t timeout, શૂન્ય **data)
{
	પૂર્ણાંક ret = z_pend_curr(lock, ચાવી, wait_q, timeout);

	જો (data != ખાલી) {
		*data = _current->base.swap_data;
	}
	પાછા ret;
}
