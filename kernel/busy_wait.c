/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/drivers/timer/system_timer.h>
#شامل <zephyr/sys/clock.h>
#شامل <kernel_arch_interface.h>

#اگر defined(CONFIG_SYSTEM_CLOCK_HW_ادوار_فی_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
ساکن اندرونی uint32_t busy_wait_us_مقصد_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	ثابت uint64_t denom = 1000000ULL;
	uint64_t عدد = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	واپس (uint32_t)((عدد + denom - 1U) / denom);
}
#اگر_ختم /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

خالی z_impl_k_busy_wait(uint32_t usec_مقصد_wait)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_مقصد_wait);
	اگر (usec_مقصد_wait == 0U) {
		SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_مقصد_wait);
		واپس;
	}

#اگر defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_مقصد_wait);
#وگرنہ_اگر defined(CONFIG_SYS_CLOCK_EXISTS)
#اگر defined(CONFIG_SYSTEM_CLOCK_HW_ادوار_فی_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t آغاز_ادوار;

	/* Get a consistent sample of the current frequency and cycle count. */
	برائے (;;) {
		uint32_t hz1 = sys_clock_hw_ادوار_فی_sec();
		uint32_t current_ادوار = k_چکر_get_32();
		uint32_t hz2 = sys_clock_hw_ادوار_فی_sec();

		اگر (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			جاری;
		}

		hz = hz1;
		آغاز_ادوار = current_ادوار;

		توڑو;
	}

	uint32_t ادوار_مقصد_wait = busy_wait_us_مقصد_cyc_ceil32(usec_مقصد_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	برائے (;;) {
		uint32_t hz1 = sys_clock_hw_ادوار_فی_sec();
		uint32_t current_ادوار = k_چکر_get_32();
		uint32_t hz2 = sys_clock_hw_ادوار_فی_sec();

		اگر (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			جاری;
		}

		اگر (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			آغاز_ادوار = (uint32_t)(((uint64_t)آغاز_ادوار * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			ادوار_مقصد_wait = busy_wait_us_مقصد_cyc_ceil32(usec_مقصد_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		اگر ((current_ادوار - آغاز_ادوار) >= ادوار_مقصد_wait) {
			توڑو;
		}
	}
#ورنہ
	uint32_t آغاز_ادوار = k_چکر_get_32();
	uint32_t ادوار_مقصد_wait = k_us_مقصد_cyc_ceil32(usec_مقصد_wait);

	برائے (;;) {
		uint32_t current_ادوار = k_چکر_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		اگر ((current_ادوار - آغاز_ادوار) >= ادوار_مقصد_wait) {
			توڑو;
		}
	}
#اگر_ختم /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#ورنہ
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	غیرعلامتی صحیح loops_فی_usec = CONFIG_BUSYWAIT_CPU_LOOPS_فی_USEC;
	غیرعلامتی صحیح loops = loops_فی_usec * usec_مقصد_wait;

	جب (loops-- > 0) {
		arch_nop();
	}
#اگر_ختم

	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_مقصد_wait);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_busy_wait(uint32_t usec_مقصد_wait)
{
	z_impl_k_busy_wait(usec_مقصد_wait);
}
#شامل <zephyr/syscalls/k_busy_wait_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
