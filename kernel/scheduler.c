/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/kernel.h>
#شامل <kspinlock.h>
#شامل <ksched.h>
#شامل <zephyr/sys/__assert.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <run_q.h>
#شامل <wait_q.h>
#شامل <scheduler.h>


تهی z_sched_init(تهی)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	برای (صحیح i = 0; i < CONFIG_MP_MAX_رقم_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#وگرنه
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

تهی k_sched_lock(تهی)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

تهی k_sched_unlock(تهی)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_کلید_t کلید = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(کلید);
}

تهی z_impl_k_reschedule(تهی)
{
	k_spinlock_کلید_t کلید;

	کلید = z_sched_spinlock_lock();

	z_sched_lock_reschedule(کلید);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_reschedule(تهی)
{
	z_impl_k_reschedule();
}
#شامل <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(تهی)
{
	بازگشت _current;
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی k_tid_t z_vrfy_k_sched_current_thread_query(تهی)
{
	بازگشت z_impl_k_sched_current_thread_query();
}
#شامل <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


صحیح z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, تهی *data)
{
	ساختار k_thread *thread;
	صحیح  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		ساختار k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#وگرنه /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			اگر (status != 0) {
				بشکن;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		اگر (post_func != پوچ) {
			post_func(status, data);
		}
	}

	بازگشت status;
}


/*
 * future scheduler.h API implementations
 */

صحیح z_sched_wait(ساختار k_spinlock *lock, k_spinlock_کلید_t کلید,
		 _wait_q_t *wait_q, k_timeout_t timeout, تهی **data)
{
	صحیح ret = z_pend_curr(lock, کلید, wait_q, timeout);

	اگر (data != پوچ) {
		*data = _current->base.swap_data;
	}
	بازگشت ret;
}
