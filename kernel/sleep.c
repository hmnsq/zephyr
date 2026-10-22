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

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <timeout_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/tracing/tracing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/util.h>

/* pending_current is owned by timeslicing.c; we reference it here to avoid
 * a spurious timeslice when the thread that is going to sleep gets picked as
 * the next thread to run while z_swap() is in progress.
 */
#ಆದರೆ defined(CONFIG_ಅದಲುಬದಲು_NONATOMIC) && defined(CONFIG_TIMESLICING)
ಬಾಹ್ಯ ರಚನೆ k_thread *pending_current;
#ಆದರೆ_ಅಂತ್ಯ

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

	__ASSERT(!arch_is_ಒಳ_isr(), "");

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* K_NO_WAIT is treated as a 'yield' */
	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_yield();
		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);
		ಹಿಂತಿರುಗಿ 0;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

#ಆದರೆ defined(CONFIG_TIMESLICING) && defined(CONFIG_ಅದಲುಬದಲು_NONATOMIC)
	pending_current = _current;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICING && CONFIG_SWAP_NONATOMIC */
	z_sched_unready_locked(_current);
	expected_wakeup_ticks = (uint32_t)z_ಕೂಡಿಸು_thread_timeout(_current, timeout);
	z_mark_thread_as_sleeping(_current);

	(ಶೂನ್ಯ)z_ಅದಲುಬದಲು_locked(ಕೀಲಿ);

	/* There is no meaningful remainder to report for K_FOREVER: reaching
	 * this point at all means a k_wakeup() cut the sleep short.
	 */
	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);
		ಹಿಂತಿರುಗಿ K_TICKS_FOREVER;
	}

	/* We require a 32 bit unsigned subtraction to handle a wraparound.
	 * A normal timeout-driven wakeup leaves zero (or a slightly negative)
	 * remainder; only an early k_wakeup() yields a positive value.
	 */
	uint32_t ಎಡ_ticks = expected_wakeup_ticks - sys_clock_tick_get_32();

	/* Use signed comparison so past-due wakeups (negative remainder) return 0.
	 * k_ticks_t may be uint32_t (!CONFIG_TIMEOUT_64BIT), so comparing ticks > 0
	 * directly would be an unsigned comparison and would misinterpret a negative
	 * remainder as a large positive value.
	 */
	int32_t signed_ಎಡ = (int32_t)ಎಡ_ticks;

	ಆದರೆ (signed_ಎಡ < 0) {
		signed_ಎಡ = 0;
	}

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, signed_ಎಡ);
	ಹಿಂತಿರುಗಿ (k_ticks_t)signed_ಎಡ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_vrfy_k_sleep_ticks(k_timeout_t timeout)
{
	ಹಿಂತಿರುಗಿ z_impl_k_sleep_ticks(timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sleep_ticks_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
