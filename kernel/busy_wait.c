/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/drivers/timer/system_timer.h>
#అన్తర్భావ <zephyr/sys/clock.h>
#అన్తర్భావ <kernel_arch_interface.h>

#ఒకవేళ defined(CONFIG_SYSTEM_CLOCK_HW_చక్రాలు_చొప్పున_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
స్థిర అంతఃవరుస uint32_t busy_wait_us_గమ్యం_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	స్థిరరాశి uint64_t denom = 1000000ULL;
	uint64_t అంకె = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	తిరిగి (uint32_t)((అంకె + denom - 1U) / denom);
}
#అయితే_ముగింపు /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

శూన్య z_impl_k_busy_wait(uint32_t usec_గమ్యం_wait)
{
	SYS_పోర్టు_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_గమ్యం_wait);
	ఒకవేళ (usec_గమ్యం_wait == 0U) {
		SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_గమ్యం_wait);
		తిరిగి;
	}

#ఒకవేళ defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_గమ్యం_wait);
#లేదావేళ defined(CONFIG_SYS_CLOCK_EXISTS)
#ఒకవేళ defined(CONFIG_SYSTEM_CLOCK_HW_చక్రాలు_చొప్పున_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t ప్రారంభం_చక్రాలు;

	/* Get a consistent sample of the current frequency and cycle count. */
	కొరకు (;;) {
		uint32_t hz1 = sys_clock_hw_చక్రాలు_చొప్పున_sec();
		uint32_t current_చక్రాలు = k_చక్రం_get_32();
		uint32_t hz2 = sys_clock_hw_చక్రాలు_చొప్పున_sec();

		ఒకవేళ (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			కొనసాగు;
		}

		hz = hz1;
		ప్రారంభం_చక్రాలు = current_చక్రాలు;

		ఆపు;
	}

	uint32_t చక్రాలు_గమ్యం_wait = busy_wait_us_గమ్యం_cyc_ceil32(usec_గమ్యం_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	కొరకు (;;) {
		uint32_t hz1 = sys_clock_hw_చక్రాలు_చొప్పున_sec();
		uint32_t current_చక్రాలు = k_చక్రం_get_32();
		uint32_t hz2 = sys_clock_hw_చక్రాలు_చొప్పున_sec();

		ఒకవేళ (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			కొనసాగు;
		}

		ఒకవేళ (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			ప్రారంభం_చక్రాలు = (uint32_t)(((uint64_t)ప్రారంభం_చక్రాలు * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			చక్రాలు_గమ్యం_wait = busy_wait_us_గమ్యం_cyc_ceil32(usec_గమ్యం_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		ఒకవేళ ((current_చక్రాలు - ప్రారంభం_చక్రాలు) >= చక్రాలు_గమ్యం_wait) {
			ఆపు;
		}
	}
#లేకపోతే
	uint32_t ప్రారంభం_చక్రాలు = k_చక్రం_get_32();
	uint32_t చక్రాలు_గమ్యం_wait = k_us_గమ్యం_cyc_ceil32(usec_గమ్యం_wait);

	కొరకు (;;) {
		uint32_t current_చక్రాలు = k_చక్రం_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		ఒకవేళ ((current_చక్రాలు - ప్రారంభం_చక్రాలు) >= చక్రాలు_గమ్యం_wait) {
			ఆపు;
		}
	}
#అయితే_ముగింపు /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#లేకపోతే
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	గుర్తులేని పూర్ణాంకం loops_చొప్పున_usec = CONFIG_BUSYWAIT_CPU_LOOPS_చొప్పున_USEC;
	గుర్తులేని పూర్ణాంకం loops = loops_చొప్పున_usec * usec_గమ్యం_wait;

	అయితే (loops-- > 0) {
		arch_nop();
	}
#అయితే_ముగింపు

	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_గమ్యం_wait);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_busy_wait(uint32_t usec_గమ్యం_wait)
{
	z_impl_k_busy_wait(usec_గమ్యం_wait);
}
#అన్తర్భావ <zephyr/syscalls/k_busy_wait_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
