/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#اگر_تعریف_نشده ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#تعریف_کن ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

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

GEN_ABS_SYM_BEGIN(جابه_جایی_Abs_Syms)

GEN_جابه_جایی_SYM(_cpu_t, current);
GEN_جابه_جایی_SYM(_cpu_t, nested);
GEN_جابه_جایی_SYM(_cpu_t, irq_پشته);
GEN_جابه_جایی_SYM(_cpu_t, arch);

GEN_جابه_جایی_SYM(_kernel_t, cpus);

#إذا defined(CONFIG_FPU_SHARING)
GEN_جابه_جایی_SYM(_cpu_t, fp_ctx);
#پایان_اگر /* CONFIG_FPU_SHARING */

#اگر_تعریف_شده CONFIG_PM
GEN_جابه_جایی_SYM(_kernel_t, idle);
#پایان_اگر /* CONFIG_PM */

#اگر_تعریف_نشده CONFIG_SCHED_CPU_ماسک_PIN_ONLY
GEN_جابه_جایی_SYM(_kernel_t, ready_q);
#پایان_اگر /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#اگر_تعریف_نشده CONFIG_SMP
GEN_جابه_جایی_SYM(_ready_q_t, cache);
#پایان_اگر /* CONFIG_SMP */

GEN_جابه_جایی_SYM(thread_پایه_t, user_options);

GEN_جابه_جایی_SYM(_thread_t, پایه);
GEN_جابه_جایی_SYM(_thread_t, callee_saved);
GEN_جابه_جایی_SYM(_thread_t, arch);

#اگر_تعریف_شده CONFIG_USE_SWITCH
GEN_جابه_جایی_SYM(_thread_t, switch_handle);
#پایان_اگر /* CONFIG_USE_SWITCH */

#اگر_تعریف_شده CONFIG_THREAD_پشته_INFO
GEN_جابه_جایی_SYM(_thread_t, پشته_info);
#پایان_اگر /* CONFIG_THREAD_STACK_INFO */

#اگر_تعریف_شده CONFIG_THREAD_LOCAL_STORAGE
GEN_جابه_جایی_SYM(_thread_t, tls);
#پایان_اگر /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, حجم(z_وقفه_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, حجم(z_وقفه_stacks));

/* member offsets in the device structure. Used in image post-processing */
#اگر_تعریف_شده CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_جابه_جایی,
		 offsetof(بنية device, deps));
#پایان_اگر /* CONFIG_DEVICE_DEPS */

#اگر_تعریف_شده CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_جابه_جایی,
		 offsetof(بنية device, pm));
#پایان_اگر /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_جابه_جایی,
		 offsetof(بنية pm_device_پایه, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#پایان_اگر /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
