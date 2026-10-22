/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ضمن <zephyr/kernel.h>
#ضمن <kspinlock.h>
#ضمن <ksched.h>
#ضمن <zephyr/sys/__assert.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <run_q.h>
#ضمن <wait_q.h>
#ضمن <scheduler.h>


فراغ z_sched_init(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	لكل (صحيح i = 0; i < CONFIG_MP_MAX_انگ_CPUS; i++) {
		priq_لاڳيتو_init(&_kernel.cpus[i].ready_q.runq);
	}
#وإلا
	priq_لاڳيتو_init(&_kernel.ready_q.runq);
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

فراغ k_sched_تالو(فراغ)
{
	Z_SCHED_SPINLOCK {
		SYS_پورٽ_TRACING_FUNC(k_thread, sched_تالو);

		__ASSERT(!arch_is_اندريون_isr(), "");
		__ASSERT(_current->بنياد.sched_locked != 1U, "");

		--_current->بنياد.sched_locked;

		compiler_barrier();
	}
}

فراغ k_sched_unlock(فراغ)
{
	SYS_پورٽ_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_ڪنجي_t ڪنجي = z_sched_spinlock_تالو();

	__ASSERT(_current->بنياد.sched_locked != 0U, "");
	__ASSERT(!arch_is_اندريون_isr(), "");
	++_current->بنياد.sched_locked;
	z_sched_تالو_reschedule(ڪنجي);
}

فراغ z_impl_k_reschedule(فراغ)
{
	k_spinlock_ڪنجي_t ڪنجي;

	ڪنجي = z_sched_spinlock_تالو();

	z_sched_تالو_reschedule(ڪنجي);
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_reschedule(فراغ)
{
	z_impl_k_reschedule();
}
#ضمن <zephyr/syscalls/k_reschedule_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(فراغ)
{
	أرجع _current;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
ساكن مضمن k_tid_t z_vrfy_k_sched_current_thread_query(فراغ)
{
	أرجع z_impl_k_sched_current_thread_query();
}
#ضمن <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */


صحيح z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, فراغ *معلومات)
{
	بنية k_thread *thread;
	صحيح  حالت = 0;

	Z_SCHED_SPINLOCK {
#جيڪڏهن_وصف_نه_ٿيل CONFIG_WAITQ_SCALABLE
		بنية k_thread *tmp;

		WAIT_Q_FOR_EACH_محفوظ(wait_q, thread, tmp)
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#جيڪڏهن_پورو /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			حالت = walk_func(thread, معلومات);
			إذا (حالت != 0) {
				اكسر;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		إذا (post_func != عدم) {
			post_func(حالت, معلومات);
		}
	}

	أرجع حالت;
}


/*
 * future scheduler.h API implementations
 */

صحيح z_sched_wait(بنية k_spinlock *تالو, k_spinlock_ڪنجي_t ڪنجي,
		 _wait_q_t *wait_q, k_timeout_t timeout, فراغ **معلومات)
{
	صحيح ret = z_pend_curr(تالو, ڪنجي, wait_q, timeout);

	إذا (معلومات != عدم) {
		*معلومات = _current->بنياد.مٽا_سٽا_معلومات;
	}
	أرجع ret;
}
