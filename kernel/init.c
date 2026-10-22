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

#ضمن <ctype.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <offsets_short.h>
#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/minmax.h>
#ضمن <zephyr/sys/printk.h>
#ضمن <zephyr/debug/stack.h>
#ضمن <zephyr/random/random.h>
#ضمن <zephyr/linker/sections.h>
#ضمن <zephyr/toolchain.h>
#ضمن <zephyr/device.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/linker/linker-defs.h>
#ضمن <zephyr/platform/hooks.h>
#ضمن <ksched.h>
#ضمن <kthread.h>
#ضمن <ipi.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <kernel_internal.h>
#ضمن <zephyr/drivers/entropy.h>
#ضمن <zephyr/logging/log_ctrl.h>
#ضمن <zephyr/tracing/tracing.h>
#ضمن <zephyr/debug/gcov.h>
#ضمن <kswap.h>
#ضمن <zephyr/timing/timing.h>
#ضمن <zephyr/logging/log.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/arch/common/init.h>
#ضمن <scheduler.h>

LOG_MODULE_رجسٽر(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
بنية z_kernel _kernel;

#جيڪڏهن_وصف_ٿيل CONFIG_PM
atomic_t cpus_سرگرم;
#جيڪڏهن_پورو

/* init/main and idle threads */
K_THREAD_ڍير_DEFINE(z_main_ڍير, CONFIG_MAIN_ڍير_سائيز);

#جيڪڏهن_وصف_ٿيل CONFIG_MULTITHREADING
بنية k_thread z_main_thread;
بنية k_thread z_idle_ڌاڳا[CONFIG_MP_MAX_انگ_CPUS];

ساكن K_KERNEL_ڍير_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_انگ_CPUS,
					  CONFIG_IDLE_ڍير_سائيز);

ساكن فراغ z_init_static_ڌاڳا(فراغ)
{
	STRUCT_SECTION_FOREACH(static_thread_معلومات, thread_معلومات) {
		z_تياري_new_thread(
			thread_معلومات->init_thread,
			thread_معلومات->init_ڍير,
			thread_معلومات->init_ڍير_سائيز,
			thread_معلومات->init_داخلا,
			thread_معلومات->init_p1,
			thread_معلومات->init_p2,
			thread_معلومات->init_p3,
			thread_معلومات->init_prio,
			thread_معلومات->init_options,
			thread_معلومات->init_name);

		thread_معلومات->init_thread->init_معلومات = thread_معلومات;
	}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		لكل (صحيح i = 0; pos->objects[i] != عدم; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_تالو();
	STRUCT_SECTION_FOREACH(static_thread_معلومات, thread_معلومات) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_معلومات);

		thread_وقت_جدول_new(thread_معلومات->init_thread, init_delay);
	}
	k_sched_unlock();
}
#وإلا
#وصف_ڪريو z_init_static_ڌاڳا() افعل { } طالما (false)
#جيڪڏهن_پورو /* CONFIG_MULTITHREADING */

خارجي ثابت بنية init_داخلا __init_start[];
خارجي ثابت بنية init_داخلا __init_EARLY_start[];
خارجي ثابت بنية init_داخلا __init_PRE_KERNEL_1_start[];
خارجي ثابت بنية init_داخلا __init_PRE_KERNEL_2_start[];
خارجي ثابت بنية init_داخلا __init_POST_KERNEL_start[];
خارجي ثابت بنية init_داخلا __init_APPLICATION_start[];
خارجي ثابت بنية init_داخلا __init_end[];

تعداد init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	INIT_LEVEL_SMP,
#جيڪڏهن_پورو /* CONFIG_SMP */
};

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
خارجي ثابت بنية init_داخلا __init_SMP_start[];
#جيڪڏهن_پورو /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ڍير_ARRAY_DEFINE(z_مداخلت_stacks,
				   CONFIG_MP_MAX_انگ_CPUS,
				   CONFIG_ISR_ڍير_سائيز);

خارجي فراغ idle(فراغ *unused1, فراغ *unused2, فراغ *unused3);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_SYSTEM
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_STATS_SYSTEM
ساكن بنية k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_سائيز = حجم(بنية k_چڪر_stats),
	.query_سائيز = حجم(بنية k_thread_runtime_stats),
	.raw   = z_cpu_stats_raw,
	.query = z_cpu_stats_query,
	.reset = عدم,
	.disable = عدم,
	.enable  = عدم,
};

/* CPU object cores have no static instances to walk; each CPU links its own
 * object core in z_init_cpu(). Register the type only.
 */
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID,
			    &cpu_stats_desc);
#وإلا
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID, عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_SYSTEM */

#جيڪڏهن_وصف_ٿيل CONFIG_REQUIRES_ڍير_CANARIES
#جيڪڏهن_وصف_ٿيل CONFIG_ڍير_CANARIES_TLS
خارجي Z_THREAD_LOCAL متطاير uintptr_t __stack_chk_guard;
#وإلا
خارجي متطاير uintptr_t __stack_chk_guard;
#جيڪڏهن_پورو /* CONFIG_STACK_CANARIES_TLS */
#جيڪڏهن_پورو /* CONFIG_REQUIRES_STACK_CANARIES */

bool z_sys_post_kernel;

/* defined in device.c */
خارجي صحيح do_device_init(ثابت بنية device *dev);

/**
 * @brief Initialize state for all static devices.
 *
 * The state object is always zero-initialized, but this may not be
 * sufficient.
 */
ساكن فراغ z_device_state_init(فراغ)
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
ساكن فراغ z_sys_init_لاڳيتو_level(تعداد init_level level)
{
	ساكن ثابت بنية init_داخلا *سطحون[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
		__init_SMP_start,
#جيڪڏهن_پورو /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	ثابت بنية init_داخلا *داخلا;

	لكل (داخلا = سطحون[level]; داخلا < سطحون[level+1]; داخلا++) {
		ثابت بنية device *dev = داخلا->dev;
		صحيح نتيجو = 0;

		sys_trace_sys_init_enter(داخلا, level);
		إذا (dev != عدم) {
			إذا ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				نتيجو = do_device_init(dev);
			}
		} وإلا {
			نتيجو = داخلا->init_fn();
		}
		sys_trace_sys_init_exit(داخلا, level, نتيجو);
	}
}

#جيڪڏهن_وصف_ٿيل CONFIG_STATIC_INIT_GNU

خارجي فراغ (*__zephyr_init_array_start[])();
خارجي فراغ (*__zephyr_init_array_end[])();

ساكن فراغ z_static_init_gnu(فراغ)
{
	فراغ	(**fn)();

	لكل (fn = __zephyr_init_array_start; fn != __zephyr_init_array_end; fn++) {
		/* MWDT toolchain sticks a NULL at the end of the array */
		إذا (*fn == عدم) {
			اكسر;
		}
		(**fn)();
	}
}

#جيڪڏهن_پورو

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
ساكن فراغ bg_thread_main(فراغ *unused1, فراغ *unused2, فراغ *unused3)
{
	ARG_اڻ_استعمال_ٿيل(unused1);
	ARG_اڻ_استعمال_ٿيل(unused2);
	ARG_اڻ_استعمال_ٿيل(unused3);

#جيڪڏهن_وصف_ٿيل CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#جيڪڏهن_پورو /* CONFIG_MMU */
	z_sys_post_kernel = true;

#إذا CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#جيڪڏهن_پورو

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_داخلا, داخلا) {
		داخلا->init_fn();
	}

	z_sys_init_لاڳيتو_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#إذا defined(CONFIG_ڍير_اشارو_RANDOM) && (CONFIG_ڍير_اشارو_RANDOM != 0)
	z_ڍير_adjust_initialized = 1;
#جيڪڏهن_پورو /* CONFIG_STACK_POINTER_RANDOM */

#جيڪڏهن_وصف_ٿيل CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#جيڪڏهن_پورو /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_لاڳيتو_level(INIT_LEVEL_APPLICATION);

	z_init_static_ڌاڳا();

#جيڪڏهن_وصف_ٿيل CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#جيڪڏهن_پورو /* CONFIG_KERNEL_COHERENCE */

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_لاڳيتو_level(INIT_LEVEL_SMP);
#جيڪڏهن_پورو /* CONFIG_SMP */

#جيڪڏهن_وصف_ٿيل CONFIG_MMU
	z_mem_manage_boot_پڄاڻي();
#جيڪڏهن_پورو /* CONFIG_MMU */

#جيڪڏهن_وصف_ٿيل CONFIG_BOOTARGS
	خارجي صحيح مکيه(صحيح argc, حرف **argv);
	خارجي حرف **sys_boot_prepare_main_دليلون(صحيح *argc);

	صحيح argc = 0;
	حرف **argv = sys_boot_prepare_main_دليلون(&argc);
	(فراغ)مکيه(argc, argv);
#وإلا
	خارجي صحيح مکيه(فراغ);

	(فراغ)مکيه();
#جيڪڏهن_پورو /* CONFIG_BOOTARGS */

#جيڪڏهن_وصف_ٿيل CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#وإلاإذا defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#جيڪڏهن_پورو /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#إذا defined(CONFIG_MULTITHREADING)
__boot_func
ساكن فراغ init_idle_thread(صحيح i)
{
	بنية k_thread *thread = &z_idle_ڌاڳا[i];
	k_thread_ڍير_t *ڍير = z_idle_stacks[i];
	size_t ڍير_سائيز = K_KERNEL_ڍير_SIZEOF(z_idle_stacks[i]);

#جيڪڏهن_وصف_ٿيل CONFIG_THREAD_NAME

#إذا CONFIG_MP_MAX_انگ_CPUS > 1
	حرف tname[8];
	snprintk(tname, 8, "idle %02d", i);
#وإلا
	حرف *tname = "idle";
#جيڪڏهن_پورو /* CONFIG_MP_MAX_NUM_CPUS */

#وإلا
	حرف *tname = عدم;
#جيڪڏهن_پورو /* CONFIG_THREAD_NAME */

	z_تياري_new_thread(thread, ڍير,
			  ڍير_سائيز, idle, &_kernel.cpus[i],
			  عدم, عدم, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	thread->بنياد.is_idle = 1U;
#جيڪڏهن_پورو /* CONFIG_SMP */
}

فراغ z_init_cpu(صحيح id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_ڌاڳا[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ڍير =
		(K_KERNEL_ڍير_بفر(z_مداخلت_stacks[id]) +
		 K_KERNEL_ڍير_SIZEOF(z_مداخلت_stacks[id]));
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_سرگرم);
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_رجسٽر(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  حجم(بنية k_چڪر_stats));
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#جيڪڏهن_پورو
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
ساكن حرف *prepare_multithreading(فراغ)
{
	حرف *ڍير_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#جيڪڏهن_وصف_نه_ٿيل CONFIG_SMP
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
#جيڪڏهن_پورو /* CONFIG_SMP */
	ڍير_ptr = z_تياري_new_thread(&z_main_thread, z_main_ڍير,
				       K_THREAD_ڍير_SIZEOF(z_main_ڍير),
				       bg_thread_main,
				       عدم, عدم, عدم,
				       CONFIG_MAIN_THREAD_ترجيح,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	أرجع ڍير_ptr;
}

__boot_func
ساكن FUNC_NORETURN فراغ switch_منزل_main_thread(حرف *ڍير_ptr)
{
#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_HAS_CUSTOM_مٽا_سٽا_منزل_MAIN
	arch_switch_منزل_main_thread(&z_main_thread, ڍير_ptr, bg_thread_main);
#وإلا
	ARG_اڻ_استعمال_ٿيل(ڍير_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_مٽا_سٽا_unlocked();
#جيڪڏهن_پورو /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	ڪوڊ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#جيڪڏهن_پورو /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ڍير_PROTECTOR
فراغ __weak z_early_rand_get(uint8_t *buf, size_t ڊيگهائي)
{
	ساكن uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	صحيح rc;

#جيڪڏهن_وصف_ٿيل CONFIG_ENTROPY_HAS_DRIVER
	ثابت بنية device *ثابت entropy = entropy_get_default_device();

	إذا ((entropy != عدم) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, ڊيگهائي, ENTROPY_BUSYWAIT);
		إذا (rc > 0) {
			ڊيگهائي -= rc;
			buf += rc;
		}
	}
#جيڪڏهن_پورو /* CONFIG_ENTROPY_HAS_DRIVER */

	طالما (ڊيگهائي > 0) {
		uint32_t val;

		state = state + k_چڪر_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(ڊيگهائي, حجم(val));
		arch_early_memcpy((فراغ *)buf, &val, rc);

		ڊيگهائي -= rc;
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
FUNC_NO_ڍير_PROTECTOR
FUNC_NORETURN فراغ z_cstart(فراغ)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_لاڳيتو_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#إذا defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#جيڪڏهن_پورو /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_داخلا, داخلا) {
		داخلا->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_لاڳيتو_level(INIT_LEVEL_PRE_KERNEL_1);
#إذا defined(CONFIG_SMP)
	arch_smp_init();
#جيڪڏهن_پورو
	z_sys_init_لاڳيتو_level(INIT_LEVEL_PRE_KERNEL_2);

#جيڪڏهن_وصف_ٿيل CONFIG_REQUIRES_ڍير_CANARIES
	uintptr_t ڍير_guard;

	z_early_rand_get((uint8_t *)&ڍير_guard, حجم(ڍير_guard));
	__stack_chk_guard = ڍير_guard;
	__stack_chk_guard <<= 8;
#جيڪڏهن_پورو	/* CONFIG_REQUIRES_STACK_CANARIES */

#جيڪڏهن_وصف_ٿيل CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_شروعات();
#جيڪڏهن_پورو /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#جيڪڏهن_وصف_ٿيل CONFIG_MULTITHREADING
	switch_منزل_main_thread(prepare_multithreading());
#وإلا
#جيڪڏهن_وصف_ٿيل ARCH_SWITCH_منزل_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_منزل_MAIN_NO_MULTITHREADING(bg_thread_main,
		عدم, عدم, عدم);
#وإلا
	bg_thread_main(عدم, عدم, عدم);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_تالو();
	طالما (true) {
	}
	/* LCOV_EXCL_STOP */
#جيڪڏهن_پورو /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#جيڪڏهن_پورو /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	ڪوڊ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
