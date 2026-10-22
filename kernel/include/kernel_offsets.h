/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/device.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/pm/device.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "kernel_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscall_list.h>

/* All of this is build time magic, but LCOV gets confused. Disable coverage
 * for this whole file.
 *
 * LCOV_EXCL_START
 */

/*
 * The final link step uses the symbol _OffsetAbsSyms to force the linkage of
 * offsets.o into the ELF image.
 */

GEN_ABS_SYM_BEGIN(ਵਿੱਥ_Abs_Syms)

GEN_ਵਿੱਥ_SYM(_cpu_t, current);
GEN_ਵਿੱਥ_SYM(_cpu_t, nested);
GEN_ਵਿੱਥ_SYM(_cpu_t, irq_ਢੇਰ);
GEN_ਵਿੱਥ_SYM(_cpu_t, arch);

GEN_ਵਿੱਥ_SYM(_kernel_t, cpus);

#ਜੇ defined(CONFIG_FPU_SHARING)
GEN_ਵਿੱਥ_SYM(_cpu_t, fp_ctx);
#ਜੇ_ਅੰਤ /* CONFIG_FPU_SHARING */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PM
GEN_ਵਿੱਥ_SYM(_kernel_t, idle);
#ਜੇ_ਅੰਤ /* CONFIG_PM */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY
GEN_ਵਿੱਥ_SYM(_kernel_t, ready_q);
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
GEN_ਵਿੱਥ_SYM(_ready_q_t, cache);
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

GEN_ਵਿੱਥ_SYM(thread_ਅਧਾਰ_t, user_options);

GEN_ਵਿੱਥ_SYM(_thread_t, ਅਧਾਰ);
GEN_ਵਿੱਥ_SYM(_thread_t, callee_saved);
GEN_ਵਿੱਥ_SYM(_thread_t, arch);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USE_SWITCH
GEN_ਵਿੱਥ_SYM(_thread_t, switch_handle);
#ਜੇ_ਅੰਤ /* CONFIG_USE_SWITCH */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_INFO
GEN_ਵਿੱਥ_SYM(_thread_t, ਢੇਰ_info);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_INFO */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_LOCAL_STORAGE
GEN_ਵਿੱਥ_SYM(_thread_t, tls);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_LOCAL_STORAGE */

GEN_ABSOLUTE_SYM(__z_interrupt_stack_SIZEOF, ਆਕਾਰ(z_ਰੁਕਾਵਟ_stacks[0]));
GEN_ABSOLUTE_SYM(__z_interrupt_all_stacks_SIZEOF, ਆਕਾਰ(z_ਰੁਕਾਵਟ_stacks));

/* member offsets in the device structure. Used in image post-processing */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEVICE_DEPS
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_HANDLES_ਵਿੱਥ,
		 offsetof(ਬਣਤਰ device, deps));
#ਜੇ_ਅੰਤ /* CONFIG_DEVICE_DEPS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PM_DEVICE
GEN_ABSOLUTE_SYM(DEVICE_STRUCT_PM_ਵਿੱਥ,
		 offsetof(ਬਣਤਰ device, pm));
#ਜੇ_ਅੰਤ /* CONFIG_PM_DEVICE */

/* member offsets in the pm_device structure. Used in image post-processing */

GEN_ABSOLUTE_SYM(PM_DEVICE_STRUCT_FLAGS_ਵਿੱਥ,
		 offsetof(ਬਣਤਰ pm_device_ਅਧਾਰ, flags));

GEN_ABSOLUTE_SYM(_PM_DEVICE_FLAG_PD, PM_DEVICE_FLAG_PD);

/* LCOV_EXCL_STOP */
#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_KERNEL_OFFSETS_H_ */
