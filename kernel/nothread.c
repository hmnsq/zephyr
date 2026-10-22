/*
 * Copyright (C) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>

/* We are not building thread.c when MULTITHREADING=n, so we
 * need to provide a few stubs here.
 */
bool k_is_ಒಳ_isr(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ arch_is_ಒಳ_isr();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
ಸ್ಥಿರ K_TIMER_DEFINE(sleep_timer, ಖಾಲಿ, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ

/* This is a fallback implementation of k_sleep_ticks() for when
 * multi-threading is disabled. The main implementation is in sleep.c.
 */
k_ticks_t z_impl_k_sleep_ticks(k_timeout_t timeout)
{
	__ASSERT(!arch_is_ಒಳ_isr(), "");

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_thread, sleep_ticks, timeout);

	/* in case of K_FOREVER, we suspend */
	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		/* In Single Thread, just wait for an interrupt saving power */
		k_cpu_idle();
		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, K_TICKS_FOREVER);

		ಹಿಂತಿರುಗಿ K_TICKS_FOREVER;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
	k_timer_ಆರಂಭ(&sleep_timer, timeout, K_NO_WAIT);

	/* When multithreading is disabled, this will enter a low power state
	 * using k_cpu_idle() until the timer has expired.
	 */
	k_timer_ಸ್ಥಿತಿ_sync(&sleep_timer);

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	/* This implementation is compiled when multithreading is disabled.
	 * Therefore, unlike its multithreaded counterpart, it shall always
	 * sleep for the entire timeout duration (no thread to awake).
	 */
	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	/* Fallback to busy-wait when no system clock is available */
	k_ticks_t ticks;
	uint32_t ticks_ಗಮ್ಯ_wait;

	ticks = timeout.ticks;
	ಆದರೆ (Z_IS_TIMEOUT_RELATIVE(timeout)) {
		/* ticks is delta timeout */
		ticks_ಗಮ್ಯ_wait = ticks;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* ticks is absolute timeout expiration */
		uint32_t curr_ticks = sys_clock_tick_get_32();

		ಆದರೆ (Z_TICK_ABS(ticks) > curr_ticks) {
			ticks_ಗಮ್ಯ_wait = Z_TICK_ABS(ticks) - curr_ticks;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ticks_ಗಮ್ಯ_wait = 0;
		}
	}

	/* busy wait to be time coherent since subsystems may depend on it */
	z_impl_k_busy_wait(k_ticks_ಗಮ್ಯ_us_ceil32(ticks_ಗಮ್ಯ_wait));

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, sleep_ticks, timeout, 0);

	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}
