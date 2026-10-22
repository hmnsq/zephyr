/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/drivers/timer/system_timer.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/logging/log.h>

LOG_MODULE_రిజిస్టరు(sys_clock_hw_చక్రాలు, CONFIG_KERNEL_LOG_LEVEL);

గుర్తులేని పూర్ణాంకం z_clock_hw_చక్రాలు_చొప్పున_sec = CONFIG_SYS_CLOCK_HW_చక్రాలు_చొప్పున_SEC;

#ఒకవేళ defined(CONFIG_SYSTEM_CLOCK_HW_చక్రాలు_చొప్పున_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
శూన్య __weak z_sys_clock_hw_చక్రాలు_చొప్పున_sec_update(uint32_t new_hz)
{
	ఒకవేళ (new_hz == 0U) {
		తిరిగి;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_చక్రాలు_చొప్పున_sec;

	ఒకవేళ (old_hz == new_hz) {
		తిరిగి;
	}

	z_clock_hw_చక్రాలు_చొప్పున_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#అయితే_ముగింపు /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#ఒకవేళ defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_చక్రాలు_చొప్పున_SEC_RUNTIME_UPDATE)
#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస గుర్తులేని పూర్ణాంకం z_vrfy_sys_clock_hw_చక్రాలు_చొప్పున_sec_runtime_get(శూన్య)
{
	తిరిగి z_impl_sys_clock_hw_చక్రాలు_చొప్పున_sec_runtime_get();
}
#అన్తర్భావ <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
#అయితే_ముగింపు /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
