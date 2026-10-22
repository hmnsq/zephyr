/*
 * Copyright (C) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kernel_internal.h>

/* We are not building thread.c when MULTITHREADING=n, so we
 * need to provide a few stubs here.
 */
bool k_is_وچ_isr(فراغ)
{
	أرجع arch_is_وچ_isr();
}

#جے_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
ساكن K_TIMER_DEFINE(sleep_timer, عدم, عدم);
#جے_انت

/* This is a fallback implementation of k_sleep_ticks() for when
 * multi-threading is disabled. The main implementation is in sleep.c.
 */
k_ticks_t z_impl_k_sleep_ticks(k_timeout_t timeout)
{
	__ASSERT(!arch_is_وچ_isr(), "");

	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* in case of K_FOREVER, we suspend */
	إذا (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		/* In Single Thread, just wait for an interrupt saving power */
		k_cpu_idle();
		SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);

		أرجع K_TICKS_FOREVER;
	}

#جے_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS
	k_timer_شروع(&sleep_timer, timeout, K_NO_WAIT);

	/* When multithreading is disabled, this will enter a low power state
	 * using k_cpu_idle() until the timer has expired.
	 */
	k_timer_حالت_sync(&sleep_timer);

	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	/* This implementation is compiled when multithreading is disabled.
	 * Therefore, unlike its multithreaded counterpart, it shall always
	 * sleep for the entire timeout duration (no thread to awake).
	 */
	أرجع 0;
#وإلا
	/* Fallback to busy-wait when no system clock is available */
	k_ticks_t ticks;
	uint32_t ticks_منزل_wait;

	ticks = timeout.ticks;
	إذا (Z_IS_TIMEOUT_RELATIVE(timeout)) {
		/* ticks is delta timeout */
		ticks_منزل_wait = ticks;
	} وإلا {
		/* ticks is absolute timeout expiration */
		uint32_t curr_ticks = sys_clock_tick_get_32();

		إذا (Z_TICK_ABS(ticks) > curr_ticks) {
			ticks_منزل_wait = Z_TICK_ABS(ticks) - curr_ticks;
		} وإلا {
			ticks_منزل_wait = 0;
		}
	}

	/* busy wait to be time coherent since subsystems may depend on it */
	z_impl_k_busy_wait(k_ticks_منزل_us_ceil32(ticks_منزل_wait));

	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	أرجع 0;
#جے_انت
}
