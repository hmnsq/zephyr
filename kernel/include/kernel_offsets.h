/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#परिभाषय ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

#अन्तर्भाव <zephyr/device.h>
#अन्तर्भाव <zephyr/pm/device.h>
#अन्तर्भाव "kernel_internal.h"

#अन्तर्भाव <zephyr/syscall_list.h>

/* All of this is build time magic, but LCOV gets confused. Disable coverage
 * for this whole file.
 *
 * LCOV_EXCL_START
 */

/*
 * The final link step uses the symbol _OffsetAbsSyms to force the linkage of
 * offsets.o into the ELF image.
 */

GEN_ABS_SYM_BEGIN(विस्थापनम्_Abs_Syms)

GEN_विस्थापनम्_SYM(_cpu_t, current);
GEN_विस्थापनम्_SYM(_cpu_t, nested);
GEN_विस्थापनम्_SYM(_cpu_t, irq_राशिः);
GEN_विस्थापनम्_SYM(_cpu_t, arch);

GEN_विस्थापनम्_SYM(_kernel_t, cpus);

#यदि defined(CONFIG_FPU_SHARING)
GEN_विस्थापनम्_SYM(_cpu_t, fp_ctx);
#यदि_अन्तः /* CONFIG_FPU_SHARING */

#यदि_परिभाषितम् CONFIG_PM
GEN_विस्थापनम्_SYM(_kernel_t, idle);
#यदि_अन्तः /* CONFIG_PM */

#यदि_अपरिभाषितम् CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY
GEN_विस्थापनम्_SYM(_kernel_t, ready_q);
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#यदि_अपरिभाषितम् CONFIG_SMP
GEN_विस्थापनम्_SYM(_ready_q_t, cache);
#यदि_अन्तः /* CONFIG_SMP */

GEN_विस्थापनम्_SYM(thread_आधारः_t, user_options);

GEN_विस्थापनम्_SYM(_thread_t, आधारः);
GEN_विस्थापनम्_SYM(_thread_t, callee_saved);
GEN_विस्थापनम्_SYM(_thread_t, arch);

#यदि_परिभाषितम् CONFIG_USE_SWITCH
GEN_विस्थापनम्_SYM(_thread_t, switch_handle);
#यदि_अन्तः /* CONFIG_USE_SWITCH */

#यदि_परिभाषितम् CONFIG_THREAD_राशिः_INFO
GEN_विस्थापनम्_SYM(_thread_t, राशिः_info);
#यदि_अन्तः /* CONFIG_THREAD_STACK_INFO */

#यदि_परिभाषितम् CONFIG_THREAD_LOCAL_STORAGE
GEN_विस्थापनम्_SYM(_thread_t, tls);
#यदि_अन्तः /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, परिमाण(z_व्यवधानम्_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, परिमाण(z_व्यवधानम्_stacks));

/* member offsets in the device structure. Used in image post-processing */
#यदि_परिभाषितम् CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_विस्थापनम्,
		 offsetof(संरचना device, deps));
#यदि_अन्तः /* CONFIG_DEVICE_DEPS */

#यदि_परिभाषितम् CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_विस्थापनम्,
		 offsetof(संरचना device, pm));
#यदि_अन्तः /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_विस्थापनम्,
		 offsetof(संरचना pm_device_आधारः, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
