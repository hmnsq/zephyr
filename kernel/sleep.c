/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Thread sleep/wake APIs
 *
 * Implements k_sleep_ticks(), the primitive that k_sleep(), k_msleep() and
 * k_usleep() are built upon.  It suspends the calling thread for a specified
 * duration and relies on the scheduler (via z_sched_unready_locked()) and the
 * timeout subsystem (via z_add_thread_timeout()) to do the heavy lifting.  The
 * millisecond and microsecond flavours are inlined in sleep.h on top of it.
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <timeout_q.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/tracing/tracing.h>
#అన్తర్భావ <zephyr/logging/log.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/minmax.h>
#అన్తర్భావ <zephyr/sys/util.h>

/* pending_current is owned by timeslicing.c; we reference it here to avoid
 * a spurious timeslice when the thread that is going to sleep gets picked as
 * the next thread to run while z_swap() is in progress.
 */
#ఒకవేళ defined(CONFIG_మార్పిడి_NONATOMIC) && defined(CONFIG_TIMESLICING)
బాహ్య నిర్మాణం k_thread *pending_current;
#అయితే_ముగింపు

/**
 * @brief Sleep the current thread for the given number of ticks.
 *
 * Removes _current from the run queue, arms a timeout for the requested
 * duration, then context-switches away.  Returns the number of ticks
 * remaining if the sleep was cut short by k_wakeup(), or 0 on normal
 * wakeup.
 *
 * @param timeout Sleep duration.
 * @return Ticks remaining when woken early, 0 on normal expiry, or
 *         K_TICKS_FOREVER if @a timeout was K_FOREVER.
 */
k_ticks_t z_impl_k_sleep_ticks(k_timeout_t timeout)
{
	uint32_t expected_wakeup_ticks;

	__ASSERT(!arch_is_లోపల_isr(), "");

	SYS_పోర్టు_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* K_NO_WAIT is treated as a 'yield' */
	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_yield();
		SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);
		తిరిగి 0;
	}

	k_spinlock_కీలకం_t కీలకం = z_sched_spinlock_తాళం();

#ఒకవేళ defined(CONFIG_TIMESLICING) && defined(CONFIG_మార్పిడి_NONATOMIC)
	pending_current = _current;
#అయితే_ముగింపు /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	z_sched_unready_locked(_current);
	expected_wakeup_ticks = (uint32_t)z_కూడు_thread_timeout(_current, timeout);
	z_mark_thread_as_sleeping(_current);

	(శూన్య)z_మార్పిడి_locked(కీలకం);

	/* There is no meaningful remainder to report for K_FOREVER: reaching
	 * this point at all means a k_wakeup() cut the sleep short.
	 */
	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);
		తిరిగి K_TICKS_FOREVER;
	}

	/* We require a 32 bit unsigned subtraction to handle a wraparound.
	 * A normal timeout-driven wakeup leaves zero (or a slightly negative)
	 * remainder; only an early k_wakeup() yields a positive value.
	 */
	uint32_t ఎడమ_ticks = expected_wakeup_ticks - sys_clock_tick_get_32();

	/* Use signed comparison so past-due wakeups (negative remainder) return 0.
	 * k_ticks_t may be uint32_t (!CONFIG_TIMEOUT_64BIT), so comparing ticks > 0
	 * directly would be an unsigned comparison and would misinterpret a negative
	 * remainder as a large positive value.
	 */
	int32_t signed_ఎడమ = (int32_t)ఎడమ_ticks;

	ఒకవేళ (signed_ఎడమ < 0) {
		signed_ఎడమ = 0;
	}

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, signed_ఎడమ);
	తిరిగి (k_ticks_t)signed_ఎడమ;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస k_ticks_t z_vrfy_k_sleep_ticks(k_timeout_t timeout)
{
	తిరిగి z_impl_k_sleep_ticks(timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_sleep_ticks_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
