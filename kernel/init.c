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

LOG_MODULE_رجسٹر(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
بنية z_kernel _kernel;

#جے_تعریف_شدہ CONFIG_PM
atomic_t cpus_سرگرم;
#جے_انت

/* init/main and idle threads */
K_THREAD_ڈھیر_DEFINE(z_main_ڈھیر, CONFIG_MAIN_ڈھیر_ناپ);

#جے_تعریف_شدہ CONFIG_MULTITHREADING
بنية k_thread z_main_thread;
بنية k_thread z_idle_دھاگے[CONFIG_MP_MAX_عدد_CPUS];

ساكن K_KERNEL_ڈھیر_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_عدد_CPUS,
					  CONFIG_IDLE_ڈھیر_ناپ);

ساكن فراغ z_init_static_دھاگے(فراغ)
{
	STRUCT_SECTION_FOREACH(static_thread_انکڑے, thread_انکڑے) {
		z_تیاری_new_thread(
			thread_انکڑے->init_thread,
			thread_انکڑے->init_ڈھیر,
			thread_انکڑے->init_ڈھیر_ناپ,
			thread_انکڑے->init_اندراج,
			thread_انکڑے->init_p1,
			thread_انکڑے->init_p2,
			thread_انکڑے->init_p3,
			thread_انکڑے->init_prio,
			thread_انکڑے->init_options,
			thread_انکڑے->init_name);

		thread_انکڑے->init_thread->init_انکڑے = thread_انکڑے;
	}

#جے_تعریف_شدہ CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		لكل (صحيح i = 0; pos->objects[i] != عدم; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#جے_انت /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_جندرا();
	STRUCT_SECTION_FOREACH(static_thread_انکڑے, thread_انکڑے) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_انکڑے);

		thread_ویلا_سارنی_new(thread_انکڑے->init_thread, init_delay);
	}
	k_sched_unlock();
}
#وإلا
#تعریف_کرو z_init_static_دھاگے() افعل { } طالما (false)
#جے_انت /* CONFIG_MULTITHREADING */

خارجي ثابت بنية init_اندراج __init_start[];
خارجي ثابت بنية init_اندراج __init_EARLY_start[];
خارجي ثابت بنية init_اندراج __init_PRE_KERNEL_1_start[];
خارجي ثابت بنية init_اندراج __init_PRE_KERNEL_2_start[];
خارجي ثابت بنية init_اندراج __init_POST_KERNEL_start[];
خارجي ثابت بنية init_اندراج __init_APPLICATION_start[];
خارجي ثابت بنية init_اندراج __init_end[];

تعداد init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#جے_تعریف_شدہ CONFIG_SMP
	INIT_LEVEL_SMP,
#جے_انت /* CONFIG_SMP */
};

#جے_تعریف_شدہ CONFIG_SMP
خارجي ثابت بنية init_اندراج __init_SMP_start[];
#جے_انت /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ڈھیر_ARRAY_DEFINE(z_رکاوٹ_stacks,
				   CONFIG_MP_MAX_عدد_CPUS,
				   CONFIG_ISR_ڈھیر_ناپ);

خارجي فراغ idle(فراغ *unused1, فراغ *unused2, فراغ *unused3);

#جے_تعریف_شدہ CONFIG_OBJ_CORE_SYSTEM
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
ساكن بنية k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_ناپ = حجم(بنية k_چکر_stats),
	.query_ناپ = حجم(بنية k_thread_runtime_stats),
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
#جے_انت /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#جے_انت /* CONFIG_OBJ_CORE_SYSTEM */

#جے_تعریف_شدہ CONFIG_REQUIRES_ڈھیر_CANARIES
#جے_تعریف_شدہ CONFIG_ڈھیر_CANARIES_TLS
خارجي Z_THREAD_LOCAL متطاير uintptr_t __stack_chk_guard;
#وإلا
خارجي متطاير uintptr_t __stack_chk_guard;
#جے_انت /* CONFIG_STACK_CANARIES_TLS */
#جے_انت /* CONFIG_REQUIRES_STACK_CANARIES */

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
ساكن فراغ z_sys_init_لگاتار_level(تعداد init_level level)
{
	ساكن ثابت بنية init_اندراج *پدھراں[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#جے_تعریف_شدہ CONFIG_SMP
		__init_SMP_start,
#جے_انت /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	ثابت بنية init_اندراج *اندراج;

	لكل (اندراج = پدھراں[level]; اندراج < پدھراں[level+1]; اندراج++) {
		ثابت بنية device *dev = اندراج->dev;
		صحيح نتیجہ = 0;

		sys_trace_sys_init_enter(اندراج, level);
		إذا (dev != عدم) {
			إذا ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				نتیجہ = do_device_init(dev);
			}
		} وإلا {
			نتیجہ = اندراج->init_fn();
		}
		sys_trace_sys_init_exit(اندراج, level, نتیجہ);
	}
}

#جے_تعریف_شدہ CONFIG_STATIC_INIT_GNU

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

#جے_انت

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
ساكن فراغ bg_thread_main(فراغ *unused1, فراغ *unused2, فراغ *unused3)
{
	ARG_بن_ورتیا(unused1);
	ARG_بن_ورتیا(unused2);
	ARG_بن_ورتیا(unused3);

#جے_تعریف_شدہ CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#جے_انت /* CONFIG_MMU */
	z_sys_post_kernel = true;

#إذا CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#جے_انت

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_اندراج, اندراج) {
		اندراج->init_fn();
	}

	z_sys_init_لگاتار_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#إذا defined(CONFIG_ڈھیر_اشارہ_گر_RANDOM) && (CONFIG_ڈھیر_اشارہ_گر_RANDOM != 0)
	z_ڈھیر_adjust_initialized = 1;
#جے_انت /* CONFIG_STACK_POINTER_RANDOM */

#جے_تعریف_شدہ CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#جے_انت /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_لگاتار_level(INIT_LEVEL_APPLICATION);

	z_init_static_دھاگے();

#جے_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#جے_انت /* CONFIG_KERNEL_COHERENCE */

#جے_تعریف_شدہ CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_لگاتار_level(INIT_LEVEL_SMP);
#جے_انت /* CONFIG_SMP */

#جے_تعریف_شدہ CONFIG_MMU
	z_mem_manage_boot_انت();
#جے_انت /* CONFIG_MMU */

#جے_تعریف_شدہ CONFIG_BOOTARGS
	خارجي صحيح مکھ(صحيح argc, حرف **argv);
	خارجي حرف **sys_boot_prepare_main_دلیلاں(صحيح *argc);

	صحيح argc = 0;
	حرف **argv = sys_boot_prepare_main_دلیلاں(&argc);
	(فراغ)مکھ(argc, argv);
#وإلا
	خارجي صحيح مکھ(فراغ);

	(فراغ)مکھ();
#جے_انت /* CONFIG_BOOTARGS */

#جے_تعریف_شدہ CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#جے_انت

#جے_تعریف_شدہ CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#وإلاإذا defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#جے_انت /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#إذا defined(CONFIG_MULTITHREADING)
__boot_func
ساكن فراغ init_idle_thread(صحيح i)
{
	بنية k_thread *thread = &z_idle_دھاگے[i];
	k_thread_ڈھیر_t *ڈھیر = z_idle_stacks[i];
	size_t ڈھیر_ناپ = K_KERNEL_ڈھیر_SIZEOF(z_idle_stacks[i]);

#جے_تعریف_شدہ CONFIG_THREAD_NAME

#إذا CONFIG_MP_MAX_عدد_CPUS > 1
	حرف tname[8];
	snprintk(tname, 8, "idle %02d", i);
#وإلا
	حرف *tname = "idle";
#جے_انت /* CONFIG_MP_MAX_NUM_CPUS */

#وإلا
	حرف *tname = عدم;
#جے_انت /* CONFIG_THREAD_NAME */

	z_تیاری_new_thread(thread, ڈھیر,
			  ڈھیر_ناپ, idle, &_kernel.cpus[i],
			  عدم, عدم, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#جے_تعریف_شدہ CONFIG_SMP
	thread->ادھار.is_idle = 1U;
#جے_انت /* CONFIG_SMP */
}

فراغ z_init_cpu(صحيح id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_دھاگے[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ڈھیر =
		(K_KERNEL_ڈھیر_بفر(z_رکاوٹ_stacks[id]) +
		 K_KERNEL_ڈھیر_SIZEOF(z_رکاوٹ_stacks[id]));
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#جے_انت

#جے_تعریف_شدہ CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_سرگرم);
#جے_انت

#جے_تعریف_شدہ CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  حجم(بنية k_چکر_stats));
#جے_انت
#جے_انت

#جے_تعریف_شدہ CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#جے_انت
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
	حرف *ڈھیر_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#جے_بنا_تعریف CONFIG_SMP
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
#جے_انت /* CONFIG_SMP */
	ڈھیر_ptr = z_تیاری_new_thread(&z_main_thread, z_main_ڈھیر,
				       K_THREAD_ڈھیر_SIZEOF(z_main_ڈھیر),
				       bg_thread_main,
				       عدم, عدم, عدم,
				       CONFIG_MAIN_THREAD_ترجیح,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	أرجع ڈھیر_ptr;
}

__boot_func
ساكن FUNC_NORETURN فراغ switch_منزل_main_thread(حرف *ڈھیر_ptr)
{
#جے_تعریف_شدہ CONFIG_ARCH_HAS_CUSTOM_وٹاندرا_منزل_MAIN
	arch_switch_منزل_main_thread(&z_main_thread, ڈھیر_ptr, bg_thread_main);
#وإلا
	ARG_بن_ورتیا(ڈھیر_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_وٹاندرا_unlocked();
#جے_انت /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	کوڈ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#جے_انت /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ڈھیر_PROTECTOR
فراغ __weak z_early_rand_get(uint8_t *buf, size_t لمائی)
{
	ساكن uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	صحيح rc;

#جے_تعریف_شدہ CONFIG_ENTROPY_HAS_DRIVER
	ثابت بنية device *ثابت entropy = entropy_get_default_device();

	إذا ((entropy != عدم) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, لمائی, ENTROPY_BUSYWAIT);
		إذا (rc > 0) {
			لمائی -= rc;
			buf += rc;
		}
	}
#جے_انت /* CONFIG_ENTROPY_HAS_DRIVER */

	طالما (لمائی > 0) {
		uint32_t val;

		state = state + k_چکر_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(لمائی, حجم(val));
		arch_early_memcpy((فراغ *)buf, &val, rc);

		لمائی -= rc;
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
FUNC_NO_ڈھیر_PROTECTOR
FUNC_NORETURN فراغ z_cstart(فراغ)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_لگاتار_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#إذا defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#جے_انت /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_اندراج, اندراج) {
		اندراج->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_لگاتار_level(INIT_LEVEL_PRE_KERNEL_1);
#إذا defined(CONFIG_SMP)
	arch_smp_init();
#جے_انت
	z_sys_init_لگاتار_level(INIT_LEVEL_PRE_KERNEL_2);

#جے_تعریف_شدہ CONFIG_REQUIRES_ڈھیر_CANARIES
	uintptr_t ڈھیر_guard;

	z_early_rand_get((uint8_t *)&ڈھیر_guard, حجم(ڈھیر_guard));
	__stack_chk_guard = ڈھیر_guard;
	__stack_chk_guard <<= 8;
#جے_انت	/* CONFIG_REQUIRES_STACK_CANARIES */

#جے_تعریف_شدہ CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_شروع();
#جے_انت /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#جے_تعریف_شدہ CONFIG_MULTITHREADING
	switch_منزل_main_thread(prepare_multithreading());
#وإلا
#جے_تعریف_شدہ ARCH_SWITCH_منزل_MAIN_NO_MULTITHREADING
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
	irq_جندرا();
	طالما (true) {
	}
	/* LCOV_EXCL_STOP */
#جے_انت /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#جے_انت /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	کوڈ_UNREACHABLE; /* LCOV_EXCL_LINE */
}
