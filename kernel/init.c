/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel initialization module
 *
 * This module contains routines that are used to initialize the kernel.
 */

#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <offsets_short.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/printk.h>
#ಅನ್ತರ್ಭಾವ <zephyr/debug/stack.h>
#ಅನ್ತರ್ಭಾವ <zephyr/random/random.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <zephyr/device.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/linker/linker-defs.h>
#ಅನ್ತರ್ಭಾವ <zephyr/platform/hooks.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/drivers/entropy.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log_ctrl.h>
#ಅನ್ತರ್ಭಾವ <zephyr/tracing/tracing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/debug/gcov.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <zephyr/timing/timing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/arch/common/init.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>

LOG_MODULE_ರಿಜಿಸ್ಟರ್(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
ರಚನೆ z_kernel _kernel;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PM
atomic_t cpus_ಸಕ್ರಿಯ;
#ಆದರೆ_ಅಂತ್ಯ

/* init/main and idle threads */
K_THREAD_ರಾಶಿ_DEFINE(z_main_ರಾಶಿ, CONFIG_MAIN_ರಾಶಿ_ಗಾತ್ರ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MULTITHREADING
ರಚನೆ k_thread z_main_thread;
ರಚನೆ k_thread z_idle_ಎಳೆಗಳು[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];

ಸ್ಥಿರ K_KERNEL_ರಾಶಿ_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_ಅಂಕೆ_CPUS,
					  CONFIG_IDLE_ರಾಶಿ_ಗಾತ್ರ);

ಸ್ಥಿರ ಶೂನ್ಯ z_init_static_ಎಳೆಗಳು(ಶೂನ್ಯ)
{
	STRUCT_SECTION_FOREACH(static_thread_ದತ್ತಾಂಶ, thread_ದತ್ತಾಂಶ) {
		z_ಸಿದ್ಧತೆ_new_thread(
			thread_ದತ್ತಾಂಶ->init_thread,
			thread_ದತ್ತಾಂಶ->init_ರಾಶಿ,
			thread_ದತ್ತಾಂಶ->init_ರಾಶಿ_ಗಾತ್ರ,
			thread_ದತ್ತಾಂಶ->init_ನಮೂದು,
			thread_ದತ್ತಾಂಶ->init_p1,
			thread_ದತ್ತಾಂಶ->init_p2,
			thread_ದತ್ತಾಂಶ->init_p3,
			thread_ದತ್ತಾಂಶ->init_prio,
			thread_ದತ್ತಾಂಶ->init_options,
			thread_ದತ್ತಾಂಶ->init_name);

		thread_ದತ್ತಾಂಶ->init_thread->init_ದತ್ತಾಂಶ = thread_ದತ್ತಾಂಶ;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; pos->objects[i] != ಖಾಲಿ; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_ಬೀಗ();
	STRUCT_SECTION_FOREACH(static_thread_ದತ್ತಾಂಶ, thread_ದತ್ತಾಂಶ) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_ದತ್ತಾಂಶ);

		thread_ವೇಳಾಪಟ್ಟಿ_new(thread_ದತ್ತಾಂಶ->init_thread, init_delay);
	}
	k_sched_unlock();
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು z_init_static_ಎಳೆಗಳು() ಮಾಡು { } ಆಗ (false)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MULTITHREADING */

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_start[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_EARLY_start[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_PRE_KERNEL_1_start[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_PRE_KERNEL_2_start[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_POST_KERNEL_start[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_APPLICATION_start[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_end[];

ಎಣಿಕೆ init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	INIT_LEVEL_SMP,
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು __init_SMP_start[];
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ರಾಶಿ_ARRAY_DEFINE(z_ಅಡಚಣೆ_stacks,
				   CONFIG_MP_MAX_ಅಂಕೆ_CPUS,
				   CONFIG_ISR_ರಾಶಿ_ಗಾತ್ರ);

ಬಾಹ್ಯ ಶೂನ್ಯ idle(ಶೂನ್ಯ *unused1, ಶೂನ್ಯ *unused2, ಶೂನ್ಯ *unused3);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_SYSTEM
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_SYSTEM
ಸ್ಥಿರ ರಚನೆ k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ k_ಚಕ್ರ_stats),
	.query_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ k_thread_runtime_stats),
	.raw   = z_cpu_stats_raw,
	.query = z_cpu_stats_query,
	.reset = ಖಾಲಿ,
	.disable = ಖಾಲಿ,
	.enable  = ಖಾಲಿ,
};

/* CPU object cores have no static instances to walk; each CPU links its own
 * object core in z_init_cpu(). Register the type only.
 */
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID,
			    &cpu_stats_desc);
#ಇಲ್ಲದಿದ್ದರೆ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_SYSTEM */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_REQUIRES_ರಾಶಿ_CANARIES
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ರಾಶಿ_CANARIES_TLS
ಬಾಹ್ಯ Z_THREAD_LOCAL ಅಸ್ಥಿರ uintptr_t __stack_chk_guard;
#ಇಲ್ಲದಿದ್ದರೆ
ಬಾಹ್ಯ ಅಸ್ಥಿರ uintptr_t __stack_chk_guard;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_CANARIES_TLS */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_REQUIRES_STACK_CANARIES */

bool z_sys_post_kernel;

/* defined in device.c */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ do_device_init(ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev);

/**
 * @brief Initialize state for all static devices.
 *
 * The state object is always zero-initialized, but this may not be
 * sufficient.
 */
ಸ್ಥಿರ ಶೂನ್ಯ z_device_state_init(ಶೂನ್ಯ)
{
	STRUCT_SECTION_FOREACH(device, dev) {
		k_object_init(dev);
	}
}

/**
 * @brief Execute all the init entry initialization functions at a given level
 *
 * @details Invokes the initialization routine for each init entry object
 * created by the INIT_ENTRY_DEFINE() macro using the specified level.
 * The linker script places the init entry objects in memory in the order
 * they need to be invoked, with symbols indicating where one level leaves
 * off and the next one begins.
 *
 * @param level init level to run.
 */
ಸ್ಥಿರ ಶೂನ್ಯ z_sys_init_ಸತತ_level(ಎಣಿಕೆ init_level level)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು *ಮಟ್ಟಗಳು[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
		__init_SMP_start,
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	ಸ್ಥಿರಾಂಕ ರಚನೆ init_ನಮೂದು *ನಮೂದು;

	ಗಾಗಿ (ನಮೂದು = ಮಟ್ಟಗಳು[level]; ನಮೂದು < ಮಟ್ಟಗಳು[level+1]; ನಮೂದು++) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ device *dev = ನಮೂದು->dev;
		ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ = 0;

		sys_trace_sys_init_enter(ನಮೂದು, level);
		ಆದರೆ (dev != ಖಾಲಿ) {
			ಆದರೆ ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				ಫಲಿತಾಂಶ = do_device_init(dev);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಫಲಿತಾಂಶ = ನಮೂದು->init_fn();
		}
		sys_trace_sys_init_exit(ನಮೂದು, level, ಫಲಿತಾಂಶ);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_STATIC_INIT_GNU

ಬಾಹ್ಯ ಶೂನ್ಯ (*__zephyr_init_array_start[])();
ಬಾಹ್ಯ ಶೂನ್ಯ (*__zephyr_init_array_end[])();

ಸ್ಥಿರ ಶೂನ್ಯ z_static_init_gnu(ಶೂನ್ಯ)
{
	ಶೂನ್ಯ	(**fn)();

	ಗಾಗಿ (fn = __zephyr_init_array_start; fn != __zephyr_init_array_end; fn++) {
		/* MWDT toolchain sticks a NULL at the end of the array */
		ಆದರೆ (*fn == ಖಾಲಿ) {
			ನಿಲ್ಲಿಸು;
		}
		(**fn)();
	}
}

#ಆದರೆ_ಅಂತ್ಯ

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
ಸ್ಥಿರ ಶೂನ್ಯ bg_thread_main(ಶೂನ್ಯ *unused1, ಶೂನ್ಯ *unused2, ಶೂನ್ಯ *unused3)
{
	ARG_ಬಳಸದ(unused1);
	ARG_ಬಳಸದ(unused2);
	ARG_ಬಳಸದ(unused3);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MMU */
	z_sys_post_kernel = true;

#ಆದರೆ CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#ಆದರೆ_ಅಂತ್ಯ

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_ನಮೂದು, ನಮೂದು) {
		ನಮೂದು->init_fn();
	}

	z_sys_init_ಸತತ_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#ಆದರೆ defined(CONFIG_ರಾಶಿ_ಸೂಚಕ_RANDOM) && (CONFIG_ರಾಶಿ_ಸೂಚಕ_RANDOM != 0)
	z_ರಾಶಿ_adjust_initialized = 1;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_POINTER_RANDOM */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_ಸತತ_level(INIT_LEVEL_APPLICATION);

	z_init_static_ಎಳೆಗಳು();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_ಸತತ_level(INIT_LEVEL_SMP);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MMU
	z_mem_manage_boot_ಮುಕ್ತಾಯ();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MMU */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_BOOTARGS
	ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv);
	ಬಾಹ್ಯ ಅಕ್ಷರ **sys_boot_prepare_main_ವಾದಗಳು(ಪೂರ್ಣಾಂಕ *argc);

	ಪೂರ್ಣಾಂಕ argc = 0;
	ಅಕ್ಷರ **argv = sys_boot_prepare_main_ವಾದಗಳು(&argc);
	(ಶೂನ್ಯ)ಮುಖ್ಯ(argc, argv);
#ಇಲ್ಲದಿದ್ದರೆ
	ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ);

	(ಶೂನ್ಯ)ಮುಖ್ಯ();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_BOOTARGS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#ಅಥವಾಆದರೆ defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#ಆದರೆ defined(CONFIG_MULTITHREADING)
__boot_func
ಸ್ಥಿರ ಶೂನ್ಯ init_idle_thread(ಪೂರ್ಣಾಂಕ i)
{
	ರಚನೆ k_thread *thread = &z_idle_ಎಳೆಗಳು[i];
	k_thread_ರಾಶಿ_t *ರಾಶಿ = z_idle_stacks[i];
	size_t ರಾಶಿ_ಗಾತ್ರ = K_KERNEL_ರಾಶಿ_SIZEOF(z_idle_stacks[i]);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME

#ಆದರೆ CONFIG_MP_MAX_ಅಂಕೆ_CPUS > 1
	ಅಕ್ಷರ tname[8];
	snprintk(tname, 8, "idle %02d", i);
#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ *tname = "idle";
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MP_MAX_NUM_CPUS */

#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ *tname = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */

	z_ಸಿದ್ಧತೆ_new_thread(thread, ರಾಶಿ,
			  ರಾಶಿ_ಗಾತ್ರ, idle, &_kernel.cpus[i],
			  ಖಾಲಿ, ಖಾಲಿ, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	thread->ತಳ.is_idle = 1U;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}

ಶೂನ್ಯ z_init_cpu(ಪೂರ್ಣಾಂಕ id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_ಎಳೆಗಳು[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ರಾಶಿ =
		(K_KERNEL_ರಾಶಿ_ಬಫರ್(z_ಅಡಚಣೆ_stacks[id]) +
		 K_KERNEL_ರಾಶಿ_SIZEOF(z_ಅಡಚಣೆ_stacks[id]));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_ಸಕ್ರಿಯ);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_ರಿಜಿಸ್ಟರ್(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  ಗಾತ್ರ(ರಚನೆ k_ಚಕ್ರ_stats));
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#ಆದರೆ_ಅಂತ್ಯ
}

/**
 *
 * @brief Initializes kernel data structures
 *
 * This routine initializes various kernel data structures, including
 * the init and idle threads and any architecture-specific initialization.
 *
 * Note that all fields of "_kernel" are set to zero on entry, which may
 * be all the initialization many of them require.
 *
 * @return initial stack pointer for the main thread
 */
__boot_func
ಸ್ಥಿರ ಅಕ್ಷರ *prepare_multithreading(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *ರಾಶಿ_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_SMP
	/*
	 * prime the cache with the main thread since:
	 *
	 * - the cache can never be NULL
	 * - the main thread will be the one to run first
	 * - no other thread is initialized yet and thus their priority fields
	 *   contain garbage, which would prevent the cache loading algorithm
	 *   to work as intended
	 */
	_kernel.ready_q.cache = &z_main_thread;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
	ರಾಶಿ_ptr = z_ಸಿದ್ಧತೆ_new_thread(&z_main_thread, z_main_ರಾಶಿ,
				       K_THREAD_ರಾಶಿ_SIZEOF(z_main_ರಾಶಿ),
				       bg_thread_main,
				       ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ,
				       CONFIG_MAIN_THREAD_ಆದ್ಯತೆ,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	ಹಿಂತಿರುಗಿ ರಾಶಿ_ptr;
}

__boot_func
ಸ್ಥಿರ FUNC_NORETURN ಶೂನ್ಯ switch_ಗಮ್ಯ_main_thread(ಅಕ್ಷರ *ರಾಶಿ_ptr)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_HAS_CUSTOM_ಅದಲುಬದಲು_ಗಮ್ಯ_MAIN
	arch_switch_ಗಮ್ಯ_main_thread(&z_main_thread, ರಾಶಿ_ptr, bg_thread_main);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(ರಾಶಿ_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_ಅದಲುಬದಲು_unlocked();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	ಸಂಕೇತ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ರಾಶಿ_PROTECTOR
ಶೂನ್ಯ __weak z_early_rand_get(uint8_t *buf, size_t ಉದ್ದಳತೆ)
{
	ಸ್ಥಿರ uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	ಪೂರ್ಣಾಂಕ rc;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ENTROPY_HAS_DRIVER
	ಸ್ಥಿರಾಂಕ ರಚನೆ device *ಸ್ಥಿರಾಂಕ entropy = entropy_get_default_device();

	ಆದರೆ ((entropy != ಖಾಲಿ) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, ಉದ್ದಳತೆ, ENTROPY_BUSYWAIT);
		ಆದರೆ (rc > 0) {
			ಉದ್ದಳತೆ -= rc;
			buf += rc;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ENTROPY_HAS_DRIVER */

	ಆಗ (ಉದ್ದಳತೆ > 0) {
		uint32_t val;

		state = state + k_ಚಕ್ರ_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(ಉದ್ದಳತೆ, ಗಾತ್ರ(val));
		arch_early_memcpy((ಶೂನ್ಯ *)buf, &val, rc);

		ಉದ್ದಳತೆ -= rc;
		buf += rc;
	}
}

/**
 *
 * @brief Initialize kernel
 *
 * This routine is invoked when the system is ready to run C code. The
 * processor must be running in 32-bit mode, and the BSS must have been
 * cleared/zeroed.
 *
 * @return Does not return
 */
__boot_func
FUNC_NO_ರಾಶಿ_PROTECTOR
FUNC_NORETURN ಶೂನ್ಯ z_cstart(ಶೂನ್ಯ)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_ಸತತ_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#ಆದರೆ defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_ನಮೂದು, ನಮೂದು) {
		ನಮೂದು->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_ಸತತ_level(INIT_LEVEL_PRE_KERNEL_1);
#ಆದರೆ defined(CONFIG_SMP)
	arch_smp_init();
#ಆದರೆ_ಅಂತ್ಯ
	z_sys_init_ಸತತ_level(INIT_LEVEL_PRE_KERNEL_2);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_REQUIRES_ರಾಶಿ_CANARIES
	uintptr_t ರಾಶಿ_guard;

	z_early_rand_get((uint8_t *)&ರಾಶಿ_guard, ಗಾತ್ರ(ರಾಶಿ_guard));
	__stack_chk_guard = ರಾಶಿ_guard;
	__stack_chk_guard <<= 8;
#ಆದರೆ_ಅಂತ್ಯ	/* CONFIG_REQUIRES_STACK_CANARIES */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_ಆರಂಭ();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MULTITHREADING
	switch_ಗಮ್ಯ_main_thread(prepare_multithreading());
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ARCH_SWITCH_ಗಮ್ಯ_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_ಗಮ್ಯ_MAIN_NO_MULTITHREADING(bg_thread_main,
		ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
	bg_thread_main(ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_ಬೀಗ();
	ಆಗ (true) {
	}
	/* LCOV_EXCL_STOP */
#ಆದರೆ_ಅಂತ್ಯ /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	ಸಂಕೇತ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
