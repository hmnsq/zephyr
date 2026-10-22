/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/drivers/timer/system_timer.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sys_clock_hw_cycles, CONFIG_KERNEL_LOG_LEVEL);

अचिन्हित पूर्णांक z_clock_hw_cycles_per_sec = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

#अगर defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
व्योम __weak z_sys_clock_hw_cycles_per_sec_update(uint32_t new_hz)
{
	अगर (new_hz == 0U) {
		वापस;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_cycles_per_sec;

	अगर (old_hz == new_hz) {
		वापस;
	}

	z_clock_hw_cycles_per_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#पूर्ण_अगर /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#अगर defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत अचिन्हित पूर्णांक z_vrfy_sys_clock_hw_cycles_per_sec_runtime_get(व्योम)
{
	वापस z_impl_sys_clock_hw_cycles_per_sec_runtime_get();
}
#समावेश <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
#पूर्ण_अगर /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
