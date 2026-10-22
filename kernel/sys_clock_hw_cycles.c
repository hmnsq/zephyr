/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/drivers/timer/system_timer.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/logging/log.h>

LOG_MODULE_رجسٹر(sys_clock_hw_ادوار, CONFIG_KERNEL_LOG_LEVEL);

غیرعلامتی صحیح z_clock_hw_ادوار_فی_sec = CONFIG_SYS_CLOCK_HW_ادوار_فی_SEC;

#اگر defined(CONFIG_SYSTEM_CLOCK_HW_ادوار_فی_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
خالی __weak z_sys_clock_hw_ادوار_فی_sec_update(uint32_t new_hz)
{
	اگر (new_hz == 0U) {
		واپس;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_ادوار_فی_sec;

	اگر (old_hz == new_hz) {
		واپس;
	}

	z_clock_hw_ادوار_فی_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#اگر_ختم /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#اگر defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_ادوار_فی_SEC_RUNTIME_UPDATE)
#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی غیرعلامتی صحیح z_vrfy_sys_clock_hw_ادوار_فی_sec_runtime_get(خالی)
{
	واپس z_impl_sys_clock_hw_ادوار_فی_sec_runtime_get();
}
#شامل <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
#اگر_ختم /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
