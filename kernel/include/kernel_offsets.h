/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

#شامل <zephyr/device.h>
#شامل <zephyr/pm/device.h>
#شامل "kernel_internal.h"

#شامل <zephyr/syscall_list.h>

/* All of this is build time magic, but LCOV gets confused. Disable coverage
 * for this whole file.
 *
 * LCOV_EXCL_START
 */

/*
 * The final link step uses the symbol _OffsetAbsSyms to force the linkage of
 * offsets.o into the ELF image.
 */

GEN_ABS_SYM_BEGIN(انحراف_Abs_Syms)

GEN_انحراف_SYM(_cpu_t, current);
GEN_انحراف_SYM(_cpu_t, nested);
GEN_انحراف_SYM(_cpu_t, irq_ڈھیر);
GEN_انحراف_SYM(_cpu_t, arch);

GEN_انحراف_SYM(_kernel_t, cpus);

#اگر defined(CONFIG_FPU_SHARING)
GEN_انحراف_SYM(_cpu_t, fp_ctx);
#اگر_ختم /* CONFIG_FPU_SHARING */

#اگر_تعریف_شدہ CONFIG_PM
GEN_انحراف_SYM(_kernel_t, idle);
#اگر_ختم /* CONFIG_PM */

#اگر_غیر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب_PIN_ONLY
GEN_انحراف_SYM(_kernel_t, ready_q);
#اگر_ختم /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#اگر_غیر_تعریف_شدہ CONFIG_SMP
GEN_انحراف_SYM(_ready_q_t, cache);
#اگر_ختم /* CONFIG_SMP */

GEN_انحراف_SYM(thread_بنیاد_t, user_options);

GEN_انحراف_SYM(_thread_t, بنیاد);
GEN_انحراف_SYM(_thread_t, callee_saved);
GEN_انحراف_SYM(_thread_t, arch);

#اگر_تعریف_شدہ CONFIG_USE_SWITCH
GEN_انحراف_SYM(_thread_t, switch_handle);
#اگر_ختم /* CONFIG_USE_SWITCH */

#اگر_تعریف_شدہ CONFIG_THREAD_ڈھیر_INFO
GEN_انحراف_SYM(_thread_t, ڈھیر_info);
#اگر_ختم /* CONFIG_THREAD_STACK_INFO */

#اگر_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
GEN_انحراف_SYM(_thread_t, tls);
#اگر_ختم /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, حجم(z_مداخلت_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, حجم(z_مداخلت_stacks));

/* member offsets in the device structure. Used in image post-processing */
#اگر_تعریف_شدہ CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_انحراف,
		 offsetof(ساخت device, deps));
#اگر_ختم /* CONFIG_DEVICE_DEPS */

#اگر_تعریف_شدہ CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_انحراف,
		 offsetof(ساخت device, pm));
#اگر_ختم /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_انحراف,
		 offsetof(ساخت pm_device_بنیاد, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
