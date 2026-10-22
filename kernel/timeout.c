/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/sys/minmax.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <timeout_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/drivers/timer/system_timer.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/clock.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/llext/symbol.h>

#ଅନ୍ତର୍ଭାଵ <timeslicing.h>

/* Absolute tick counter: ticks since boot. */
ସ୍ଥିର uint64_t curr_tick;

/*
 * The timeout code shall take no locks other than its own (timeout_lock), nor
 * shall it call any other subsystem while holding this lock. Code outside this
 * file takes it through sys_clock_lock()/sys_clock_unlock().
 */
ସ୍ଥିର ସଂରଚନା k_spinlock timeout_lock;

/* Ticks left to process in the currently-executing sys_clock_announce() */
ସ୍ଥିର uint32_t announce_remaining;

/* CPU id currently inside sys_clock_announce_locked()'s firing loop, or -1
 * when no CPU is. The SMP early-return below ensures at most one CPU is in
 * the loop at a time, so a single int suffices. Used by code that needs to
 * know whether *this* CPU is at a tick edge (announcing) versus somewhere
 * within a tick (any other context, including running on a CPU while a
 * different CPU is the announcer).
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା announcing_cpu = -1;

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool this_cpu_announcing(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ announcing_cpu == CPU_ID;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool any_cpu_announcing(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ announcing_cpu != -1;
}

/* Timeout whose handler is currently being dispatched, or NULL when no
 * handler is in flight. The announcing CPU sets the pointer before
 * calling the handler and clears it afterwards; any aborter may set the
 * low "superseded" bit (struct _timeout is pointer-aligned so bit 0 is
 * free). Accessors below mask the bit.
 */
ସ୍ଥିର ସଂରଚନା _timeout *inflight_timeout;

#define INFLIGHT_SUPERSEDED_BIT 1UL

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *inflight_ptr(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ (ସଂରଚନା _timeout *)((uintptr_t)inflight_timeout & ~INFLIGHT_SUPERSEDED_BIT);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ inflight_mark_superseded(ଶୂନ୍ୟ)
{
	inflight_timeout = (ସଂରଚନା _timeout *)((uintptr_t)inflight_timeout |
					       INFLIGHT_SUPERSEDED_BIT);
}

ସ୍ଥିର uint32_t elapsed(ଶୂନ୍ୟ)
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
	ଯଦି (this_cpu_announcing()) {
		ଫେରନ୍ତୁ 0U;
	}
	ଫେରନ୍ତୁ sys_clock_elapsed() +
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
#ଯଦି defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)
#ଅନ୍ତର୍ଭାଵ "timeout/minheap.h"
#ନଚେତଯଦି defined(CONFIG_TIMEOUT_BACKEND_WHEEL)
#ଅନ୍ତର୍ଭାଵ "timeout/wheel.h"
#ନଚେତଯଦି defined(CONFIG_TIMEOUT_BACKEND_BUCKET)
#ଅନ୍ତର୍ଭାଵ "timeout/bucket.h"
#ନଚେତଯଦି defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)
#ଅନ୍ତର୍ଭାଵ "timeout/skiplist.h"
#ଅନ୍ୟଥା /* CONFIG_TIMEOUT_BACKEND_DLIST */
#ଅନ୍ତର୍ଭାଵ "timeout/list.h"
#endif

/*
 * Ticks the driver may wait before the next sys_clock_announce(). The
 * announce-range cap lives here, in the backend-independent front end, so no
 * backend has to reproduce it: the backend only reports the delta to its
 * earliest pending timeout via z_timeout_q_next_expiry().
 */
ସ୍ଥିର uint32_t next_timeout(uint32_t ticks_elapsed)
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
	ଯଦି (ticks_elapsed >= SYS_CLOCK_MAX_WAIT) {
		ଫେରନ୍ତୁ 0;
	}

	/*
	 * No deadline, or one further out than can be scheduled in a single
	 * step: wait the capped budget and re-evaluate at the next announce.
	 * Testing next (which may be 64-bit) against the cap keeps the
	 * remaining arithmetic in 32 bits. The empty case still returns the
	 * budget so a driver keeps waking to maintain uptime.
	 */
	ଯଦି (next == K_TICKS_FOREVER || next >= SYS_CLOCK_MAX_WAIT) {
		ଫେରନ୍ତୁ SYS_CLOCK_MAX_WAIT - ticks_elapsed;
	}

	/* Otherwise wait until the timeout, relative to now (0 if due). */
	dticks = (uint32_t)next;

	ଫେରନ୍ତୁ (dticks > ticks_elapsed) ? (dticks - ticks_elapsed) : 0;
}

/*
 * Reprogram the timer for the next pending timeout, or, when nothing is
 * pending and CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE tolerates a drifting uptime, tell
 * the driver its clock is unused so it may stop. Only meaningful where the
 * timeout queue may have just drained (abort, end of announce); the add path
 * always has a pending timeout and calls sys_clock_set_timeout() directly.
 */
ସ୍ଥିର ଶୂନ୍ୟ reprogram_next(uint32_t ticks_elapsed)
{
	ଯଦି (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
	    z_timeout_q_next_expiry() == K_TICKS_FOREVER) {
		sys_clock_no_timeout();
	} ଅନ୍ୟଥା {
		sys_clock_set_timeout(next_timeout(ticks_elapsed), false);
	}
}

k_ticks_t z_add_timeout(ସଂରଚନା _timeout *to, _timeout_func_t fn, k_timeout_t timeout)
{
	k_ticks_t ticks = 0;

	ଯଦି (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		ଫେରନ୍ତୁ 0;
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

		ଯଦି (Z_IS_TIMEOUT_RELATIVE(timeout)) {
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
		} ଅନ୍ୟଥା {
			dticks = Z_TICK_ABS(timeout.ticks) - curr_tick;
			dticks = max(1, dticks);
			ticks = timeout.ticks;
		}

		ଯଦି (z_timeout_q_ଭର୍ତ୍ତି(to, dticks) && !any_cpu_announcing()) {
			ଯଦି (!has_elapsed) {
				/* In case of absolute timeout that is first to expire
				 * elapsed need to be read from the system clock.
				 */
				ticks_elapsed = elapsed();
			}
			sys_clock_set_timeout(next_timeout(ticks_elapsed), false);
		}
	}

	ଫେରନ୍ତୁ ticks;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_try_abort_timeout(ସଂରଚନା _timeout *to)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -EINVAL;

	K_SPINLOCK(&timeout_lock) {
		ଯଦି (!z_is_inactive_timeout(to)) {
			bool was_first = z_timeout_q_remove(to);

			ret = 0;
			ଯଦି (was_first) {
				reprogram_next(elapsed());
			}
		} ଅନ୍ୟଥା ଯଦି (IS_ENABLED(CONFIG_SMP) && inflight_ptr() == to &&
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
		ଯଦି (inflight_ptr() == to) {
			inflight_mark_superseded();
		}
	}

	ଯଦି (IS_ENABLED(CONFIG_SMP) && ret == -EAGAIN) {
		arch_spin_relax();
	}

	ଫେରନ୍ତୁ ret;
}

bool z_timeout_inflight_superseded(ଅଚଳ ସଂରଚନା _timeout *to)
{
	bool superseded = false;

	K_SPINLOCK(&timeout_lock) {
		superseded = inflight_timeout ==
			     (ସଂରଚନା _timeout *)((uintptr_t)to | INFLIGHT_SUPERSEDED_BIT);
	}

	ଫେରନ୍ତୁ superseded;
}

k_ticks_t z_timeout_remaining(ଅଚଳ ସଂରଚନା _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_lock) {
		ଯଦି (!z_is_inactive_timeout(timeout)) {
			k_ticks_t rem = z_timeout_q_remainder(timeout);
			uint32_t el = elapsed();

			/* elapsed() can exceed rem when announce is late. */
			ticks = (rem > el) ? (rem - el) : 0;
		}
	}

	ଫେରନ୍ତୁ ticks;
}
EXPORT_ପ୍ରତୀକ(z_timeout_remaining);

k_ticks_t z_timeout_expires(ଅଚଳ ସଂରଚନା _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_lock) {
		ticks = curr_tick;
		ଯଦି (!z_is_inactive_timeout(timeout)) {
			ticks += z_timeout_q_remainder(timeout);
		}
	}

	ଫେରନ୍ତୁ ticks;
}
EXPORT_ପ୍ରତୀକ(z_timeout_expires);

uint32_t z_get_next_timeout_expiry(ଶୂନ୍ୟ)
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
		ଯଦି (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
		    z_timeout_q_next_expiry() == K_TICKS_FOREVER) {
			ret = (uint32_t)K_TICKS_FOREVER;
		} ଅନ୍ୟଥା {
			ret = next_timeout(elapsed());
		}
	}
	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ sys_clock_announce_locked(uint32_t ticks, k_spinlock_ଚାବି_t ଚାବି)
{
	/* We release the lock around the callbacks below, so on SMP
	 * systems someone might be already running the loop.  Don't
	 * race (which will cause parallel execution of "sequential"
	 * timeouts and confuse apps), just increment the tick count
	 * and return.
	 */
	ଯଦି (IS_ENABLED(CONFIG_SMP) && any_cpu_announcing()) {
		announce_remaining += ticks;
		k_spin_unlock(&timeout_lock, ଚାବି);
		ଫେରନ୍ତୁ;
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
	ଚାବି = z_timeout_q_announce(ଚାବି);
#ଅନ୍ୟଥା
	ଯେତେବେଳେ (announce_remaining > 0) {
		ସଂରଚନା _timeout *t;
		int32_t dt = z_timeout_q_next_gap();

		ଯଦି ((uint32_t)dt > announce_remaining) {
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
		ଯେତେବେଳେ ((t = z_timeout_q_କାଢ଼_due()) != ଖାଲି) {
			_timeout_func_t handler = t->fn;

			inflight_timeout = t;

			k_spin_unlock(&timeout_lock, ଚାବି);
			handler(t);
			ଚାବି = k_spin_lock(&timeout_lock);
			inflight_timeout = ଖାଲି;
		}
	}
#endif /* _TIMEOUT_BACKEND_OWNS_ANNOUNCE */

	announcing_cpu = -1;

	reprogram_next(0);

	k_spin_unlock(&timeout_lock, ଚାବି);

#ifdef CONFIG_TIMESLICING
	z_time_slice();
#endif /* CONFIG_TIMESLICING */
}

#ଯଦି defined(CONFIG_SMP) || defined(CONFIG_SPIN_VALIDATE)
k_spinlock_ଚାବି_t sys_clock_lock(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ k_spin_lock(&timeout_lock);
}

ଶୂନ୍ୟ sys_clock_unlock(k_spinlock_ଚାବି_t ଚାବି)
{
	k_spin_unlock(&timeout_lock, ଚାବି);
}
#endif

#ଯଦି defined(CONFIG_TEST) || defined(CONFIG_ASSERT)
bool sys_clock_is_locked(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ z_spin_is_locked(&timeout_lock);
}
#endif

int64_t sys_clock_tick_get(ଶୂନ୍ୟ)
{
	uint64_t t = 0U;

	K_SPINLOCK(&timeout_lock) {
		t = curr_tick + elapsed();
	}
	ଫେରନ୍ତୁ t;
}

uint32_t sys_clock_tick_get_32(ଶୂନ୍ୟ)
{
#ifdef CONFIG_TICKLESS_KERNEL
	ଫେରନ୍ତୁ (uint32_t)sys_clock_tick_get();
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ (uint32_t)curr_tick;
#endif /* CONFIG_TICKLESS_KERNEL */
}

int64_t z_impl_k_uptime_ticks(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ sys_clock_tick_get();
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ int64_t z_vrfy_k_uptime_ticks(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ z_impl_k_uptime_ticks();
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_uptime_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
	k_timepoint_t timepoint;

	ଯଦି (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		timepoint.tick = UINT64_MAX;
	} ଅନ୍ୟଥା ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		timepoint.tick = 0;
	} ଅନ୍ୟଥା {
		k_ticks_t dt = timeout.ticks;

		ଯଦି (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			timepoint.tick = sys_clock_tick_get() + max(1, dt);
		} ଅନ୍ୟଥା {
			timepoint.tick = Z_TICK_ABS(dt);
		}
	}

	ଫେରନ୍ତୁ timepoint;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
	uint64_t now, remaining;

	ଯଦି (timepoint.tick == UINT64_MAX) {
		ଫେରନ୍ତୁ K_FOREVER;
	}
	ଯଦି (timepoint.tick == 0) {
		ଫେରନ୍ତୁ K_NO_WAIT;
	}

	now = sys_clock_tick_get();
	remaining = (timepoint.tick > now) ? (timepoint.tick - now) : 0;
	ଫେରନ୍ତୁ K_TICKS(remaining);
}

#ifdef CONFIG_ZTEST
ଶୂନ୍ୟ z_impl_sys_clock_tick_set(uint64_t tick)
{
	curr_tick = tick;
}

ଶୂନ୍ୟ z_vrfy_sys_clock_tick_set(uint64_t tick)
{
	z_impl_sys_clock_tick_set(tick);
}
#endif /* CONFIG_ZTEST */
