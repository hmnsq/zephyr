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
#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک_PIN_ONLY
	لكل (صحيح i = 0; i < CONFIG_MP_MAX_رقم_CPUS; i++) {
		priq_پیاپی_init(&_kernel.cpus[i].ready_q.runq);
	}
#وإلا
	priq_پیاپی_init(&_kernel.ready_q.runq);
#پایان_اگر /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

فراغ k_sched_قفل(فراغ)
{
	Z_SCHED_SPINLOCK {
		SYS_پورت_TRACING_FUNC(k_thread, sched_قفل);

		__ASSERT(!arch_is_درون_isr(), "");
		__ASSERT(_current->پایه.sched_locked != 1U, "");

		--_current->پایه.sched_locked;

		compiler_barrier();
	}
}

فراغ k_sched_unlock(فراغ)
{
	SYS_پورت_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_کلید_t کلید = z_sched_spinlock_قفل();

	__ASSERT(_current->پایه.sched_locked != 0U, "");
	__ASSERT(!arch_is_درون_isr(), "");
	++_current->پایه.sched_locked;
	z_sched_قفل_reschedule(کلید);
}

فراغ z_impl_k_reschedule(فراغ)
{
	k_spinlock_کلید_t کلید;

	کلید = z_sched_spinlock_قفل();

	z_sched_قفل_reschedule(کلید);
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_reschedule(فراغ)
{
	z_impl_k_reschedule();
}
#ضمن <zephyr/syscalls/k_reschedule_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(فراغ)
{
	أرجع _current;
}

#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن k_tid_t z_vrfy_k_sched_current_thread_query(فراغ)
{
	أرجع z_impl_k_sched_current_thread_query();
}
#ضمن <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */


صحيح z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, فراغ *داده)
{
	بنية k_thread *thread;
	صحيح  وضعیت = 0;

	Z_SCHED_SPINLOCK {
#اگر_تعریف_نشده CONFIG_WAITQ_SCALABLE
		بنية k_thread *tmp;

		WAIT_Q_FOR_EACH_امن(wait_q, thread, tmp)
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#پایان_اگر /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			وضعیت = walk_func(thread, داده);
			إذا (وضعیت != 0) {
				اكسر;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		إذا (post_func != عدم) {
			post_func(وضعیت, داده);
		}
	}

	أرجع وضعیت;
}


/*
 * future scheduler.h API implementations
 */

صحيح z_sched_wait(بنية k_spinlock *قفل, k_spinlock_کلید_t کلید,
		 _wait_q_t *wait_q, k_timeout_t timeout, فراغ **داده)
{
	صحيح ret = z_pend_curr(قفل, کلید, wait_q, timeout);

	إذا (داده != عدم) {
		*داده = _current->پایه.جابجا_داده;
	}
	أرجع ret;
}
