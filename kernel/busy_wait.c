/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/drivers/timer/system_timer.h>
#अन्तर्भाव <zephyr/sys/clock.h>
#अन्तर्भाव <kernel_arch_interface.h>

#यदि defined(CONFIG_SYSTEM_CLOCK_HW_आवृत्तयः_प्रत्येकम्_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
स्थिर अन्तःस्थ uint32_t busy_wait_us_गन्तव्यम्_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	ध्रुव uint64_t denom = 1000000ULL;
	uint64_t अङ्कः = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	प्रत्यागम (uint32_t)((अङ्कः + denom - 1U) / denom);
}
#यदि_अन्तः /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

शून्य z_impl_k_busy_wait(uint32_t usec_गन्तव्यम्_wait)
{
	SYS_पत्तनम्_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_गन्तव्यम्_wait);
	यदि (usec_गन्तव्यम्_wait == 0U) {
		SYS_पत्तनम्_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_गन्तव्यम्_wait);
		प्रत्यागम;
	}

#यदि defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_गन्तव्यम्_wait);
#नोचेद्यदि defined(CONFIG_SYS_CLOCK_EXISTS)
#यदि defined(CONFIG_SYSTEM_CLOCK_HW_आवृत्तयः_प्रत्येकम्_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t start_आवृत्तयः;

	/* Get a consistent sample of the current frequency and cycle count. */
	प्रति (;;) {
		uint32_t hz1 = sys_clock_hw_आवृत्तयः_प्रत्येकम्_sec();
		uint32_t current_आवृत्तयः = k_आवृत्तिः_get_32();
		uint32_t hz2 = sys_clock_hw_आवृत्तयः_प्रत्येकम्_sec();

		यदि (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			अनुवर्त;
		}

		hz = hz1;
		start_आवृत्तयः = current_आवृत्तयः;

		भङ्ग;
	}

	uint32_t आवृत्तयः_गन्तव्यम्_wait = busy_wait_us_गन्तव्यम्_cyc_ceil32(usec_गन्तव्यम्_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	प्रति (;;) {
		uint32_t hz1 = sys_clock_hw_आवृत्तयः_प्रत्येकम्_sec();
		uint32_t current_आवृत्तयः = k_आवृत्तिः_get_32();
		uint32_t hz2 = sys_clock_hw_आवृत्तयः_प्रत्येकम्_sec();

		यदि (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			अनुवर्त;
		}

		यदि (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			start_आवृत्तयः = (uint32_t)(((uint64_t)start_आवृत्तयः * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			आवृत्तयः_गन्तव्यम्_wait = busy_wait_us_गन्तव्यम्_cyc_ceil32(usec_गन्तव्यम्_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		यदि ((current_आवृत्तयः - start_आवृत्तयः) >= आवृत्तयः_गन्तव्यम्_wait) {
			भङ्ग;
		}
	}
#अन्यथा
	uint32_t start_आवृत्तयः = k_आवृत्तिः_get_32();
	uint32_t आवृत्तयः_गन्तव्यम्_wait = k_us_गन्तव्यम्_cyc_ceil32(usec_गन्तव्यम्_wait);

	प्रति (;;) {
		uint32_t current_आवृत्तयः = k_आवृत्तिः_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		यदि ((current_आवृत्तयः - start_आवृत्तयः) >= आवृत्तयः_गन्तव्यम्_wait) {
			भङ्ग;
		}
	}
#यदि_अन्तः /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#अन्यथा
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	अचिह्नित पूर्णाङ्क loops_प्रत्येकम्_usec = CONFIG_BUSYWAIT_CPU_LOOPS_प्रत्येकम्_USEC;
	अचिह्नित पूर्णाङ्क loops = loops_प्रत्येकम्_usec * usec_गन्तव्यम्_wait;

	यावत् (loops-- > 0) {
		arch_nop();
	}
#यदि_अन्तः

	SYS_पत्तनम्_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_गन्तव्यम्_wait);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_busy_wait(uint32_t usec_गन्तव्यम्_wait)
{
	z_impl_k_busy_wait(usec_गन्तव्यम्_wait);
}
#अन्तर्भाव <zephyr/syscalls/k_busy_wait_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
