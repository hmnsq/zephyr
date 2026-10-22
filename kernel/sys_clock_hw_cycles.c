/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/drivers/timer/system_timer.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sys_clock_hw_cycles, CONFIG_KERNEL_LOG_LEVEL);

unsigned ಪೂರ್ಣಾಙ್ಕ z_clock_hw_cycles_per_sec = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

#ಯದಿ defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
ಶೂನ್ಯ __weak z_sys_clock_hw_cycles_per_sec_update(uint32_t new_hz)
{
	ಯದಿ (new_hz == 0U) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_cycles_per_sec;

	ಯದಿ (old_hz == new_hz) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	z_clock_hw_cycles_per_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#endif /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#ಯದಿ defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ unsigned ಪೂರ್ಣಾಙ್ಕ z_vrfy_sys_clock_hw_cycles_per_sec_runtime_get(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_sys_clock_hw_cycles_per_sec_runtime_get();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
