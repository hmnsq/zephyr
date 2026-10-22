/*
 * Copyright (C) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kernel_internal.h>

/* We are not building thread.c when MULTITHREADING=n, so we
 * need to provide a few stubs here.
 */
bool k_is_లోపల_isr(శూన్య)
{
	తిరిగి arch_is_లోపల_isr();
}

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
స్థిర K_TIMER_DEFINE(sleep_timer, ఖాళీ, ఖాళీ);
#అయితే_ముగింపు

/* This is a fallback implementation of k_sleep_ticks() for when
 * multi-threading is disabled. The main implementation is in sleep.c.
 */
k_ticks_t z_impl_k_sleep_ticks(k_timeout_t timeout)
{
	__ASSERT(!arch_is_లోపల_isr(), "");

	SYS_పోర్టు_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* in case of K_FOREVER, we suspend */
	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		/* In Single Thread, just wait for an interrupt saving power */
		k_cpu_idle();
		SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);

		తిరిగి K_TICKS_FOREVER;
	}

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
	k_timer_ప్రారంభం(&sleep_timer, timeout, K_NO_WAIT);

	/* When multithreading is disabled, this will enter a low power state
	 * using k_cpu_idle() until the timer has expired.
	 */
	k_timer_స్థితి_sync(&sleep_timer);

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	/* This implementation is compiled when multithreading is disabled.
	 * Therefore, unlike its multithreaded counterpart, it shall always
	 * sleep for the entire timeout duration (no thread to awake).
	 */
	తిరిగి 0;
#లేకపోతే
	/* Fallback to busy-wait when no system clock is available */
	k_ticks_t ticks;
	uint32_t ticks_గమ్యం_wait;

	ticks = timeout.ticks;
	ఒకవేళ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
		/* ticks is delta timeout */
		ticks_గమ్యం_wait = ticks;
	} లేకపోతే {
		/* ticks is absolute timeout expiration */
		uint32_t curr_ticks = sys_clock_tick_get_32();

		ఒకవేళ (Z_TICK_ABS(ticks) > curr_ticks) {
			ticks_గమ్యం_wait = Z_TICK_ABS(ticks) - curr_ticks;
		} లేకపోతే {
			ticks_గమ్యం_wait = 0;
		}
	}

	/* busy wait to be time coherent since subsystems may depend on it */
	z_impl_k_busy_wait(k_ticks_గమ్యం_us_ceil32(ticks_గమ్యం_wait));

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	తిరిగి 0;
#అయితే_ముగింపు
}
