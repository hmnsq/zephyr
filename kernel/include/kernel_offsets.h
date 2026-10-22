/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#جے_بنا_تعریف ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

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

GEN_ABS_SYM_BEGIN(وتھ_Abs_Syms)

GEN_وتھ_SYM(_cpu_t, current);
GEN_وتھ_SYM(_cpu_t, nested);
GEN_وتھ_SYM(_cpu_t, irq_ڈھیر);
GEN_وتھ_SYM(_cpu_t, arch);

GEN_وتھ_SYM(_kernel_t, cpus);

#إذا defined(CONFIG_FPU_SHARING)
GEN_وتھ_SYM(_cpu_t, fp_ctx);
#جے_انت /* CONFIG_FPU_SHARING */

#جے_تعریف_شدہ CONFIG_PM
GEN_وتھ_SYM(_kernel_t, idle);
#جے_انت /* CONFIG_PM */

#جے_بنا_تعریف CONFIG_SCHED_CPU_نقاب_PIN_ONLY
GEN_وتھ_SYM(_kernel_t, ready_q);
#جے_انت /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#جے_بنا_تعریف CONFIG_SMP
GEN_وتھ_SYM(_ready_q_t, cache);
#جے_انت /* CONFIG_SMP */

GEN_وتھ_SYM(thread_ادھار_t, user_options);

GEN_وتھ_SYM(_thread_t, ادھار);
GEN_وتھ_SYM(_thread_t, callee_saved);
GEN_وتھ_SYM(_thread_t, arch);

#جے_تعریف_شدہ CONFIG_USE_SWITCH
GEN_وتھ_SYM(_thread_t, switch_handle);
#جے_انت /* CONFIG_USE_SWITCH */

#جے_تعریف_شدہ CONFIG_THREAD_ڈھیر_INFO
GEN_وتھ_SYM(_thread_t, ڈھیر_info);
#جے_انت /* CONFIG_THREAD_STACK_INFO */

#جے_تعریف_شدہ CONFIG_THREAD_LOCAL_STORAGE
GEN_وتھ_SYM(_thread_t, tls);
#جے_انت /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, حجم(z_رکاوٹ_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, حجم(z_رکاوٹ_stacks));

/* member offsets in the device structure. Used in image post-processing */
#جے_تعریف_شدہ CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_وتھ,
		 offsetof(بنية device, deps));
#جے_انت /* CONFIG_DEVICE_DEPS */

#جے_تعریف_شدہ CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_وتھ,
		 offsetof(بنية device, pm));
#جے_انت /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_وتھ,
		 offsetof(بنية pm_device_ادھار, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#جے_انت /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
