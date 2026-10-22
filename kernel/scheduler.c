/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kspinlock.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <zephyr/sys/__assert.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <run_q.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <scheduler.h>


வெறுமை z_sched_init(வெறுமை)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	க்கு (முழு i = 0; i < CONFIG_MP_MAX_இலக்கம்_CPUS; i++) {
		_priq_run_init(&_kernel.cpus[i].ready_q.runq);
	}
#இல்லையெனில்
	_priq_run_init(&_kernel.ready_q.runq);
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

வெறுமை k_sched_lock(வெறுமை)
{
	Z_SCHED_SPINLOCK {
		SYS_PORT_TRACING_FUNC(k_thread, sched_lock);

		__ASSERT(!arch_is_in_isr(), "");
		__ASSERT(_current->base.sched_locked != 1U, "");

		--_current->base.sched_locked;

		compiler_barrier();
	}
}

வெறுமை k_sched_unlock(வெறுமை)
{
	SYS_PORT_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();

	__ASSERT(_current->base.sched_locked != 0U, "");
	__ASSERT(!arch_is_in_isr(), "");
	++_current->base.sched_locked;
	z_sched_lock_reschedule(சாவி);
}

வெறுமை z_impl_k_reschedule(வெறுமை)
{
	k_spinlock_சாவி_t சாவி;

	சாவி = z_sched_spinlock_lock();

	z_sched_lock_reschedule(சாவி);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_reschedule(வெறுமை)
{
	z_impl_k_reschedule();
}
#அந்தர்பாவ <zephyr/syscalls/k_reschedule_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(வெறுமை)
{
	திரும்பு _current;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி k_tid_t z_vrfy_k_sched_current_thread_query(வெறுமை)
{
	திரும்பு z_impl_k_sched_current_thread_query();
}
#அந்தர்பாவ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#endif /* CONFIG_USERSPACE */


முழு z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, வெறுமை *data)
{
	கட்டமைப்பு k_thread *thread;
	முழு  status = 0;

	Z_SCHED_SPINLOCK {
#ifndef CONFIG_WAITQ_SCALABLE
		கட்டமைப்பு k_thread *tmp;

		_WAIT_Q_FOR_EACH_SAFE(wait_q, thread, tmp)
#இல்லையெனில் /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#endif /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			status = walk_func(thread, data);
			எனில் (status != 0) {
				நிறுத்து;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		எனில் (post_func != வெற்று) {
			post_func(status, data);
		}
	}

	திரும்பு status;
}


/*
 * future scheduler.h API implementations
 */

முழு z_sched_wait(கட்டமைப்பு k_spinlock *lock, k_spinlock_சாவி_t சாவி,
		 _wait_q_t *wait_q, k_timeout_t timeout, வெறுமை **data)
{
	முழு ret = z_pend_curr(lock, சாவி, wait_q, timeout);

	எனில் (data != வெற்று) {
		*data = _current->base.swap_data;
	}
	திரும்பு ret;
}
