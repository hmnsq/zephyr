/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/drivers/timer/system_timer.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sys_clock_hw_cycles, CONFIG_KERNEL_LOG_LEVEL);

unsigned முழு z_clock_hw_cycles_per_sec = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

#எனில் defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
வெறுமை __weak z_sys_clock_hw_cycles_per_sec_update(uint32_t new_hz)
{
	எனில் (new_hz == 0U) {
		திரும்பு;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_cycles_per_sec;

	எனில் (old_hz == new_hz) {
		திரும்பு;
	}

	z_clock_hw_cycles_per_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#endif /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#எனில் defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
#ifdef CONFIG_USERSPACE
நிலையான உள்வரி unsigned முழு z_vrfy_sys_clock_hw_cycles_per_sec_runtime_get(வெறுமை)
{
	திரும்பு z_impl_sys_clock_hw_cycles_per_sec_runtime_get();
}
#அந்தர்பாவ <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
