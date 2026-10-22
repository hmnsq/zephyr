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
#که_تعریف_شوی CONFIG_SCHED_CPU_ماسک_PIN_ONLY
	لكل (صحيح i = 0; i < CONFIG_MP_MAX_عدد_CPUS; i++) {
		priq_پرلپسې_init(&_kernel.cpus[i].ready_q.runq);
	}
#وإلا
	priq_پرلپسې_init(&_kernel.ready_q.runq);
#د_که_پای /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

فراغ k_sched_قلف(فراغ)
{
	Z_SCHED_SPINLOCK {
		SYS_پورټ_TRACING_FUNC(k_thread, sched_قلف);

		__ASSERT(!arch_is_دننه_isr(), "");
		__ASSERT(_current->بنسټ.sched_locked != 1U, "");

		--_current->بنسټ.sched_locked;

		compiler_barrier();
	}
}

فراغ k_sched_unlock(فراغ)
{
	SYS_پورټ_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_کلی_t کلی = z_sched_spinlock_قلف();

	__ASSERT(_current->بنسټ.sched_locked != 0U, "");
	__ASSERT(!arch_is_دننه_isr(), "");
	++_current->بنسټ.sched_locked;
	z_sched_قلف_reschedule(کلی);
}

فراغ z_impl_k_reschedule(فراغ)
{
	k_spinlock_کلی_t کلی;

	کلی = z_sched_spinlock_قلف();

	z_sched_قلف_reschedule(کلی);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_reschedule(فراغ)
{
	z_impl_k_reschedule();
}
#ضمن <zephyr/syscalls/k_reschedule_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(فراغ)
{
	أرجع _current;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن k_tid_t z_vrfy_k_sched_current_thread_query(فراغ)
{
	أرجع z_impl_k_sched_current_thread_query();
}
#ضمن <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */


صحيح z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, فراغ *معلومات)
{
	بنية k_thread *thread;
	صحيح  حالت = 0;

	Z_SCHED_SPINLOCK {
#که_نه_تعریف_شوی CONFIG_WAITQ_SCALABLE
		بنية k_thread *tmp;

		WAIT_Q_FOR_EACH_خوندي(wait_q, thread, tmp)
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#د_که_پای /* !CONFIG_WAITQ_SCALABLE */
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

صحيح z_sched_wait(بنية k_spinlock *قلف, k_spinlock_کلی_t کلی,
		 _wait_q_t *wait_q, k_timeout_t timeout, فراغ **معلومات)
{
	صحيح ret = z_pend_curr(قلف, کلی, wait_q, timeout);

	إذا (معلومات != عدم) {
		*معلومات = _current->بنسټ.بدلون_معلومات;
	}
	أرجع ret;
}
