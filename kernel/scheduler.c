/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#समावेश <zephyr/kernel.h>
#समावेश <kspinlock.h>
#समावेश <ksched.h>
#समावेश <zephyr/sys/__assert.h>
#समावेश <zephyr/spinlock.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <run_q.h>
#समावेश <wait_q.h>
#समावेश <scheduler.h>


व्योम z_sched_init(व्योम)
{
#अगर_घोषित CONFIG_SCHED_CPU_MASK_PIN_ONLY
	क्रम (पूर्णांक i = 0; i < CONFIG_MP_MAX_अंक_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#अन्यथा
	_priq_run_init(&_kernel.ready_q.runq);
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

व्योम k_sched_lock(व्योम)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

व्योम k_sched_unlock(व्योम)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_कुंजी_t कुंजी = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(कुंजी);
}

व्योम z_impl_k_reschedule(व्योम)
{
	k_spinlock_कुंजी_t कुंजी;

	कुंजी = z_sched_spinlock_lock();

	z_sched_lock_reschedule(कुंजी);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_reschedule(व्योम)
{
	z_impl_k_reschedule();
}
#समावेश <zephyr/syscalls/k_reschedule_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(व्योम)
{
	वापस _current;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत k_tid_t z_vrfy_k_sched_current_thread_query(व्योम)
{
	वापस z_impl_k_sched_current_thread_query();
}
#समावेश <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */


पूर्णांक z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, व्योम *तथ्य)
{
	काष्ठा k_thread *thread;
	पूर्णांक  status = 0;

	Z_SCHED_SPINLOCK {
#अगर_अघोषित CONFIG_WAITQ_SCALABLE
		काष्ठा k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#अन्यथा /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#पूर्ण_अगर /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, तथ्य);
			अगर (status != 0) {
				अवरोध;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		अगर (post_func != शून्य) {
			post_func(status, तथ्य);
		}
	}

	वापस status;
}


/*
 * future scheduler.h API implementations
 */

पूर्णांक z_sched_wait(काष्ठा k_spinlock *lock, k_spinlock_कुंजी_t कुंजी,
		 _wait_q_t *wait_q, k_timeout_t timeout, व्योम **तथ्य)
{
	पूर्णांक ret = z_pend_curr(lock, कुंजी, wait_q, timeout);

	अगर (तथ्य != शून्य) {
		*तथ्य = _current->base.swap_तथ्य;
	}
	वापस ret;
}
