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

LOG_MODULE_ثبتوونکی(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
بنية z_kernel _kernel;

#که_تعریف_شوی CONFIG_PM
atomic_t cpus_فعال;
#د_که_پای

/* init/main and idle threads */
K_THREAD_ډېری_DEFINE(z_main_ډېری, CONFIG_MAIN_ډېری_اندازه);

#که_تعریف_شوی CONFIG_MULTITHREADING
بنية k_thread z_main_thread;
بنية k_thread z_idle_تارونه[CONFIG_MP_MAX_عدد_CPUS];

ساكن K_KERNEL_ډېری_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_عدد_CPUS,
					  CONFIG_IDLE_ډېری_اندازه);

ساكن فراغ z_init_static_تارونه(فراغ)
{
	STRUCT_SECTION_FOREACH(static_thread_معلومات, thread_معلومات) {
		z_تیاری_new_thread(
			thread_معلومات->init_thread,
			thread_معلومات->init_ډېری,
			thread_معلومات->init_ډېری_اندازه,
			thread_معلومات->init_ثبت,
			thread_معلومات->init_p1,
			thread_معلومات->init_p2,
			thread_معلومات->init_p3,
			thread_معلومات->init_prio,
			thread_معلومات->init_options,
			thread_معلومات->init_name);

		thread_معلومات->init_thread->init_معلومات = thread_معلومات;
	}

#که_تعریف_شوی CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		لكل (صحيح i = 0; pos->objects[i] != عدم; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#د_که_پای /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_قلف();
	STRUCT_SECTION_FOREACH(static_thread_معلومات, thread_معلومات) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_معلومات);

		thread_مهالوېش_new(thread_معلومات->init_thread, init_delay);
	}
	k_sched_unlock();
}
#وإلا
#تعریف_کړه z_init_static_تارونه() افعل { } طالما (false)
#د_که_پای /* CONFIG_MULTITHREADING */

خارجي ثابت بنية init_ثبت __init_start[];
خارجي ثابت بنية init_ثبت __init_EARLY_start[];
خارجي ثابت بنية init_ثبت __init_PRE_KERNEL_1_start[];
خارجي ثابت بنية init_ثبت __init_PRE_KERNEL_2_start[];
خارجي ثابت بنية init_ثبت __init_POST_KERNEL_start[];
خارجي ثابت بنية init_ثبت __init_APPLICATION_start[];
خارجي ثابت بنية init_ثبت __init_end[];

تعداد init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#که_تعریف_شوی CONFIG_SMP
	INIT_LEVEL_SMP,
#د_که_پای /* CONFIG_SMP */
};

#که_تعریف_شوی CONFIG_SMP
خارجي ثابت بنية init_ثبت __init_SMP_start[];
#د_که_پای /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ډېری_ARRAY_DEFINE(z_مداخله_stacks,
				   CONFIG_MP_MAX_عدد_CPUS,
				   CONFIG_ISR_ډېری_اندازه);

خارجي فراغ idle(فراغ *unused1, فراغ *unused2, فراغ *unused3);

#که_تعریف_شوی CONFIG_OBJ_CORE_SYSTEM
#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_SYSTEM
ساكن بنية k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_اندازه = حجم(بنية k_دوره_stats),
	.query_اندازه = حجم(بنية k_thread_runtime_stats),
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
#د_که_پای /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#د_که_پای /* CONFIG_OBJ_CORE_SYSTEM */

#که_تعریف_شوی CONFIG_REQUIRES_ډېری_CANARIES
#که_تعریف_شوی CONFIG_ډېری_CANARIES_TLS
خارجي Z_THREAD_LOCAL متطاير uintptr_t __stack_chk_guard;
#وإلا
خارجي متطاير uintptr_t __stack_chk_guard;
#د_که_پای /* CONFIG_STACK_CANARIES_TLS */
#د_که_پای /* CONFIG_REQUIRES_STACK_CANARIES */

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
ساكن فراغ z_sys_init_پرلپسې_level(تعداد init_level level)
{
	ساكن ثابت بنية init_ثبت *پوړونه[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#که_تعریف_شوی CONFIG_SMP
		__init_SMP_start,
#د_که_پای /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	ثابت بنية init_ثبت *ثبت;

	لكل (ثبت = پوړونه[level]; ثبت < پوړونه[level+1]; ثبت++) {
		ثابت بنية device *dev = ثبت->dev;
		صحيح پایله = 0;

		sys_trace_sys_init_enter(ثبت, level);
		إذا (dev != عدم) {
			إذا ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				پایله = do_device_init(dev);
			}
		} وإلا {
			پایله = ثبت->init_fn();
		}
		sys_trace_sys_init_exit(ثبت, level, پایله);
	}
}

#که_تعریف_شوی CONFIG_STATIC_INIT_GNU

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

#د_که_پای

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
ساكن فراغ bg_thread_main(فراغ *unused1, فراغ *unused2, فراغ *unused3)
{
	ARG_ناکارول_شوی(unused1);
	ARG_ناکارول_شوی(unused2);
	ARG_ناکارول_شوی(unused3);

#که_تعریف_شوی CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#د_که_پای /* CONFIG_MMU */
	z_sys_post_kernel = true;

#إذا CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#د_که_پای

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_ثبت, ثبت) {
		ثبت->init_fn();
	}

	z_sys_init_پرلپسې_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#إذا defined(CONFIG_ډېری_اشاره_ګر_RANDOM) && (CONFIG_ډېری_اشاره_ګر_RANDOM != 0)
	z_ډېری_adjust_initialized = 1;
#د_که_پای /* CONFIG_STACK_POINTER_RANDOM */

#که_تعریف_شوی CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#د_که_پای /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_پرلپسې_level(INIT_LEVEL_APPLICATION);

	z_init_static_تارونه();

#که_تعریف_شوی CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#د_که_پای /* CONFIG_KERNEL_COHERENCE */

#که_تعریف_شوی CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_پرلپسې_level(INIT_LEVEL_SMP);
#د_که_پای /* CONFIG_SMP */

#که_تعریف_شوی CONFIG_MMU
	z_mem_manage_boot_پای();
#د_که_پای /* CONFIG_MMU */

#که_تعریف_شوی CONFIG_BOOTARGS
	خارجي صحيح اصلي(صحيح argc, حرف **argv);
	خارجي حرف **sys_boot_prepare_main_دلیلونه(صحيح *argc);

	صحيح argc = 0;
	حرف **argv = sys_boot_prepare_main_دلیلونه(&argc);
	(فراغ)اصلي(argc, argv);
#وإلا
	خارجي صحيح اصلي(فراغ);

	(فراغ)اصلي();
#د_که_پای /* CONFIG_BOOTARGS */

#که_تعریف_شوی CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#د_که_پای

#که_تعریف_شوی CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#وإلاإذا defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#د_که_پای /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#إذا defined(CONFIG_MULTITHREADING)
__boot_func
ساكن فراغ init_idle_thread(صحيح i)
{
	بنية k_thread *thread = &z_idle_تارونه[i];
	k_thread_ډېری_t *ډېری = z_idle_stacks[i];
	size_t ډېری_اندازه = K_KERNEL_ډېری_SIZEOF(z_idle_stacks[i]);

#که_تعریف_شوی CONFIG_THREAD_NAME

#إذا CONFIG_MP_MAX_عدد_CPUS > 1
	حرف tname[8];
	snprintk(tname, 8, "idle %02d", i);
#وإلا
	حرف *tname = "idle";
#د_که_پای /* CONFIG_MP_MAX_NUM_CPUS */

#وإلا
	حرف *tname = عدم;
#د_که_پای /* CONFIG_THREAD_NAME */

	z_تیاری_new_thread(thread, ډېری,
			  ډېری_اندازه, idle, &_kernel.cpus[i],
			  عدم, عدم, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#که_تعریف_شوی CONFIG_SMP
	thread->بنسټ.is_idle = 1U;
#د_که_پای /* CONFIG_SMP */
}

فراغ z_init_cpu(صحيح id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_تارونه[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ډېری =
		(K_KERNEL_ډېری_بفر(z_مداخله_stacks[id]) +
		 K_KERNEL_ډېری_SIZEOF(z_مداخله_stacks[id]));
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#د_که_پای

#که_تعریف_شوی CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_فعال);
#د_که_پای

#که_تعریف_شوی CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_ثبتوونکی(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  حجم(بنية k_دوره_stats));
#د_که_پای
#د_که_پای

#که_تعریف_شوی CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#د_که_پای
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
	حرف *ډېری_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#که_نه_تعریف_شوی CONFIG_SMP
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
#د_که_پای /* CONFIG_SMP */
	ډېری_ptr = z_تیاری_new_thread(&z_main_thread, z_main_ډېری,
				       K_THREAD_ډېری_SIZEOF(z_main_ډېری),
				       bg_thread_main,
				       عدم, عدم, عدم,
				       CONFIG_MAIN_THREAD_لومړیتوب,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	أرجع ډېری_ptr;
}

__boot_func
ساكن FUNC_NORETURN فراغ switch_منزل_main_thread(حرف *ډېری_ptr)
{
#که_تعریف_شوی CONFIG_ARCH_HAS_CUSTOM_بدلون_منزل_MAIN
	arch_switch_منزل_main_thread(&z_main_thread, ډېری_ptr, bg_thread_main);
#وإلا
	ARG_ناکارول_شوی(ډېری_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_بدلون_unlocked();
#د_که_پای /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	کوډ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#د_که_پای /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ډېری_PROTECTOR
فراغ __weak z_early_rand_get(uint8_t *buf, size_t اوږدتیا)
{
	ساكن uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	صحيح rc;

#که_تعریف_شوی CONFIG_ENTROPY_HAS_DRIVER
	ثابت بنية device *ثابت entropy = entropy_get_default_device();

	إذا ((entropy != عدم) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, اوږدتیا, ENTROPY_BUSYWAIT);
		إذا (rc > 0) {
			اوږدتیا -= rc;
			buf += rc;
		}
	}
#د_که_پای /* CONFIG_ENTROPY_HAS_DRIVER */

	طالما (اوږدتیا > 0) {
		uint32_t val;

		state = state + k_دوره_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(اوږدتیا, حجم(val));
		arch_early_memcpy((فراغ *)buf, &val, rc);

		اوږدتیا -= rc;
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
FUNC_NO_ډېری_PROTECTOR
FUNC_NORETURN فراغ z_cstart(فراغ)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_پرلپسې_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#إذا defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#د_که_پای /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_ثبت, ثبت) {
		ثبت->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_پرلپسې_level(INIT_LEVEL_PRE_KERNEL_1);
#إذا defined(CONFIG_SMP)
	arch_smp_init();
#د_که_پای
	z_sys_init_پرلپسې_level(INIT_LEVEL_PRE_KERNEL_2);

#که_تعریف_شوی CONFIG_REQUIRES_ډېری_CANARIES
	uintptr_t ډېری_guard;

	z_early_rand_get((uint8_t *)&ډېری_guard, حجم(ډېری_guard));
	__stack_chk_guard = ډېری_guard;
	__stack_chk_guard <<= 8;
#د_که_پای	/* CONFIG_REQUIRES_STACK_CANARIES */

#که_تعریف_شوی CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_پیل();
#د_که_پای /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#که_تعریف_شوی CONFIG_MULTITHREADING
	switch_منزل_main_thread(prepare_multithreading());
#وإلا
#که_تعریف_شوی ARCH_SWITCH_منزل_MAIN_NO_MULTITHREADING
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
	irq_قلف();
	طالما (true) {
	}
	/* LCOV_EXCL_STOP */
#د_که_پای /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#د_که_پای /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	کوډ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
