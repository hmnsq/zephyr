/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>
#ಅನ್ತರ್ಭಾವ <timeslicing.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತುಂಡು_ticks = DIV_ಸುತ್ತು_ಮೇಲೆ(CONFIG_TIMESLICE_ಗಾತ್ರ * Z_HZ_ticks, Z_HZ_ms);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತುಂಡು_max_prio = CONFIG_TIMESLICE_ಆದ್ಯತೆ;
ಸ್ಥಿರ ರಚನೆ _timeout ತುಂಡು_timeouts[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ಸ್ಥಿರ atomic_t ತುಂಡು_expired[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ಅದಲುಬದಲು_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
ರಚನೆ k_thread *pending_current;
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ತುಂಡು_ಸಮಯ(ರಚನೆ k_thread *thread)
{
	ಪೂರ್ಣಾಂಕ ret = ತುಂಡು_ticks;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICE_ಪ್ರತಿ_THREAD
	ಆದರೆ (thread->ತಳ.ತುಂಡು_ticks != 0) {
		ret = thread->ತಳ.ತುಂಡು_ticks;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ z_ಸಮಯ_ತುಂಡು_ಗಾತ್ರ(ರಚನೆ k_thread *thread)
{
	ಆದರೆ (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (ತುಂಡು_ಸಮಯ(thread) == 0)) {
		ಹಿಂತಿರುಗಿ 0;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICE_ಪ್ರತಿ_THREAD
	ಆದರೆ (thread->ತಳ.ತುಂಡು_ticks != 0) {
		ಹಿಂತಿರುಗಿ thread->ತಳ.ತುಂಡು_ticks;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->ತಳ.prio, ತುಂಡು_max_prio)) {
		ಹಿಂತಿರುಗಿ ತುಂಡು_ticks;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಡು_timeout(ರಚನೆ _timeout *timeout)
{
	ಪೂರ್ಣಾಂಕ cpu = ARRAY_ಸೂಚ್ಯಂಕ(ತುಂಡು_timeouts, timeout);

	atomic_ಗಣ(&ತುಂಡು_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	ಆದರೆ (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_ಮುಖವಾಡ(cpu));
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಡು_reset(ಪೂರ್ಣಾಂಕ ತುಂಡು_ಗಾತ್ರ)
{
	ಪೂರ್ಣಾಂಕ cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(ಶೂನ್ಯ)z_try_abort_timeout(&ತುಂಡು_timeouts[cpu]);
	ಆದರೆ (ತುಂಡು_ಗಾತ್ರ != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		ಪೂರ್ಣಾಂಕ delay = atomic_get(&ತುಂಡು_expired[cpu]) ? ತುಂಡು_ಗಾತ್ರ - 1 : ತುಂಡು_ಗಾತ್ರ;

		z_ಕೂಡಿಸು_timeout(&ತುಂಡು_timeouts[cpu], ತುಂಡು_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&ತುಂಡು_expired[cpu]);
}

ಶೂನ್ಯ z_ಸಮಯ_ತುಂಡು_reset(ರಚನೆ k_thread *thread)
{
	ತುಂಡು_reset(z_ಸಮಯ_ತುಂಡು_ಗಾತ್ರ(thread));
}

ಸ್ಥಿರ ALWAYS_INLINE bool thread_ನಿರೂಪಿಸುತ್ತದೆ_ಸಮಯ_ತುಂಡು_ಗಾತ್ರ(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICE_ಪ್ರತಿ_THREAD
	ಹಿಂತಿರುಗಿ (thread->ತಳ.ತುಂಡು_ticks != 0);
#ಇಲ್ಲದಿದ್ದರೆ  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_ಬಳಸದ(thread);
	ಹಿಂತಿರುಗಿ false;
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_TIMESLICE_PER_THREAD */
}

ಶೂನ್ಯ k_sched_ಸಮಯ_ತುಂಡು_ಗಣ(int32_t ತುಂಡು, ಪೂರ್ಣಾಂಕ prio)
{
	Z_SCHED_SPINLOCK {

		ತುಂಡು_ticks = k_ms_ಗಮ್ಯ_ticks_ceil32(ತುಂಡು);
		ತುಂಡು_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		ಆದರೆ (!thread_ನಿರೂಪಿಸುತ್ತದೆ_ಸಮಯ_ತುಂಡು_ಗಾತ್ರ(_current)) {
			z_ಸಮಯ_ತುಂಡು_reset(_current);
		}
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICE_ಪ್ರತಿ_THREAD
ಶೂನ್ಯ k_thread_ಸಮಯ_ತುಂಡು_ಗಣ(ರಚನೆ k_thread *thread, int32_t thread_ತುಂಡು_ticks,
			     k_thread_timeslice_fn_t expired, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	Z_SCHED_SPINLOCK {
		thread->ತಳ.ತುಂಡು_ticks = thread_ತುಂಡು_ticks;
		thread->ತಳ.ತುಂಡು_expired = expired;
		thread->ತಳ.ತುಂಡು_ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
		z_ಸಮಯ_ತುಂಡು_reset(thread);
	}
}
#ಆದರೆ_ಅಂತ್ಯ

/* Called out of each timer and IPI interrupt */
ಶೂನ್ಯ z_ಸಮಯ_ತುಂಡು(ಶೂನ್ಯ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	ಆದರೆ (!IS_ENABLED(CONFIG_ಅದಲುಬದಲು_NONATOMIC) &&
	    !atomic_get(&ತುಂಡು_expired[_current_cpu->id])) {
		ಹಿಂತಿರುಗಿ;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();
	ರಚನೆ k_thread *curr = _current;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ಅದಲುಬದಲು_NONATOMIC
	ಆದರೆ (pending_current == curr) {
		z_ಸಮಯ_ತುಂಡು_reset(curr);
		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}
	pending_current = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	ಪೂರ್ಣಾಂಕ ತುಂಡು_ಗಾತ್ರ = 0;

	ಆದರೆ (atomic_get(&ತುಂಡು_expired[_current_cpu->id])) {
		ತುಂಡು_ಗಾತ್ರ = z_ಸಮಯ_ತುಂಡು_ಗಾತ್ರ(curr);
	}

	ಆದರೆ (ತುಂಡು_ಗಾತ್ರ != 0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICE_ಪ್ರತಿ_THREAD
		k_thread_timeslice_fn_t handler = curr->ತಳ.ತುಂಡು_expired;

		ಆದರೆ (handler != ಖಾಲಿ) {
			z_sched_spinlock_unlock(ಕೀಲಿ);
			handler(curr, curr->ತಳ.ತುಂಡು_ದತ್ತಾಂಶ);
			ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			ತುಂಡು_ಗಾತ್ರ = z_ಸಮಯ_ತುಂಡು_ಗಾತ್ರ(curr);
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (!z_is_thread_prevented_from_running(curr)) {
			move_current_ಗಮ್ಯ_ಕೊನೆ_ದ_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
			ರಚನೆ k_thread *ಮುಂದಿನ = runq_ಉತ್ತಮ();

			ಆದರೆ (ಮುಂದಿನ == ಖಾಲಿ || z_is_idle_thread_object(ಮುಂದಿನ)) {
				ತುಂಡು_reset(ತುಂಡು_ಗಾತ್ರ);
			}
#ಇಲ್ಲದಿದ್ದರೆ
			ಆದರೆ (_kernel.ready_q.cache == curr) {
				ತುಂಡು_reset(ತುಂಡು_ಗಾತ್ರ);
			}
#ಆದರೆ_ಅಂತ್ಯ
		}
	}
	z_sched_spinlock_unlock(ಕೀಲಿ);
}
