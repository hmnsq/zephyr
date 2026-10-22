/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/__assert.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <run_q.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>


ಶೂನ್ಯ z_sched_init(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY
	ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < CONFIG_MP_MAX_ಅಂಕೆ_CPUS; i++) {
		priq_ಸತತ_init(&_kernel.cpus[i].ready_q.runq);
	}
#ಇಲ್ಲದಿದ್ದರೆ
	priq_ಸತತ_init(&_kernel.ready_q.runq);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ಶೂನ್ಯ k_sched_ಬೀಗ(ಶೂನ್ಯ)
{
	Z_SCHED_SPINLOCK {
		SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, sched_ಬೀಗ);

		__ASSERT(!arch_is_ಒಳ_isr(), "");
		__ASSERT(_current->ತಳ.sched_locked != 1U, "");

		--_current->ತಳ.sched_locked;

		compiler_barrier();
	}
}

ಶೂನ್ಯ k_sched_unlock(ಶೂನ್ಯ)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, sched_unlock);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	__ASSERT(_current->ತಳ.sched_locked != 0U, "");
	__ASSERT(!arch_is_ಒಳ_isr(), "");
	++_current->ತಳ.sched_locked;
	z_sched_ಬೀಗ_reschedule(ಕೀಲಿ);
}

ಶೂನ್ಯ z_impl_k_reschedule(ಶೂನ್ಯ)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	z_sched_ಬೀಗ_reschedule(ಕೀಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_reschedule(ಶೂನ್ಯ)
{
	z_impl_k_reschedule();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_reschedule_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

k_tid_t z_impl_k_sched_current_thread_query(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ _current;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_tid_t z_vrfy_k_sched_current_thread_query(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ z_impl_k_sched_current_thread_query();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sched_current_thread_query_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */


ಪೂರ್ಣಾಂಕ z_sched_waitq_walk(_wait_q_t *wait_q, _waitq_walk_cb_t walk_func,
		       _waitq_post_walk_cb_t post_func, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ರಚನೆ k_thread *thread;
	ಪೂರ್ಣಾಂಕ  ಸ್ಥಿತಿ_2 = 0;

	Z_SCHED_SPINLOCK {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_WAITQ_SCALABLE
		ರಚನೆ k_thread *tmp;

		WAIT_Q_FOR_EACH_ಸುರಕ್ಷಿತ(wait_q, thread, tmp)
#ಇಲ್ಲದಿದ್ದರೆ /* !CONFIG_WAITQ_SCALABLE */
		_WAIT_Q_FOR_EACH(wait_q, thread)
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_WAITQ_SCALABLE */
		{

			/*
			 * Invoke the callback function on each waiting thread
			 * for as long as there are both waiting threads AND
			 * it returns 0.
			 */

			ಸ್ಥಿತಿ_2 = walk_func(thread, ದತ್ತಾಂಶ);
			ಆದರೆ (ಸ್ಥಿತಿ_2 != 0) {
				ನಿಲ್ಲಿಸು;
			}
		}

		/*
		 * Invoke post-walk callback. This is done while
		 * still holding the scheduler spinlock to enable atomic
		 * operations (from the scheduler's point of view).
		 */
		ಆದರೆ (post_func != ಖಾಲಿ) {
			post_func(ಸ್ಥಿತಿ_2, ದತ್ತಾಂಶ);
		}
	}

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


/*
 * future scheduler.h API implementations
 */

ಪೂರ್ಣಾಂಕ z_sched_wait(ರಚನೆ k_spinlock *ಬೀಗ, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ,
		 _wait_q_t *wait_q, k_timeout_t timeout, ಶೂನ್ಯ **ದತ್ತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ ret = z_pend_curr(ಬೀಗ, ಕೀಲಿ, wait_q, timeout);

	ಆದರೆ (ದತ್ತಾಂಶ != ಖಾಲಿ) {
		*ದತ್ತಾಂಶ = _current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;
	}
	ಹಿಂತಿರುಗಿ ret;
}
