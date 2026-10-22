/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/drivers/timer/system_timer.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sys_clock_hw_cycles, CONFIG_KERNEL_LOG_LEVEL);

unsigned পূর্ণাঙ্ক z_clock_hw_cycles_per_sec = CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

#যদি defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
শূন্য __weak z_sys_clock_hw_cycles_per_sec_update(uint32_t new_hz)
{
	যদি (new_hz == 0U) {
		ঘূৰাই;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_cycles_per_sec;

	যদি (old_hz == new_hz) {
		ঘূৰাই;
	}

	z_clock_hw_cycles_per_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#endif /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#যদি defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE)
#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ unsigned পূর্ণাঙ্ক z_vrfy_sys_clock_hw_cycles_per_sec_runtime_get(শূন্য)
{
	ঘূৰাই z_impl_sys_clock_hw_cycles_per_sec_runtime_get();
}
#অন্তর্ভাৱ <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
