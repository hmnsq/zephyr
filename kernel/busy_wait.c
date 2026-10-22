/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/drivers/timer/system_timer.h>
#समावेश <zephyr/sys/clock.h>
#समावेश <kernel_arch_interface.h>

#अगर defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
/* Convert microseconds to cycles using a caller-supplied frequency sample.
 *
 * When the system timer frequency can change at runtime, conversions must use
 * the same stable frequency sample that was paired with the cycle counter read.
 */
अटल अंतरभूत uint32_t busy_wait_us_to_cyc_ceil32(uint32_t usec, uint32_t hz)
{
	/* microseconds per second */
	स्थिर uint64_t denom = 1000000ULL;
	uint64_t अंक = (uint64_t)usec * (uint64_t)hz;

	/* Round up to avoid returning early. */
	वापस (uint32_t)((अंक + denom - 1U) / denom);
}
#पूर्ण_अगर /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

व्योम z_impl_k_busy_wait(uint32_t usec_to_wait)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, busy_wait, usec_to_wait);
	अगर (usec_to_wait == 0U) {
		SYS_PORT_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_to_wait);
		वापस;
	}

#अगर defined(CONFIG_ARCH_HAS_CUSTOM_BUSY_WAIT)
	arch_busy_wait(usec_to_wait);
#या_अगर defined(CONFIG_SYS_CLOCK_EXISTS)
#अगर defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
	/*
	 * When the system timer frequency can change at runtime, the frequency
	 * used for k_cycle_get_32() conversions may change during the busy wait.
	 * Sample frequency and cycle count consistently, and adjust if the
	 * frequency changes.
	 */
	uint32_t hz;
	uint32_t start_cycles;

	/* Get a consistent sample of the current frequency and cycle count. */
	क्रम (;;) {
		uint32_t hz1 = sys_clock_hw_cycles_per_sec();
		uint32_t current_cycles = k_cycle_get_32();
		uint32_t hz2 = sys_clock_hw_cycles_per_sec();

		अगर (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			जारी;
		}

		hz = hz1;
		start_cycles = current_cycles;

		अवरोध;
	}

	uint32_t cycles_to_wait = busy_wait_us_to_cyc_ceil32(usec_to_wait, hz);

	/* Now busy wait, adjusting if the frequency changes. */
	क्रम (;;) {
		uint32_t hz1 = sys_clock_hw_cycles_per_sec();
		uint32_t current_cycles = k_cycle_get_32();
		uint32_t hz2 = sys_clock_hw_cycles_per_sec();

		अगर (hz1 != hz2) {
			/* Frequency changed while sampling; retry. */
			जारी;
		}

		अगर (hz1 != hz) {
			/* Keep our reference point in the same scale as the cycle counter. */
			start_cycles = (uint32_t)(((uint64_t)start_cycles * (uint64_t)hz1) /
						  (uint64_t)hz);
			hz = hz1;
			cycles_to_wait = busy_wait_us_to_cyc_ceil32(usec_to_wait, hz);
		}

		/* This handles the rollover on an unsigned 32-bit value. */
		अगर ((current_cycles - start_cycles) >= cycles_to_wait) {
			अवरोध;
		}
	}
#अन्यथा
	uint32_t start_cycles = k_cycle_get_32();
	uint32_t cycles_to_wait = k_us_to_cyc_ceil32(usec_to_wait);

	क्रम (;;) {
		uint32_t current_cycles = k_cycle_get_32();

		/* This handles the rollover on an unsigned 32-bit value */
		अगर ((current_cycles - start_cycles) >= cycles_to_wait) {
			अवरोध;
		}
	}
#पूर्ण_अगर /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */
#अन्यथा
	/*
	 * Crude busy loop for the purpose of being able to configure out
	 * system timer support.
	 */
	अचिन्हित पूर्णांक loops_per_usec = CONFIG_BUSYWAIT_CPU_LOOPS_PER_USEC;
	अचिन्हित पूर्णांक loops = loops_per_usec * usec_to_wait;

	जबतक (loops-- > 0) {
		arch_nop();
	}
#पूर्ण_अगर

	SYS_PORT_TRACING_FUNC_EXIT(k_thread, busy_wait, usec_to_wait);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_busy_wait(uint32_t usec_to_wait)
{
	z_impl_k_busy_wait(usec_to_wait);
}
#समावेश <zephyr/syscalls/k_busy_wait_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
