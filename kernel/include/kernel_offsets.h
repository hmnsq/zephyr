/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

#ಅನ್ತರ್ಭಾವ <zephyr/device.h>
#ಅನ್ತರ್ಭಾವ <zephyr/pm/device.h>
#ಅನ್ತರ್ಭಾವ "kernel_internal.h"

#ಅನ್ತರ್ಭಾವ <zephyr/syscall_list.h>

/* All of this is build time magic, but LCOV gets confused. Disable coverage
 * for this whole file.
 *
 * LCOV_EXCL_START
 */

/*
 * The final link step uses the symbol _OffsetAbsSyms to force the linkage of
 * offsets.o into the ELF image.
 */

GEN_ABS_SYM_BEGIN(ಸ್ಥಾನಾಂತರ_Abs_Syms)

GEN_ಸ್ಥಾನಾಂತರ_SYM(_cpu_t, current);
GEN_ಸ್ಥಾನಾಂತರ_SYM(_cpu_t, nested);
GEN_ಸ್ಥಾನಾಂತರ_SYM(_cpu_t, irq_ರಾಶಿ);
GEN_ಸ್ಥಾನಾಂತರ_SYM(_cpu_t, arch);

GEN_ಸ್ಥಾನಾಂತರ_SYM(_kernel_t, cpus);

#ಆದರೆ defined(CONFIG_FPU_SHARING)
GEN_ಸ್ಥಾನಾಂತರ_SYM(_cpu_t, fp_ctx);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_FPU_SHARING */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PM
GEN_ಸ್ಥಾನಾಂತರ_SYM(_kernel_t, idle);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_PM */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY
GEN_ಸ್ಥಾನಾಂತರ_SYM(_kernel_t, ready_q);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
GEN_ಸ್ಥಾನಾಂತರ_SYM(_ready_q_t, cache);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

GEN_ಸ್ಥಾನಾಂತರ_SYM(thread_ತಳ_t, user_options);

GEN_ಸ್ಥಾನಾಂತರ_SYM(_thread_t, ತಳ);
GEN_ಸ್ಥಾನಾಂತರ_SYM(_thread_t, callee_saved);
GEN_ಸ್ಥಾನಾಂತರ_SYM(_thread_t, arch);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USE_SWITCH
GEN_ಸ್ಥಾನಾಂತರ_SYM(_thread_t, switch_handle);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USE_SWITCH */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_INFO
GEN_ಸ್ಥಾನಾಂತರ_SYM(_thread_t, ರಾಶಿ_info);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_INFO */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_LOCAL_STORAGE
GEN_ಸ್ಥಾನಾಂತರ_SYM(_thread_t, tls);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, ಗಾತ್ರ(z_ಅಡಚಣೆ_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, ಗಾತ್ರ(z_ಅಡಚಣೆ_stacks));

/* member offsets in the device structure. Used in image post-processing */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_ಸ್ಥಾನಾಂತರ,
		 offsetof(ರಚನೆ device, deps));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DEVICE_DEPS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_ಸ್ಥಾನಾಂತರ,
		 offsetof(ರಚನೆ device, pm));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_ಸ್ಥಾನಾಂತರ,
		 offsetof(ರಚನೆ pm_device_ತಳ, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
