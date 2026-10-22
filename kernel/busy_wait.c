/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/drivers/timer/system_timer.h>
#ضمن <zephyr/sys/clock.h>
#ضمن <kernel_arch_interface.h>

#إذا defined(CONFIG_SYSTEM_CLOCK_HW_دورې_فی_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
ساكن مضمن uint32_t busy_wait_us_منزل_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	ثابت uint64_t denom = 1000000ULL;
	uint64_t عدد = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	أرجع (uint32_t)((عدد + denom - 1U) / denom);
}
#د_که_پای /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

فراغ z_impl_k_busy_wait(uint32_t usec_منزل_wait)
{
	SYS_پورټ_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_منزل_wait);
	إذا (usec_منزل_wait == 0U) {
		SYS_پورټ_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_منزل_wait);
		أرجع;
	}

#إذا defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_منزل_wait);
#وإلاإذا defined(CONFIG_SYS_CLOCK_EXISTS)
#إذا defined(CONFIG_SYSTEM_CLOCK_HW_دورې_فی_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t پیل_دورې;

	/* Get a consistent sample of the current frequency and cycle count. */
	لكل (;;) {
		uint32_t hz1 = sys_clock_hw_دورې_فی_sec();
		uint32_t current_دورې = k_دوره_get_32();
		uint32_t hz2 = sys_clock_hw_دورې_فی_sec();

		إذا (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			استمر;
		}

		hz = hz1;
		پیل_دورې = current_دورې;

		اكسر;
	}

	uint32_t دورې_منزل_wait = busy_wait_us_منزل_cyc_ceil32(usec_منزل_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	لكل (;;) {
		uint32_t hz1 = sys_clock_hw_دورې_فی_sec();
		uint32_t current_دورې = k_دوره_get_32();
		uint32_t hz2 = sys_clock_hw_دورې_فی_sec();

		إذا (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			استمر;
		}

		إذا (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			پیل_دورې = (uint32_t)(((uint64_t)پیل_دورې * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			دورې_منزل_wait = busy_wait_us_منزل_cyc_ceil32(usec_منزل_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		إذا ((current_دورې - پیل_دورې) >= دورې_منزل_wait) {
			اكسر;
		}
	}
#وإلا
	uint32_t پیل_دورې = k_دوره_get_32();
	uint32_t دورې_منزل_wait = k_us_منزل_cyc_ceil32(usec_منزل_wait);

	لكل (;;) {
		uint32_t current_دورې = k_دوره_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		إذا ((current_دورې - پیل_دورې) >= دورې_منزل_wait) {
			اكسر;
		}
	}
#د_که_پای /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#وإلا
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	بینښه صحيح loops_فی_usec = CONFIG_BUSYWAIT_CPU_LOOPS_فی_USEC;
	بینښه صحيح loops = loops_فی_usec * usec_منزل_wait;

	طالما (loops-- > 0) {
		arch_nop();
	}
#د_که_پای

	SYS_پورټ_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_منزل_wait);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_busy_wait(uint32_t usec_منزل_wait)
{
	z_impl_k_busy_wait(usec_منزل_wait);
}
#ضمن <zephyr/syscalls/k_busy_wait_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
