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


خالی z_sched_init(خالی)
{
#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	برائے (صحیح i = 0; i < CONFIG_MP_MAX_عدد_CPUS; i++) {
		priq_تسلسل_init(&_kernel.cpus[i].ready_q.runq);
	}
#ورنہ
	priq_تسلسل_init(&_kernel.ready_q.runq);
#اگر_ختم /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

خالی k_sched_تالا(خالی)
{
	Z_SCHED_SPINLOCK {
		SYS_پورٹ_TRACING_FUNC(k_thread, sched_تالا);

		__ASSERT(!arch_is_اندر_isr(), "");
		__ASSERT(_current->بنیاد.sched_locked != 1U, "");

		--_current->بنیاد.sched_locked;

		compiler_barrier();
	}
}

خالی k_sched_unlock(خالی)
{
	SYS_پورٹ_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_کلید_t کلید = z_sched_spinlock_تالا();

	__ASSERT(_current->بنیاد.sched_locked != 0U, "");
	__ASSERT(!arch_is_اندر_isr(), "");
	++_current->بنیاد.sched_locked;
	z_sched_تالا_reschedule(کلید);
}

خالی z_impl_k_reschedule(خالی)
{
	k_spinlock_کلید_t کلید;

	کلید = z_sched_spinlock_تالا();

	z_sched_تالا_reschedule(کلید);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_reschedule(خالی)
{
	z_impl_k_reschedule();
}
#شامل <zephyr/syscalls/k_reschedule_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(خالی)
{
	واپس _current;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی k_tid_t z_vrfy_k_sched_current_thread_query(خالی)
{
	واپس z_impl_k_sched_current_thread_query();
}
#شامل <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */


صحیح z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, خالی *ڈیٹا)
{
	ساخت k_thread *thread;
	صحیح  حالت_2 = 0;

	Z_SCHED_SPINLOCK {
#اگر_غیر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
		ساخت k_thread *tmp;

		WAIT_Q_FOR_EACH_محفوظ(wait_q, thread, tmp)
#ورنہ /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#اگر_ختم /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			حالت_2 = walk_func(thread, ڈیٹا);
			اگر (حالت_2 != 0) {
				توڑو;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		اگر (post_func != تہی) {
			post_func(حالت_2, ڈیٹا);
		}
	}

	واپس حالت_2;
}


/*
 * future scheduler.h API implementations
 */

صحیح z_sched_wait(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید,
		 _wait_q_t *wait_q, k_timeout_t timeout, خالی **ڈیٹا)
{
	صحیح ret = z_pend_curr(تالا, کلید, wait_q, timeout);

	اگر (ڈیٹا != تہی) {
		*ڈیٹا = _current->بنیاد.تبادلہ_ڈیٹا;
	}
	واپس ret;
}
