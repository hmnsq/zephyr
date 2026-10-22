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
ಸ್ಥಿರ ರಚನೆ k_spinlock timeout_ಬೀಗ;

/* Ticks left to process in the currently-executing sys_clock_announce() */
ಸ್ಥಿರ uint32_t announce_ಉಳಿದ;

/* CPU id currently inside sys_clock_announce_locked()'s firing loop, or -1
 * when no CPU is. The SMP early-return below ensures at most one CPU is in
 * the loop at a time, so a single int suffices. Used by code that needs to
 * know whether *this* CPU is at a tick edge (announcing) versus somewhere
 * within a tick (any other context, including running on a CPU while a
 * different CPU is the announcer).
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ announcing_cpu = -1;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool this_cpu_announcing(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ announcing_cpu == CPU_ID;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool any_cpu_announcing(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ announcing_cpu != -1;
}

/* Timeout whose handler is currently being dispatched, or NULL when no
 * handler is in flight. The announcing CPU sets the pointer before
 * calling the handler and clears it afterwards; any aborter may set the
 * low "superseded" bit (struct _timeout is pointer-aligned so bit 0 is
 * free). Accessors below mask the bit.
 */
ಸ್ಥಿರ ರಚನೆ _timeout *inflight_timeout;

#ವ್ಯಾಖ್ಯಾನಿಸು INFLIGHT_SUPERSEDED_ಬಿಟ್ 1UL

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *inflight_ptr(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ (ರಚನೆ _timeout *)((uintptr_t)inflight_timeout & ~INFLIGHT_SUPERSEDED_ಬಿಟ್);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ inflight_mark_superseded(ಶೂನ್ಯ)
{
	inflight_timeout = (ರಚನೆ _timeout *)((uintptr_t)inflight_timeout |
					       INFLIGHT_SUPERSEDED_ಬಿಟ್);
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
	ಆದರೆ (this_cpu_announcing()) {
		ಹಿಂತಿರುಗಿ 0U;
	}
	ಹಿಂತಿರುಗಿ sys_clock_elapsed() +
	       (IS_ENABLED(CONFIG_SMP) ? announce_ಉಳಿದ : 0);
}

/*
 * Backend queue implementation. The selected backend's queue instance and its
 * z_timeout_q_*() operations are private to this file: the backend is a header
 * included only here, after the shared state above, so those operations can
 * reach curr_tick / announce_remaining / inflight_timeout / timeout_lock
 * directly. The per-node helpers (z_init_timeout / z_is_inactive_timeout) are
 * tree-wide and live in timeout_q.h.
 */
#ಆದರೆ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)
#ಅನ್ತರ್ಭಾವ "timeout/minheap.h"
#ಅಥವಾಆದರೆ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)
#ಅನ್ತರ್ಭಾವ "timeout/wheel.h"
#ಅಥವಾಆದರೆ defined(CONFIG_TIMEOUT_BACKEND_ಪಾತ್ರೆ)
#ಅನ್ತರ್ಭಾವ "timeout/bucket.h"
#ಅಥವಾಆದರೆ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)
#ಅನ್ತರ್ಭಾವ "timeout/skiplist.h"
#ಇಲ್ಲದಿದ್ದರೆ /* CONFIG_TIMEOUT_BACKEND_DLIST */
#ಅನ್ತರ್ಭಾವ "timeout/list.h"
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Ticks the driver may wait before the next sys_clock_announce(). The
 * announce-range cap lives here, in the backend-independent front end, so no
 * backend has to reproduce it: the backend only reports the delta to its
 * earliest pending timeout via z_timeout_q_next_expiry().
 */
ಸ್ಥಿರ uint32_t ಮುಂದಿನ_timeout(uint32_t ticks_elapsed)
{
	k_ticks_t ಮುಂದಿನ = z_timeout_q_ಮುಂದಿನ_expiry();
	uint32_t dticks;

	/*
	 * sys_clock_announce() reports the ticks elapsed since the previous
	 * announce, so the gap between two announces must not exceed
	 * SYS_CLOCK_MAX_WAIT for the announced count to fit. The budget left
	 * from now on is SYS_CLOCK_MAX_WAIT - ticks_elapsed; if it is already
	 * spent ask for an announce right away.
	 */
	ಆದರೆ (ticks_elapsed >= SYS_CLOCK_MAX_WAIT) {
		ಹಿಂತಿರುಗಿ 0;
	}

	/*
	 * No deadline, or one further out than can be scheduled in a single
	 * step: wait the capped budget and re-evaluate at the next announce.
	 * Testing next (which may be 64-bit) against the cap keeps the
	 * remaining arithmetic in 32 bits. The empty case still returns the
	 * budget so a driver keeps waking to maintain uptime.
	 */
	ಆದರೆ (ಮುಂದಿನ == K_TICKS_FOREVER || ಮುಂದಿನ >= SYS_CLOCK_MAX_WAIT) {
		ಹಿಂತಿರುಗಿ SYS_CLOCK_MAX_WAIT - ticks_elapsed;
	}

	/* Otherwise wait until the timeout, relative to now (0 if due). */
	dticks = (uint32_t)ಮುಂದಿನ;

	ಹಿಂತಿರುಗಿ (dticks > ticks_elapsed) ? (dticks - ticks_elapsed) : 0;
}

/*
 * Reprogram the timer for the next pending timeout, or, when nothing is
 * pending and CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE tolerates a drifting uptime, tell
 * the driver its clock is unused so it may stop. Only meaningful where the
 * timeout queue may have just drained (abort, end of announce); the add path
 * always has a pending timeout and calls sys_clock_set_timeout() directly.
 */
ಸ್ಥಿರ ಶೂನ್ಯ reprogram_ಮುಂದಿನ(uint32_t ticks_elapsed)
{
	ಆದರೆ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
	    z_timeout_q_ಮುಂದಿನ_expiry() == K_TICKS_FOREVER) {
		sys_clock_no_timeout();
	} ಇಲ್ಲದಿದ್ದರೆ {
		sys_clock_ಗಣ_timeout(ಮುಂದಿನ_timeout(ticks_elapsed), false);
	}
}

k_ticks_t z_ಕೂಡಿಸು_timeout(ರಚನೆ _timeout *ಗಮ್ಯ, _timeout_func_t fn, k_timeout_t timeout)
{
	k_ticks_t ticks = 0;

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		ಹಿಂತಿರುಗಿ 0;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(ಗಮ್ಯ));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */

	__ASSERT(z_is_inactive_timeout(ಗಮ್ಯ), "");
	ಗಮ್ಯ->fn = fn;

	K_SPINLOCK(&timeout_ಬೀಗ) {
		uint32_t ticks_elapsed = 0;
		bool has_elapsed = false;
		k_ticks_t dticks;

		ಆದರೆ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
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
		} ಇಲ್ಲದಿದ್ದರೆ {
			dticks = Z_TICK_ABS(timeout.ticks) - curr_tick;
			dticks = max(1, dticks);
			ticks = timeout.ticks;
		}

		ಆದರೆ (z_timeout_q_ಸೇರಿಸು(ಗಮ್ಯ, dticks) && !any_cpu_announcing()) {
			ಆದರೆ (!has_elapsed) {
				/* In case of absolute timeout that is first to expire
				 * elapsed need to be read from the system clock.
				 */
				ticks_elapsed = elapsed();
			}
			sys_clock_ಗಣ_timeout(ಮುಂದಿನ_timeout(ticks_elapsed), false);
		}
	}

	ಹಿಂತಿರುಗಿ ticks;
}

ಪೂರ್ಣಾಂಕ z_try_abort_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret = -EINVAL;

	K_SPINLOCK(&timeout_ಬೀಗ) {
		ಆದರೆ (!z_is_inactive_timeout(ಗಮ್ಯ)) {
			bool was_ಮೊದಲ = z_timeout_q_remove(ಗಮ್ಯ);

			ret = 0;
			ಆದರೆ (was_ಮೊದಲ) {
				reprogram_ಮುಂದಿನ(elapsed());
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_ENABLED(CONFIG_SMP) && inflight_ptr() == ಗಮ್ಯ &&
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
		ಆದರೆ (inflight_ptr() == ಗಮ್ಯ) {
			inflight_mark_superseded();
		}
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SMP) && ret == -EAGAIN) {
		arch_spin_relax();
	}

	ಹಿಂತಿರುಗಿ ret;
}

bool z_timeout_inflight_superseded(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	bool superseded = false;

	K_SPINLOCK(&timeout_ಬೀಗ) {
		superseded = inflight_timeout ==
			     (ರಚನೆ _timeout *)((uintptr_t)ಗಮ್ಯ | INFLIGHT_SUPERSEDED_ಬಿಟ್);
	}

	ಹಿಂತಿರುಗಿ superseded;
}

k_ticks_t z_timeout_ಉಳಿದ(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_ಬೀಗ) {
		ಆದರೆ (!z_is_inactive_timeout(timeout)) {
			k_ticks_t rem = z_timeout_q_remainder(timeout);
			uint32_t el = elapsed();

			/* elapsed() can exceed rem when announce is late. */
			ticks = (rem > el) ? (rem - el) : 0;
		}
	}

	ಹಿಂತಿರುಗಿ ticks;
}
EXPORT_ಚಿಹ್ನೆ(z_timeout_ಉಳಿದ);

k_ticks_t z_timeout_expires(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_ಬೀಗ) {
		ticks = curr_tick;
		ಆದರೆ (!z_is_inactive_timeout(timeout)) {
			ticks += z_timeout_q_remainder(timeout);
		}
	}

	ಹಿಂತಿರುಗಿ ticks;
}
EXPORT_ಚಿಹ್ನೆ(z_timeout_expires);

uint32_t z_get_ಮುಂದಿನ_timeout_expiry(ಶೂನ್ಯ)
{
	uint32_t ret = (uint32_t)K_TICKS_FOREVER;

	K_SPINLOCK(&timeout_ಬೀಗ) {
		/*
		 * Same decision as reprogram_next(). Sloppy idle lets the
		 * uptime drift, so an empty list means nothing to wake up for
		 * and that is reported as K_TICKS_FOREVER. Otherwise the
		 * answer is a wait: either a real deadline, or the synthetic
		 * one that keeps the announce range covered.
		 */
		ಆದರೆ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
		    z_timeout_q_ಮುಂದಿನ_expiry() == K_TICKS_FOREVER) {
			ret = (uint32_t)K_TICKS_FOREVER;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ret = ಮುಂದಿನ_timeout(elapsed());
		}
	}
	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ sys_clock_announce_locked(uint32_t ticks, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	/* We release the lock around the callbacks below, so on SMP
	 * systems someone might be already running the loop.  Don't
	 * race (which will cause parallel execution of "sequential"
	 * timeouts and confuse apps), just increment the tick count
	 * and return.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_SMP) && any_cpu_announcing()) {
		announce_ಉಳಿದ += ticks;
		k_spin_unlock(&timeout_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	announce_ಉಳಿದ = ticks;
	announcing_cpu = CPU_ID;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _TIMEOUT_BACKEND_OWNS_ANNOUNCE
	/* The backend (timer wheel) runs the firing loop itself: its per-tick
	 * advance is a bitmap jump over empty ticks and its sift is a
	 * time-driven event with no single timeout to drive a generic loop.
	 * It advances curr_tick / announce_remaining and fires handlers via
	 * the same inflight_timeout dance as below.
	 */
	ಕೀಲಿ = z_timeout_q_announce(ಕೀಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
	ಆಗ (announce_ಉಳಿದ > 0) {
		ರಚನೆ _timeout *t;
		int32_t dt = z_timeout_q_ಮುಂದಿನ_gap();

		ಆದರೆ ((uint32_t)dt > announce_ಉಳಿದ) {
			/* Next event is past this announce window: advance the
			 * backend by the residual ticks without firing anything.
			 */
			dt = (int32_t)announce_ಉಳಿದ;
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
		announce_ಉಳಿದ -= dt;

		/* Drain everything due at the new curr_tick. */
		ಆಗ ((t = z_timeout_q_ತೆಗೆ_due()) != ಖಾಲಿ) {
			_timeout_func_t handler = t->fn;

			inflight_timeout = t;

			k_spin_unlock(&timeout_ಬೀಗ, ಕೀಲಿ);
			handler(t);
			ಕೀಲಿ = k_spin_ಬೀಗ(&timeout_ಬೀಗ);
			inflight_timeout = ಖಾಲಿ;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* _TIMEOUT_BACKEND_OWNS_ANNOUNCE */

	announcing_cpu = -1;

	reprogram_ಮುಂದಿನ(0);

	k_spin_unlock(&timeout_ಬೀಗ, ಕೀಲಿ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICING
	z_ಸಮಯ_ತುಂಡು();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICING */
}

#ಆದರೆ defined(CONFIG_SMP) || defined(CONFIG_SPIN_VALIDATE)
k_spinlock_ಕೀಲಿ_t sys_clock_ಬೀಗ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ k_spin_ಬೀಗ(&timeout_ಬೀಗ);
}

ಶೂನ್ಯ sys_clock_unlock(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	k_spin_unlock(&timeout_ಬೀಗ, ಕೀಲಿ);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_TEST) || defined(CONFIG_ASSERT)
bool sys_clock_is_locked(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ z_spin_is_locked(&timeout_ಬೀಗ);
}
#ಆದರೆ_ಅಂತ್ಯ

int64_t sys_clock_tick_get(ಶೂನ್ಯ)
{
	uint64_t t = 0U;

	K_SPINLOCK(&timeout_ಬೀಗ) {
		t = curr_tick + elapsed();
	}
	ಹಿಂತಿರುಗಿ t;
}

uint32_t sys_clock_tick_get_32(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TICKLESS_KERNEL
	ಹಿಂತಿರುಗಿ (uint32_t)sys_clock_tick_get();
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ (uint32_t)curr_tick;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TICKLESS_KERNEL */
}

int64_t z_impl_k_uptime_ticks(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ sys_clock_tick_get();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ int64_t z_vrfy_k_uptime_ticks(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ z_impl_k_uptime_ticks();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_uptime_ticks_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
	k_timepoint_t timepoint;

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		timepoint.tick = UINT64_MAX;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		timepoint.tick = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_ticks_t dt = timeout.ticks;

		ಆದರೆ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			timepoint.tick = sys_clock_tick_get() + max(1, dt);
		} ಇಲ್ಲದಿದ್ದರೆ {
			timepoint.tick = Z_TICK_ABS(dt);
		}
	}

	ಹಿಂತಿರುಗಿ timepoint;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
	uint64_t now, ಉಳಿದ;

	ಆದರೆ (timepoint.tick == UINT64_MAX) {
		ಹಿಂತಿರುಗಿ K_FOREVER;
	}
	ಆದರೆ (timepoint.tick == 0) {
		ಹಿಂತಿರುಗಿ K_NO_WAIT;
	}

	now = sys_clock_tick_get();
	ಉಳಿದ = (timepoint.tick > now) ? (timepoint.tick - now) : 0;
	ಹಿಂತಿರುಗಿ K_TICKS(ಉಳಿದ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ZTEST
ಶೂನ್ಯ z_impl_sys_clock_tick_ಗಣ(uint64_t tick)
{
	curr_tick = tick;
}

ಶೂನ್ಯ z_vrfy_sys_clock_tick_ಗಣ(uint64_t tick)
{
	z_impl_sys_clock_tick_ಗಣ(tick);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ZTEST */
