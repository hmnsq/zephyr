/*
 * Copyright (c) 2018, 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/atomic.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kswap.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ipi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeslicing.h>

ਸਥਿਰ ਪੂਰਨ ਟੁਕੜਾ_ticks = DIV_ਗੇੜਾ_ਉੱਪਰ(CONFIG_TIMESLICE_ਮਾਪ * Z_HZ_ticks, Z_HZ_ms);
ਸਥਿਰ ਪੂਰਨ ਟੁਕੜਾ_max_prio = CONFIG_TIMESLICE_ਤਰਜੀਹ;
ਸਥਿਰ ਬਣਤਰ _timeout ਟੁਕੜਾ_timeouts[CONFIG_MP_MAX_ਅੰਕ_CPUS];

/* Timeouts can expire on another CPU, so each CPU's flag must be atomic. */
ਸਥਿਰ atomic_t ਟੁਕੜਾ_expired[CONFIG_MP_MAX_ਅੰਕ_CPUS];

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਅਦਲਾ_ਬਦਲੀ_NONATOMIC
/* If z_swap() isn't atomic, then it's possible for a timer interrupt
 * to try to timeslice away _current after it has already pended
 * itself but before the corresponding context switch.  Treat that as
 * a noop condition in z_time_slice().
 */
ਬਣਤਰ k_thread *pending_current;
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ਟੁਕੜਾ_ਸਮਾਂ(ਬਣਤਰ k_thread *thread)
{
	ਪੂਰਨ ret = ਟੁਕੜਾ_ticks;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICE_ਪ੍ਰਤੀ_THREAD
	ਜੇ (thread->ਅਧਾਰ.ਟੁਕੜਾ_ticks != 0) {
		ret = thread->ਅਧਾਰ.ਟੁਕੜਾ_ticks;
	}
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ z_ਸਮਾਂ_ਟੁਕੜਾ_ਮਾਪ(ਬਣਤਰ k_thread *thread)
{
	ਜੇ (z_is_thread_prevented_from_running(thread) ||
	    z_is_idle_thread_object(thread) ||
	    (ਟੁਕੜਾ_ਸਮਾਂ(thread) == 0)) {
		ਵਾਪਸ 0;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICE_ਪ੍ਰਤੀ_THREAD
	ਜੇ (thread->ਅਧਾਰ.ਟੁਕੜਾ_ticks != 0) {
		ਵਾਪਸ thread->ਅਧਾਰ.ਟੁਕੜਾ_ticks;
	}
#ਜੇ_ਅੰਤ

	ਜੇ (thread_is_preemptible(thread) &&
	    !z_is_prio_higher(thread->ਅਧਾਰ.prio, ਟੁਕੜਾ_max_prio)) {
		ਵਾਪਸ ਟੁਕੜਾ_ticks;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ ਟੁਕੜਾ_timeout(ਬਣਤਰ _timeout *timeout)
{
	ਪੂਰਨ cpu = ARRAY_ਸੂਚਕਾਂਕ(ਟੁਕੜਾ_timeouts, timeout);

	atomic_ਸੈੱਟ(&ਟੁਕੜਾ_expired[cpu], 1);

	/* We need an IPI if we just handled a timeslice expiration
	 * for a different CPU.
	 */
	ਜੇ (cpu != _current_cpu->id) {
		flag_ipi(IPI_CPU_ਨਕਾਬ(cpu));
	}
}

ਸਥਿਰ ਖਾਲੀ ਟੁਕੜਾ_reset(ਪੂਰਨ ਟੁਕੜਾ_ਮਾਪ)
{
	ਪੂਰਨ cpu = _current_cpu->id;

	/* Best-effort cancel: if the slice timeout is already in flight,
	 * its handler only flips slice_expired[cpu] (which we clear below)
	 * and possibly raises an IPI -- harmless either way.
	 */
	(ਖਾਲੀ)z_try_abort_timeout(&ਟੁਕੜਾ_timeouts[cpu]);
	ਜੇ (ਟੁਕੜਾ_ਮਾਪ != 0) {
		/* When invoked because the slicer just fired (this CPU or
		 * via IPI from another), we're at a tick edge but past the
		 * announce window, so subtract 1 to cancel z_add_timeout()'s
		 * "+1" round-up and land at exactly slice_size ticks.
		 */
		ਪੂਰਨ delay = atomic_get(&ਟੁਕੜਾ_expired[cpu]) ? ਟੁਕੜਾ_ਮਾਪ - 1 : ਟੁਕੜਾ_ਮਾਪ;

		z_ਜੋੜੋ_timeout(&ਟੁਕੜਾ_timeouts[cpu], ਟੁਕੜਾ_timeout,
			      K_TICKS(delay));
	}
	atomic_clear(&ਟੁਕੜਾ_expired[cpu]);
}

ਖਾਲੀ z_ਸਮਾਂ_ਟੁਕੜਾ_reset(ਬਣਤਰ k_thread *thread)
{
	ਟੁਕੜਾ_reset(z_ਸਮਾਂ_ਟੁਕੜਾ_ਮਾਪ(thread));
}

ਸਥਿਰ ALWAYS_INLINE bool thread_ਨਿਰਧਾਰਤ_ਕਰਦਾ_ਸਮਾਂ_ਟੁਕੜਾ_ਮਾਪ(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICE_ਪ੍ਰਤੀ_THREAD
	ਵਾਪਸ (thread->ਅਧਾਰ.ਟੁਕੜਾ_ticks != 0);
#ਨਹੀਂਤਾਂ  /* !CONFIG_TIMESLICE_PER_THREAD */
	ARG_ਅਣਵਰਤਿਆ(thread);
	ਵਾਪਸ false;
#ਜੇ_ਅੰਤ /* !CONFIG_TIMESLICE_PER_THREAD */
}

ਖਾਲੀ k_sched_ਸਮਾਂ_ਟੁਕੜਾ_ਸੈੱਟ(int32_t ਟੁਕੜਾ, ਪੂਰਨ prio)
{
	Z_SCHED_SPINLOCK {

		ਟੁਕੜਾ_ticks = k_ms_ਮੰਜ਼ਿਲ_ticks_ceil32(ਟੁਕੜਾ);
		ਟੁਕੜਾ_max_prio = prio;

		/*
		 * Threads that define their own time slice size should not have
		 * their time slices reset here as a thread-specific time slice size
		 * take precedence over the global time slice size.
		 */

		ਜੇ (!thread_ਨਿਰਧਾਰਤ_ਕਰਦਾ_ਸਮਾਂ_ਟੁਕੜਾ_ਮਾਪ(_current)) {
			z_ਸਮਾਂ_ਟੁਕੜਾ_reset(_current);
		}
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICE_ਪ੍ਰਤੀ_THREAD
ਖਾਲੀ k_thread_ਸਮਾਂ_ਟੁਕੜਾ_ਸੈੱਟ(ਬਣਤਰ k_thread *thread, int32_t thread_ਟੁਕੜਾ_ticks,
			     k_thread_timeslice_fn_t expired, ਖਾਲੀ *ਅੰਕੜੇ)
{
	Z_SCHED_SPINLOCK {
		thread->ਅਧਾਰ.ਟੁਕੜਾ_ticks = thread_ਟੁਕੜਾ_ticks;
		thread->ਅਧਾਰ.ਟੁਕੜਾ_expired = expired;
		thread->ਅਧਾਰ.ਟੁਕੜਾ_ਅੰਕੜੇ = ਅੰਕੜੇ;
		z_ਸਮਾਂ_ਟੁਕੜਾ_reset(thread);
	}
}
#ਜੇ_ਅੰਤ

/* Called out of each timer and IPI interrupt */
ਖਾਲੀ z_ਸਮਾਂ_ਟੁਕੜਾ(ਖਾਲੀ)
{
	/*
	 * Atomic context switches need no scheduler work until this CPU's
	 * time slice expires. Non-atomic context switches must still take
	 * the scheduler lock to synchronize pending_current.
	 */
	ਜੇ (!IS_ENABLED(CONFIG_ਅਦਲਾ_ਬਦਲੀ_NONATOMIC) &&
	    !atomic_get(&ਟੁਕੜਾ_expired[_current_cpu->id])) {
		ਵਾਪਸ;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();
	ਬਣਤਰ k_thread *curr = _current;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਅਦਲਾ_ਬਦਲੀ_NONATOMIC
	ਜੇ (pending_current == curr) {
		z_ਸਮਾਂ_ਟੁਕੜਾ_reset(curr);
		z_sched_spinlock_unlock(ਕੁੰਜੀ);
		ਵਾਪਸ;
	}
	pending_current = ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	ਪੂਰਨ ਟੁਕੜਾ_ਮਾਪ = 0;

	ਜੇ (atomic_get(&ਟੁਕੜਾ_expired[_current_cpu->id])) {
		ਟੁਕੜਾ_ਮਾਪ = z_ਸਮਾਂ_ਟੁਕੜਾ_ਮਾਪ(curr);
	}

	ਜੇ (ਟੁਕੜਾ_ਮਾਪ != 0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICE_ਪ੍ਰਤੀ_THREAD
		k_thread_timeslice_fn_t handler = curr->ਅਧਾਰ.ਟੁਕੜਾ_expired;

		ਜੇ (handler != ਨਿਰਾ) {
			z_sched_spinlock_unlock(ਕੁੰਜੀ);
			handler(curr, curr->ਅਧਾਰ.ਟੁਕੜਾ_ਅੰਕੜੇ);
			ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

			/* The handler ran with the lock dropped and may have
			 * changed this thread's slice configuration, so the
			 * cached size is stale; recompute before rearming.
			 */
			ਟੁਕੜਾ_ਮਾਪ = z_ਸਮਾਂ_ਟੁਕੜਾ_ਮਾਪ(curr);
		}
#ਜੇ_ਅੰਤ
		ਜੇ (!z_is_thread_prevented_from_running(curr)) {
			move_current_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_prio_q();
			/* If the rotation kept curr at the front (no other
			 * runnable thread of equal-or-higher priority is
			 * waiting), no swap will happen and we must rearm
			 * here. Otherwise the dispatch path will rearm for
			 * the new thread with slice_expired still set,
			 * picking up the tick-aligned delay.
			 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
			ਬਣਤਰ k_thread *ਅਗਲਾ = runq_ਉੱਤਮ();

			ਜੇ (ਅਗਲਾ == ਨਿਰਾ || z_is_idle_thread_object(ਅਗਲਾ)) {
				ਟੁਕੜਾ_reset(ਟੁਕੜਾ_ਮਾਪ);
			}
#ਨਹੀਂਤਾਂ
			ਜੇ (_kernel.ready_q.cache == curr) {
				ਟੁਕੜਾ_reset(ਟੁਕੜਾ_ਮਾਪ);
			}
#ਜੇ_ਅੰਤ
		}
	}
	z_sched_spinlock_unlock(ਕੁੰਜੀ);
}
