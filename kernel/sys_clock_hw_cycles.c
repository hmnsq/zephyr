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

LOG_MODULE_ರಿಜಿಸ್ಟರ್(sys_clock_hw_ಚಕ್ರಗಳು, CONFIG_KERNEL_LOG_LEVEL);

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ z_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec = CONFIG_SYS_CLOCK_HW_ಚಕ್ರಗಳು_ಪ್ರತಿ_SEC;

#ಆದರೆ defined(CONFIG_SYSTEM_CLOCK_HW_ಚಕ್ರಗಳು_ಪ್ರತಿ_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
ಶೂನ್ಯ __weak z_sys_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec_update(uint32_t new_hz)
{
	ಆದರೆ (new_hz == 0U) {
		ಹಿಂತಿರುಗಿ;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec;

	ಆದರೆ (old_hz == new_hz) {
		ಹಿಂತಿರುಗಿ;
	}

	z_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#ಆದರೆ defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_ಚಕ್ರಗಳು_ಪ್ರತಿ_SEC_RUNTIME_UPDATE)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ z_vrfy_sys_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec_runtime_get(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ z_impl_sys_clock_hw_ಚಕ್ರಗಳು_ಪ್ರತಿ_sec_runtime_get();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
