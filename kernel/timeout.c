/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <timeout_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/drivers/timer/system_timer.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/clock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/llext/symbol.h>

#ಅನ್ತರ್ಭಾವ <timeslicing.h>

/* Absolute tick counter: ticks since boot. */
ಸ್ಥಿರ uint64_t curr_tick;

/*
 * The timeout code shall take no locks other than its own (timeout_lock), nor
 * shall it call any other subsystem while holding this lock. Code outside this
 * file takes it through sys_clock_lock()/sys_clock_unlock().
 */
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock timeout_lock;

/* Ticks left to process in the currently-executing sys_clock_announce() */
ಸ್ಥಿರ uint32_t announce_remaining;

/* CPU id currently inside sys_clock_announce_locked()'s firing loop, or -1
 * when no CPU is. The SMP early-return below ensures at most one CPU is in
 * the loop at a time, so a single int suffices. Used by code that needs to
 * know whether *this* CPU is at a tick edge (announcing) versus somewhere
 * within a tick (any other context, including running on a CPU while a
 * different CPU is the announcer).
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ announcing_cpu = -1;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool this_cpu_announcing(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ announcing_cpu == CPU_ID;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool any_cpu_announcing(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ announcing_cpu != -1;
}

/* Timeout whose handler is currently being dispatched, or NULL when no
 * handler is in flight. The announcing CPU sets the pointer before
 * calling the handler and clears it afterwards; any aborter may set the
 * low "superseded" bit (struct _timeout is pointer-aligned so bit 0 is
 * free). Accessors below mask the bit.
 */
ಸ್ಥಿರ ಸಂರಚನಾ _timeout *inflight_timeout;

#define INFLIGHT_SUPERSEDED_BIT 1UL

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *inflight_ptr(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ (ಸಂರಚನಾ _timeout *)((uintptr_t)inflight_timeout & ~INFLIGHT_SUPERSEDED_BIT);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ inflight_mark_superseded(ಶೂನ್ಯ)
{
	inflight_timeout = (ಸಂರಚನಾ _timeout *)((uintptr_t)inflight_timeout |
					       INFLIGHT_SUPERSEDED_BIT);
}

ಸ್ಥಿರ uint32_t elapsed(ಶೂನ್ಯ)
{
	/*
	 * While *this* CPU is executing sys_clock_announce_locked()'s firing
	 * loop, new relative timeouts scheduled from a callback (or from a
	 * higher-priority ISR that preempted one) are anchored to the currently
	 * firing tick (curr_tick), so we report 0.
	 *
	 * On any other CPU the picture is different: we are not at a tick edge,
	 * and curr_tick is partway through being advanced by the announcing
	 * CPU's loop. The invariant we want to preserve is
	 *
	 *   T_real - curr_tick = announce_remaining + sys_clock_elapsed()
	 *
	 * because the driver bumped its internal announced-cycle baseline to
	 * (curr_tick_initial + N) * CYC_PER_TICK at ISR entry, while the kernel
	 * has only advanced curr_tick by the K ticks processed so far -- so
	 * announce_remaining (= N - K) is the residual that must be added to
	 * sys_clock_elapsed() to get the real-time delta from curr_tick. This
	 * keeps sys_clock_tick_get() monotonic across the announce window.
	 */
	ಯದಿ (this_cpu_announcing()) {
		ಪ್ರತ್ಯಾಗಮ 0U;
	}
	ಪ್ರತ್ಯಾಗಮ sys_clock_elapsed() +
	       (IS_ENABLED(CONFIG_SMP) ? announce_remaining : 0);
}

/*
 * Backend queue implementation. The selected backend's queue instance and its
 * z_timeout_q_*() operations are private to this file: the backend is a header
 * included only here, after the shared state above, so those operations can
 * reach curr_tick / announce_remaining / inflight_timeout / timeout_lock
 * directly. The per-node helpers (z_init_timeout / z_is_inactive_timeout) are
 * tree-wide and live in timeout_q.h.
 */
#ಯದಿ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)
#ಅನ್ತರ್ಭಾವ "timeout/minheap.h"
#ನೋಚೇದ್ಯದಿ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)
#ಅನ್ತರ್ಭಾವ "timeout/wheel.h"
#ನೋಚೇದ್ಯದಿ defined(CONFIG_TIMEOUT_BACKEND_BUCKET)
#ಅನ್ತರ್ಭಾವ "timeout/bucket.h"
#ನೋಚೇದ್ಯದಿ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)
#ಅನ್ತರ್ಭಾವ "timeout/skiplist.h"
#ಅನ್ಯಥಾ /* CONFIG_TIMEOUT_BACKEND_DLIST */
#ಅನ್ತರ್ಭಾವ "timeout/list.h"
#endif

/*
 * Ticks the driver may wait before the next sys_clock_announce(). The
 * announce-range cap lives here, in the backend-independent front end, so no
 * backend has to reproduce it: the backend only reports the delta to its
 * earliest pending timeout via z_timeout_q_next_expiry().
 */
ಸ್ಥಿರ uint32_t next_timeout(uint32_t ticks_elapsed)
{
	k_ticks_t next = z_timeout_q_next_expiry();
	uint32_t dticks;

	/*
	 * sys_clock_announce() reports the ticks elapsed since the previous
	 * announce, so the gap between two announces must not exceed
	 * SYS_CLOCK_MAX_WAIT for the announced count to fit. The budget left
	 * from now on is SYS_CLOCK_MAX_WAIT - ticks_elapsed; if it is already
	 * spent ask for an announce right away.
	 */
	ಯದಿ (ticks_elapsed >= SYS_CLOCK_MAX_WAIT) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/*
	 * No deadline, or one further out than can be scheduled in a single
	 * step: wait the capped budget and re-evaluate at the next announce.
	 * Testing next (which may be 64-bit) against the cap keeps the
	 * remaining arithmetic in 32 bits. The empty case still returns the
	 * budget so a driver keeps waking to maintain uptime.
	 */
	ಯದಿ (next == K_TICKS_FOREVER || next >= SYS_CLOCK_MAX_WAIT) {
		ಪ್ರತ್ಯಾಗಮ SYS_CLOCK_MAX_WAIT - ticks_elapsed;
	}

	/* Otherwise wait until the timeout, relative to now (0 if due). */
	dticks = (uint32_t)next;

	ಪ್ರತ್ಯಾಗಮ (dticks > ticks_elapsed) ? (dticks - ticks_elapsed) : 0;
}

/*
 * Reprogram the timer for the next pending timeout, or, when nothing is
 * pending and CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE tolerates a drifting uptime, tell
 * the driver its clock is unused so it may stop. Only meaningful where the
 * timeout queue may have just drained (abort, end of announce); the add path
 * always has a pending timeout and calls sys_clock_set_timeout() directly.
 */
ಸ್ಥಿರ ಶೂನ್ಯ reprogram_next(uint32_t ticks_elapsed)
{
	ಯದಿ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
	    z_timeout_q_next_expiry() == K_TICKS_FOREVER) {
		sys_clock_no_timeout();
	} ಅನ್ಯಥಾ {
		sys_clock_set_timeout(next_timeout(ticks_elapsed), false);
	}
}

k_ticks_t z_add_timeout(ಸಂರಚನಾ _timeout *to, _timeout_func_t fn, k_timeout_t timeout)
{
	k_ticks_t ticks = 0;

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(to));
#endif /* CONFIG_KERNEL_COHERENCE */

	__ASSERT(z_is_inactive_timeout(to), "");
	to->fn = fn;

	K_SPINLOCK(&timeout_lock) {
		uint32_t ticks_elapsed = 0;
		bool has_elapsed = false;
		k_ticks_t dticks;

		ಯದಿ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			ticks_elapsed = elapsed();
			has_elapsed = true;
			/*
			 * In the general case, "now" may be anywhere within
			 * the current tick. Rounding up by one tick guarantees
			 * "at least N ticks" semantics -- otherwise a request
			 * made partway through a tick would fire on the next
			 * tick edge, yielding less than N full ticks.
			 *
			 * The one moment we know *this* CPU is at a tick edge
			 * is while it is processing timeouts inside its own
			 * sys_clock_announce_locked() loop. Periodic timers
			 * rely on this when rescheduling themselves from the
			 * timer ISR: the round-up would otherwise accumulate
			 * and make every period one tick late. The check is
			 * per-CPU: a thread on another CPU running while we
			 * announce is *not* at a tick edge and still needs
			 * the round-up.
			 */
			dticks = timeout.ticks + ticks_elapsed +
				 (this_cpu_announcing() ? 0 : 1);
			ticks = curr_tick + dticks;
		} ಅನ್ಯಥಾ {
			dticks = Z_TICK_ABS(timeout.ticks) - curr_tick;
			dticks = max(1, dticks);
			ticks = timeout.ticks;
		}

		ಯದಿ (z_timeout_q_ಸೇರಿಸು(to, dticks) && !any_cpu_announcing()) {
			ಯದಿ (!has_elapsed) {
				/* In case of absolute timeout that is first to expire
				 * elapsed need to be read from the system clock.
				 */
				ticks_elapsed = elapsed();
			}
			sys_clock_set_timeout(next_timeout(ticks_elapsed), false);
		}
	}

	ಪ್ರತ್ಯಾಗಮ ticks;
}

ಪೂರ್ಣಾಙ್ಕ z_try_abort_timeout(ಸಂರಚನಾ _timeout *to)
{
	ಪೂರ್ಣಾಙ್ಕ ret = -EINVAL;

	K_SPINLOCK(&timeout_lock) {
		ಯದಿ (!z_is_inactive_timeout(to)) {
			bool was_first = z_timeout_q_remove(to);

			ret = 0;
			ಯದಿ (was_first) {
				reprogram_next(elapsed());
			}
		} ಅನ್ಯಥಾ ಯದಿ (IS_ENABLED(CONFIG_SMP) && inflight_ptr() == to &&
			   !this_cpu_announcing()) {
			/* Handler in flight on another CPU. Free-safety
			 * callers retry on -EAGAIN to wait it out; others
			 * rely on the superseded mark below and don't.
			 */
			ret = -EAGAIN;
		}

		/* Record that the in-flight timeout has been aborted, so a
		 * handler that checks (z_timeout_inflight_superseded) can
		 * tell its dispatch was overtaken.
		 */
		ಯದಿ (inflight_ptr() == to) {
			inflight_mark_superseded();
		}
	}

	ಯದಿ (IS_ENABLED(CONFIG_SMP) && ret == -EAGAIN) {
		arch_spin_relax();
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

bool z_timeout_inflight_superseded(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	bool superseded = false;

	K_SPINLOCK(&timeout_lock) {
		superseded = inflight_timeout ==
			     (ಸಂರಚನಾ _timeout *)((uintptr_t)to | INFLIGHT_SUPERSEDED_BIT);
	}

	ಪ್ರತ್ಯಾಗಮ superseded;
}

k_ticks_t z_timeout_remaining(ಧ್ರುವ ಸಂರಚನಾ _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_lock) {
		ಯದಿ (!z_is_inactive_timeout(timeout)) {
			k_ticks_t rem = z_timeout_q_remainder(timeout);
			uint32_t el = elapsed();

			/* elapsed() can exceed rem when announce is late. */
			ticks = (rem > el) ? (rem - el) : 0;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ticks;
}
EXPORT_ಚಿಹ್ನೆ(z_timeout_remaining);

k_ticks_t z_timeout_expires(ಧ್ರುವ ಸಂರಚನಾ _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_lock) {
		ticks = curr_tick;
		ಯದಿ (!z_is_inactive_timeout(timeout)) {
			ticks += z_timeout_q_remainder(timeout);
		}
	}

	ಪ್ರತ್ಯಾಗಮ ticks;
}
EXPORT_ಚಿಹ್ನೆ(z_timeout_expires);

uint32_t z_get_next_timeout_expiry(ಶೂನ್ಯ)
{
	uint32_t ret = (uint32_t)K_TICKS_FOREVER;

	K_SPINLOCK(&timeout_lock) {
		/*
		 * Same decision as reprogram_next(). Sloppy idle lets the
		 * uptime drift, so an empty list means nothing to wake up for
		 * and that is reported as K_TICKS_FOREVER. Otherwise the
		 * answer is a wait: either a real deadline, or the synthetic
		 * one that keeps the announce range covered.
		 */
		ಯದಿ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
		    z_timeout_q_next_expiry() == K_TICKS_FOREVER) {
			ret = (uint32_t)K_TICKS_FOREVER;
		} ಅನ್ಯಥಾ {
			ret = next_timeout(elapsed());
		}
	}
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ sys_clock_announce_locked(uint32_t ticks, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	/* We release the lock around the callbacks below, so on SMP
	 * systems someone might be already running the loop.  Don't
	 * race (which will cause parallel execution of "sequential"
	 * timeouts and confuse apps), just increment the tick count
	 * and return.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_SMP) && any_cpu_announcing()) {
		announce_remaining += ticks;
		k_spin_unlock(&timeout_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	announce_remaining = ticks;
	announcing_cpu = CPU_ID;

#ifdef _TIMEOUT_BACKEND_OWNS_ANNOUNCE
	/* The backend (timer wheel) runs the firing loop itself: its per-tick
	 * advance is a bitmap jump over empty ticks and its sift is a
	 * time-driven event with no single timeout to drive a generic loop.
	 * It advances curr_tick / announce_remaining and fires handlers via
	 * the same inflight_timeout dance as below.
	 */
	ಕೀಲಿ = z_timeout_q_announce(ಕೀಲಿ);
#ಅನ್ಯಥಾ
	ಯಾವತ್ (announce_remaining > 0) {
		ಸಂರಚನಾ _timeout *t;
		int32_t dt = z_timeout_q_next_gap();

		ಯದಿ ((uint32_t)dt > announce_remaining) {
			/* Next event is past this announce window: advance the
			 * backend by the residual ticks without firing anything.
			 */
			dt = (int32_t)announce_remaining;
		}

		/* Advance curr_tick and decrement announce_remaining together
		 * under the lock so non-announcing CPUs observe a consistent
		 * (curr_tick + announce_remaining + sys_clock_elapsed()) ==
		 * T_real even while we drop the lock around handlers. The "we
		 * are announcing" state is carried by announcing_cpu, so
		 * announce_remaining reaching 0 mid-loop is harmless: same-tick
		 * handlers' z_add_timeout() still anchors via
		 * this_cpu_announcing(), and another CPU's announce still folds
		 * into ours via any_cpu_announcing() in the SMP early-return.
		 */
		z_timeout_q_advance(dt);
		curr_tick += dt;
		announce_remaining -= dt;

		/* Drain everything due at the new curr_tick. */
		ಯಾವತ್ ((t = z_timeout_q_ತೆಗೆ_due()) != ರಿಕ್ತ) {
			_timeout_func_t handler = t->fn;

			inflight_timeout = t;

			k_spin_unlock(&timeout_lock, ಕೀಲಿ);
			handler(t);
			ಕೀಲಿ = k_spin_lock(&timeout_lock);
			inflight_timeout = ರಿಕ್ತ;
		}
	}
#endif /* _TIMEOUT_BACKEND_OWNS_ANNOUNCE */

	announcing_cpu = -1;

	reprogram_next(0);

	k_spin_unlock(&timeout_lock, ಕೀಲಿ);

#ifdef CONFIG_TIMESLICING
	z_time_slice();
#endif /* CONFIG_TIMESLICING */
}

#ಯದಿ defined(CONFIG_SMP) || defined(CONFIG_SPIN_VALIDATE)
k_spinlock_ಕೀಲಿ_t sys_clock_lock(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ k_spin_lock(&timeout_lock);
}

ಶೂನ್ಯ sys_clock_unlock(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	k_spin_unlock(&timeout_lock, ಕೀಲಿ);
}
#endif

#ಯದಿ defined(CONFIG_TEST) || defined(CONFIG_ASSERT)
bool sys_clock_is_locked(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ z_spin_is_locked(&timeout_lock);
}
#endif

int64_t sys_clock_tick_get(ಶೂನ್ಯ)
{
	uint64_t t = 0U;

	K_SPINLOCK(&timeout_lock) {
		t = curr_tick + elapsed();
	}
	ಪ್ರತ್ಯಾಗಮ t;
}

uint32_t sys_clock_tick_get_32(ಶೂನ್ಯ)
{
#ifdef CONFIG_TICKLESS_KERNEL
	ಪ್ರತ್ಯಾಗಮ (uint32_t)sys_clock_tick_get();
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ (uint32_t)curr_tick;
#endif /* CONFIG_TICKLESS_KERNEL */
}

int64_t z_impl_k_uptime_ticks(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ sys_clock_tick_get();
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ int64_t z_vrfy_k_uptime_ticks(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_uptime_ticks();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_uptime_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
	k_timepoint_t timepoint;

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		timepoint.tick = UINT64_MAX;
	} ಅನ್ಯಥಾ ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		timepoint.tick = 0;
	} ಅನ್ಯಥಾ {
		k_ticks_t dt = timeout.ticks;

		ಯದಿ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			timepoint.tick = sys_clock_tick_get() + max(1, dt);
		} ಅನ್ಯಥಾ {
			timepoint.tick = Z_TICK_ABS(dt);
		}
	}

	ಪ್ರತ್ಯಾಗಮ timepoint;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
	uint64_t now, remaining;

	ಯದಿ (timepoint.tick == UINT64_MAX) {
		ಪ್ರತ್ಯಾಗಮ K_FOREVER;
	}
	ಯದಿ (timepoint.tick == 0) {
		ಪ್ರತ್ಯಾಗಮ K_NO_WAIT;
	}

	now = sys_clock_tick_get();
	remaining = (timepoint.tick > now) ? (timepoint.tick - now) : 0;
	ಪ್ರತ್ಯಾಗಮ K_TICKS(remaining);
}

#ifdef CONFIG_ZTEST
ಶೂನ್ಯ z_impl_sys_clock_tick_set(uint64_t tick)
{
	curr_tick = tick;
}

ಶೂನ್ಯ z_vrfy_sys_clock_tick_set(uint64_t tick)
{
	z_impl_sys_clock_tick_set(tick);
}
#endif /* CONFIG_ZTEST */
