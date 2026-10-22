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

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <offsets_short.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/minmax.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/printk.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/debug/stack.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/random/random.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/device.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/linker/linker-defs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/platform/hooks.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ipi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/drivers/entropy.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log_ctrl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/tracing/tracing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/debug/gcov.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kswap.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/timing/timing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/arch/common/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>

LOG_MODULE_ਰਜਿਸਟਰ(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
ਬਣਤਰ z_kernel _kernel;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PM
atomic_t cpus_ਸਰਗਰਮ;
#ਜੇ_ਅੰਤ

/* init/main and idle threads */
K_THREAD_ਢੇਰ_DEFINE(z_main_ਢੇਰ, CONFIG_MAIN_ਢੇਰ_ਮਾਪ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MULTITHREADING
ਬਣਤਰ k_thread z_main_thread;
ਬਣਤਰ k_thread z_idle_ਧਾਗੇ[CONFIG_MP_MAX_ਅੰਕ_CPUS];

ਸਥਿਰ K_KERNEL_ਢੇਰ_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_ਅੰਕ_CPUS,
					  CONFIG_IDLE_ਢੇਰ_ਮਾਪ);

ਸਥਿਰ ਖਾਲੀ z_init_static_ਧਾਗੇ(ਖਾਲੀ)
{
	STRUCT_SECTION_FOREACH(static_thread_ਅੰਕੜੇ, thread_ਅੰਕੜੇ) {
		z_ਤਿਆਰੀ_new_thread(
			thread_ਅੰਕੜੇ->init_thread,
			thread_ਅੰਕੜੇ->init_ਢੇਰ,
			thread_ਅੰਕੜੇ->init_ਢੇਰ_ਮਾਪ,
			thread_ਅੰਕੜੇ->init_ਇੰਦਰਾਜ,
			thread_ਅੰਕੜੇ->init_p1,
			thread_ਅੰਕੜੇ->init_p2,
			thread_ਅੰਕੜੇ->init_p3,
			thread_ਅੰਕੜੇ->init_prio,
			thread_ਅੰਕੜੇ->init_options,
			thread_ਅੰਕੜੇ->init_name);

		thread_ਅੰਕੜੇ->init_thread->init_ਅੰਕੜੇ = thread_ਅੰਕੜੇ;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		ਲਈ (ਪੂਰਨ i = 0; pos->objects[i] != ਨਿਰਾ; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_ਤਾਲਾ();
	STRUCT_SECTION_FOREACH(static_thread_ਅੰਕੜੇ, thread_ਅੰਕੜੇ) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_ਅੰਕੜੇ);

		thread_ਸਮਾਂ_ਸਾਰਣੀ_new(thread_ਅੰਕੜੇ->init_thread, init_delay);
	}
	k_sched_unlock();
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_init_static_ਧਾਗੇ() ਕਰੋ { } ਜਦੋਂ (false)
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */

ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_start[];
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_EARLY_start[];
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_PRE_KERNEL_1_start[];
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_PRE_KERNEL_2_start[];
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_POST_KERNEL_start[];
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_APPLICATION_start[];
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_end[];

ਗਿਣਤੀ init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	INIT_LEVEL_SMP,
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
ਬਾਹਰੀ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ __init_SMP_start[];
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ਢੇਰ_ARRAY_DEFINE(z_ਰੁਕਾਵਟ_stacks,
				   CONFIG_MP_MAX_ਅੰਕ_CPUS,
				   CONFIG_ISR_ਢੇਰ_ਮਾਪ);

ਬਾਹਰੀ ਖਾਲੀ idle(ਖਾਲੀ *unused1, ਖਾਲੀ *unused2, ਖਾਲੀ *unused3);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_SYSTEM
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
ਸਥਿਰ ਬਣਤਰ k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ k_ਚੱਕਰ_stats),
	.query_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ k_thread_runtime_stats),
	.raw   = z_cpu_stats_raw,
	.query = z_cpu_stats_query,
	.reset = ਨਿਰਾ,
	.disable = ਨਿਰਾ,
	.enable  = ਨਿਰਾ,
};

/* CPU object cores have no static instances to walk; each CPU links its own
 * object core in z_init_cpu(). Register the type only.
 */
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID,
			    &cpu_stats_desc);
#ਨਹੀਂਤਾਂ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_SYSTEM */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_REQUIRES_ਢੇਰ_CANARIES
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਢੇਰ_CANARIES_TLS
ਬਾਹਰੀ Z_THREAD_LOCAL ਅਸਥਿਰ uintptr_t __stack_chk_guard;
#ਨਹੀਂਤਾਂ
ਬਾਹਰੀ ਅਸਥਿਰ uintptr_t __stack_chk_guard;
#ਜੇ_ਅੰਤ /* CONFIG_STACK_CANARIES_TLS */
#ਜੇ_ਅੰਤ /* CONFIG_REQUIRES_STACK_CANARIES */

bool z_sys_post_kernel;

/* defined in device.c */
ਬਾਹਰੀ ਪੂਰਨ do_device_init(ਅਚਲ ਬਣਤਰ device *dev);

/**
 * @brief Initialize state for all static devices.
 *
 * The state object is always zero-initialized, but this may not be
 * sufficient.
 */
ਸਥਿਰ ਖਾਲੀ z_device_state_init(ਖਾਲੀ)
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
ਸਥਿਰ ਖਾਲੀ z_sys_init_ਲੜੀਵਾਰ_level(ਗਿਣਤੀ init_level level)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ *ਪੱਧਰ[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
		__init_SMP_start,
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	ਅਚਲ ਬਣਤਰ init_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;

	ਲਈ (ਇੰਦਰਾਜ = ਪੱਧਰ[level]; ਇੰਦਰਾਜ < ਪੱਧਰ[level+1]; ਇੰਦਰਾਜ++) {
		ਅਚਲ ਬਣਤਰ device *dev = ਇੰਦਰਾਜ->dev;
		ਪੂਰਨ ਨਤੀਜਾ = 0;

		sys_trace_sys_init_enter(ਇੰਦਰਾਜ, level);
		ਜੇ (dev != ਨਿਰਾ) {
			ਜੇ ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				ਨਤੀਜਾ = do_device_init(dev);
			}
		} ਨਹੀਂਤਾਂ {
			ਨਤੀਜਾ = ਇੰਦਰਾਜ->init_fn();
		}
		sys_trace_sys_init_exit(ਇੰਦਰਾਜ, level, ਨਤੀਜਾ);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_STATIC_INIT_GNU

ਬਾਹਰੀ ਖਾਲੀ (*__zephyr_init_array_start[])();
ਬਾਹਰੀ ਖਾਲੀ (*__zephyr_init_array_end[])();

ਸਥਿਰ ਖਾਲੀ z_static_init_gnu(ਖਾਲੀ)
{
	ਖਾਲੀ	(**fn)();

	ਲਈ (fn = __zephyr_init_array_start; fn != __zephyr_init_array_end; fn++) {
		/* MWDT toolchain sticks a NULL at the end of the array */
		ਜੇ (*fn == ਨਿਰਾ) {
			ਰੋਕੋ;
		}
		(**fn)();
	}
}

#ਜੇ_ਅੰਤ

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
ਸਥਿਰ ਖਾਲੀ bg_thread_main(ਖਾਲੀ *unused1, ਖਾਲੀ *unused2, ਖਾਲੀ *unused3)
{
	ARG_ਅਣਵਰਤਿਆ(unused1);
	ARG_ਅਣਵਰਤਿਆ(unused2);
	ARG_ਅਣਵਰਤਿਆ(unused3);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#ਜੇ_ਅੰਤ /* CONFIG_MMU */
	z_sys_post_kernel = true;

#ਜੇ CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#ਜੇ_ਅੰਤ

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_ਇੰਦਰਾਜ, ਇੰਦਰਾਜ) {
		ਇੰਦਰਾਜ->init_fn();
	}

	z_sys_init_ਲੜੀਵਾਰ_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#ਜੇ defined(CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM) && (CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM != 0)
	z_ਢੇਰ_adjust_initialized = 1;
#ਜੇ_ਅੰਤ /* CONFIG_STACK_POINTER_RANDOM */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#ਜੇ_ਅੰਤ /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_ਲੜੀਵਾਰ_level(INIT_LEVEL_APPLICATION);

	z_init_static_ਧਾਗੇ();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_ਲੜੀਵਾਰ_level(INIT_LEVEL_SMP);
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MMU
	z_mem_manage_boot_ਅੰਤ();
#ਜੇ_ਅੰਤ /* CONFIG_MMU */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_BOOTARGS
	ਬਾਹਰੀ ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv);
	ਬਾਹਰੀ ਅੱਖਰ **sys_boot_prepare_main_ਦਲੀਲਾਂ(ਪੂਰਨ *argc);

	ਪੂਰਨ argc = 0;
	ਅੱਖਰ **argv = sys_boot_prepare_main_ਦਲੀਲਾਂ(&argc);
	(ਖਾਲੀ)ਮੁੱਖ(argc, argv);
#ਨਹੀਂਤਾਂ
	ਬਾਹਰੀ ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ);

	(ਖਾਲੀ)ਮੁੱਖ();
#ਜੇ_ਅੰਤ /* CONFIG_BOOTARGS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#ਜਾਂਜੇ defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#ਜੇ_ਅੰਤ /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#ਜੇ defined(CONFIG_MULTITHREADING)
__boot_func
ਸਥਿਰ ਖਾਲੀ init_idle_thread(ਪੂਰਨ i)
{
	ਬਣਤਰ k_thread *thread = &z_idle_ਧਾਗੇ[i];
	k_thread_ਢੇਰ_t *ਢੇਰ = z_idle_stacks[i];
	size_t ਢੇਰ_ਮਾਪ = K_KERNEL_ਢੇਰ_SIZEOF(z_idle_stacks[i]);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME

#ਜੇ CONFIG_MP_MAX_ਅੰਕ_CPUS > 1
	ਅੱਖਰ tname[8];
	snprintk(tname, 8, "idle %02d", i);
#ਨਹੀਂਤਾਂ
	ਅੱਖਰ *tname = "idle";
#ਜੇ_ਅੰਤ /* CONFIG_MP_MAX_NUM_CPUS */

#ਨਹੀਂਤਾਂ
	ਅੱਖਰ *tname = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */

	z_ਤਿਆਰੀ_new_thread(thread, ਢੇਰ,
			  ਢੇਰ_ਮਾਪ, idle, &_kernel.cpus[i],
			  ਨਿਰਾ, ਨਿਰਾ, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	thread->ਅਧਾਰ.is_idle = 1U;
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}

ਖਾਲੀ z_init_cpu(ਪੂਰਨ id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_ਧਾਗੇ[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ਢੇਰ =
		(K_KERNEL_ਢੇਰ_ਬਫਰ(z_ਰੁਕਾਵਟ_stacks[id]) +
		 K_KERNEL_ਢੇਰ_SIZEOF(z_ਰੁਕਾਵਟ_stacks[id]));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_ਸਰਗਰਮ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_ਰਜਿਸਟਰ(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  ਆਕਾਰ(ਬਣਤਰ k_ਚੱਕਰ_stats));
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਅੱਖਰ *prepare_multithreading(ਖਾਲੀ)
{
	ਅੱਖਰ *ਢੇਰ_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
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
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
	ਢੇਰ_ptr = z_ਤਿਆਰੀ_new_thread(&z_main_thread, z_main_ਢੇਰ,
				       K_THREAD_ਢੇਰ_SIZEOF(z_main_ਢੇਰ),
				       bg_thread_main,
				       ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ,
				       CONFIG_MAIN_THREAD_ਤਰਜੀਹ,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	ਵਾਪਸ ਢੇਰ_ptr;
}

__boot_func
ਸਥਿਰ FUNC_NORETURN ਖਾਲੀ switch_ਮੰਜ਼ਿਲ_main_thread(ਅੱਖਰ *ਢੇਰ_ptr)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_HAS_CUSTOM_ਅਦਲਾ_ਬਦਲੀ_ਮੰਜ਼ਿਲ_MAIN
	arch_switch_ਮੰਜ਼ਿਲ_main_thread(&z_main_thread, ਢੇਰ_ptr, bg_thread_main);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(ਢੇਰ_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_ਅਦਲਾ_ਬਦਲੀ_unlocked();
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	ਕੋਡ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ਢੇਰ_PROTECTOR
ਖਾਲੀ __weak z_early_rand_get(uint8_t *buf, size_t ਲੰਮਾਈ)
{
	ਸਥਿਰ uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	ਪੂਰਨ rc;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ENTROPY_HAS_DRIVER
	ਅਚਲ ਬਣਤਰ device *ਅਚਲ entropy = entropy_get_default_device();

	ਜੇ ((entropy != ਨਿਰਾ) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, ਲੰਮਾਈ, ENTROPY_BUSYWAIT);
		ਜੇ (rc > 0) {
			ਲੰਮਾਈ -= rc;
			buf += rc;
		}
	}
#ਜੇ_ਅੰਤ /* CONFIG_ENTROPY_HAS_DRIVER */

	ਜਦੋਂ (ਲੰਮਾਈ > 0) {
		uint32_t val;

		state = state + k_ਚੱਕਰ_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(ਲੰਮਾਈ, ਆਕਾਰ(val));
		arch_early_memcpy((ਖਾਲੀ *)buf, &val, rc);

		ਲੰਮਾਈ -= rc;
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
FUNC_NO_ਢੇਰ_PROTECTOR
FUNC_NORETURN ਖਾਲੀ z_cstart(ਖਾਲੀ)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_ਲੜੀਵਾਰ_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#ਜੇ defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_ਇੰਦਰਾਜ, ਇੰਦਰਾਜ) {
		ਇੰਦਰਾਜ->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_ਲੜੀਵਾਰ_level(INIT_LEVEL_PRE_KERNEL_1);
#ਜੇ defined(CONFIG_SMP)
	arch_smp_init();
#ਜੇ_ਅੰਤ
	z_sys_init_ਲੜੀਵਾਰ_level(INIT_LEVEL_PRE_KERNEL_2);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_REQUIRES_ਢੇਰ_CANARIES
	uintptr_t ਢੇਰ_guard;

	z_early_rand_get((uint8_t *)&ਢੇਰ_guard, ਆਕਾਰ(ਢੇਰ_guard));
	__stack_chk_guard = ਢੇਰ_guard;
	__stack_chk_guard <<= 8;
#ਜੇ_ਅੰਤ	/* CONFIG_REQUIRES_STACK_CANARIES */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_ਸ਼ੁਰੂ();
#ਜੇ_ਅੰਤ /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MULTITHREADING
	switch_ਮੰਜ਼ਿਲ_main_thread(prepare_multithreading());
#ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ARCH_SWITCH_ਮੰਜ਼ਿਲ_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_ਮੰਜ਼ਿਲ_MAIN_NO_MULTITHREADING(bg_thread_main,
		ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
	bg_thread_main(ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_ਤਾਲਾ();
	ਜਦੋਂ (true) {
	}
	/* LCOV_EXCL_STOP */
#ਜੇ_ਅੰਤ /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	ਕੋਡ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
