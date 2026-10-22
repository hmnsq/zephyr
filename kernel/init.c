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

#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <offsets_short.h>
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/sys/minmax.h>
#అన్తర్భావ <zephyr/sys/printk.h>
#అన్తర్భావ <zephyr/debug/stack.h>
#అన్తర్భావ <zephyr/random/random.h>
#అన్తర్భావ <zephyr/linker/sections.h>
#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <zephyr/device.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/linker/linker-defs.h>
#అన్తర్భావ <zephyr/platform/hooks.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <ipi.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/drivers/entropy.h>
#అన్తర్భావ <zephyr/logging/log_ctrl.h>
#అన్తర్భావ <zephyr/tracing/tracing.h>
#అన్తర్భావ <zephyr/debug/gcov.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <zephyr/timing/timing.h>
#అన్తర్భావ <zephyr/logging/log.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/arch/common/init.h>
#అన్తర్భావ <scheduler.h>

LOG_MODULE_రిజిస్టరు(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
నిర్మాణం z_kernel _kernel;

#నిర్వచితమైతే CONFIG_PM
atomic_t cpus_చురుకైన;
#అయితే_ముగింపు

/* init/main and idle threads */
K_THREAD_దొంతర_DEFINE(z_main_దొంతర, CONFIG_MAIN_దొంతర_కొలత);

#నిర్వచితమైతే CONFIG_MULTITHREADING
నిర్మాణం k_thread z_main_thread;
నిర్మాణం k_thread z_idle_దారాలు[CONFIG_MP_MAX_అంకె_CPUS];

స్థిర K_KERNEL_దొంతర_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_అంకె_CPUS,
					  CONFIG_IDLE_దొంతర_కొలత);

స్థిర శూన్య z_init_static_దారాలు(శూన్య)
{
	STRUCT_SECTION_FOREACH(static_thread_దత్తాంశం, thread_దత్తాంశం) {
		z_అమరిక_new_thread(
			thread_దత్తాంశం->init_thread,
			thread_దత్తాంశం->init_దొంతర,
			thread_దత్తాంశం->init_దొంతర_కొలత,
			thread_దత్తాంశం->init_నమోదు,
			thread_దత్తాంశం->init_p1,
			thread_దత్తాంశం->init_p2,
			thread_దత్తాంశం->init_p3,
			thread_దత్తాంశం->init_prio,
			thread_దత్తాంశం->init_options,
			thread_దత్తాంశం->init_name);

		thread_దత్తాంశం->init_thread->init_దత్తాంశం = thread_దత్తాంశం;
	}

#నిర్వచితమైతే CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		కొరకు (పూర్ణాంకం i = 0; pos->objects[i] != ఖాళీ; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#అయితే_ముగింపు /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_తాళం();
	STRUCT_SECTION_FOREACH(static_thread_దత్తాంశం, thread_దత్తాంశం) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_దత్తాంశం);

		thread_కాలపట్టిక_new(thread_దత్తాంశం->init_thread, init_delay);
	}
	k_sched_unlock();
}
#లేకపోతే
#నిర్వచించు z_init_static_దారాలు() చేయి { } అయితే (false)
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */

బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_start[];
బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_EARLY_start[];
బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_PRE_KERNEL_1_start[];
బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_PRE_KERNEL_2_start[];
బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_POST_KERNEL_start[];
బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_APPLICATION_start[];
బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_end[];

గణన init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#నిర్వచితమైతే CONFIG_SMP
	INIT_LEVEL_SMP,
#అయితే_ముగింపు /* CONFIG_SMP */
};

#నిర్వచితమైతే CONFIG_SMP
బాహ్య స్థిరరాశి నిర్మాణం init_నమోదు __init_SMP_start[];
#అయితే_ముగింపు /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_దొంతర_ARRAY_DEFINE(z_అంతరాయం_stacks,
				   CONFIG_MP_MAX_అంకె_CPUS,
				   CONFIG_ISR_దొంతర_కొలత);

బాహ్య శూన్య idle(శూన్య *unused1, శూన్య *unused2, శూన్య *unused3);

#నిర్వచితమైతే CONFIG_OBJ_CORE_SYSTEM
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
స్థిర నిర్మాణం k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_కొలత = పరిమాణం(నిర్మాణం k_చక్రం_stats),
	.query_కొలత = పరిమాణం(నిర్మాణం k_thread_runtime_stats),
	.raw   = z_cpu_stats_raw,
	.query = z_cpu_stats_query,
	.reset = ఖాళీ,
	.disable = ఖాళీ,
	.enable  = ఖాళీ,
};

/* CPU object cores have no static instances to walk; each CPU links its own
 * object core in z_init_cpu(). Register the type only.
 */
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID,
			    &cpu_stats_desc);
#లేకపోతే
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_SYSTEM */

#నిర్వచితమైతే CONFIG_REQUIRES_దొంతర_CANARIES
#నిర్వచితమైతే CONFIG_దొంతర_CANARIES_TLS
బాహ్య Z_THREAD_LOCAL అస్థిర uintptr_t __stack_chk_guard;
#లేకపోతే
బాహ్య అస్థిర uintptr_t __stack_chk_guard;
#అయితే_ముగింపు /* CONFIG_STACK_CANARIES_TLS */
#అయితే_ముగింపు /* CONFIG_REQUIRES_STACK_CANARIES */

bool z_sys_post_kernel;

/* defined in device.c */
బాహ్య పూర్ణాంకం do_device_init(స్థిరరాశి నిర్మాణం device *dev);

/**
 * @brief Initialize state for all static devices.
 *
 * The state object is always zero-initialized, but this may not be
 * sufficient.
 */
స్థిర శూన్య z_device_state_init(శూన్య)
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
స్థిర శూన్య z_sys_init_నిరంతరం_level(గణన init_level level)
{
	స్థిర స్థిరరాశి నిర్మాణం init_నమోదు *స్థాయిలు[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#నిర్వచితమైతే CONFIG_SMP
		__init_SMP_start,
#అయితే_ముగింపు /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	స్థిరరాశి నిర్మాణం init_నమోదు *నమోదు_2;

	కొరకు (నమోదు_2 = స్థాయిలు[level]; నమోదు_2 < స్థాయిలు[level+1]; నమోదు_2++) {
		స్థిరరాశి నిర్మాణం device *dev = నమోదు_2->dev;
		పూర్ణాంకం ఫలితం = 0;

		sys_trace_sys_init_enter(నమోదు_2, level);
		ఒకవేళ (dev != ఖాళీ) {
			ఒకవేళ ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				ఫలితం = do_device_init(dev);
			}
		} లేకపోతే {
			ఫలితం = నమోదు_2->init_fn();
		}
		sys_trace_sys_init_exit(నమోదు_2, level, ఫలితం);
	}
}

#నిర్వచితమైతే CONFIG_STATIC_INIT_GNU

బాహ్య శూన్య (*__zephyr_init_array_start[])();
బాహ్య శూన్య (*__zephyr_init_array_end[])();

స్థిర శూన్య z_static_init_gnu(శూన్య)
{
	శూన్య	(**fn)();

	కొరకు (fn = __zephyr_init_array_start; fn != __zephyr_init_array_end; fn++) {
		/* MWDT toolchain sticks a NULL at the end of the array */
		ఒకవేళ (*fn == ఖాళీ) {
			ఆపు;
		}
		(**fn)();
	}
}

#అయితే_ముగింపు

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
స్థిర శూన్య bg_thread_main(శూన్య *unused1, శూన్య *unused2, శూన్య *unused3)
{
	ARG_వాడని(unused1);
	ARG_వాడని(unused2);
	ARG_వాడని(unused3);

#నిర్వచితమైతే CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#అయితే_ముగింపు /* CONFIG_MMU */
	z_sys_post_kernel = true;

#ఒకవేళ CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#అయితే_ముగింపు

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_నమోదు, నమోదు_2) {
		నమోదు_2->init_fn();
	}

	z_sys_init_నిరంతరం_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#ఒకవేళ defined(CONFIG_దొంతర_సూచిక_RANDOM) && (CONFIG_దొంతర_సూచిక_RANDOM != 0)
	z_దొంతర_adjust_initialized = 1;
#అయితే_ముగింపు /* CONFIG_STACK_POINTER_RANDOM */

#నిర్వచితమైతే CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#అయితే_ముగింపు /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_నిరంతరం_level(INIT_LEVEL_APPLICATION);

	z_init_static_దారాలు();

#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */

#నిర్వచితమైతే CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_నిరంతరం_level(INIT_LEVEL_SMP);
#అయితే_ముగింపు /* CONFIG_SMP */

#నిర్వచితమైతే CONFIG_MMU
	z_mem_manage_boot_ముగింపు();
#అయితే_ముగింపు /* CONFIG_MMU */

#నిర్వచితమైతే CONFIG_BOOTARGS
	బాహ్య పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv);
	బాహ్య అక్షరం **sys_boot_prepare_main_వాదనలు(పూర్ణాంకం *argc);

	పూర్ణాంకం argc = 0;
	అక్షరం **argv = sys_boot_prepare_main_వాదనలు(&argc);
	(శూన్య)ప్రధాన(argc, argv);
#లేకపోతే
	బాహ్య పూర్ణాంకం ప్రధాన(శూన్య);

	(శూన్య)ప్రధాన();
#అయితే_ముగింపు /* CONFIG_BOOTARGS */

#నిర్వచితమైతే CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#లేదావేళ defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#అయితే_ముగింపు /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#ఒకవేళ defined(CONFIG_MULTITHREADING)
__boot_func
స్థిర శూన్య init_idle_thread(పూర్ణాంకం i)
{
	నిర్మాణం k_thread *thread = &z_idle_దారాలు[i];
	k_thread_దొంతర_t *దొంతర = z_idle_stacks[i];
	size_t దొంతర_కొలత = K_KERNEL_దొంతర_SIZEOF(z_idle_stacks[i]);

#నిర్వచితమైతే CONFIG_THREAD_NAME

#ఒకవేళ CONFIG_MP_MAX_అంకె_CPUS > 1
	అక్షరం tname[8];
	snprintk(tname, 8, "idle %02d", i);
#లేకపోతే
	అక్షరం *tname = "idle";
#అయితే_ముగింపు /* CONFIG_MP_MAX_NUM_CPUS */

#లేకపోతే
	అక్షరం *tname = ఖాళీ;
#అయితే_ముగింపు /* CONFIG_THREAD_NAME */

	z_అమరిక_new_thread(thread, దొంతర,
			  దొంతర_కొలత, idle, &_kernel.cpus[i],
			  ఖాళీ, ఖాళీ, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#నిర్వచితమైతే CONFIG_SMP
	thread->పునాది.is_idle = 1U;
#అయితే_ముగింపు /* CONFIG_SMP */
}

శూన్య z_init_cpu(పూర్ణాంకం id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_దారాలు[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_దొంతర =
		(K_KERNEL_దొంతర_బఫరు(z_అంతరాయం_stacks[id]) +
		 K_KERNEL_దొంతర_SIZEOF(z_అంతరాయం_stacks[id]));
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&cpus_చురుకైన);
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_రిజిస్టరు(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  పరిమాణం(నిర్మాణం k_చక్రం_stats));
#అయితే_ముగింపు
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#అయితే_ముగింపు
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
స్థిర అక్షరం *prepare_multithreading(శూన్య)
{
	అక్షరం *దొంతర_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#నిర్వచితం_కాకపోతే CONFIG_SMP
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
#అయితే_ముగింపు /* CONFIG_SMP */
	దొంతర_ptr = z_అమరిక_new_thread(&z_main_thread, z_main_దొంతర,
				       K_THREAD_దొంతర_SIZEOF(z_main_దొంతర),
				       bg_thread_main,
				       ఖాళీ, ఖాళీ, ఖాళీ,
				       CONFIG_MAIN_THREAD_ప్రాధాన్యత,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	తిరిగి దొంతర_ptr;
}

__boot_func
స్థిర FUNC_NORETURN శూన్య switch_గమ్యం_main_thread(అక్షరం *దొంతర_ptr)
{
#నిర్వచితమైతే CONFIG_ARCH_HAS_CUSTOM_మార్పిడి_గమ్యం_MAIN
	arch_switch_గమ్యం_main_thread(&z_main_thread, దొంతర_ptr, bg_thread_main);
#లేకపోతే
	ARG_వాడని(దొంతర_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_మార్పిడి_unlocked();
#అయితే_ముగింపు /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_దొంతర_PROTECTOR
శూన్య __weak z_early_rand_get(uint8_t *buf, size_t నిడివి)
{
	స్థిర uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	పూర్ణాంకం rc;

#నిర్వచితమైతే CONFIG_ENTROPY_HAS_DRIVER
	స్థిరరాశి నిర్మాణం device *స్థిరరాశి entropy = entropy_get_default_device();

	ఒకవేళ ((entropy != ఖాళీ) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, నిడివి, ENTROPY_BUSYWAIT);
		ఒకవేళ (rc > 0) {
			నిడివి -= rc;
			buf += rc;
		}
	}
#అయితే_ముగింపు /* CONFIG_ENTROPY_HAS_DRIVER */

	అయితే (నిడివి > 0) {
		uint32_t val;

		state = state + k_చక్రం_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(నిడివి, పరిమాణం(val));
		arch_early_memcpy((శూన్య *)buf, &val, rc);

		నిడివి -= rc;
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
FUNC_NO_దొంతర_PROTECTOR
FUNC_NORETURN శూన్య z_cstart(శూన్య)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_నిరంతరం_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#ఒకవేళ defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_నమోదు, నమోదు_2) {
		నమోదు_2->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_నిరంతరం_level(INIT_LEVEL_PRE_KERNEL_1);
#ఒకవేళ defined(CONFIG_SMP)
	arch_smp_init();
#అయితే_ముగింపు
	z_sys_init_నిరంతరం_level(INIT_LEVEL_PRE_KERNEL_2);

#నిర్వచితమైతే CONFIG_REQUIRES_దొంతర_CANARIES
	uintptr_t దొంతర_guard;

	z_early_rand_get((uint8_t *)&దొంతర_guard, పరిమాణం(దొంతర_guard));
	__stack_chk_guard = దొంతర_guard;
	__stack_chk_guard <<= 8;
#అయితే_ముగింపు	/* CONFIG_REQUIRES_STACK_CANARIES */

#నిర్వచితమైతే CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_ప్రారంభం();
#అయితే_ముగింపు /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#నిర్వచితమైతే CONFIG_MULTITHREADING
	switch_గమ్యం_main_thread(prepare_multithreading());
#లేకపోతే
#నిర్వచితమైతే ARCH_SWITCH_గమ్యం_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_గమ్యం_MAIN_NO_MULTITHREADING(bg_thread_main,
		ఖాళీ, ఖాళీ, ఖాళీ);
#లేకపోతే
	bg_thread_main(ఖాళీ, ఖాళీ, ఖాళీ);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_తాళం();
	అయితే (true) {
	}
	/* LCOV_EXCL_STOP */
#అయితే_ముగింపు /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	సంకేతం_UNREACHABLE; /* LCOV_EXCL_LINE */
}
