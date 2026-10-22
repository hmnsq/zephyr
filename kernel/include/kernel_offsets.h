/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

#అన్తర్భావ <zephyr/device.h>
#అన్తర్భావ <zephyr/pm/device.h>
#అన్తర్భావ "kernel_internal.h"

#అన్తర్భావ <zephyr/syscall_list.h>

/* All of this is build time magic, but LCOV gets confused. Disable coverage
 * for this whole file.
 *
 * LCOV_EXCL_START
 */

/*
 * The final link step uses the symbol _OffsetAbsSyms to force the linkage of
 * offsets.o into the ELF image.
 */

GEN_ABS_SYM_BEGIN(స్థానభ్రంశం_Abs_Syms)

GEN_స్థానభ్రంశం_SYM(_cpu_t, current);
GEN_స్థానభ్రంశం_SYM(_cpu_t, nested);
GEN_స్థానభ్రంశం_SYM(_cpu_t, irq_దొంతర);
GEN_స్థానభ్రంశం_SYM(_cpu_t, arch);

GEN_స్థానభ్రంశం_SYM(_kernel_t, cpus);

#ఒకవేళ defined(CONFIG_FPU_SHARING)
GEN_స్థానభ్రంశం_SYM(_cpu_t, fp_ctx);
#అయితే_ముగింపు /* CONFIG_FPU_SHARING */

#నిర్వచితమైతే CONFIG_PM
GEN_స్థానభ్రంశం_SYM(_kernel_t, idle);
#అయితే_ముగింపు /* CONFIG_PM */

#నిర్వచితం_కాకపోతే CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY
GEN_స్థానభ్రంశం_SYM(_kernel_t, ready_q);
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#నిర్వచితం_కాకపోతే CONFIG_SMP
GEN_స్థానభ్రంశం_SYM(_ready_q_t, cache);
#అయితే_ముగింపు /* CONFIG_SMP */

GEN_స్థానభ్రంశం_SYM(thread_పునాది_t, user_options);

GEN_స్థానభ్రంశం_SYM(_thread_t, పునాది);
GEN_స్థానభ్రంశం_SYM(_thread_t, callee_saved);
GEN_స్థానభ్రంశం_SYM(_thread_t, arch);

#నిర్వచితమైతే CONFIG_USE_SWITCH
GEN_స్థానభ్రంశం_SYM(_thread_t, switch_handle);
#అయితే_ముగింపు /* CONFIG_USE_SWITCH */

#నిర్వచితమైతే CONFIG_THREAD_దొంతర_INFO
GEN_స్థానభ్రంశం_SYM(_thread_t, దొంతర_info);
#అయితే_ముగింపు /* CONFIG_THREAD_STACK_INFO */

#నిర్వచితమైతే CONFIG_THREAD_LOCAL_STORAGE
GEN_స్థానభ్రంశం_SYM(_thread_t, tls);
#అయితే_ముగింపు /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, పరిమాణం(z_అంతరాయం_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, పరిమాణం(z_అంతరాయం_stacks));

/* member offsets in the device structure. Used in image post-processing */
#నిర్వచితమైతే CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_స్థానభ్రంశం,
		 offsetof(నిర్మాణం device, deps));
#అయితే_ముగింపు /* CONFIG_DEVICE_DEPS */

#నిర్వచితమైతే CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_స్థానభ్రంశం,
		 offsetof(నిర్మాణం device, pm));
#అయితే_ముగింపు /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_స్థానభ్రంశం,
		 offsetof(నిర్మాణం pm_device_పునాది, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
