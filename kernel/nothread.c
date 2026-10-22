/*
 * Copyright (C) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kernel_internal.h>

/* We are not building thread.c when MULTITHREADING=n, so we
 * need to provide a few stubs here.
 */
bool k_is_in_isr(வெறுமை)
{
	திரும்பு arch_is_in_isr();
}

#ifdef CONFIG_SYS_CLOCK_EXISTS
நிலையான K_TIMER_DEFINE(sleep_timer, வெற்று, வெற்று);
#endif

/* This is a fallback implementation of k_sleep_ticks() for when
 * multi-threading is disabled. The main implementation is in sleep.c.
 */
k_ticks_t z_impl_k_sleep_ticks(k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* in case of K_FOREVER, we suspend */
	எனில் (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		/* In Single Thread, just wait for an interrupt saving power */
		k_cpu_idle();
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);

		திரும்பு K_TICKS_FOREVER;
	}

#ifdef CONFIG_SYS_CLOCK_EXISTS
	k_timer_start(&sleep_timer, timeout, K_NO_WAIT);

	/* When multithreading is disabled, this will enter a low power state
	 * using k_cpu_idle() until the timer has expired.
	 */
	k_timer_status_sync(&sleep_timer);

	SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	/* This implementation is compiled when multithreading is disabled.
	 * Therefore, unlike its multithreaded counterpart, it shall always
	 * sleep for the entire timeout duration (no thread to awake).
	 */
	திரும்பு 0;
#இல்லையெனில்
	/* Fallback to busy-wait when no system clock is available */
	k_ticks_t ticks;
	uint32_t ticks_to_wait;

	ticks = timeout.ticks;
	எனில் (Z_IS_TIMEOUT_RELATIVE(timeout)) {
		/* ticks is delta timeout */
		ticks_to_wait = ticks;
	} இல்லையெனில் {
		/* ticks is absolute timeout expiration */
		uint32_t curr_ticks = sys_clock_tick_get_32();

		எனில் (Z_TICK_ABS(ticks) > curr_ticks) {
			ticks_to_wait = Z_TICK_ABS(ticks) - curr_ticks;
		} இல்லையெனில் {
			ticks_to_wait = 0;
		}
	}

	/* busy wait to be time coherent since subsystems may depend on it */
	z_impl_k_busy_wait(k_ticks_to_us_ceil32(ticks_to_wait));

	SYS_PORT_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	திரும்பு 0;
#endif
}
