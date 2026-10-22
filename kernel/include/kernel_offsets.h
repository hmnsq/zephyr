/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

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

GEN_ABS_SYM_BEGIN(دننه_واټن_Abs_Syms)

GEN_دننه_واټن_SYM(_cpu_t, current);
GEN_دننه_واټن_SYM(_cpu_t, nested);
GEN_دننه_واټن_SYM(_cpu_t, irq_ډېری);
GEN_دننه_واټن_SYM(_cpu_t, arch);

GEN_دننه_واټن_SYM(_kernel_t, cpus);

#إذا defined(CONFIG_FPU_SHARING)
GEN_دننه_واټن_SYM(_cpu_t, fp_ctx);
#د_که_پای /* CONFIG_FPU_SHARING */

#که_تعریف_شوی CONFIG_PM
GEN_دننه_واټن_SYM(_kernel_t, idle);
#د_که_پای /* CONFIG_PM */

#که_نه_تعریف_شوی CONFIG_SCHED_CPU_ماسک_PIN_ONLY
GEN_دننه_واټن_SYM(_kernel_t, ready_q);
#د_که_پای /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#که_نه_تعریف_شوی CONFIG_SMP
GEN_دننه_واټن_SYM(_ready_q_t, cache);
#د_که_پای /* CONFIG_SMP */

GEN_دننه_واټن_SYM(thread_بنسټ_t, user_options);

GEN_دننه_واټن_SYM(_thread_t, بنسټ);
GEN_دننه_واټن_SYM(_thread_t, callee_saved);
GEN_دننه_واټن_SYM(_thread_t, arch);

#که_تعریف_شوی CONFIG_USE_SWITCH
GEN_دننه_واټن_SYM(_thread_t, switch_handle);
#د_که_پای /* CONFIG_USE_SWITCH */

#که_تعریف_شوی CONFIG_THREAD_ډېری_INFO
GEN_دننه_واټن_SYM(_thread_t, ډېری_info);
#د_که_پای /* CONFIG_THREAD_STACK_INFO */

#که_تعریف_شوی CONFIG_THREAD_LOCAL_STORAGE
GEN_دننه_واټن_SYM(_thread_t, tls);
#د_که_پای /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, حجم(z_مداخله_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, حجم(z_مداخله_stacks));

/* member offsets in the device structure. Used in image post-processing */
#که_تعریف_شوی CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_دننه_واټن,
		 offsetof(بنية device, deps));
#د_که_پای /* CONFIG_DEVICE_DEPS */

#که_تعریف_شوی CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_دننه_واټن,
		 offsetof(بنية device, pm));
#د_که_پای /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_دننه_واټن,
		 offsetof(بنية pm_device_بنسټ, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
