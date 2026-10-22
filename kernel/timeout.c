/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/minmax.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeout_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/drivers/timer/system_timer.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/clock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/llext/symbol.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <timeslicing.h>

/* Absolute tick counter: ticks since boot. */
ਸਥਿਰ uint64_t curr_tick;

/*
 * The timeout code shall take no locks other than its own (timeout_lock), nor
 * shall it call any other subsystem while holding this lock. Code outside this
 * file takes it through sys_clock_lock()/sys_clock_unlock().
 */
ਸਥਿਰ ਬਣਤਰ k_spinlock timeout_ਤਾਲਾ;

/* Ticks left to process in the currently-executing sys_clock_announce() */
ਸਥਿਰ uint32_t announce_ਬਚਿਆ;

/* CPU id currently inside sys_clock_announce_locked()'s firing loop, or -1
 * when no CPU is. The SMP early-return below ensures at most one CPU is in
 * the loop at a time, so a single int suffices. Used by code that needs to
 * know whether *this* CPU is at a tick edge (announcing) versus somewhere
 * within a tick (any other context, including running on a CPU while a
 * different CPU is the announcer).
 */
ਸਥਿਰ ਪੂਰਨ announcing_cpu = -1;

ਸਥਿਰ ਸਤਰਅੰਦਰ bool this_cpu_announcing(ਖਾਲੀ)
{
	ਵਾਪਸ announcing_cpu == CPU_ID;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool any_cpu_announcing(ਖਾਲੀ)
{
	ਵਾਪਸ announcing_cpu != -1;
}

/* Timeout whose handler is currently being dispatched, or NULL when no
 * handler is in flight. The announcing CPU sets the pointer before
 * calling the handler and clears it afterwards; any aborter may set the
 * low "superseded" bit (struct _timeout is pointer-aligned so bit 0 is
 * free). Accessors below mask the bit.
 */
ਸਥਿਰ ਬਣਤਰ _timeout *inflight_timeout;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INFLIGHT_SUPERSEDED_ਦੋਆਧਾਰੀ_ਅੰਕ 1UL

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *inflight_ptr(ਖਾਲੀ)
{
	ਵਾਪਸ (ਬਣਤਰ _timeout *)((uintptr_t)inflight_timeout & ~INFLIGHT_SUPERSEDED_ਦੋਆਧਾਰੀ_ਅੰਕ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ inflight_mark_superseded(ਖਾਲੀ)
{
	inflight_timeout = (ਬਣਤਰ _timeout *)((uintptr_t)inflight_timeout |
					       INFLIGHT_SUPERSEDED_ਦੋਆਧਾਰੀ_ਅੰਕ);
}

ਸਥਿਰ uint32_t elapsed(ਖਾਲੀ)
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
	ਜੇ (this_cpu_announcing()) {
		ਵਾਪਸ 0U;
	}
	ਵਾਪਸ sys_clock_elapsed() +
	       (IS_ENABLED(CONFIG_SMP) ? announce_ਬਚਿਆ : 0);
}

/*
 * Backend queue implementation. The selected backend's queue instance and its
 * z_timeout_q_*() operations are private to this file: the backend is a header
 * included only here, after the shared state above, so those operations can
 * reach curr_tick / announce_remaining / inflight_timeout / timeout_lock
 * directly. The per-node helpers (z_init_timeout / z_is_inactive_timeout) are
 * tree-wide and live in timeout_q.h.
 */
#ਜੇ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)
#ਅਨ੍ਤਰ੍ਭਾਵ "timeout/minheap.h"
#ਜਾਂਜੇ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)
#ਅਨ੍ਤਰ੍ਭਾਵ "timeout/wheel.h"
#ਜਾਂਜੇ defined(CONFIG_TIMEOUT_BACKEND_ਬਾਲਟੀ)
#ਅਨ੍ਤਰ੍ਭਾਵ "timeout/bucket.h"
#ਜਾਂਜੇ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)
#ਅਨ੍ਤਰ੍ਭਾਵ "timeout/skiplist.h"
#ਨਹੀਂਤਾਂ /* CONFIG_TIMEOUT_BACKEND_DLIST */
#ਅਨ੍ਤਰ੍ਭਾਵ "timeout/list.h"
#ਜੇ_ਅੰਤ

/*
 * Ticks the driver may wait before the next sys_clock_announce(). The
 * announce-range cap lives here, in the backend-independent front end, so no
 * backend has to reproduce it: the backend only reports the delta to its
 * earliest pending timeout via z_timeout_q_next_expiry().
 */
ਸਥਿਰ uint32_t ਅਗਲਾ_timeout(uint32_t ticks_elapsed)
{
	k_ticks_t ਅਗਲਾ = z_timeout_q_ਅਗਲਾ_expiry();
	uint32_t dticks;

	/*
	 * sys_clock_announce() reports the ticks elapsed since the previous
	 * announce, so the gap between two announces must not exceed
	 * SYS_CLOCK_MAX_WAIT for the announced count to fit. The budget left
	 * from now on is SYS_CLOCK_MAX_WAIT - ticks_elapsed; if it is already
	 * spent ask for an announce right away.
	 */
	ਜੇ (ticks_elapsed >= SYS_CLOCK_MAX_WAIT) {
		ਵਾਪਸ 0;
	}

	/*
	 * No deadline, or one further out than can be scheduled in a single
	 * step: wait the capped budget and re-evaluate at the next announce.
	 * Testing next (which may be 64-bit) against the cap keeps the
	 * remaining arithmetic in 32 bits. The empty case still returns the
	 * budget so a driver keeps waking to maintain uptime.
	 */
	ਜੇ (ਅਗਲਾ == K_TICKS_FOREVER || ਅਗਲਾ >= SYS_CLOCK_MAX_WAIT) {
		ਵਾਪਸ SYS_CLOCK_MAX_WAIT - ticks_elapsed;
	}

	/* Otherwise wait until the timeout, relative to now (0 if due). */
	dticks = (uint32_t)ਅਗਲਾ;

	ਵਾਪਸ (dticks > ticks_elapsed) ? (dticks - ticks_elapsed) : 0;
}

/*
 * Reprogram the timer for the next pending timeout, or, when nothing is
 * pending and CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE tolerates a drifting uptime, tell
 * the driver its clock is unused so it may stop. Only meaningful where the
 * timeout queue may have just drained (abort, end of announce); the add path
 * always has a pending timeout and calls sys_clock_set_timeout() directly.
 */
ਸਥਿਰ ਖਾਲੀ reprogram_ਅਗਲਾ(uint32_t ticks_elapsed)
{
	ਜੇ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
	    z_timeout_q_ਅਗਲਾ_expiry() == K_TICKS_FOREVER) {
		sys_clock_no_timeout();
	} ਨਹੀਂਤਾਂ {
		sys_clock_ਸੈੱਟ_timeout(ਅਗਲਾ_timeout(ticks_elapsed), false);
	}
}

k_ticks_t z_ਜੋੜੋ_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, _timeout_func_t fn, k_timeout_t timeout)
{
	k_ticks_t ticks = 0;

	ਜੇ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		ਵਾਪਸ 0;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(ਮੰਜ਼ਿਲ));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */

	__ASSERT(z_is_inactive_timeout(ਮੰਜ਼ਿਲ), "");
	ਮੰਜ਼ਿਲ->fn = fn;

	K_SPINLOCK(&timeout_ਤਾਲਾ) {
		uint32_t ticks_elapsed = 0;
		bool has_elapsed = false;
		k_ticks_t dticks;

		ਜੇ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
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
		} ਨਹੀਂਤਾਂ {
			dticks = Z_TICK_ABS(timeout.ticks) - curr_tick;
			dticks = max(1, dticks);
			ticks = timeout.ticks;
		}

		ਜੇ (z_timeout_q_ਪਾਓ(ਮੰਜ਼ਿਲ, dticks) && !any_cpu_announcing()) {
			ਜੇ (!has_elapsed) {
				/* In case of absolute timeout that is first to expire
				 * elapsed need to be read from the system clock.
				 */
				ticks_elapsed = elapsed();
			}
			sys_clock_ਸੈੱਟ_timeout(ਅਗਲਾ_timeout(ticks_elapsed), false);
		}
	}

	ਵਾਪਸ ticks;
}

ਪੂਰਨ z_try_abort_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਪੂਰਨ ret = -EINVAL;

	K_SPINLOCK(&timeout_ਤਾਲਾ) {
		ਜੇ (!z_is_inactive_timeout(ਮੰਜ਼ਿਲ)) {
			bool was_ਪਹਿਲਾ = z_timeout_q_remove(ਮੰਜ਼ਿਲ);

			ret = 0;
			ਜੇ (was_ਪਹਿਲਾ) {
				reprogram_ਅਗਲਾ(elapsed());
			}
		} ਨਹੀਂਤਾਂ ਜੇ (IS_ENABLED(CONFIG_SMP) && inflight_ptr() == ਮੰਜ਼ਿਲ &&
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
		ਜੇ (inflight_ptr() == ਮੰਜ਼ਿਲ) {
			inflight_mark_superseded();
		}
	}

	ਜੇ (IS_ENABLED(CONFIG_SMP) && ret == -EAGAIN) {
		arch_spin_relax();
	}

	ਵਾਪਸ ret;
}

bool z_timeout_inflight_superseded(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	bool superseded = false;

	K_SPINLOCK(&timeout_ਤਾਲਾ) {
		superseded = inflight_timeout ==
			     (ਬਣਤਰ _timeout *)((uintptr_t)ਮੰਜ਼ਿਲ | INFLIGHT_SUPERSEDED_ਦੋਆਧਾਰੀ_ਅੰਕ);
	}

	ਵਾਪਸ superseded;
}

k_ticks_t z_timeout_ਬਚਿਆ(ਅਚਲ ਬਣਤਰ _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_ਤਾਲਾ) {
		ਜੇ (!z_is_inactive_timeout(timeout)) {
			k_ticks_t rem = z_timeout_q_remainder(timeout);
			uint32_t el = elapsed();

			/* elapsed() can exceed rem when announce is late. */
			ticks = (rem > el) ? (rem - el) : 0;
		}
	}

	ਵਾਪਸ ticks;
}
EXPORT_ਚਿੰਨ੍ਹ(z_timeout_ਬਚਿਆ);

k_ticks_t z_timeout_expires(ਅਚਲ ਬਣਤਰ _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_ਤਾਲਾ) {
		ticks = curr_tick;
		ਜੇ (!z_is_inactive_timeout(timeout)) {
			ticks += z_timeout_q_remainder(timeout);
		}
	}

	ਵਾਪਸ ticks;
}
EXPORT_ਚਿੰਨ੍ਹ(z_timeout_expires);

uint32_t z_get_ਅਗਲਾ_timeout_expiry(ਖਾਲੀ)
{
	uint32_t ret = (uint32_t)K_TICKS_FOREVER;

	K_SPINLOCK(&timeout_ਤਾਲਾ) {
		/*
		 * Same decision as reprogram_next(). Sloppy idle lets the
		 * uptime drift, so an empty list means nothing to wake up for
		 * and that is reported as K_TICKS_FOREVER. Otherwise the
		 * answer is a wait: either a real deadline, or the synthetic
		 * one that keeps the announce range covered.
		 */
		ਜੇ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
		    z_timeout_q_ਅਗਲਾ_expiry() == K_TICKS_FOREVER) {
			ret = (uint32_t)K_TICKS_FOREVER;
		} ਨਹੀਂਤਾਂ {
			ret = ਅਗਲਾ_timeout(elapsed());
		}
	}
	ਵਾਪਸ ret;
}

ਖਾਲੀ sys_clock_announce_locked(uint32_t ticks, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	/* We release the lock around the callbacks below, so on SMP
	 * systems someone might be already running the loop.  Don't
	 * race (which will cause parallel execution of "sequential"
	 * timeouts and confuse apps), just increment the tick count
	 * and return.
	 */
	ਜੇ (IS_ENABLED(CONFIG_SMP) && any_cpu_announcing()) {
		announce_ਬਚਿਆ += ticks;
		k_spin_unlock(&timeout_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	announce_ਬਚਿਆ = ticks;
	announcing_cpu = CPU_ID;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _TIMEOUT_BACKEND_OWNS_ANNOUNCE
	/* The backend (timer wheel) runs the firing loop itself: its per-tick
	 * advance is a bitmap jump over empty ticks and its sift is a
	 * time-driven event with no single timeout to drive a generic loop.
	 * It advances curr_tick / announce_remaining and fires handlers via
	 * the same inflight_timeout dance as below.
	 */
	ਕੁੰਜੀ = z_timeout_q_announce(ਕੁੰਜੀ);
#ਨਹੀਂਤਾਂ
	ਜਦੋਂ (announce_ਬਚਿਆ > 0) {
		ਬਣਤਰ _timeout *t;
		int32_t dt = z_timeout_q_ਅਗਲਾ_gap();

		ਜੇ ((uint32_t)dt > announce_ਬਚਿਆ) {
			/* Next event is past this announce window: advance the
			 * backend by the residual ticks without firing anything.
			 */
			dt = (int32_t)announce_ਬਚਿਆ;
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
		announce_ਬਚਿਆ -= dt;

		/* Drain everything due at the new curr_tick. */
		ਜਦੋਂ ((t = z_timeout_q_ਕੱਢੋ_due()) != ਨਿਰਾ) {
			_timeout_func_t handler = t->fn;

			inflight_timeout = t;

			k_spin_unlock(&timeout_ਤਾਲਾ, ਕੁੰਜੀ);
			handler(t);
			ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timeout_ਤਾਲਾ);
			inflight_timeout = ਨਿਰਾ;
		}
	}
#ਜੇ_ਅੰਤ /* _TIMEOUT_BACKEND_OWNS_ANNOUNCE */

	announcing_cpu = -1;

	reprogram_ਅਗਲਾ(0);

	k_spin_unlock(&timeout_ਤਾਲਾ, ਕੁੰਜੀ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICING
	z_ਸਮਾਂ_ਟੁਕੜਾ();
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICING */
}

#ਜੇ defined(CONFIG_SMP) || defined(CONFIG_SPIN_VALIDATE)
k_spinlock_ਕੁੰਜੀ_t sys_clock_ਤਾਲਾ(ਖਾਲੀ)
{
	ਵਾਪਸ k_spin_ਤਾਲਾ(&timeout_ਤਾਲਾ);
}

ਖਾਲੀ sys_clock_unlock(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	k_spin_unlock(&timeout_ਤਾਲਾ, ਕੁੰਜੀ);
}
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_TEST) || defined(CONFIG_ASSERT)
bool sys_clock_is_locked(ਖਾਲੀ)
{
	ਵਾਪਸ z_spin_is_locked(&timeout_ਤਾਲਾ);
}
#ਜੇ_ਅੰਤ

int64_t sys_clock_tick_get(ਖਾਲੀ)
{
	uint64_t t = 0U;

	K_SPINLOCK(&timeout_ਤਾਲਾ) {
		t = curr_tick + elapsed();
	}
	ਵਾਪਸ t;
}

uint32_t sys_clock_tick_get_32(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TICKLESS_KERNEL
	ਵਾਪਸ (uint32_t)sys_clock_tick_get();
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ (uint32_t)curr_tick;
#ਜੇ_ਅੰਤ /* CONFIG_TICKLESS_KERNEL */
}

int64_t z_impl_k_uptime_ticks(ਖਾਲੀ)
{
	ਵਾਪਸ sys_clock_tick_get();
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ int64_t z_vrfy_k_uptime_ticks(ਖਾਲੀ)
{
	ਵਾਪਸ z_impl_k_uptime_ticks();
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_uptime_ticks_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
	k_timepoint_t timepoint;

	ਜੇ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		timepoint.tick = UINT64_MAX;
	} ਨਹੀਂਤਾਂ ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		timepoint.tick = 0;
	} ਨਹੀਂਤਾਂ {
		k_ticks_t dt = timeout.ticks;

		ਜੇ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			timepoint.tick = sys_clock_tick_get() + max(1, dt);
		} ਨਹੀਂਤਾਂ {
			timepoint.tick = Z_TICK_ABS(dt);
		}
	}

	ਵਾਪਸ timepoint;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
	uint64_t now, ਬਚਿਆ;

	ਜੇ (timepoint.tick == UINT64_MAX) {
		ਵਾਪਸ K_FOREVER;
	}
	ਜੇ (timepoint.tick == 0) {
		ਵਾਪਸ K_NO_WAIT;
	}

	now = sys_clock_tick_get();
	ਬਚਿਆ = (timepoint.tick > now) ? (timepoint.tick - now) : 0;
	ਵਾਪਸ K_TICKS(ਬਚਿਆ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ZTEST
ਖਾਲੀ z_impl_sys_clock_tick_ਸੈੱਟ(uint64_t tick)
{
	curr_tick = tick;
}

ਖਾਲੀ z_vrfy_sys_clock_tick_ਸੈੱਟ(uint64_t tick)
{
	z_impl_sys_clock_tick_ਸੈੱਟ(tick);
}
#ਜੇ_ਅੰਤ /* CONFIG_ZTEST */
