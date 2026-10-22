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

#ضمن <zephyr/kernel.h>
#ضمن <ksched.h>
#ضمن <kthread.h>
#ضمن <kswap.h>
#ضمن <timeout_q.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/tracing/tracing.h>
#ضمن <zephyr/logging/log.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/minmax.h>
#ضمن <zephyr/sys/util.h>

/* pending_current is owned by timeslicing.c; we reference it here to avoid
 * a spurious timeslice when the thread that is going to sleep gets picked as
 * the next thread to run while z_swap() is in progress.
 */
#إذا defined(CONFIG_SWAP_NONATOMIC) && defined(CONFIG_TIMESLICING)
خارجي بنية k_thread *pending_current;
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
	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_yield();
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);
		أرجع 0;
	}

	k_spinlock_کونچي_t کونچي = z_sched_spinlock_lock();

#إذا defined(CONFIG_TIMESLICING) && defined(CONFIG_SWAP_NONATOMIC)
	pending_current = _current;
#endif /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	z_sched_unready_locked(_current);
	expected_wakeup_ticks = (uint32_t)z_add_thread_timeout(_current, timeout);
	z_mark_thread_as_sleeping(_current);

	(فراغ)z_swap_locked(کونچي);

	/* There is no meaningful remainder to report for K_FOREVER: reaching
	 * this point at all means a k_wakeup() cut the sleep short.
	 */
	إذا (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);
		أرجع K_TICKS_FOREVER;
	}

	/* We require a 32 bit unsigned subtraction to handle a wraparound.
	 * A normal timeout-driven wakeup leaves zero (or a slightly negative)
	 * remainder; only an early k_wakeup() yields a positive value.
	 */
	uint32_t کيري_ticks = expected_wakeup_ticks - sys_clock_tick_get_32();

	/* Use signed comparison so past-due wakeups (negative remainder) return 0.
	 * k_ticks_t may be uint32_t (!CONFIG_TIMEOUT_64BIT), so comparing ticks > 0
	 * directly would be an unsigned comparison and would misinterpret a negative
	 * remainder as a large positive value.
	 */
	int32_t signed_کيري = (int32_t)کيري_ticks;

	إذا (signed_کيري < 0) {
		signed_کيري = 0;
	}

	SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, signed_کيري);
	أرجع (k_ticks_t)signed_کيري;
}

#ifdef CONFIG_USERSPACE
ساكن مضمن k_ticks_t z_vrfy_k_sleep_ticks(k_timeout_t timeout)
{
	أرجع z_impl_k_sleep_ticks(timeout);
}
#ضمن <zephyr/syscalls/k_sleep_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */
