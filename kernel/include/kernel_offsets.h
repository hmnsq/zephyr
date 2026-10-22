/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

#ضمن <zephyr/device.h>
#ضمن <zephyr/pm/device.h>
#ضمن "kernel_internal.h"

#ضمن <zephyr/syscall_list.h>

/* All of this is build time magic, but LCOV gets confused. Disable coverage
 * for this whole file.
 *
 * LCOV_EXCL_START
 */

/*
 * The final link step uses the symbol _OffsetAbsSyms to force the linkage of
 * offsets.o into the ELF image.
 */

GEN_ABS_SYM_BEGIN(وٿي_Abs_Syms)

GEN_وٿي_SYM(_cpu_t, current);
GEN_وٿي_SYM(_cpu_t, nested);
GEN_وٿي_SYM(_cpu_t, irq_ڍير);
GEN_وٿي_SYM(_cpu_t, arch);

GEN_وٿي_SYM(_kernel_t, cpus);

#إذا defined(CONFIG_FPU_SHARING)
GEN_وٿي_SYM(_cpu_t, fp_ctx);
#جيڪڏهن_پورو /* CONFIG_FPU_SHARING */

#جيڪڏهن_وصف_ٿيل CONFIG_PM
GEN_وٿي_SYM(_kernel_t, idle);
#جيڪڏهن_پورو /* CONFIG_PM */

#جيڪڏهن_وصف_نه_ٿيل CONFIG_SCHED_CPU_نقاب_PIN_ONLY
GEN_وٿي_SYM(_kernel_t, ready_q);
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
GEN_وٿي_SYM(_ready_q_t, cache);
#جيڪڏهن_پورو /* CONFIG_SMP */

GEN_وٿي_SYM(thread_بنياد_t, user_options);

GEN_وٿي_SYM(_thread_t, بنياد);
GEN_وٿي_SYM(_thread_t, callee_saved);
GEN_وٿي_SYM(_thread_t, arch);

#جيڪڏهن_وصف_ٿيل CONFIG_USE_SWITCH
GEN_وٿي_SYM(_thread_t, switch_handle);
#جيڪڏهن_پورو /* CONFIG_USE_SWITCH */

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_ڍير_INFO
GEN_وٿي_SYM(_thread_t, ڍير_info);
#جيڪڏهن_پورو /* CONFIG_THREAD_STACK_INFO */

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_LOCAL_STORAGE
GEN_وٿي_SYM(_thread_t, tls);
#جيڪڏهن_پورو /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, حجم(z_مداخلت_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, حجم(z_مداخلت_stacks));

/* member offsets in the device structure. Used in image post-processing */
#جيڪڏهن_وصف_ٿيل CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_وٿي,
		 offsetof(بنية device, deps));
#جيڪڏهن_پورو /* CONFIG_DEVICE_DEPS */

#جيڪڏهن_وصف_ٿيل CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_وٿي,
		 offsetof(بنية device, pm));
#جيڪڏهن_پورو /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_وٿي,
		 offsetof(بنية pm_device_بنياد, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
