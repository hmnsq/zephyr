/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

#समावेश <zephyr/device.h>
#समावेश <zephyr/pm/device.h>
#समावेश "kernel_internal.h"

#समावेश <zephyr/syscall_list.h>

/* All of this is build time magic, but LCOV gets confused. Disable coverage
 * for this whole file.
 *
 * LCOV_EXCL_START
 */

/*
 * The final link step uses the symbol _OffsetAbsSyms to force the linkage of
 * offsets.o into the ELF image.
 */

GEN_ABS_SYM_BEGIN(_OffsetAbsSyms)

GEN_OFFSET_SYM(_cpu_t, current);
GEN_OFFSET_SYM(_cpu_t, nested);
GEN_OFFSET_SYM(_cpu_t, irq_ढेर);
GEN_OFFSET_SYM(_cpu_t, arch);

GEN_OFFSET_SYM(_kernel_t, cpus);

#अगर defined(CONFIG_FPU_SHARING)
GEN_OFFSET_SYM(_cpu_t, fp_ctx);
#पूर्ण_अगर /* CONFIG_FPU_SHARING */

#अगर_घोषित CONFIG_PM
GEN_OFFSET_SYM(_kernel_t, idle);
#पूर्ण_अगर /* CONFIG_PM */

#अगर_अघोषित CONFIG_SCHED_CPU_MASK_PIN_ONLY
GEN_OFFSET_SYM(_kernel_t, ready_q);
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#अगर_अघोषित CONFIG_SMP
GEN_OFFSET_SYM(_ready_q_t, cache);
#पूर्ण_अगर /* CONFIG_SMP */

GEN_OFFSET_SYM(_thread_base_t, user_options);

GEN_OFFSET_SYM(_thread_t, base);
GEN_OFFSET_SYM(_thread_t, callee_saved);
GEN_OFFSET_SYM(_thread_t, arch);

#अगर_घोषित CONFIG_USE_SWITCH
GEN_OFFSET_SYM(_thread_t, switch_handle);
#पूर्ण_अगर /* CONFIG_USE_SWITCH */

#अगर_घोषित CONFIG_THREAD_ढेर_INFO
GEN_OFFSET_SYM(_thread_t, ढेर_info);
#पूर्ण_अगर /* CONFIG_THREAD_STACK_INFO */

#अगर_घोषित CONFIG_THREAD_LOCAL_STORAGE
GEN_OFFSET_SYM(_thread_t, tls);
#पूर्ण_अगर /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, माप(z_interrupt_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, माप(z_interrupt_stacks));

/* member offsets in the device structure. Used in image post-processing */
#अगर_घोषित CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(_DEVICE_STRUCT_HANDLES_OFFSET,
		 दुरत्व(काष्ठा device, deps));
#पूर्ण_अगर /* CONFIG_DEVICE_DEPS */

#अगर_घोषित CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(_DEVICE_STRUCT_PM_OFFSET,
		 दुरत्व(काष्ठा device, pm));
#पूर्ण_अगर /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(_PM_DEVICE_STRUCT_FLAGS_OFFSET,
		 दुरत्व(काष्ठा pm_device_base, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
