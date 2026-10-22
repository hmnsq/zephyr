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

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <kthread.h>
#அந்தர்பாவ <kswap.h>
#அந்தர்பாவ <timeout_q.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/tracing/tracing.h>
#அந்தர்பாவ <zephyr/logging/log.h>
#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/sys/minmax.h>
#அந்தர்பாவ <zephyr/sys/util.h>

/* pending_current is owned by timeslicing.c; we reference it here to avoid
 * a spurious timeslice when the thread that is going to sleep gets picked as
 * the next thread to run while z_swap() is in progress.
 */
#எனில் defined(CONFIG_SWAP_NONATOMIC) && defined(CONFIG_TIMESLICING)
வெளி கட்டமைப்பு k_thread *pending_current;
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
	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_yield();
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);
		திரும்பு 0;
	}

	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();

#எனில் defined(CONFIG_TIMESLICING) && defined(CONFIG_SWAP_NONATOMIC)
	pending_current = _current;
#endif /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	z_sched_unready_locked(_current);
	expected_wakeup_ticks = (uint32_t)z_add_thread_timeout(_current, timeout);
	z_mark_thread_as_sleeping(_current);

	(வெறுமை)z_swap_locked(சாவி);

	/* There is no meaningful remainder to report for K_FOREVER: reaching
	 * this point at all means a k_wakeup() cut the sleep short.
	 */
	எனில் (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);
		திரும்பு K_TICKS_FOREVER;
	}

	/* We require a 32 bit unsigned subtraction to handle a wraparound.
	 * A normal timeout-driven wakeup leaves zero (or a slightly negative)
	 * remainder; only an early k_wakeup() yields a positive value.
	 */
	uint32_t இடது_ticks = expected_wakeup_ticks - sys_clock_tick_get_32();

	/* Use signed comparison so past-due wakeups (negative remainder) return 0.
	 * k_ticks_t may be uint32_t (!CONFIG_TIMEOUT_64BIT), so comparing ticks > 0
	 * directly would be an unsigned comparison and would misinterpret a negative
	 * remainder as a large positive value.
	 */
	int32_t signed_இடது = (int32_t)இடது_ticks;

	எனில் (signed_இடது < 0) {
		signed_இடது = 0;
	}

	SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, signed_இடது);
	திரும்பு (k_ticks_t)signed_இடது;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி k_ticks_t z_vrfy_k_sleep_ticks(k_timeout_t timeout)
{
	திரும்பு z_impl_k_sleep_ticks(timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_sleep_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */
