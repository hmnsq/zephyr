/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/sys/minmax.h>
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/spinlock.h>
#समावेश <ksched.h>
#समावेश <timeout_q.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/drivers/timer/system_timer.h>
#समावेश <zephyr/sys/clock.h>
#समावेश <zephyr/llext/symbol.h>

#समावेश <timeslicing.h>

/* Absolute tick counter: ticks since boot. */
अटल uint64_t curr_tick;

/*
 * The timeout code shall take no locks other than its own (timeout_lock), nor
 * shall it call any other subsystem while holding this lock. Code outside this
 * file takes it through sys_clock_lock()/sys_clock_unlock().
 */
अटल काष्ठा k_spinlock timeout_lock;

/* Ticks left to process in the currently-executing sys_clock_announce() */
अटल uint32_t announce_remaining;

/* CPU id currently inside sys_clock_announce_locked()'s firing loop, or -1
 * when no CPU is. The SMP early-return below ensures at most one CPU is in
 * the loop at a time, so a single int suffices. Used by code that needs to
 * know whether *this* CPU is at a tick edge (announcing) versus somewhere
 * within a tick (any other context, including running on a CPU while a
 * different CPU is the announcer).
 */
अटल पूर्णांक announcing_cpu = -1;

अटल अंतरभूत bool this_cpu_announcing(व्योम)
{
	वापस announcing_cpu == CPU_ID;
}

अटल अंतरभूत bool any_cpu_announcing(व्योम)
{
	वापस announcing_cpu != -1;
}

/* Timeout whose handler is currently being dispatched, or NULL when no
 * handler is in flight. The announcing CPU sets the pointer before
 * calling the handler and clears it afterwards; any aborter may set the
 * low "superseded" bit (struct _timeout is pointer-aligned so bit 0 is
 * free). Accessors below mask the bit.
 */
अटल काष्ठा _timeout *inflight_timeout;

#घोषणा INFLIGHT_SUPERSEDED_BIT 1UL

अटल अंतरभूत काष्ठा _timeout *inflight_ptr(व्योम)
{
	वापस (काष्ठा _timeout *)((uintptr_t)inflight_timeout & ~INFLIGHT_SUPERSEDED_BIT);
}

अटल अंतरभूत व्योम inflight_mark_superseded(व्योम)
{
	inflight_timeout = (काष्ठा _timeout *)((uintptr_t)inflight_timeout |
					       INFLIGHT_SUPERSEDED_BIT);
}

अटल uint32_t elapsed(व्योम)
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
	अगर (this_cpu_announcing()) {
		वापस 0U;
	}
	वापस sys_clock_elapsed() +
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
#अगर defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)
#समावेश "timeout/minheap.h"
#या_अगर defined(CONFIG_TIMEOUT_BACKEND_WHEEL)
#समावेश "timeout/wheel.h"
#या_अगर defined(CONFIG_TIMEOUT_BACKEND_BUCKET)
#समावेश "timeout/bucket.h"
#या_अगर defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)
#समावेश "timeout/skiplist.h"
#अन्यथा /* CONFIG_TIMEOUT_BACKEND_DLIST */
#समावेश "timeout/list.h"
#पूर्ण_अगर

/*
 * Ticks the driver may wait before the next sys_clock_announce(). The
 * announce-range cap lives here, in the backend-independent front end, so no
 * backend has to reproduce it: the backend only reports the delta to its
 * earliest pending timeout via z_timeout_q_next_expiry().
 */
अटल uint32_t अग्र_timeout(uint32_t ticks_elapsed)
{
	k_ticks_t अग्र = z_timeout_q_अग्र_expiry();
	uint32_t dticks;

	/*
	 * sys_clock_announce() reports the ticks elapsed since the previous
	 * announce, so the gap between two announces must not exceed
	 * SYS_CLOCK_MAX_WAIT for the announced count to fit. The budget left
	 * from now on is SYS_CLOCK_MAX_WAIT - ticks_elapsed; if it is already
	 * spent ask for an announce right away.
	 */
	अगर (ticks_elapsed >= SYS_CLOCK_MAX_WAIT) {
		वापस 0;
	}

	/*
	 * No deadline, or one further out than can be scheduled in a single
	 * step: wait the capped budget and re-evaluate at the next announce.
	 * Testing next (which may be 64-bit) against the cap keeps the
	 * remaining arithmetic in 32 bits. The empty case still returns the
	 * budget so a driver keeps waking to maintain uptime.
	 */
	अगर (अग्र == K_TICKS_FOREVER || अग्र >= SYS_CLOCK_MAX_WAIT) {
		वापस SYS_CLOCK_MAX_WAIT - ticks_elapsed;
	}

	/* Otherwise wait until the timeout, relative to now (0 if due). */
	dticks = (uint32_t)अग्र;

	वापस (dticks > ticks_elapsed) ? (dticks - ticks_elapsed) : 0;
}

/*
 * Reprogram the timer for the next pending timeout, or, when nothing is
 * pending and CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE tolerates a drifting uptime, tell
 * the driver its clock is unused so it may stop. Only meaningful where the
 * timeout queue may have just drained (abort, end of announce); the add path
 * always has a pending timeout and calls sys_clock_set_timeout() directly.
 */
अटल व्योम reprogram_अग्र(uint32_t ticks_elapsed)
{
	अगर (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
	    z_timeout_q_अग्र_expiry() == K_TICKS_FOREVER) {
		sys_clock_no_timeout();
	} अन्यथा {
		sys_clock_set_timeout(अग्र_timeout(ticks_elapsed), false);
	}
}

k_ticks_t z_add_timeout(काष्ठा _timeout *to, _timeout_func_t fn, k_timeout_t timeout)
{
	k_ticks_t ticks = 0;

	अगर (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		वापस 0;
	}

#अगर_घोषित CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(to));
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */

	__ASSERT(z_is_inactive_timeout(to), "");
	to->fn = fn;

	K_SPINLOCK(&timeout_lock) {
		uint32_t ticks_elapsed = 0;
		bool has_elapsed = false;
		k_ticks_t dticks;

		अगर (Z_IS_TIMEOUT_RELATIVE(timeout)) {
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
		} अन्यथा {
			dticks = Z_TICK_ABS(timeout.ticks) - curr_tick;
			dticks = max(1, dticks);
			ticks = timeout.ticks;
		}

		अगर (z_timeout_q_जोड़ो(to, dticks) && !any_cpu_announcing()) {
			अगर (!has_elapsed) {
				/* In case of absolute timeout that is first to expire
				 * elapsed need to be read from the system clock.
				 */
				ticks_elapsed = elapsed();
			}
			sys_clock_set_timeout(अग्र_timeout(ticks_elapsed), false);
		}
	}

	वापस ticks;
}

पूर्णांक z_try_abort_timeout(काष्ठा _timeout *to)
{
	पूर्णांक ret = -EINVAL;

	K_SPINLOCK(&timeout_lock) {
		अगर (!z_is_inactive_timeout(to)) {
			bool was_first = z_timeout_q_remove(to);

			ret = 0;
			अगर (was_first) {
				reprogram_अग्र(elapsed());
			}
		} अन्यथा अगर (IS_ENABLED(CONFIG_SMP) && inflight_ptr() == to &&
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
		अगर (inflight_ptr() == to) {
			inflight_mark_superseded();
		}
	}

	अगर (IS_ENABLED(CONFIG_SMP) && ret == -EAGAIN) {
		arch_spin_relax();
	}

	वापस ret;
}

bool z_timeout_inflight_superseded(स्थिर काष्ठा _timeout *to)
{
	bool superseded = false;

	K_SPINLOCK(&timeout_lock) {
		superseded = inflight_timeout ==
			     (काष्ठा _timeout *)((uintptr_t)to | INFLIGHT_SUPERSEDED_BIT);
	}

	वापस superseded;
}

k_ticks_t z_timeout_remaining(स्थिर काष्ठा _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_lock) {
		अगर (!z_is_inactive_timeout(timeout)) {
			k_ticks_t rem = z_timeout_q_remainder(timeout);
			uint32_t el = elapsed();

			/* elapsed() can exceed rem when announce is late. */
			ticks = (rem > el) ? (rem - el) : 0;
		}
	}

	वापस ticks;
}
EXPORT_प्रतीक(z_timeout_remaining);

k_ticks_t z_timeout_expires(स्थिर काष्ठा _timeout *timeout)
{
	k_ticks_t ticks = 0;

	K_SPINLOCK(&timeout_lock) {
		ticks = curr_tick;
		अगर (!z_is_inactive_timeout(timeout)) {
			ticks += z_timeout_q_remainder(timeout);
		}
	}

	वापस ticks;
}
EXPORT_प्रतीक(z_timeout_expires);

uint32_t z_get_अग्र_timeout_expiry(व्योम)
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
		अगर (IS_ENABLED(CONFIG_SYSTEM_CLOCK_SLOPPY_IDLE) &&
		    z_timeout_q_अग्र_expiry() == K_TICKS_FOREVER) {
			ret = (uint32_t)K_TICKS_FOREVER;
		} अन्यथा {
			ret = अग्र_timeout(elapsed());
		}
	}
	वापस ret;
}

व्योम sys_clock_announce_locked(uint32_t ticks, k_spinlock_कुंजी_t कुंजी)
{
	/* We release the lock around the callbacks below, so on SMP
	 * systems someone might be already running the loop.  Don't
	 * race (which will cause parallel execution of "sequential"
	 * timeouts and confuse apps), just increment the tick count
	 * and return.
	 */
	अगर (IS_ENABLED(CONFIG_SMP) && any_cpu_announcing()) {
		announce_remaining += ticks;
		k_spin_unlock(&timeout_lock, कुंजी);
		वापस;
	}

	announce_remaining = ticks;
	announcing_cpu = CPU_ID;

#अगर_घोषित _TIMEOUT_BACKEND_OWNS_ANNOUNCE
	/* The backend (timer wheel) runs the firing loop itself: its per-tick
	 * advance is a bitmap jump over empty ticks and its sift is a
	 * time-driven event with no single timeout to drive a generic loop.
	 * It advances curr_tick / announce_remaining and fires handlers via
	 * the same inflight_timeout dance as below.
	 */
	कुंजी = z_timeout_q_announce(कुंजी);
#अन्यथा
	जबतक (announce_remaining > 0) {
		काष्ठा _timeout *t;
		int32_t dt = z_timeout_q_अग्र_gap();

		अगर ((uint32_t)dt > announce_remaining) {
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
		जबतक ((t = z_timeout_q_निकालो_due()) != शून्य) {
			_timeout_func_t handler = t->fn;

			inflight_timeout = t;

			k_spin_unlock(&timeout_lock, कुंजी);
			handler(t);
			कुंजी = k_spin_lock(&timeout_lock);
			inflight_timeout = शून्य;
		}
	}
#पूर्ण_अगर /* _TIMEOUT_BACKEND_OWNS_ANNOUNCE */

	announcing_cpu = -1;

	reprogram_अग्र(0);

	k_spin_unlock(&timeout_lock, कुंजी);

#अगर_घोषित CONFIG_TIMESLICING
	z_time_slice();
#पूर्ण_अगर /* CONFIG_TIMESLICING */
}

#अगर defined(CONFIG_SMP) || defined(CONFIG_SPIN_VALIDATE)
k_spinlock_कुंजी_t sys_clock_lock(व्योम)
{
	वापस k_spin_lock(&timeout_lock);
}

व्योम sys_clock_unlock(k_spinlock_कुंजी_t कुंजी)
{
	k_spin_unlock(&timeout_lock, कुंजी);
}
#पूर्ण_अगर

#अगर defined(CONFIG_TEST) || defined(CONFIG_ASSERT)
bool sys_clock_is_locked(व्योम)
{
	वापस z_spin_is_locked(&timeout_lock);
}
#पूर्ण_अगर

int64_t sys_clock_tick_get(व्योम)
{
	uint64_t t = 0U;

	K_SPINLOCK(&timeout_lock) {
		t = curr_tick + elapsed();
	}
	वापस t;
}

uint32_t sys_clock_tick_get_32(व्योम)
{
#अगर_घोषित CONFIG_TICKLESS_KERNEL
	वापस (uint32_t)sys_clock_tick_get();
#अन्यथा
	वापस (uint32_t)curr_tick;
#पूर्ण_अगर /* CONFIG_TICKLESS_KERNEL */
}

int64_t z_impl_k_uptime_ticks(व्योम)
{
	वापस sys_clock_tick_get();
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत int64_t z_vrfy_k_uptime_ticks(व्योम)
{
	वापस z_impl_k_uptime_ticks();
}
#समावेश <zephyr/syscalls/k_uptime_ticks_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
	k_timepoint_t timepoint;

	अगर (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		timepoint.tick = UINT64_MAX;
	} अन्यथा अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		timepoint.tick = 0;
	} अन्यथा {
		k_ticks_t dt = timeout.ticks;

		अगर (Z_IS_TIMEOUT_RELATIVE(timeout)) {
			timepoint.tick = sys_clock_tick_get() + max(1, dt);
		} अन्यथा {
			timepoint.tick = Z_TICK_ABS(dt);
		}
	}

	वापस timepoint;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
	uint64_t now, remaining;

	अगर (timepoint.tick == UINT64_MAX) {
		वापस K_FOREVER;
	}
	अगर (timepoint.tick == 0) {
		वापस K_NO_WAIT;
	}

	now = sys_clock_tick_get();
	remaining = (timepoint.tick > now) ? (timepoint.tick - now) : 0;
	वापस K_TICKS(remaining);
}

#अगर_घोषित CONFIG_ZTEST
व्योम z_impl_sys_clock_tick_set(uint64_t tick)
{
	curr_tick = tick;
}

व्योम z_vrfy_sys_clock_tick_set(uint64_t tick)
{
	z_impl_sys_clock_tick_set(tick);
}
#पूर्ण_अगर /* CONFIG_ZTEST */
