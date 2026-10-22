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

LOG_MODULE_ثبات(sys_clock_hw_چرخهها, CONFIG_KERNEL_LOG_LEVEL);

بیعلامت صحيح z_clock_hw_چرخهها_در_هر_sec = CONFIG_SYS_CLOCK_HW_چرخهها_در_هر_SEC;

#إذا defined(CONFIG_SYSTEM_CLOCK_HW_چرخهها_در_هر_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
فراغ __weak z_sys_clock_hw_چرخهها_در_هر_sec_update(uint32_t new_hz)
{
	إذا (new_hz == 0U) {
		أرجع;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_چرخهها_در_هر_sec;

	إذا (old_hz == new_hz) {
		أرجع;
	}

	z_clock_hw_چرخهها_در_هر_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#پایان_اگر /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#إذا defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_چرخهها_در_هر_SEC_RUNTIME_UPDATE)
#اگر_تعریف_شده CONFIG_USERSPACE
ساكن مضمن بیعلامت صحيح z_vrfy_sys_clock_hw_چرخهها_در_هر_sec_runtime_get(فراغ)
{
	أرجع z_impl_sys_clock_hw_چرخهها_در_هر_sec_runtime_get();
}
#ضمن <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#پایان_اگر /* CONFIG_USERSPACE */
#پایان_اگر /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
