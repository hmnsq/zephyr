/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/drivers/timer/system_timer.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>

LOG_MODULE_ਰਜਿਸਟਰ(sys_clock_hw_ਚੱਕਰਾਂ, CONFIG_KERNEL_LOG_LEVEL);

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ z_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec = CONFIG_SYS_CLOCK_HW_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_SEC;

#ਜੇ defined(CONFIG_SYSTEM_CLOCK_HW_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
ਖਾਲੀ __weak z_sys_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec_update(uint32_t new_hz)
{
	ਜੇ (new_hz == 0U) {
		ਵਾਪਸ;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec;

	ਜੇ (old_hz == new_hz) {
		ਵਾਪਸ;
	}

	z_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#ਜੇ_ਅੰਤ /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#ਜੇ defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_SEC_RUNTIME_UPDATE)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ z_vrfy_sys_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec_runtime_get(ਖਾਲੀ)
{
	ਵਾਪਸ z_impl_sys_clock_hw_ਚੱਕਰਾਂ_ਪ੍ਰਤੀ_sec_runtime_get();
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
#ਜੇ_ਅੰਤ /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
