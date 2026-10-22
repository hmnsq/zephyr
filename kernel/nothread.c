/*
 * Copyright (C) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>

/* We are not building thread.c when MULTITHREADING=n, so we
 * need to provide a few stubs here.
 */
bool k_is_ਵਿੱਚ_isr(ਖਾਲੀ)
{
	ਵਾਪਸ arch_is_ਵਿੱਚ_isr();
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CLOCK_EXISTS
ਸਥਿਰ K_TIMER_DEFINE(sleep_timer, ਨਿਰਾ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ

/* This is a fallback implementation of k_sleep_ticks() for when
 * multi-threading is disabled. The main implementation is in sleep.c.
 */
k_ticks_t z_impl_k_sleep_ticks(k_timeout_t timeout)
{
	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "");

	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* in case of K_FOREVER, we suspend */
	ਜੇ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		/* In Single Thread, just wait for an interrupt saving power */
		k_cpu_idle();
		SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);

		ਵਾਪਸ K_TICKS_FOREVER;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CLOCK_EXISTS
	k_timer_ਸ਼ੁਰੂ(&sleep_timer, timeout, K_NO_WAIT);

	/* When multithreading is disabled, this will enter a low power state
	 * using k_cpu_idle() until the timer has expired.
	 */
	k_timer_ਸਥਿਤੀ_sync(&sleep_timer);

	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	/* This implementation is compiled when multithreading is disabled.
	 * Therefore, unlike its multithreaded counterpart, it shall always
	 * sleep for the entire timeout duration (no thread to awake).
	 */
	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	/* Fallback to busy-wait when no system clock is available */
	k_ticks_t ticks;
	uint32_t ticks_ਮੰਜ਼ਿਲ_wait;

	ticks = timeout.ticks;
	ਜੇ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
		/* ticks is delta timeout */
		ticks_ਮੰਜ਼ਿਲ_wait = ticks;
	} ਨਹੀਂਤਾਂ {
		/* ticks is absolute timeout expiration */
		uint32_t curr_ticks = sys_clock_tick_get_32();

		ਜੇ (Z_TICK_ABS(ticks) > curr_ticks) {
			ticks_ਮੰਜ਼ਿਲ_wait = Z_TICK_ABS(ticks) - curr_ticks;
		} ਨਹੀਂਤਾਂ {
			ticks_ਮੰਜ਼ਿਲ_wait = 0;
		}
	}

	/* busy wait to be time coherent since subsystems may depend on it */
	z_impl_k_busy_wait(k_ticks_ਮੰਜ਼ਿਲ_us_ceil32(ticks_ਮੰਜ਼ਿਲ_wait));

	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}
