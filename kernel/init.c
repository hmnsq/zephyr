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

#شامل <ctype.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <offsets_short.h>
#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/minmax.h>
#شامل <zephyr/sys/printk.h>
#شامل <zephyr/debug/stack.h>
#شامل <zephyr/random/random.h>
#شامل <zephyr/linker/sections.h>
#شامل <zephyr/toolchain.h>
#شامل <zephyr/device.h>
#شامل <zephyr/init.h>
#شامل <zephyr/linker/linker-defs.h>
#شامل <zephyr/platform/hooks.h>
#شامل <ksched.h>
#شامل <kthread.h>
#شامل <ipi.h>
#شامل <zephyr/sys/dlist.h>
#شامل <kernel_internal.h>
#شامل <zephyr/drivers/entropy.h>
#شامل <zephyr/logging/log_ctrl.h>
#شامل <zephyr/tracing/tracing.h>
#شامل <zephyr/debug/gcov.h>
#شامل <kswap.h>
#شامل <zephyr/timing/timing.h>
#شامل <zephyr/logging/log.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/arch/common/init.h>
#شامل <scheduler.h>

LOG_MODULE_رجسٹر(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
ساخت z_kernel _kernel;

#اگر_تعریف_شدہ CONFIG_PM
atomic_t cpus_فعال;
#اگر_ختم

/* init/main and idle threads */
K_THREAD_ڈھیر_DEFINE(z_main_ڈھیر, CONFIG_MAIN_ڈھیر_سائز);

#اگر_تعریف_شدہ CONFIG_MULTITHREADING
ساخت k_thread z_main_thread;
ساخت k_thread z_idle_دھاگے[CONFIG_MP_MAX_عدد_CPUS];

ساکن K_KERNEL_ڈھیر_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_عدد_CPUS,
					  CONFIG_IDLE_ڈھیر_سائز);

ساکن خالی z_init_static_دھاگے(خالی)
{
	STRUCT_SECTION_FOREACH(static_thread_ڈیٹا, thread_ڈیٹا) {
		z_تیاری_new_thread(
			thread_ڈیٹا->init_thread,
			thread_ڈیٹا->init_ڈھیر,
			thread_ڈیٹا->init_ڈھیر_سائز,
			thread_ڈیٹا->init_اندراج,
			thread_ڈیٹا->init_p1,
			thread_ڈیٹا->init_p2,
			thread_ڈیٹا->init_p3,
			thread_ڈیٹا->init_prio,
			thread_ڈیٹا->init_options,
			thread_ڈیٹا->init_name);

		thread_ڈیٹا->init_thread->init_ڈیٹا = thread_ڈیٹا;
	}

#اگر_تعریف_شدہ CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		برائے (صحیح i = 0; pos->objects[i] != تہی; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#اگر_ختم /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_تالا();
	STRUCT_SECTION_FOREACH(static_thread_ڈیٹا, thread_ڈیٹا) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_ڈیٹا);

		thread_جدول_وقت_new(thread_ڈیٹا->init_thread, init_delay);
	}
	k_sched_unlock();
}
#ورنہ
#تعریف_کرو z_init_static_دھاگے() کرو { } جب (false)
#اگر_ختم /* CONFIG_MULTITHREADING */

خارجی ثابت ساخت init_اندراج __init_start[];
خارجی ثابت ساخت init_اندراج __init_EARLY_start[];
خارجی ثابت ساخت init_اندراج __init_PRE_KERNEL_1_start[];
خارجی ثابت ساخت init_اندراج __init_PRE_KERNEL_2_start[];
خارجی ثابت ساخت init_اندراج __init_POST_KERNEL_start[];
خارجی ثابت ساخت init_اندراج __init_APPLICATION_start[];
خارجی ثابت ساخت init_اندراج __init_end[];

شمار init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#اگر_تعریف_شدہ CONFIG_SMP
	INIT_LEVEL_SMP,
#اگر_ختم /* CONFIG_SMP */
};

#اگر_تعریف_شدہ CONFIG_SMP
خارجی ثابت ساخت init_اندراج __init_SMP_start[];
#اگر_ختم /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ڈھیر_ARRAY_DEFINE(z_مداخلت_stacks,
				   CONFIG_MP_MAX_عدد_CPUS,
				   CONFIG_ISR_ڈھیر_سائز);

خارجی خالی idle(خالی *unused1, خالی *unused2, خالی *unused3);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_SYSTEM
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
ساکن ساخت k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_سائز = حجم(ساخت k_چکر_stats),
	.query_سائز = حجم(ساخت k_thread_runtime_stats),
	.raw   = z_cpu_stats_raw,
	.query = z_cpu_stats_query,
	.reset = تہی,
	.disable = تہی,
	.enable  = تہی,
};

/* CPU object cores have no static instances to walk; each CPU links its own
 * object core in z_init_cpu(). Register the type only.
 */
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID,
			    &cpu_stats_desc);
#ورنہ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#اگر_ختم /* CONFIG_OBJ_CORE_SYSTEM */

#اگر_تعریف_شدہ CONFIG_REQUIRES_ڈھیر_CANARIES
#اگر_تعریف_شدہ CONFIG_ڈھیر_CANARIES_TLS
خارجی Z_THREAD_LOCAL متلون uintptr_t __stack_chk_guard;
#ورنہ
خارجی متلون uintptr_t __stack_chk_guard;
#اگر_ختم /* CONFIG_STACK_CANARIES_TLS */
#اگر_ختم /* CONFIG_REQUIRES_STACK_CANARIES */

bool z_sys_post_kernel;

/* defined in device.c */
خارجی صحیح do_device_init(ثابت ساخت device *dev);

/**
 * @brief Initialize state for all static devices.
 *
 * The state object is always zero-initialized, but this may not be
 * sufficient.
 */
ساکن خالی z_device_state_init(خالی)
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
ساکن خالی z_sys_init_تسلسل_level(شمار init_level level)
{
	ساکن ثابت ساخت init_اندراج *سطحیں[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#اگر_تعریف_شدہ CONFIG_SMP
		__init_SMP_start,
#اگر_ختم /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	ثابت ساخت init_اندراج *اندراج_2;

	برائے (اندراج_2 = سطحیں[level]; اندراج_2 < سطحیں[level+1]; اندراج_2++) {
		ثابت ساخت device *dev = اندراج_2->dev;
		صحیح نتیجہ = 0;

		sys_trace_sys_init_enter(اندراج_2, level);
		اگر (dev != تہی) {
			اگر ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				نتیجہ = do_device_init(dev);
			}
		} ورنہ {
			نتیجہ = اندراج_2->init_fn();
		}
		sys_trace_sys_init_exit(اندراج_2, level, نتیجہ);
	}
}

#اگر_تعریف_شدہ CONFIG_STATIC_INIT_GNU

خارجی خالی (*__zephyr_init_array_start[])();
خارجی خالی (*__zephyr_init_array_end[])();

ساکن خالی z_static_init_gnu(خالی)
{
	خالی	(**fn)();

	برائے (fn = __zephyr_init_array_start; fn != __zephyr_init_array_end; fn++) {
		/* MWDT toolchain sticks a NULL at the end of the array */
		اگر (*fn == تہی) {
			توڑو;
		}
		(**fn)();
	}
}

#اگر_ختم

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
ساکن خالی bg_thread_main(خالی *unused1, خالی *unused2, خالی *unused3)
{
	ARG_غیر_مستعمل(unused1);
	ARG_غیر_مستعمل(unused2);
	ARG_غیر_مستعمل(unused3);

#اگر_تعریف_شدہ CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#اگر_ختم /* CONFIG_MMU */
	z_sys_post_kernel = true;

#اگر CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#اگر_ختم

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_اندراج, اندراج_2) {
		اندراج_2->init_fn();
	}

	z_sys_init_تسلسل_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#اگر defined(CONFIG_ڈھیر_اشارہ_گر_RANDOM) && (CONFIG_ڈھیر_اشارہ_گر_RANDOM != 0)
	z_ڈھیر_adjust_initialized = 1;
#اگر_ختم /* CONFIG_STACK_POINTER_RANDOM */

#اگر_تعریف_شدہ CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#اگر_ختم /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_تسلسل_level(INIT_LEVEL_APPLICATION);

	z_init_static_دھاگے();

#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */

#اگر_تعریف_شدہ CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_تسلسل_level(INIT_LEVEL_SMP);
#اگر_ختم /* CONFIG_SMP */

#اگر_تعریف_شدہ CONFIG_MMU
	z_mem_manage_boot_اختتام();
#اگر_ختم /* CONFIG_MMU */

#اگر_تعریف_شدہ CONFIG_BOOTARGS
	خارجی صحیح مین(صحیح argc, حرف **argv);
	خارجی حرف **sys_boot_prepare_main_دلائل(صحیح *argc);

	صحیح argc = 0;
	حرف **argv = sys_boot_prepare_main_دلائل(&argc);
	(خالی)مین(argc, argv);
#ورنہ
	خارجی صحیح مین(خالی);

	(خالی)مین();
#اگر_ختم /* CONFIG_BOOTARGS */

#اگر_تعریف_شدہ CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#وگرنہ_اگر defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#اگر_ختم /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#اگر defined(CONFIG_MULTITHREADING)
__boot_func
ساکن خالی init_idle_thread(صحیح i)
{
	ساخت k_thread *thread = &z_idle_دھاگے[i];
	k_thread_ڈھیر_t *ڈھیر = z_idle_stacks[i];
	size_t ڈھیر_سائز = K_KERNEL_ڈھیر_SIZEOF(z_idle_stacks[i]);

#اگر_تعریف_شدہ CONFIG_THREAD_NAME

#اگر CONFIG_MP_MAX_عدد_CPUS > 1
	حرف tname[8];
	snprintk(tname, 8, "idle %02d", i);
#ورنہ
	حرف *tname = "idle";
#اگر_ختم /* CONFIG_MP_MAX_NUM_CPUS */

#ورنہ
	حرف *tname = تہی;
#اگر_ختم /* CONFIG_THREAD_NAME */

	z_تیاری_new_thread(thread, ڈھیر,
			  ڈھیر_سائز, idle, &_kernel.cpus[i],
			  تہی, تہی, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#اگر_تعریف_شدہ CONFIG_SMP
	thread->بنیاد.is_idle = 1U;
#اگر_ختم /* CONFIG_SMP */
}

خالی z_init_cpu(صحیح id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_دھاگے[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ڈھیر =
		(K_KERNEL_ڈھیر_بفر(z_مداخلت_stacks[id]) +
		 K_KERNEL_ڈھیر_SIZEOF(z_مداخلت_stacks[id]));
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_فعال);
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_رجسٹر(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  حجم(ساخت k_چکر_stats));
#اگر_ختم
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#اگر_ختم
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
ساکن حرف *prepare_multithreading(خالی)
{
	حرف *ڈھیر_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#اگر_غیر_تعریف_شدہ CONFIG_SMP
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
#اگر_ختم /* CONFIG_SMP */
	ڈھیر_ptr = z_تیاری_new_thread(&z_main_thread, z_main_ڈھیر,
				       K_THREAD_ڈھیر_SIZEOF(z_main_ڈھیر),
				       bg_thread_main,
				       تہی, تہی, تہی,
				       CONFIG_MAIN_THREAD_ترجیح,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	واپس ڈھیر_ptr;
}

__boot_func
ساکن FUNC_NORETURN خالی switch_مقصد_main_thread(حرف *ڈھیر_ptr)
{
#اگر_تعریف_شدہ CONFIG_ARCH_HAS_CUSTOM_تبادلہ_مقصد_MAIN
	arch_switch_مقصد_main_thread(&z_main_thread, ڈھیر_ptr, bg_thread_main);
#ورنہ
	ARG_غیر_مستعمل(ڈھیر_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_تبادلہ_unlocked();
#اگر_ختم /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	رمز_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#اگر_ختم /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ڈھیر_PROTECTOR
خالی __weak z_early_rand_get(uint8_t *buf, size_t لمبائی)
{
	ساکن uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	صحیح rc;

#اگر_تعریف_شدہ CONFIG_ENTROPY_HAS_DRIVER
	ثابت ساخت device *ثابت entropy = entropy_get_default_device();

	اگر ((entropy != تہی) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, لمبائی, ENTROPY_BUSYWAIT);
		اگر (rc > 0) {
			لمبائی -= rc;
			buf += rc;
		}
	}
#اگر_ختم /* CONFIG_ENTROPY_HAS_DRIVER */

	جب (لمبائی > 0) {
		uint32_t val;

		state = state + k_چکر_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(لمبائی, حجم(val));
		arch_early_memcpy((خالی *)buf, &val, rc);

		لمبائی -= rc;
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
FUNC_NORETURN خالی z_cstart(خالی)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_تسلسل_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#اگر defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#اگر_ختم /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_اندراج, اندراج_2) {
		اندراج_2->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_تسلسل_level(INIT_LEVEL_PRE_KERNEL_1);
#اگر defined(CONFIG_SMP)
	arch_smp_init();
#اگر_ختم
	z_sys_init_تسلسل_level(INIT_LEVEL_PRE_KERNEL_2);

#اگر_تعریف_شدہ CONFIG_REQUIRES_ڈھیر_CANARIES
	uintptr_t ڈھیر_guard;

	z_early_rand_get((uint8_t *)&ڈھیر_guard, حجم(ڈھیر_guard));
	__stack_chk_guard = ڈھیر_guard;
	__stack_chk_guard <<= 8;
#اگر_ختم	/* CONFIG_REQUIRES_STACK_CANARIES */

#اگر_تعریف_شدہ CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_آغاز();
#اگر_ختم /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#اگر_تعریف_شدہ CONFIG_MULTITHREADING
	switch_مقصد_main_thread(prepare_multithreading());
#ورنہ
#اگر_تعریف_شدہ ARCH_SWITCH_مقصد_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_مقصد_MAIN_NO_MULTITHREADING(bg_thread_main,
		تہی, تہی, تہی);
#ورنہ
	bg_thread_main(تہی, تہی, تہی);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_تالا();
	جب (true) {
	}
	/* LCOV_EXCL_STOP */
#اگر_ختم /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#اگر_ختم /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	رمز_UNREACHABLE; /* LCOV_EXCL_LINE */
}
