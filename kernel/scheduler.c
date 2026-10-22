/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kspinlock.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/sys/__assert.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <run_q.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <scheduler.h>


शून्य z_sched_init(शून्य)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	प्रति (पूर्णाङ्क i = 0; i < CONFIG_MP_MAX_अङ्कः_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#अण्णहा
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

शून्य k_sched_lock(शून्य)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

शून्य k_sched_unlock(शून्य)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_कुञ्चिका_t कुञ्चिका = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(कुञ्चिका);
}

शून्य z_impl_k_reschedule(शून्य)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	कुञ्चिका = z_sched_spinlock_lock();

	z_sched_lock_reschedule(कुञ्चिका);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_reschedule(शून्य)
{
	z_impl_k_reschedule();
}
#अन्तर्भाव <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(शून्य)
{
	प्रत्यागम _current;
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ k_tid_t z_vrfy_k_sched_current_thread_query(शून्य)
{
	प्रत्यागम z_impl_k_sched_current_thread_query();
}
#अन्तर्भाव <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


पूर्णाङ्क z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, शून्य *data)
{
	संरचना k_thread *thread;
	पूर्णाङ्क  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		संरचना k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#अण्णहा /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			जइ (status != 0) {
				भङ्ग;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		जइ (post_func != रिक्त) {
			post_func(status, data);
		}
	}

	प्रत्यागम status;
}


/*
 * future scheduler.h API implementations
 */

पूर्णाङ्क z_sched_wait(संरचना k_spinlock *lock, k_spinlock_कुञ्चिका_t कुञ्चिका,
		 _wait_q_t *wait_q, k_timeout_t timeout, शून्य **data)
{
	पूर्णाङ्क ret = z_pend_curr(lock, कुञ्चिका, wait_q, timeout);

	जइ (data != रिक्त) {
		*data = _current->base.swap_data;
	}
	प्रत्यागम ret;
}
