/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/__assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <run_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>


ਖਾਲੀ z_sched_init(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY
	ਲਈ (ਪੂਰਨ i = 0; i < CONFIG_MP_MAX_ਅੰਕ_CPUS; i++) {
		priq_ਲੜੀਵਾਰ_init(&_kernel.cpus[i].ready_q.runq);
	}
#ਨਹੀਂਤਾਂ
	priq_ਲੜੀਵਾਰ_init(&_kernel.ready_q.runq);
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ਖਾਲੀ k_sched_ਤਾਲਾ(ਖਾਲੀ)
{
	Z_SCHED_SPINLOCK {
		SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, sched_ਤਾਲਾ);

		__ASSERT(!arch_is_ਵਿੱਚ_isr(), "");
		__ASSERT(_current->ਅਧਾਰ.sched_locked != 1U, "");

		--_current->ਅਧਾਰ.sched_locked;

		compiler_barrier();
	}
}

ਖਾਲੀ k_sched_unlock(ਖਾਲੀ)
{
	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	__ASSERT(_current->ਅਧਾਰ.sched_locked != 0U, "");
	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "");
	++_current->ਅਧਾਰ.sched_locked;
	z_sched_ਤਾਲਾ_reschedule(ਕੁੰਜੀ);
}

ਖਾਲੀ z_impl_k_reschedule(ਖਾਲੀ)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	z_sched_ਤਾਲਾ_reschedule(ਕੁੰਜੀ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_reschedule(ਖਾਲੀ)
{
	z_impl_k_reschedule();
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_reschedule_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(ਖਾਲੀ)
{
	ਵਾਪਸ _current;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ k_tid_t z_vrfy_k_sched_current_thread_query(ਖਾਲੀ)
{
	ਵਾਪਸ z_impl_k_sched_current_thread_query();
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */


ਪੂਰਨ z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਬਣਤਰ k_thread *thread;
	ਪੂਰਨ  ਸਥਿਤੀ = 0;

	Z_SCHED_SPINLOCK {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
		ਬਣਤਰ k_thread *tmp;

		WAIT_Q_FOR_EACH_ਸੁਰੱਖਿਅਤ(wait_q, thread, tmp)
#ਨਹੀਂਤਾਂ /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#ਜੇ_ਅੰਤ /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			ਸਥਿਤੀ = walk_func(thread, ਅੰਕੜੇ);
			ਜੇ (ਸਥਿਤੀ != 0) {
				ਰੋਕੋ;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		ਜੇ (post_func != ਨਿਰਾ) {
			post_func(ਸਥਿਤੀ, ਅੰਕੜੇ);
		}
	}

	ਵਾਪਸ ਸਥਿਤੀ;
}


/*
 * future scheduler.h API implementations
 */

ਪੂਰਨ z_sched_wait(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ,
		 _wait_q_t *wait_q, k_timeout_t timeout, ਖਾਲੀ **ਅੰਕੜੇ)
{
	ਪੂਰਨ ret = z_pend_curr(ਤਾਲਾ, ਕੁੰਜੀ, wait_q, timeout);

	ਜੇ (ਅੰਕੜੇ != ਨਿਰਾ) {
		*ਅੰਕੜੇ = _current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;
	}
	ਵਾਪਸ ret;
}
