/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/__assert.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <run_q.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>


ಶೂನ್ಯ z_sched_init(ಶೂನ್ಯ)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < CONFIG_MP_MAX_ಅಂಕೆ_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#ಅನ್ಯಥಾ
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ಶೂನ್ಯ k_sched_lock(ಶೂನ್ಯ)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

ಶೂನ್ಯ k_sched_unlock(ಶೂನ್ಯ)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(ಕೀಲಿ);
}

ಶೂನ್ಯ z_impl_k_reschedule(ಶೂನ್ಯ)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = z_sched_spinlock_lock();

	z_sched_lock_reschedule(ಕೀಲಿ);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_reschedule(ಶೂನ್ಯ)
{
	z_impl_k_reschedule();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ _current;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_tid_t z_vrfy_k_sched_current_thread_query(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_sched_current_thread_query();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


ಪೂರ್ಣಾಙ್ಕ z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, ಶೂನ್ಯ *data)
{
	ಸಂರಚನಾ k_thread *thread;
	ಪೂರ್ಣಾಙ್ಕ  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		ಸಂರಚನಾ k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#ಅನ್ಯಥಾ /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			ಯದಿ (status != 0) {
				ಭಙ್ಗ;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		ಯದಿ (post_func != ರಿಕ್ತ) {
			post_func(status, data);
		}
	}

	ಪ್ರತ್ಯಾಗಮ status;
}


/*
 * future scheduler.h API implementations
 */

ಪೂರ್ಣಾಙ್ಕ z_sched_wait(ಸಂರಚನಾ k_spinlock *lock, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ,
		 _wait_q_t *wait_q, k_timeout_t timeout, ಶೂನ್ಯ **data)
{
	ಪೂರ್ಣಾಙ್ಕ ret = z_pend_curr(lock, ಕೀಲಿ, wait_q, timeout);

	ಯದಿ (data != ರಿಕ್ತ) {
		*data = _current->base.swap_data;
	}
	ಪ್ರತ್ಯಾಗಮ ret;
}
