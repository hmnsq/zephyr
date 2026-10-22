/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/drivers/timer/system_timer.h>
#அந்தர்பாவ <zephyr/sys/clock.h>
#அந்தர்பாவ <kernel_arch_interface.h>

#எனில் defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
நிலையான உள்வரி uint32_t busy_wait_us_to_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	மாறிலி uint64_t denom = 1000000ULL;
	uint64_t இலக்கம் = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	திரும்பு (uint32_t)((இலக்கம் + denom - 1U) / denom);
}
#endif /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

வெறுமை z_impl_k_busy_wait(uint32_t usec_to_wait)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_to_wait);
	எனில் (usec_to_wait == 0U) {
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_to_wait);
		திரும்பு;
	}

#எனில் defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_to_wait);
#அல்லதெனில் defined(CONFIG_SYS_CLOCK_EXISTS)
#எனில் defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t start_cycles;

	/* Get a consistent sample of the current frequency and cycle count. */
	க்கு (;;) {
		uint32_t hz1 = sys_clock_hw_cycles_per_sec();
		uint32_t current_cycles = k_cycle_get_32();
		uint32_t hz2 = sys_clock_hw_cycles_per_sec();

		எனில் (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			தொடர்;
		}

		hz = hz1;
		start_cycles = current_cycles;

		நிறுத்து;
	}

	uint32_t cycles_to_wait = busy_wait_us_to_cyc_ceil32(usec_to_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	க்கு (;;) {
		uint32_t hz1 = sys_clock_hw_cycles_per_sec();
		uint32_t current_cycles = k_cycle_get_32();
		uint32_t hz2 = sys_clock_hw_cycles_per_sec();

		எனில் (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			தொடர்;
		}

		எனில் (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			start_cycles = (uint32_t)(((uint64_t)start_cycles * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			cycles_to_wait = busy_wait_us_to_cyc_ceil32(usec_to_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		எனில் ((current_cycles - start_cycles) >= cycles_to_wait) {
			நிறுத்து;
		}
	}
#இல்லையெனில்
	uint32_t start_cycles = k_cycle_get_32();
	uint32_t cycles_to_wait = k_us_to_cyc_ceil32(usec_to_wait);

	க்கு (;;) {
		uint32_t current_cycles = k_cycle_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		எனில் ((current_cycles - start_cycles) >= cycles_to_wait) {
			நிறுத்து;
		}
	}
#endif /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#இல்லையெனில்
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	unsigned முழு loops_per_usec = CONFIG_BUSYWAIT_CPU_LOOPS_PER_USEC;
	unsigned முழு loops = loops_per_usec * usec_to_wait;

	போது (loops-- > 0) {
		arch_nop();
	}
#endif

	SYS_PORT_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_to_wait);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_busy_wait(uint32_t usec_to_wait)
{
	z_impl_k_busy_wait(usec_to_wait);
}
#அந்தர்பாவ <zephyr/syscalls/k_busy_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
