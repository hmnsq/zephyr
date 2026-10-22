/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/drivers/timer/system_timer.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/clock.h>
#ಅನ್ತರ್ಭಾವ <kernel_arch_interface.h>

#ಆದರೆ defined(CONFIG_SYSTEM_CLOCK_HW_ಚಕ್ರಗಳು_ಪ್ರತಿ_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t busy_wait_us_ಗಮ್ಯ_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	ಸ್ಥಿರಾಂಕ uint64_t denom = 1000000ULL;
	uint64_t ಅಂಕೆ = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	ಹಿಂತಿರುಗಿ (uint32_t)((ಅಂಕೆ + denom - 1U) / denom);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

ಶೂನ್ಯ z_impl_k_busy_wait(uint32_t usec_ಗಮ್ಯ_wait)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_ಗಮ್ಯ_wait);
	ಆದರೆ (usec_ಗಮ್ಯ_wait == 0U) {
		SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_ಗಮ್ಯ_wait);
		ಹಿಂತಿರುಗಿ;
	}

#ಆದರೆ defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_ಗಮ್ಯ_wait);
#ಅಥವಾಆದರೆ defined(CONFIG_SYS_CLOCK_EXISTS)
#ಆದರೆ defined(CONFIG_SYSTEM_CLOCK_HW_ಚಕ್ರಗಳು_ಪ್ರತಿ_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t ಆರಂಭ_ಚಕ್ರಗಳು;

	/* Get a consistent sample of the current frequency and cycle count. */
	ಗಾಗಿ (;;) {
		uint32_t hz1 = sys_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec();
		uint32_t current_ಚಕ್ರಗಳು = k_ಚಕ್ರ_get_32();
		uint32_t hz2 = sys_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec();

		ಆದರೆ (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			ಮುಂದುವರಿ;
		}

		hz = hz1;
		ಆರಂಭ_ಚಕ್ರಗಳು = current_ಚಕ್ರಗಳು;

		ನಿಲ್ಲಿಸು;
	}

	uint32_t ಚಕ್ರಗಳು_ಗಮ್ಯ_wait = busy_wait_us_ಗಮ್ಯ_cyc_ceil32(usec_ಗಮ್ಯ_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	ಗಾಗಿ (;;) {
		uint32_t hz1 = sys_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec();
		uint32_t current_ಚಕ್ರಗಳು = k_ಚಕ್ರ_get_32();
		uint32_t hz2 = sys_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec();

		ಆದರೆ (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			ಆರಂಭ_ಚಕ್ರಗಳು = (uint32_t)(((uint64_t)ಆರಂಭ_ಚಕ್ರಗಳು * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			ಚಕ್ರಗಳು_ಗಮ್ಯ_wait = busy_wait_us_ಗಮ್ಯ_cyc_ceil32(usec_ಗಮ್ಯ_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		ಆದರೆ ((current_ಚಕ್ರಗಳು - ಆರಂಭ_ಚಕ್ರಗಳು) >= ಚಕ್ರಗಳು_ಗಮ್ಯ_wait) {
			ನಿಲ್ಲಿಸು;
		}
	}
#ಇಲ್ಲದಿದ್ದರೆ
	uint32_t ಆರಂಭ_ಚಕ್ರಗಳು = k_ಚಕ್ರ_get_32();
	uint32_t ಚಕ್ರಗಳು_ಗಮ್ಯ_wait = k_us_ಗಮ್ಯ_cyc_ceil32(usec_ಗಮ್ಯ_wait);

	ಗಾಗಿ (;;) {
		uint32_t current_ಚಕ್ರಗಳು = k_ಚಕ್ರ_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		ಆದರೆ ((current_ಚಕ್ರಗಳು - ಆರಂಭ_ಚಕ್ರಗಳು) >= ಚಕ್ರಗಳು_ಗಮ್ಯ_wait) {
			ನಿಲ್ಲಿಸು;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#ಇಲ್ಲದಿದ್ದರೆ
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ loops_ಪ್ರತಿ_usec = CONFIG_BUSYWAIT_CPU_LOOPS_ಪ್ರತಿ_USEC;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ loops = loops_ಪ್ರತಿ_usec * usec_ಗಮ್ಯ_wait;

	ಆಗ (loops-- > 0) {
		arch_nop();
	}
#ಆದರೆ_ಅಂತ್ಯ

	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_ಗಮ್ಯ_wait);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_busy_wait(uint32_t usec_ಗಮ್ಯ_wait)
{
	z_impl_k_busy_wait(usec_ಗಮ್ಯ_wait);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_busy_wait_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
