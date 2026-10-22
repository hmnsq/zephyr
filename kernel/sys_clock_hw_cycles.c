/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/drivers/timer/system_timer.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/logging/log.h>

LOG_MODULE_पञ्जिकाकोशः(sys_clock_hw_आवृत्तयः, CONFIG_KERNEL_LOG_LEVEL);

अचिह्नित पूर्णाङ्क z_clock_hw_आवृत्तयः_प्रत्येकम्_sec = CONFIG_SYS_CLOCK_HW_आवृत्तयः_प्रत्येकम्_SEC;

#यदि defined(CONFIG_SYSTEM_CLOCK_HW_आवृत्तयः_प्रत्येकम्_SEC_RUNTIME_UPDATE)
/*
 * Weak default: only publish the new frequency value.
 * System timer drivers that rescale the cycle counter, cache derived
 * constants, or need to reprogram hardware should override this symbol.
 */
शून्य __weak z_sys_clock_hw_आवृत्तयः_प्रत्येकम्_sec_update(uint32_t new_hz)
{
	यदि (new_hz == 0U) {
		प्रत्यागम;
	}

	uint32_t old_hz = (uint32_t)z_clock_hw_आवृत्तयः_प्रत्येकम्_sec;

	यदि (old_hz == new_hz) {
		प्रत्यागम;
	}

	z_clock_hw_आवृत्तयः_प्रत्येकम्_sec = new_hz;

	LOG_WRN("system timer frequency updated at runtime (%u -> %u Hz) "
		"without a %s() override",
		old_hz, new_hz, __func__);
}
#यदि_अन्तः /* CONFIG_SYSTEM_CLOCK_HW_CYCLES_PER_SEC_RUNTIME_UPDATE */

#यदि defined(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) || \
	defined(CONFIG_SYSTEM_CLOCK_HW_आवृत्तयः_प्रत्येकम्_SEC_RUNTIME_UPDATE)
#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ अचिह्नित पूर्णाङ्क z_vrfy_sys_clock_hw_आवृत्तयः_प्रत्येकम्_sec_runtime_get(शून्य)
{
	प्रत्यागम z_impl_sys_clock_hw_आवृत्तयः_प्रत्येकम्_sec_runtime_get();
}
#अन्तर्भाव <zephyr/syscalls/sys_clock_hw_cycles_per_sec_runtime_get_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
#यदि_अन्तः /* CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME */
