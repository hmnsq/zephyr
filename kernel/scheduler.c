/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/sys/__assert.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <run_q.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <scheduler.h>


శూన్య z_sched_init(శూన్య)
{
#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY
	కొరకు (పూర్ణాంకం i = 0; i < CONFIG_MP_MAX_అంకె_CPUS; i++) {
		priq_నిరంతరం_init(&_kernel.cpus[i].ready_q.runq);
	}
#లేకపోతే
	priq_నిరంతరం_init(&_kernel.ready_q.runq);
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

శూన్య k_sched_తాళం(శూన్య)
{
	Z_SCHED_SPINLOCK {
		SYS_పోర్టు_TRACING_FUNC(k_thread, sched_తాళం);

		__ASSERT(!arch_is_లోపల_isr(), "");
		__ASSERT(_current->పునాది.sched_locked != 1U, "");

		--_current->పునాది.sched_locked;

		compiler_barrier();
	}
}

శూన్య k_sched_unlock(శూన్య)
{
	SYS_పోర్టు_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

	__ASSERT(_current->పునాది.sched_locked != 0U, "");
	__ASSERT(!arch_is_లోపల_isr(), "");
	++_current->పునాది.sched_locked;
	z_sched_తాళం_reschedule(కీలకం);
}

శూన్య z_impl_k_reschedule(శూన్య)
{
	k_spinlock_కీలకం_t కీలకం;

	కీలకం = z_sched_spinlock_తాళం();

	z_sched_తాళం_reschedule(కీలకం);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_reschedule(శూన్య)
{
	z_impl_k_reschedule();
}
#అన్తర్భావ <zephyr/syscalls/k_reschedule_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(శూన్య)
{
	తిరిగి _current;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస k_tid_t z_vrfy_k_sched_current_thread_query(శూన్య)
{
	తిరిగి z_impl_k_sched_current_thread_query();
}
#అన్తర్భావ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */


పూర్ణాంకం z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, శూన్య *దత్తాంశం)
{
	నిర్మాణం k_thread *thread;
	పూర్ణాంకం  స్థితి_2 = 0;

	Z_SCHED_SPINLOCK {
#నిర్వచితం_కాకపోతే CONFIG_WAITQ_SCALABLE
		నిర్మాణం k_thread *tmp;

		WAIT_Q_FOR_EACH_సురక్షిత(wait_q, thread, tmp)
#లేకపోతే /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#అయితే_ముగింపు /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			స్థితి_2 = walk_func(thread, దత్తాంశం);
			ఒకవేళ (స్థితి_2 != 0) {
				ఆపు;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		ఒకవేళ (post_func != ఖాళీ) {
			post_func(స్థితి_2, దత్తాంశం);
		}
	}

	తిరిగి స్థితి_2;
}


/*
 * future scheduler.h API implementations
 */

పూర్ణాంకం z_sched_wait(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం,
		 _wait_q_t *wait_q, k_timeout_t timeout, శూన్య **దత్తాంశం)
{
	పూర్ణాంకం ret = z_pend_curr(తాళం, కీలకం, wait_q, timeout);

	ఒకవేళ (దత్తాంశం != ఖాళీ) {
		*దత్తాంశం = _current->పునాది.మార్పిడి_దత్తాంశం;
	}
	తిరిగి ret;
}
