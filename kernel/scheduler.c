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
#جے_تعریف_شدہ CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	لكل (صحيح i = 0; i < CONFIG_MP_MAX_عدد_CPUS; i++) {
		priq_لگاتار_init(&_kernel.cpus[i].ready_q.runq);
	}
#وإلا
	priq_لگاتار_init(&_kernel.ready_q.runq);
#جے_انت /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

فراغ k_sched_جندرا(فراغ)
{
	Z_SCHED_SPINLOCK {
		SYS_پورٹ_TRACING_FUNC(k_thread, sched_جندرا);

		__ASSERT(!arch_is_وچ_isr(), "");
		__ASSERT(_current->ادھار.sched_locked != 1U, "");

		--_current->ادھار.sched_locked;

		compiler_barrier();
	}
}

فراغ k_sched_unlock(فراغ)
{
	SYS_پورٹ_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_کلید_t کلید = z_sched_spinlock_جندرا();

	__ASSERT(_current->ادھار.sched_locked != 0U, "");
	__ASSERT(!arch_is_وچ_isr(), "");
	++_current->ادھار.sched_locked;
	z_sched_جندرا_reschedule(کلید);
}

فراغ z_impl_k_reschedule(فراغ)
{
	k_spinlock_کلید_t کلید;

	کلید = z_sched_spinlock_جندرا();

	z_sched_جندرا_reschedule(کلید);
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_reschedule(فراغ)
{
	z_impl_k_reschedule();
}
#ضمن <zephyr/syscalls/k_reschedule_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(فراغ)
{
	أرجع _current;
}

#جے_تعریف_شدہ CONFIG_USERSPACE
ساكن مضمن k_tid_t z_vrfy_k_sched_current_thread_query(فراغ)
{
	أرجع z_impl_k_sched_current_thread_query();
}
#ضمن <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#جے_انت /* CONFIG_USERSPACE */


صحيح z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, فراغ *انکڑے)
{
	بنية k_thread *thread;
	صحيح  حالت = 0;

	Z_SCHED_SPINLOCK {
#جے_بنا_تعریف CONFIG_WAITQ_SCALABLE
		بنية k_thread *tmp;

		WAIT_Q_FOR_EACH_محفوظ(wait_q, thread, tmp)
#وإلا /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#جے_انت /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			حالت = walk_func(thread, انکڑے);
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
			post_func(حالت, انکڑے);
		}
	}

	أرجع حالت;
}


/*
 * future scheduler.h API implementations
 */

صحيح z_sched_wait(بنية k_spinlock *جندرا, k_spinlock_کلید_t کلید,
		 _wait_q_t *wait_q, k_timeout_t timeout, فراغ **انکڑے)
{
	صحيح ret = z_pend_curr(جندرا, کلید, wait_q, timeout);

	إذا (انکڑے != عدم) {
		*انکڑے = _current->ادھار.وٹاندرا_انکڑے;
	}
	أرجع ret;
}
