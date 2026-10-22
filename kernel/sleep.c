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

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <kthread.h>
#അന്തര്ഭാവ <kswap.h>
#അന്തര്ഭാവ <timeout_q.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/tracing/tracing.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/sys/minmax.h>
#അന്തര്ഭാവ <zephyr/sys/util.h>

/* pending_current is owned by timeslicing.c; we reference it here to avoid
 * a spurious timeslice when the thread that is going to sleep gets picked as
 * the next thread to run while z_swap() is in progress.
 */
#എങ്കിൽ defined(CONFIG_SWAP_NONATOMIC) && defined(CONFIG_TIMESLICING)
ബാഹ്യ ഘടന k_thread *pending_current;
#endif

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

	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* K_NO_WAIT is treated as a 'yield' */
	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_yield();
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);
		തിരികെ 0;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

#എങ്കിൽ defined(CONFIG_TIMESLICING) && defined(CONFIG_SWAP_NONATOMIC)
	pending_current = _current;
#endif /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	z_sched_unready_locked(_current);
	expected_wakeup_ticks = (uint32_t)z_add_thread_timeout(_current, timeout);
	z_mark_thread_as_sleeping(_current);

	(ശൂന്യം)z_swap_locked(താക്കോൽ);

	/* There is no meaningful remainder to report for K_FOREVER: reaching
	 * this point at all means a k_wakeup() cut the sleep short.
	 */
	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);
		തിരികെ K_TICKS_FOREVER;
	}

	/* We require a 32 bit unsigned subtraction to handle a wraparound.
	 * A normal timeout-driven wakeup leaves zero (or a slightly negative)
	 * remainder; only an early k_wakeup() yields a positive value.
	 */
	uint32_t ഇടത്_ticks = expected_wakeup_ticks - sys_clock_tick_get_32();

	/* Use signed comparison so past-due wakeups (negative remainder) return 0.
	 * k_ticks_t may be uint32_t (!CONFIG_TIMEOUT_64BIT), so comparing ticks > 0
	 * directly would be an unsigned comparison and would misinterpret a negative
	 * remainder as a large positive value.
	 */
	int32_t signed_ഇടത് = (int32_t)ഇടത്_ticks;

	എങ്കിൽ (signed_ഇടത് < 0) {
		signed_ഇടത് = 0;
	}

	SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, signed_ഇടത്);
	തിരികെ (k_ticks_t)signed_ഇടത്;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ k_ticks_t z_vrfy_k_sleep_ticks(k_timeout_t timeout)
{
	തിരികെ z_impl_k_sleep_ticks(timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_sleep_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */
