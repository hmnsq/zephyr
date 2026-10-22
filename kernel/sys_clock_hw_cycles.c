/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/drivers/timer/system_timer.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sys_clock_hw_cycles, CONFIG_KERNEL_LOG_LEVEL);

unsigned صحيح z_clock_hw_cycles_per_sec = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

#אם defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
فراغ __weak z_sys_clock_hw_cycles_per_sec_update(uint32_t new_hz)
{
	אם (new_hz == 0U) {
		החזר;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_cycles_per_sec;

	אם (old_hz == new_hz) {
		החזר;
	}

	z_clock_hw_cycles_per_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#endif /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#אם defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
#ifdef CONFIG_USERSPACE
ساكن مضمن unsigned صحيح z_vrfy_sys_clock_hw_cycles_per_sec_runtime_get(فراغ)
{
	החזר z_impl_sys_clock_hw_cycles_per_sec_runtime_get();
}
#ضمن <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
