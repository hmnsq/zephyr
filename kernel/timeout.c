/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/sys/minmax.h>
#شامل <zephyr/kernel.h>
#شامل <zephyr/spinlock.h>
#شامل <ksched.h>
#شامل <timeout_q.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/drivers/timer/system_timer.h>
#شامل <zephyr/sys/clock.h>
#شامل <zephyr/llext/symbol.h>

#شامل <timeslicing.h>

/* Absolute tick counter: ticks since boot. */
ساکن uint64_t curr_tick;

/*
 * The timeout code shall take no locks other than its own (timeout_lock), nor
 * shall it call any other subsystem while holding this lock. Code outside this
 * file takes it through sys_clock_lock()/sys_clock_unlock().
 */
ساکن ساخت k_spinlock timeout_تالا;

/* Ticks left to process in the currently-executing sys_clock_announce() */
ساکن uint32_t announce_بقایا;

/* CPU id currently inside sys_clock_announce_locked()'s firing loop, or -1
 * when no CPU is. The SMP early-return below ensures at most one CPU is in
 * the loop at a time, so a single int suffices. Used by code that needs to
 * know whether *this* CPU is at a tick edge (announcing) versus somewhere
 * within a tick (any other context, including running on a CPU while a
 * different CPU is the announcer).
 */
ساکن صحیح announcing_cpu = -1;

ساکن اندرونی bool this_cpu_announcing(خالی)
{
	واپس announcing_cpu == CPU_ID;
}

ساکن اندرونی bool any_cpu_announcing(خالی)
{
	واپس announcing_cpu != -1;
}

/* Timeout whose handler is currently being dispatched, or NULL when no
 * handler is in flight. The announcing CPU sets the pointer before
 * calling the handler and clears it afterwards; any aborter may set the
 * low "superseded" bit (struct _timeout is pointer-aligned so bit 0 is
 * free). Accessors below mask the bit.
 */
ساکن ساخت _timeout *inflight_timeout;

#تعریف_کرو INFLIGHT_SUPERSEDED_ثنائی_ہندسہ 1UL

ساکن اندرونی ساخت _timeout *inflight_ptr(خالی)
{
	واپس (ساخت _timeout *)((uintptr_t)inflight_timeout & ~INFLIGHT_SUPERSEDED_ثنائی_ہندسہ);
}

ساکن اندرونی خالی inflight_mark_superseded(خالی)
{
	inflight_timeout = (ساخت _timeout *)((uintptr_t)inflight_timeout |
					       INFLIGHT_SUPERSEDED_ثنائی_ہندسہ);
}

ساکن uint32_t elapsed(خالی)
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
	اگر (this_cpu_announcing()) {
		واپس 0U;
	}
	واپس sys_clock_elapsed() +
	       (IS_ENABLED(CONFIG_SMP) ? announce_بقایا : 0);
}

/*
 * Backend queue implementation. The selected backend's queue instance and its
 * z_timeout_q_*() operations are private to this file: the backend is a header
 * included only here, after the shared state above, so those operations can
 * reach curr_tick / announce_remaining / inflight_timeout / timeout_lock
 * directly. The per-node helpers (z_init_timeout / z_is_inactive_timeout) are
 * tree-wide and live in timeout_q.h.
 */
#اگر defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)
#شامل "timeout/minheap.h"
#وگرنہ_اگر defined(CONFIG_TIMEOUT_BACKEND_WHEEL)
#شامل "timeout/wheel.h"
#وگرنہ_اگر defined(CONFIG_TIMEOUT_BACKEND_بالٹی)
#شامل "timeout/bucket.h"
#وگرنہ_اگر defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)
#شامل "timeout/skiplist.h"
#ورنہ /* CONFIG_TIMEOUT_BACKEND_DLIST */
#شامل "timeout/list.h"
#اگر_ختم

/*
 * Ticks the driver may wait before the next sys_clock_announce(). The
 * announce-range cap lives here, in the backend-independent front end, so no
 * backend has to reproduce it: the backend only reports the delta to its
 * earliest pending timeout via z_timeout_q_next_expiry().
 */
ساکن uint32_t اگلا_timeout(uint32_t ticks_elapsed)
{
	k_ticks_t اگلا = z_timeout_q_اگلا_expiry();
	uint32_t dticks;

	/*
	 * sys_clock_announce() reports the ticks elapsed since the previous
	 * announce, so the gap between two announces must not exceed
	 * SYS_CLOCK_MAX_WAIT for the announced count to fit. The budget left
	 * from now on is SYS_CLOCK_MAX_WAIT - ticks_elapsed; if it is already
	 * spent ask for an announce right away.
	 */
	اگر (ticks_elapsed >= SYS_CLOCK_MAX_WAIT) {
		واپس 0;
	}

	/*
	 * No deadline, or one further out than can be scheduled in a single
	 * step: wait the capped budget and re-evaluate at the next announce.
	 * Testing next (which may be 64-bit) against the cap keeps the
	 * remaining arithmetic in 32 bits. The empty case still returns the
	 * budget so a driver keeps waking to maintain uptime.
	 */
	اگر (اگلا == K_TICKS_FOREVER || اگلا >= SYS_CLOCK_MAX_WAIT) {
		واپس SYS_CLOCK_MAX_WAIT - ticks_elapsed;
	}

	/* Otherwise wait until the timeout, relative to now (0 if due). */
	dticks = (uint32_t)اگلا;

	واپس (dticks > ticks_elapsed) ? (dticks - ticks_elapsed) : 0;
}

/*
 * Reprogram the timer for the next pending timeout, or, when nothing is
 * pending and CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE tolerates a drifting uptime, tell
 * the driver its clock is unused so it may stop. Only meaningful where the
 * timeout queue may have just drained (abort, end of announce); the add path
 * always has a pending timeout and calls sys_clock_set_timeout() directly.
 */
ساکن خالی reprogram_اگلا(uint32_t ticks_elapsed)
{
	اگر (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
	    z_timeout_q_اگلا_expiry() == K_TICKS_FOREVER) {
		sys_clock_no_timeout();
	} ورنہ {
		sys_clock_سیٹ_timeout(اگلا_timeout(ticks_elapsed), false);
	}
}

k_ticks_t z_جوڑو_timeout(ساخت _timeout *مقصد, _timeout_func_t fn, k_timeout_t timeout)
{
	k_ticks_t ticks = 0;

	اگر (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		واپس 0;
	}

#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(مقصد));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */

	__ASSERT(z_is_inactive_timeout(مقصد), "");
	مقصد->fn = fn;

	K_SPINLOCK(&timeout_تالا) {
		uint32_t ticks_elapsed = 0;
		bool has_elapsed = false;
		k_ticks_t dticks;

		اگر (Z_IS_TIMEOUT_RELATIVE(timeout)) {
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
		} ورنہ {
			dticks = Z_TICK_ABS(timeout.ticks) - curr_tick;
			dticks = max(1, dticks);
			ticks = timeout.ticks;
		}

		اگر (z_timeout_q_داخل_کرو(مقصد, dticks) && !any_cpu_announcing()) {
			اگر (!has_elapsed) {
				/* In case of absolute timeout that is first to expire
				 * elapsed need to be read from the system clock.
				 */
				ticks_elapsed = elapsed();
			}
			sys_clock_سیٹ_timeout(اگلا_timeout(ticks_elapsed), false);
		}
	}

	واپس ticks;
}

صحیح z_try_abort_timeout(ساخت _timeout *مقصد)
{
	صحیح ret = -EINVAL;

	K_SPINLOCK(&timeout_تالا) {
		اگر (!z_is_inactive_timeout(مقصد)) {
			bool was_پہلا = z_timeout_q_remove(مقصد);

			ret = 0;
			اگر (was_پہلا) {
				reprogram_اگلا(elapsed());
			}
		} ورنہ اگر (IS_ENABLED(CONFIG_SMP) && inflight_ptr() == مقصد &&
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
		اگر (inflight_ptr() == مقصد) {
			inflight_mark_superseded();
		}
	}

	اگر (IS_ENABLED(CONFIG_SMP) && ret == -EAGAIN) {
		arch_spin_relax();
	}

	واپس ret;
}

bool z_timeout_inflight_superseded(ثابت ساخت _timeout *مقصد)
{
	bool superseded = false;

	K_SPINLOCK(&timeout_تالا) {
		superseded = inflight_timeout ==
			     (ساخت _timeout *)((uintptr_t)مقصد | INFLIGHT_SUPERSEDED_ثنائی_ہندسہ);
	}

	واپس superseded;
}

k_ticks_t z_timeout_بقایا(ثابت ساخت _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_تالا) {
		اگر (!z_is_inactive_timeout(timeout)) {
			k_ticks_t rem = z_timeout_q_remainder(timeout);
			uint32_t el = elapsed();

			/* elapsed() can exceed rem when announce is late. */
			ticks = (rem > el) ? (rem - el) : 0;
		}
	}

	واپس ticks;
}
EXPORT_علامت(z_timeout_بقایا);

k_ticks_t z_timeout_expires(ثابت ساخت _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_تالا) {
		ticks = curr_tick;
		اگر (!z_is_inactive_timeout(timeout)) {
			ticks += z_timeout_q_remainder(timeout);
		}
	}

	واپس ticks;
}
EXPORT_علامت(z_timeout_expires);

uint32_t z_get_اگلا_timeout_expiry(خالی)
{
	uint32_t ret = (uint32_t)K_TICKS_FOREVER;

	K_SPINLOCK(&timeout_تالا) {
		/*
		 * Same decision as reprogram_next(). Sloppy idle lets the
		 * uptime drift, so an empty list means nothing to wake up for
		 * and that is reported as K_TICKS_FOREVER. Otherwise the
		 * answer is a wait: either a real deadline, or the synthetic
		 * one that keeps the announce range covered.
		 */
		اگر (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
		    z_timeout_q_اگلا_expiry() == K_TICKS_FOREVER) {
			ret = (uint32_t)K_TICKS_FOREVER;
		} ورنہ {
			ret = اگلا_timeout(elapsed());
		}
	}
	واپس ret;
}

خالی sys_clock_announce_locked(uint32_t ticks, k_spinlock_کلید_t کلید)
{
	/* We release the lock around the callbacks below, so on SMP
	 * systems someone might be already running the loop.  Don't
	 * race (which will cause parallel execution of "sequential"
	 * timeouts and confuse apps), just increment the tick count
	 * and return.
	 */
	اگر (IS_ENABLED(CONFIG_SMP) && any_cpu_announcing()) {
		announce_بقایا += ticks;
		k_spin_unlock(&timeout_تالا, کلید);
		واپس;
	}

	announce_بقایا = ticks;
	announcing_cpu = CPU_ID;

#اگر_تعریف_شدہ _TIMEOUT_BACKEND_OWNS_ANNOUNCE
	/* The backend (timer wheel) runs the firing loop itself: its per-tick
	 * advance is a bitmap jump over empty ticks and its sift is a
	 * time-driven event with no single timeout to drive a generic loop.
	 * It advances curr_tick / announce_remaining and fires handlers via
	 * the same inflight_timeout dance as below.
	 */
	کلید = z_timeout_q_announce(کلید);
#ورنہ
	جب (announce_بقایا > 0) {
		ساخت _timeout *t;
		int32_t dt = z_timeout_q_اگلا_gap();

		اگر ((uint32_t)dt > announce_بقایا) {
			/* Next event is past this announce window: advance the
			 * backend by the residual ticks without firing anything.
			 */
			dt = (int32_t)announce_بقایا;
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
		announce_بقایا -= dt;

		/* Drain everything due at the new curr_tick. */
		جب ((t = z_timeout_q_نکالو_due()) != تہی) {
			_timeout_func_t handler = t->fn;

			inflight_timeout = t;

			k_spin_unlock(&timeout_تالا, کلید);
			handler(t);
			کلید = k_spin_تالا(&timeout_تالا);
			inflight_timeout = تہی;
		}
	}
#اگر_ختم /* _TIMEOUT_BACKEND_OWNS_ANNOUNCE */

	announcing_cpu = -1;

	reprogram_اگلا(0);

	k_spin_unlock(&timeout_تالا, کلید);

#اگر_تعریف_شدہ CONFIG_TIMESLICING
	z_وقت_قاش();
#اگر_ختم /* CONFIG_TIMESLICING */
}

#اگر defined(CONFIG_SMP) || defined(CONFIG_SPIN_VALIDATE)
k_spinlock_کلید_t sys_clock_تالا(خالی)
{
	واپس k_spin_تالا(&timeout_تالا);
}

خالی sys_clock_unlock(k_spinlock_کلید_t کلید)
{
	k_spin_unlock(&timeout_تالا, کلید);
}
#اگر_ختم

#اگر defined(CONFIG_TEST) || defined(CONFIG_ASSERT)
bool sys_clock_is_locked(خالی)
{
	واپس z_spin_is_locked(&timeout_تالا);
}
#اگر_ختم

int64_t sys_clock_tick_get(خالی)
{
	uint64_t t = 0U;

	K_SPINLOCK(&timeout_تالا) {
		t = curr_tick + elapsed();
	}
	واپس t;
}

uint32_t sys_clock_tick_get_32(خالی)
{
#اگر_تعریف_شدہ CONFIG_TICKLESS_KERNEL
	واپس (uint32_t)sys_clock_tick_get();
#ورنہ
	واپس (uint32_t)curr_tick;
#اگر_ختم /* CONFIG_TICKLESS_KERNEL */
}

int64_t z_impl_k_uptime_ticks(خالی)
{
	واپس sys_clock_tick_get();
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی int64_t z_vrfy_k_uptime_ticks(خالی)
{
	واپس z_impl_k_uptime_ticks();
}
#شامل <zephyr/syscalls/k_uptime_ticks_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
	k_timepoint_t timepoint;

	اگر (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		timepoint.tick = UINT64_MAX;
	} ورنہ اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		timepoint.tick = 0;
	} ورنہ {
		k_ticks_t dt = timeout.ticks;

		اگر (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			timepoint.tick = sys_clock_tick_get() + max(1, dt);
		} ورنہ {
			timepoint.tick = Z_TICK_ABS(dt);
		}
	}

	واپس timepoint;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
	uint64_t now, بقایا;

	اگر (timepoint.tick == UINT64_MAX) {
		واپس K_FOREVER;
	}
	اگر (timepoint.tick == 0) {
		واپس K_NO_WAIT;
	}

	now = sys_clock_tick_get();
	بقایا = (timepoint.tick > now) ? (timepoint.tick - now) : 0;
	واپس K_TICKS(بقایا);
}

#اگر_تعریف_شدہ CONFIG_ZTEST
خالی z_impl_sys_clock_tick_سیٹ(uint64_t tick)
{
	curr_tick = tick;
}

خالی z_vrfy_sys_clock_tick_سیٹ(uint64_t tick)
{
	z_impl_sys_clock_tick_سیٹ(tick);
}
#اگر_ختم /* CONFIG_ZTEST */
