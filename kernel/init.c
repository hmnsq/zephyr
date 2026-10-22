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

LOG_MODULE_ثبات(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
بنية z_kernel _kernel;

#اگر_تعریف_شده CONFIG_PM
atomic_t cpus_فعال;
#پایان_اگر

/* init/main and idle threads */
K_THREAD_پشته_DEFINE(z_main_پشته, CONFIG_MAIN_پشته_اندازه);

#اگر_تعریف_شده CONFIG_MULTITHREADING
بنية k_thread z_main_thread;
بنية k_thread z_idle_نخها[CONFIG_MP_MAX_رقم_CPUS];

ساكن K_KERNEL_پشته_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_رقم_CPUS,
					  CONFIG_IDLE_پشته_اندازه);

ساكن فراغ z_init_static_نخها(فراغ)
{
	STRUCT_SECTION_FOREACH(static_thread_داده, thread_داده) {
		z_راهاندازی_new_thread(
			thread_داده->init_thread,
			thread_داده->init_پشته,
			thread_داده->init_پشته_اندازه,
			thread_داده->init_مدخل,
			thread_داده->init_p1,
			thread_داده->init_p2,
			thread_داده->init_p3,
			thread_داده->init_prio,
			thread_داده->init_options,
			thread_داده->init_name);

		thread_داده->init_thread->init_داده = thread_داده;
	}

#اگر_تعریف_شده CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		لكل (صحيح i = 0; pos->objects[i] != عدم; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#پایان_اگر /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_قفل();
	STRUCT_SECTION_FOREACH(static_thread_داده, thread_داده) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_داده);

		thread_زمانبندی_new(thread_داده->init_thread, init_delay);
	}
	k_sched_unlock();
}
#وإلا
#تعریف_کن z_init_static_نخها() افعل { } طالما (false)
#پایان_اگر /* CONFIG_MULTITHREADING */

خارجي ثابت بنية init_مدخل __init_start[];
خارجي ثابت بنية init_مدخل __init_EARLY_start[];
خارجي ثابت بنية init_مدخل __init_PRE_KERNEL_1_start[];
خارجي ثابت بنية init_مدخل __init_PRE_KERNEL_2_start[];
خارجي ثابت بنية init_مدخل __init_POST_KERNEL_start[];
خارجي ثابت بنية init_مدخل __init_APPLICATION_start[];
خارجي ثابت بنية init_مدخل __init_end[];

تعداد init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#اگر_تعریف_شده CONFIG_SMP
	INIT_LEVEL_SMP,
#پایان_اگر /* CONFIG_SMP */
};

#اگر_تعریف_شده CONFIG_SMP
خارجي ثابت بنية init_مدخل __init_SMP_start[];
#پایان_اگر /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_پشته_ARRAY_DEFINE(z_وقفه_stacks,
				   CONFIG_MP_MAX_رقم_CPUS,
				   CONFIG_ISR_پشته_اندازه);

خارجي فراغ idle(فراغ *unused1, فراغ *unused2, فراغ *unused3);

#اگر_تعریف_شده CONFIG_OBJ_CORE_SYSTEM
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_SYSTEM
ساكن بنية k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_اندازه = حجم(بنية k_چرخه_stats),
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
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#پایان_اگر /* CONFIG_OBJ_CORE_SYSTEM */

#اگر_تعریف_شده CONFIG_REQUIRES_پشته_CANARIES
#اگر_تعریف_شده CONFIG_پشته_CANARIES_TLS
خارجي Z_THREAD_LOCAL متطاير uintptr_t __stack_chk_guard;
#وإلا
خارجي متطاير uintptr_t __stack_chk_guard;
#پایان_اگر /* CONFIG_STACK_CANARIES_TLS */
#پایان_اگر /* CONFIG_REQUIRES_STACK_CANARIES */

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
ساكن فراغ z_sys_init_پیاپی_level(تعداد init_level level)
{
	ساكن ثابت بنية init_مدخل *سطوح[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#اگر_تعریف_شده CONFIG_SMP
		__init_SMP_start,
#پایان_اگر /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	ثابت بنية init_مدخل *مدخل;

	لكل (مدخل = سطوح[level]; مدخل < سطوح[level+1]; مدخل++) {
		ثابت بنية device *dev = مدخل->dev;
		صحيح نتیجه = 0;

		sys_trace_sys_init_enter(مدخل, level);
		إذا (dev != عدم) {
			إذا ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				نتیجه = do_device_init(dev);
			}
		} وإلا {
			نتیجه = مدخل->init_fn();
		}
		sys_trace_sys_init_exit(مدخل, level, نتیجه);
	}
}

#اگر_تعریف_شده CONFIG_STATIC_INIT_GNU

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

#پایان_اگر

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
ساكن فراغ bg_thread_main(فراغ *unused1, فراغ *unused2, فراغ *unused3)
{
	ARG_استفادهنشده(unused1);
	ARG_استفادهنشده(unused2);
	ARG_استفادهنشده(unused3);

#اگر_تعریف_شده CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#پایان_اگر /* CONFIG_MMU */
	z_sys_post_kernel = true;

#إذا CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#پایان_اگر

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_مدخل, مدخل) {
		مدخل->init_fn();
	}

	z_sys_init_پیاپی_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#إذا defined(CONFIG_پشته_اشاره_گر_RANDOM) && (CONFIG_پشته_اشاره_گر_RANDOM != 0)
	z_پشته_adjust_initialized = 1;
#پایان_اگر /* CONFIG_STACK_POINTER_RANDOM */

#اگر_تعریف_شده CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#پایان_اگر /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_پیاپی_level(INIT_LEVEL_APPLICATION);

	z_init_static_نخها();

#اگر_تعریف_شده CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#پایان_اگر /* CONFIG_KERNEL_COHERENCE */

#اگر_تعریف_شده CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_پیاپی_level(INIT_LEVEL_SMP);
#پایان_اگر /* CONFIG_SMP */

#اگر_تعریف_شده CONFIG_MMU
	z_mem_manage_boot_پایان();
#پایان_اگر /* CONFIG_MMU */

#اگر_تعریف_شده CONFIG_BOOTARGS
	خارجي صحيح اصلی(صحيح argc, حرف **argv);
	خارجي حرف **sys_boot_prepare_main_آرگومانها(صحيح *argc);

	صحيح argc = 0;
	حرف **argv = sys_boot_prepare_main_آرگومانها(&argc);
	(فراغ)اصلی(argc, argv);
#وإلا
	خارجي صحيح اصلی(فراغ);

	(فراغ)اصلی();
#پایان_اگر /* CONFIG_BOOTARGS */

#اگر_تعریف_شده CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#پایان_اگر

#اگر_تعریف_شده CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#وإلاإذا defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#پایان_اگر /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#إذا defined(CONFIG_MULTITHREADING)
__boot_func
ساكن فراغ init_idle_thread(صحيح i)
{
	بنية k_thread *thread = &z_idle_نخها[i];
	k_thread_پشته_t *پشته = z_idle_stacks[i];
	size_t پشته_اندازه = K_KERNEL_پشته_SIZEOF(z_idle_stacks[i]);

#اگر_تعریف_شده CONFIG_THREAD_NAME

#إذا CONFIG_MP_MAX_رقم_CPUS > 1
	حرف tname[8];
	snprintk(tname, 8, "idle %02d", i);
#وإلا
	حرف *tname = "idle";
#پایان_اگر /* CONFIG_MP_MAX_NUM_CPUS */

#وإلا
	حرف *tname = عدم;
#پایان_اگر /* CONFIG_THREAD_NAME */

	z_راهاندازی_new_thread(thread, پشته,
			  پشته_اندازه, idle, &_kernel.cpus[i],
			  عدم, عدم, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#اگر_تعریف_شده CONFIG_SMP
	thread->پایه.is_idle = 1U;
#پایان_اگر /* CONFIG_SMP */
}

فراغ z_init_cpu(صحيح id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_نخها[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_پشته =
		(K_KERNEL_پشته_میانگیر(z_وقفه_stacks[id]) +
		 K_KERNEL_پشته_SIZEOF(z_وقفه_stacks[id]));
#اگر_تعریف_شده CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#پایان_اگر

#اگر_تعریف_شده CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_فعال);
#پایان_اگر

#اگر_تعریف_شده CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_ثبات(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  حجم(بنية k_چرخه_stats));
#پایان_اگر
#پایان_اگر

#اگر_تعریف_شده CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#پایان_اگر
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
	حرف *پشته_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#اگر_تعریف_نشده CONFIG_SMP
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
#پایان_اگر /* CONFIG_SMP */
	پشته_ptr = z_راهاندازی_new_thread(&z_main_thread, z_main_پشته,
				       K_THREAD_پشته_SIZEOF(z_main_پشته),
				       bg_thread_main,
				       عدم, عدم, عدم,
				       CONFIG_MAIN_THREAD_اولویت,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	أرجع پشته_ptr;
}

__boot_func
ساكن FUNC_NORETURN فراغ switch_مقصد_main_thread(حرف *پشته_ptr)
{
#اگر_تعریف_شده CONFIG_ARCH_HAS_CUSTOM_جابجا_مقصد_MAIN
	arch_switch_مقصد_main_thread(&z_main_thread, پشته_ptr, bg_thread_main);
#وإلا
	ARG_استفادهنشده(پشته_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_جابجا_unlocked();
#پایان_اگر /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	کد_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#پایان_اگر /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_پشته_PROTECTOR
فراغ __weak z_early_rand_get(uint8_t *buf, size_t درازی)
{
	ساكن uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	صحيح rc;

#اگر_تعریف_شده CONFIG_ENTROPY_HAS_DRIVER
	ثابت بنية device *ثابت entropy = entropy_get_default_device();

	إذا ((entropy != عدم) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, درازی, ENTROPY_BUSYWAIT);
		إذا (rc > 0) {
			درازی -= rc;
			buf += rc;
		}
	}
#پایان_اگر /* CONFIG_ENTROPY_HAS_DRIVER */

	طالما (درازی > 0) {
		uint32_t val;

		state = state + k_چرخه_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(درازی, حجم(val));
		arch_early_memcpy((فراغ *)buf, &val, rc);

		درازی -= rc;
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
FUNC_NO_پشته_PROTECTOR
FUNC_NORETURN فراغ z_cstart(فراغ)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_پیاپی_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#إذا defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#پایان_اگر /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_مدخل, مدخل) {
		مدخل->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_پیاپی_level(INIT_LEVEL_PRE_KERNEL_1);
#إذا defined(CONFIG_SMP)
	arch_smp_init();
#پایان_اگر
	z_sys_init_پیاپی_level(INIT_LEVEL_PRE_KERNEL_2);

#اگر_تعریف_شده CONFIG_REQUIRES_پشته_CANARIES
	uintptr_t پشته_guard;

	z_early_rand_get((uint8_t *)&پشته_guard, حجم(پشته_guard));
	__stack_chk_guard = پشته_guard;
	__stack_chk_guard <<= 8;
#پایان_اگر	/* CONFIG_REQUIRES_STACK_CANARIES */

#اگر_تعریف_شده CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_شروع();
#پایان_اگر /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#اگر_تعریف_شده CONFIG_MULTITHREADING
	switch_مقصد_main_thread(prepare_multithreading());
#وإلا
#اگر_تعریف_شده ARCH_SWITCH_مقصد_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_مقصد_MAIN_NO_MULTITHREADING(bg_thread_main,
		عدم, عدم, عدم);
#وإلا
	bg_thread_main(عدم, عدم, عدم);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_قفل();
	طالما (true) {
	}
	/* LCOV_EXCL_STOP */
#پایان_اگر /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#پایان_اگر /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	کد_UNREACHABLE; /* LCOV_EXCL_LINE */
}
