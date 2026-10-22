/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/drivers/timer/system_timer.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/clock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_arch_interface.h>

#ਜੇ defined(CONFIG_SYSTEM_CLOCK_HW_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t busy_wait_us_ਮੰਜ਼ਿਲ_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	ਅਚਲ uint64_t denom = 1000000ULL;
	uint64_t ਅੰਕ = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	ਵਾਪਸ (uint32_t)((ਅੰਕ + denom - 1U) / denom);
}
#ਜੇ_ਅੰਤ /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

ਖਾਲੀ z_impl_k_busy_wait(uint32_t usec_ਮੰਜ਼ਿਲ_wait)
{
	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_ਮੰਜ਼ਿਲ_wait);
	ਜੇ (usec_ਮੰਜ਼ਿਲ_wait == 0U) {
		SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_ਮੰਜ਼ਿਲ_wait);
		ਵਾਪਸ;
	}

#ਜੇ defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_ਮੰਜ਼ਿਲ_wait);
#ਜਾਂਜੇ defined(CONFIG_SYS_CLOCK_EXISTS)
#ਜੇ defined(CONFIG_SYSTEM_CLOCK_HW_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t ਸ਼ੁਰੂ_ਚੱਕਰਾਂ;

	/* Get a consistent sample of the current frequency and cycle count. */
	ਲਈ (;;) {
		uint32_t hz1 = sys_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec();
		uint32_t current_ਚੱਕਰਾਂ = k_ਚੱਕਰ_get_32();
		uint32_t hz2 = sys_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec();

		ਜੇ (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			ਜਾਰੀ;
		}

		hz = hz1;
		ਸ਼ੁਰੂ_ਚੱਕਰਾਂ = current_ਚੱਕਰਾਂ;

		ਰੋਕੋ;
	}

	uint32_t ਚੱਕਰਾਂ_ਮੰਜ਼ਿਲ_wait = busy_wait_us_ਮੰਜ਼ਿਲ_cyc_ceil32(usec_ਮੰਜ਼ਿਲ_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	ਲਈ (;;) {
		uint32_t hz1 = sys_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec();
		uint32_t current_ਚੱਕਰਾਂ = k_ਚੱਕਰ_get_32();
		uint32_t hz2 = sys_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec();

		ਜੇ (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			ਜਾਰੀ;
		}

		ਜੇ (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			ਸ਼ੁਰੂ_ਚੱਕਰਾਂ = (uint32_t)(((uint64_t)ਸ਼ੁਰੂ_ਚੱਕਰਾਂ * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			ਚੱਕਰਾਂ_ਮੰਜ਼ਿਲ_wait = busy_wait_us_ਮੰਜ਼ਿਲ_cyc_ceil32(usec_ਮੰਜ਼ਿਲ_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		ਜੇ ((current_ਚੱਕਰਾਂ - ਸ਼ੁਰੂ_ਚੱਕਰਾਂ) >= ਚੱਕਰਾਂ_ਮੰਜ਼ਿਲ_wait) {
			ਰੋਕੋ;
		}
	}
#ਨਹੀਂਤਾਂ
	uint32_t ਸ਼ੁਰੂ_ਚੱਕਰਾਂ = k_ਚੱਕਰ_get_32();
	uint32_t ਚੱਕਰਾਂ_ਮੰਜ਼ਿਲ_wait = k_us_ਮੰਜ਼ਿਲ_cyc_ceil32(usec_ਮੰਜ਼ਿਲ_wait);

	ਲਈ (;;) {
		uint32_t current_ਚੱਕਰਾਂ = k_ਚੱਕਰ_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		ਜੇ ((current_ਚੱਕਰਾਂ - ਸ਼ੁਰੂ_ਚੱਕਰਾਂ) >= ਚੱਕਰਾਂ_ਮੰਜ਼ਿਲ_wait) {
			ਰੋਕੋ;
		}
	}
#ਜੇ_ਅੰਤ /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#ਨਹੀਂਤਾਂ
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ loops_ਪ੍ਰਤੀ_usec = CONFIG_BUSYWAIT_CPU_LOOPS_ਪ੍ਰਤੀ_USEC;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ loops = loops_ਪ੍ਰਤੀ_usec * usec_ਮੰਜ਼ਿਲ_wait;

	ਜਦੋਂ (loops-- > 0) {
		arch_nop();
	}
#ਜੇ_ਅੰਤ

	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_ਮੰਜ਼ਿਲ_wait);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_busy_wait(uint32_t usec_ਮੰਜ਼ਿਲ_wait)
{
	z_impl_k_busy_wait(usec_ਮੰਜ਼ਿਲ_wait);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_busy_wait_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
