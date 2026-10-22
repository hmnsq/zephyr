/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kspinlock.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/sys/__assert.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <run_q.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <scheduler.h>


ശൂന്യം z_sched_init(ശൂന്യം)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < CONFIG_MP_MAX_അക്കം_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#അല്ലെങ്കിൽ
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ശൂന്യം k_sched_lock(ശൂന്യം)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

ശൂന്യം k_sched_unlock(ശൂന്യം)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(താക്കോൽ);
}

ശൂന്യം z_impl_k_reschedule(ശൂന്യം)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	താക്കോൽ = z_sched_spinlock_lock();

	z_sched_lock_reschedule(താക്കോൽ);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_reschedule(ശൂന്യം)
{
	z_impl_k_reschedule();
}
#അന്തര്ഭാവ <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(ശൂന്യം)
{
	തിരികെ _current;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ k_tid_t z_vrfy_k_sched_current_thread_query(ശൂന്യം)
{
	തിരികെ z_impl_k_sched_current_thread_query();
}
#അന്തര്ഭാവ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


പൂർണ്ണസംഖ്യ z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, ശൂന്യം *data)
{
	ഘടന k_thread *thread;
	പൂർണ്ണസംഖ്യ  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		ഘടന k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#അല്ലെങ്കിൽ /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			എങ്കിൽ (status != 0) {
				നിർത്തുക;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		എങ്കിൽ (post_func != ശൂന്യ) {
			post_func(status, data);
		}
	}

	തിരികെ status;
}


/*
 * future scheduler.h API implementations
 */

പൂർണ്ണസംഖ്യ z_sched_wait(ഘടന k_spinlock *lock, k_spinlock_താക്കോൽ_t താക്കോൽ,
		 _wait_q_t *wait_q, k_timeout_t timeout, ശൂന്യം **data)
{
	പൂർണ്ണസംഖ്യ ret = z_pend_curr(lock, താക്കോൽ, wait_q, timeout);

	എങ്കിൽ (data != ശൂന്യ) {
		*data = _current->base.swap_data;
	}
	തിരികെ ret;
}
