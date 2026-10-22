/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/sys/minmax.h>
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <timeout_q.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/drivers/timer/system_timer.h>
#అన్తర్భావ <zephyr/sys/clock.h>
#అన్తర్భావ <zephyr/llext/symbol.h>

#అన్తర్భావ <timeslicing.h>

/* Absolute tick counter: ticks since boot. */
స్థిర uint64_t curr_tick;

/*
 * The timeout code shall take no locks other than its own (timeout_lock), nor
 * shall it call any other subsystem while holding this lock. Code outside this
 * file takes it through sys_clock_lock()/sys_clock_unlock().
 */
స్థిర నిర్మాణం k_spinlock timeout_తాళం;

/* Ticks left to process in the currently-executing sys_clock_announce() */
స్థిర uint32_t announce_మిగిలిన;

/* CPU id currently inside sys_clock_announce_locked()'s firing loop, or -1
 * when no CPU is. The SMP early-return below ensures at most one CPU is in
 * the loop at a time, so a single int suffices. Used by code that needs to
 * know whether *this* CPU is at a tick edge (announcing) versus somewhere
 * within a tick (any other context, including running on a CPU while a
 * different CPU is the announcer).
 */
స్థిర పూర్ణాంకం announcing_cpu = -1;

స్థిర అంతఃవరుస bool this_cpu_announcing(శూన్య)
{
	తిరిగి announcing_cpu == CPU_ID;
}

స్థిర అంతఃవరుస bool any_cpu_announcing(శూన్య)
{
	తిరిగి announcing_cpu != -1;
}

/* Timeout whose handler is currently being dispatched, or NULL when no
 * handler is in flight. The announcing CPU sets the pointer before
 * calling the handler and clears it afterwards; any aborter may set the
 * low "superseded" bit (struct _timeout is pointer-aligned so bit 0 is
 * free). Accessors below mask the bit.
 */
స్థిర నిర్మాణం _timeout *inflight_timeout;

#నిర్వచించు INFLIGHT_SUPERSEDED_బిట్టు 1UL

స్థిర అంతఃవరుస నిర్మాణం _timeout *inflight_ptr(శూన్య)
{
	తిరిగి (నిర్మాణం _timeout *)((uintptr_t)inflight_timeout & ~INFLIGHT_SUPERSEDED_బిట్టు);
}

స్థిర అంతఃవరుస శూన్య inflight_mark_superseded(శూన్య)
{
	inflight_timeout = (నిర్మాణం _timeout *)((uintptr_t)inflight_timeout |
					       INFLIGHT_SUPERSEDED_బిట్టు);
}

స్థిర uint32_t elapsed(శూన్య)
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
	ఒకవేళ (this_cpu_announcing()) {
		తిరిగి 0U;
	}
	తిరిగి sys_clock_elapsed() +
	       (IS_ENABLED(CONFIG_SMP) ? announce_మిగిలిన : 0);
}

/*
 * Backend queue implementation. The selected backend's queue instance and its
 * z_timeout_q_*() operations are private to this file: the backend is a header
 * included only here, after the shared state above, so those operations can
 * reach curr_tick / announce_remaining / inflight_timeout / timeout_lock
 * directly. The per-node helpers (z_init_timeout / z_is_inactive_timeout) are
 * tree-wide and live in timeout_q.h.
 */
#ఒకవేళ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)
#అన్తర్భావ "timeout/minheap.h"
#లేదావేళ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)
#అన్తర్భావ "timeout/wheel.h"
#లేదావేళ defined(CONFIG_TIMEOUT_BACKEND_పాత్ర)
#అన్తర్భావ "timeout/bucket.h"
#లేదావేళ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)
#అన్తర్భావ "timeout/skiplist.h"
#లేకపోతే /* CONFIG_TIMEOUT_BACKEND_DLIST */
#అన్తర్భావ "timeout/list.h"
#అయితే_ముగింపు

/*
 * Ticks the driver may wait before the next sys_clock_announce(). The
 * announce-range cap lives here, in the backend-independent front end, so no
 * backend has to reproduce it: the backend only reports the delta to its
 * earliest pending timeout via z_timeout_q_next_expiry().
 */
స్థిర uint32_t తదుపరి_timeout(uint32_t ticks_elapsed)
{
	k_ticks_t తదుపరి = z_timeout_q_తదుపరి_expiry();
	uint32_t dticks;

	/*
	 * sys_clock_announce() reports the ticks elapsed since the previous
	 * announce, so the gap between two announces must not exceed
	 * SYS_CLOCK_MAX_WAIT for the announced count to fit. The budget left
	 * from now on is SYS_CLOCK_MAX_WAIT - ticks_elapsed; if it is already
	 * spent ask for an announce right away.
	 */
	ఒకవేళ (ticks_elapsed >= SYS_CLOCK_MAX_WAIT) {
		తిరిగి 0;
	}

	/*
	 * No deadline, or one further out than can be scheduled in a single
	 * step: wait the capped budget and re-evaluate at the next announce.
	 * Testing next (which may be 64-bit) against the cap keeps the
	 * remaining arithmetic in 32 bits. The empty case still returns the
	 * budget so a driver keeps waking to maintain uptime.
	 */
	ఒకవేళ (తదుపరి == K_TICKS_FOREVER || తదుపరి >= SYS_CLOCK_MAX_WAIT) {
		తిరిగి SYS_CLOCK_MAX_WAIT - ticks_elapsed;
	}

	/* Otherwise wait until the timeout, relative to now (0 if due). */
	dticks = (uint32_t)తదుపరి;

	తిరిగి (dticks > ticks_elapsed) ? (dticks - ticks_elapsed) : 0;
}

/*
 * Reprogram the timer for the next pending timeout, or, when nothing is
 * pending and CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE tolerates a drifting uptime, tell
 * the driver its clock is unused so it may stop. Only meaningful where the
 * timeout queue may have just drained (abort, end of announce); the add path
 * always has a pending timeout and calls sys_clock_set_timeout() directly.
 */
స్థిర శూన్య reprogram_తదుపరి(uint32_t ticks_elapsed)
{
	ఒకవేళ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
	    z_timeout_q_తదుపరి_expiry() == K_TICKS_FOREVER) {
		sys_clock_no_timeout();
	} లేకపోతే {
		sys_clock_సమితి_timeout(తదుపరి_timeout(ticks_elapsed), false);
	}
}

k_ticks_t z_కూడు_timeout(నిర్మాణం _timeout *గమ్యం, _timeout_func_t fn, k_timeout_t timeout)
{
	k_ticks_t ticks = 0;

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		తిరిగి 0;
	}

#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(గమ్యం));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */

	__ASSERT(z_is_inactive_timeout(గమ్యం), "");
	గమ్యం->fn = fn;

	K_SPINLOCK(&timeout_తాళం) {
		uint32_t ticks_elapsed = 0;
		bool has_elapsed = false;
		k_ticks_t dticks;

		ఒకవేళ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
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
		} లేకపోతే {
			dticks = Z_TICK_ABS(timeout.ticks) - curr_tick;
			dticks = max(1, dticks);
			ticks = timeout.ticks;
		}

		ఒకవేళ (z_timeout_q_చొప్పించు(గమ్యం, dticks) && !any_cpu_announcing()) {
			ఒకవేళ (!has_elapsed) {
				/* In case of absolute timeout that is first to expire
				 * elapsed need to be read from the system clock.
				 */
				ticks_elapsed = elapsed();
			}
			sys_clock_సమితి_timeout(తదుపరి_timeout(ticks_elapsed), false);
		}
	}

	తిరిగి ticks;
}

పూర్ణాంకం z_try_abort_timeout(నిర్మాణం _timeout *గమ్యం)
{
	పూర్ణాంకం ret = -EINVAL;

	K_SPINLOCK(&timeout_తాళం) {
		ఒకవేళ (!z_is_inactive_timeout(గమ్యం)) {
			bool was_మొదటి = z_timeout_q_remove(గమ్యం);

			ret = 0;
			ఒకవేళ (was_మొదటి) {
				reprogram_తదుపరి(elapsed());
			}
		} లేకపోతే ఒకవేళ (IS_ENABLED(CONFIG_SMP) && inflight_ptr() == గమ్యం &&
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
		ఒకవేళ (inflight_ptr() == గమ్యం) {
			inflight_mark_superseded();
		}
	}

	ఒకవేళ (IS_ENABLED(CONFIG_SMP) && ret == -EAGAIN) {
		arch_spin_relax();
	}

	తిరిగి ret;
}

bool z_timeout_inflight_superseded(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	bool superseded = false;

	K_SPINLOCK(&timeout_తాళం) {
		superseded = inflight_timeout ==
			     (నిర్మాణం _timeout *)((uintptr_t)గమ్యం | INFLIGHT_SUPERSEDED_బిట్టు);
	}

	తిరిగి superseded;
}

k_ticks_t z_timeout_మిగిలిన(స్థిరరాశి నిర్మాణం _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_తాళం) {
		ఒకవేళ (!z_is_inactive_timeout(timeout)) {
			k_ticks_t rem = z_timeout_q_remainder(timeout);
			uint32_t el = elapsed();

			/* elapsed() can exceed rem when announce is late. */
			ticks = (rem > el) ? (rem - el) : 0;
		}
	}

	తిరిగి ticks;
}
EXPORT_చిహ్నం(z_timeout_మిగిలిన);

k_ticks_t z_timeout_expires(స్థిరరాశి నిర్మాణం _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_తాళం) {
		ticks = curr_tick;
		ఒకవేళ (!z_is_inactive_timeout(timeout)) {
			ticks += z_timeout_q_remainder(timeout);
		}
	}

	తిరిగి ticks;
}
EXPORT_చిహ్నం(z_timeout_expires);

uint32_t z_get_తదుపరి_timeout_expiry(శూన్య)
{
	uint32_t ret = (uint32_t)K_TICKS_FOREVER;

	K_SPINLOCK(&timeout_తాళం) {
		/*
		 * Same decision as reprogram_next(). Sloppy idle lets the
		 * uptime drift, so an empty list means nothing to wake up for
		 * and that is reported as K_TICKS_FOREVER. Otherwise the
		 * answer is a wait: either a real deadline, or the synthetic
		 * one that keeps the announce range covered.
		 */
		ఒకవేళ (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
		    z_timeout_q_తదుపరి_expiry() == K_TICKS_FOREVER) {
			ret = (uint32_t)K_TICKS_FOREVER;
		} లేకపోతే {
			ret = తదుపరి_timeout(elapsed());
		}
	}
	తిరిగి ret;
}

శూన్య sys_clock_announce_locked(uint32_t ticks, k_spinlock_కీలకం_t కీలకం)
{
	/* We release the lock around the callbacks below, so on SMP
	 * systems someone might be already running the loop.  Don't
	 * race (which will cause parallel execution of "sequential"
	 * timeouts and confuse apps), just increment the tick count
	 * and return.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_SMP) && any_cpu_announcing()) {
		announce_మిగిలిన += ticks;
		k_spin_unlock(&timeout_తాళం, కీలకం);
		తిరిగి;
	}

	announce_మిగిలిన = ticks;
	announcing_cpu = CPU_ID;

#నిర్వచితమైతే _TIMEOUT_BACKEND_OWNS_ANNOUNCE
	/* The backend (timer wheel) runs the firing loop itself: its per-tick
	 * advance is a bitmap jump over empty ticks and its sift is a
	 * time-driven event with no single timeout to drive a generic loop.
	 * It advances curr_tick / announce_remaining and fires handlers via
	 * the same inflight_timeout dance as below.
	 */
	కీలకం = z_timeout_q_announce(కీలకం);
#లేకపోతే
	అయితే (announce_మిగిలిన > 0) {
		నిర్మాణం _timeout *t;
		int32_t dt = z_timeout_q_తదుపరి_gap();

		ఒకవేళ ((uint32_t)dt > announce_మిగిలిన) {
			/* Next event is past this announce window: advance the
			 * backend by the residual ticks without firing anything.
			 */
			dt = (int32_t)announce_మిగిలిన;
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
		announce_మిగిలిన -= dt;

		/* Drain everything due at the new curr_tick. */
		అయితే ((t = z_timeout_q_తీయి_due()) != ఖాళీ) {
			_timeout_func_t handler = t->fn;

			inflight_timeout = t;

			k_spin_unlock(&timeout_తాళం, కీలకం);
			handler(t);
			కీలకం = k_spin_తాళం(&timeout_తాళం);
			inflight_timeout = ఖాళీ;
		}
	}
#అయితే_ముగింపు /* _TIMEOUT_BACKEND_OWNS_ANNOUNCE */

	announcing_cpu = -1;

	reprogram_తదుపరి(0);

	k_spin_unlock(&timeout_తాళం, కీలకం);

#నిర్వచితమైతే CONFIG_TIMESLICING
	z_సమయం_ముక్క();
#అయితే_ముగింపు /* CONFIG_TIMESLICING */
}

#ఒకవేళ defined(CONFIG_SMP) || defined(CONFIG_SPIN_VALIDATE)
k_spinlock_కీలకం_t sys_clock_తాళం(శూన్య)
{
	తిరిగి k_spin_తాళం(&timeout_తాళం);
}

శూన్య sys_clock_unlock(k_spinlock_కీలకం_t కీలకం)
{
	k_spin_unlock(&timeout_తాళం, కీలకం);
}
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_TEST) || defined(CONFIG_ASSERT)
bool sys_clock_is_locked(శూన్య)
{
	తిరిగి z_spin_is_locked(&timeout_తాళం);
}
#అయితే_ముగింపు

int64_t sys_clock_tick_get(శూన్య)
{
	uint64_t t = 0U;

	K_SPINLOCK(&timeout_తాళం) {
		t = curr_tick + elapsed();
	}
	తిరిగి t;
}

uint32_t sys_clock_tick_get_32(శూన్య)
{
#నిర్వచితమైతే CONFIG_TICKLESS_KERNEL
	తిరిగి (uint32_t)sys_clock_tick_get();
#లేకపోతే
	తిరిగి (uint32_t)curr_tick;
#అయితే_ముగింపు /* CONFIG_TICKLESS_KERNEL */
}

int64_t z_impl_k_uptime_ticks(శూన్య)
{
	తిరిగి sys_clock_tick_get();
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస int64_t z_vrfy_k_uptime_ticks(శూన్య)
{
	తిరిగి z_impl_k_uptime_ticks();
}
#అన్తర్భావ <zephyr/syscalls/k_uptime_ticks_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
	k_timepoint_t timepoint;

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		timepoint.tick = UINT64_MAX;
	} లేకపోతే ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		timepoint.tick = 0;
	} లేకపోతే {
		k_ticks_t dt = timeout.ticks;

		ఒకవేళ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			timepoint.tick = sys_clock_tick_get() + max(1, dt);
		} లేకపోతే {
			timepoint.tick = Z_TICK_ABS(dt);
		}
	}

	తిరిగి timepoint;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
	uint64_t now, మిగిలిన;

	ఒకవేళ (timepoint.tick == UINT64_MAX) {
		తిరిగి K_FOREVER;
	}
	ఒకవేళ (timepoint.tick == 0) {
		తిరిగి K_NO_WAIT;
	}

	now = sys_clock_tick_get();
	మిగిలిన = (timepoint.tick > now) ? (timepoint.tick - now) : 0;
	తిరిగి K_TICKS(మిగిలిన);
}

#నిర్వచితమైతే CONFIG_ZTEST
శూన్య z_impl_sys_clock_tick_సమితి(uint64_t tick)
{
	curr_tick = tick;
}

శూన్య z_vrfy_sys_clock_tick_సమితి(uint64_t tick)
{
	z_impl_sys_clock_tick_సమితి(tick);
}
#అయితే_ముగింపు /* CONFIG_ZTEST */
