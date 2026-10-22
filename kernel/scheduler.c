/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <kspinlock.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <zephyr/sys/__assert.h>
#অন্তর্ভাব <zephyr/spinlock.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <run_q.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <scheduler.h>


শূন্য z_sched_init(শূন্য)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	প্রতি (পূর্ণাঙ্ক i = 0; i < CONFIG_MP_MAX_অঙ্ক_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#অন্যথা
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

শূন্য k_sched_lock(শূন্য)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

শূন্য k_sched_unlock(শূন্য)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_চাবি_t চাবি = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(চাবি);
}

শূন্য z_impl_k_reschedule(শূন্য)
{
	k_spinlock_চাবি_t চাবি;

	চাবি = z_sched_spinlock_lock();

	z_sched_lock_reschedule(চাবি);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_reschedule(শূন্য)
{
	z_impl_k_reschedule();
}
#অন্তর্ভাব <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(শূন্য)
{
	প্রত্যাগম _current;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ k_tid_t z_vrfy_k_sched_current_thread_query(শূন্য)
{
	প্রত্যাগম z_impl_k_sched_current_thread_query();
}
#অন্তর্ভাব <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


পূর্ণাঙ্ক z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, শূন্য *data)
{
	সংরচনা k_thread *thread;
	পূর্ণাঙ্ক  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		সংরচনা k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#অন্যথা /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			যদি (status != 0) {
				ভঙ্গ;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		যদি (post_func != রিক্ত) {
			post_func(status, data);
		}
	}

	প্রত্যাগম status;
}


/*
 * future scheduler.h API implementations
 */

পূর্ণাঙ্ক z_sched_wait(সংরচনা k_spinlock *lock, k_spinlock_চাবি_t চাবি,
		 _wait_q_t *wait_q, k_timeout_t timeout, শূন্য **data)
{
	পূর্ণাঙ্ক ret = z_pend_curr(lock, চাবি, wait_q, timeout);

	যদি (data != রিক্ত) {
		*data = _current->base.swap_data;
	}
	প্রত্যাগম ret;
}
