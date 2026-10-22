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

#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <offsets_short.h>
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/sys/minmax.h>
#અન્તર્ભાવ <zephyr/sys/printk.h>
#અન્તર્ભાવ <zephyr/debug/stack.h>
#અન્તર્ભાવ <zephyr/random/random.h>
#અન્તર્ભાવ <zephyr/linker/sections.h>
#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <zephyr/device.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/linker/linker-defs.h>
#અન્તર્ભાવ <zephyr/platform/hooks.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <ipi.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/drivers/entropy.h>
#અન્તર્ભાવ <zephyr/logging/log_ctrl.h>
#અન્તર્ભાવ <zephyr/tracing/tracing.h>
#અન્તર્ભાવ <zephyr/debug/gcov.h>
#અન્તર્ભાવ <kswap.h>
#અન્તર્ભાવ <zephyr/timing/timing.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/arch/common/init.h>
#અન્તર્ભાવ <scheduler.h>

LOG_MODULE_REGISTER(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
સંરચના z_kernel _kernel;

#ifdef CONFIG_PM
atomic_t _cpus_active;
#endif

/* init/main and idle threads */
K_THREAD_ઢગલો_DEFINE(z_main_ઢગલો, CONFIG_MAIN_ઢગલો_SIZE);

#ifdef CONFIG_MULTITHREADING
સંરચના k_thread z_main_thread;
સંરચના k_thread z_idle_threads[CONFIG_MP_MAX_અંક_CPUS];

સ્થિર K_KERNEL_ઢગલો_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_અંક_CPUS,
					  CONFIG_IDLE_ઢગલો_SIZE);

સ્થિર શૂન્ય z_init_static_threads(શૂન્ય)
{
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data) {
		z_setup_new_thread(
			thread_data->init_thread,
			thread_data->init_ઢગલો,
			thread_data->init_ઢગલો_size,
			thread_data->init_entry,
			thread_data->init_p1,
			thread_data->init_p2,
			thread_data->init_p3,
			thread_data->init_prio,
			thread_data->init_options,
			thread_data->init_name);

		thread_data->init_thread->init_data = thread_data;
	}

#ifdef CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		માટે (પૂર્ણાંક i = 0; pos->objects[i] != ખાલી; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#endif /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_lock();
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_data);

		thread_schedule_new(thread_data->init_thread, init_delay);
	}
	k_sched_unlock();
}
#નહીંતર
#define z_init_static_threads() કરો { } જ્યારે (false)
#endif /* CONFIG_MULTITHREADING */

બાહ્ય અચલ સંરચના init_entry __init_start[];
બાહ્ય અચલ સંરચના init_entry __init_EARLY_start[];
બાહ્ય અચલ સંરચના init_entry __init_PRE_KERNEL_1_start[];
બાહ્ય અચલ સંરચના init_entry __init_PRE_KERNEL_2_start[];
બાહ્ય અચલ સંરચના init_entry __init_POST_KERNEL_start[];
બાહ્ય અચલ સંરચના init_entry __init_APPLICATION_start[];
બાહ્ય અચલ સંરચના init_entry __init_end[];

ગણના init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#ifdef CONFIG_SMP
	INIT_LEVEL_SMP,
#endif /* CONFIG_SMP */
};

#ifdef CONFIG_SMP
બાહ્ય અચલ સંરચના init_entry __init_SMP_start[];
#endif /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ઢગલો_ARRAY_DEFINE(z_interrupt_stacks,
				   CONFIG_MP_MAX_અંક_CPUS,
				   CONFIG_ISR_ઢગલો_SIZE);

બાહ્ય શૂન્ય idle(શૂન્ય *unused1, શૂન્ય *unused2, શૂન્ય *unused3);

#ifdef CONFIG_OBJ_CORE_SYSTEM
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
સ્થિર સંરચના k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_size = કદ(સંરચના k_cycle_stats),
	.query_size = કદ(સંરચના k_thread_runtime_stats),
	.raw   = z_cpu_stats_raw,
	.query = z_cpu_stats_query,
	.reset = ખાલી,
	.disable = ખાલી,
	.enable  = ખાલી,
};

/* CPU object cores have no static instances to walk; each CPU links its own
 * object core in z_init_cpu(). Register the type only.
 */
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID,
			    &cpu_stats_desc);
#નહીંતર
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#endif /* CONFIG_OBJ_CORE_SYSTEM */

#ifdef CONFIG_REQUIRES_ઢગલો_CANARIES
#ifdef CONFIG_ઢગલો_CANARIES_TLS
બાહ્ય Z_THREAD_LOCAL અસ્થિર uintptr_t __stack_chk_guard;
#નહીંતર
બાહ્ય અસ્થિર uintptr_t __stack_chk_guard;
#endif /* CONFIG_STACK_CANARIES_TLS */
#endif /* CONFIG_REQUIRES_STACK_CANARIES */

bool z_sys_post_kernel;

/* defined in device.c */
બાહ્ય પૂર્ણાંક do_device_init(અચલ સંરચના device *dev);

/**
 * @brief Initialize state for all static devices.
 *
 * The state object is always zero-initialized, but this may not be
 * sufficient.
 */
સ્થિર શૂન્ય z_device_state_init(શૂન્ય)
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
સ્થિર શૂન્ય z_sys_init_run_level(ગણના init_level level)
{
	સ્થિર અચલ સંરચના init_entry *levels[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#ifdef CONFIG_SMP
		__init_SMP_start,
#endif /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	અચલ સંરચના init_entry *entry;

	માટે (entry = levels[level]; entry < levels[level+1]; entry++) {
		અચલ સંરચના device *dev = entry->dev;
		પૂર્ણાંક result = 0;

		sys_trace_sys_init_enter(entry, level);
		જો (dev != ખાલી) {
			જો ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				result = do_device_init(dev);
			}
		} નહીંતર {
			result = entry->init_fn();
		}
		sys_trace_sys_init_exit(entry, level, result);
	}
}

#ifdef CONFIG_STATIC_INIT_GNU

બાહ્ય શૂન્ય (*__zephyr_init_array_start[])();
બાહ્ય શૂન્ય (*__zephyr_init_array_end[])();

સ્થિર શૂન્ય z_static_init_gnu(શૂન્ય)
{
	શૂન્ય	(**fn)();

	માટે (fn = __zephyr_init_array_start; fn != __zephyr_init_array_end; fn++) {
		/* MWDT toolchain sticks a NULL at the end of the array */
		જો (*fn == ખાલી) {
			તોડો;
		}
		(**fn)();
	}
}

#endif

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
સ્થિર શૂન્ય bg_thread_main(શૂન્ય *unused1, શૂન્ય *unused2, શૂન્ય *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

#ifdef CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#endif /* CONFIG_MMU */
	z_sys_post_kernel = true;

#જો CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#endif

	/* Run kernel post-init hooks before POST_KERNEL device init, so that
	 * drivers initialized there may use them (e.g. submit to the system
	 * work queue). Entries are present only for subsystems that are linked
	 * in (pay-per-use); the walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_post_entry, entry) {
		entry->init_fn();
	}

	z_sys_init_run_level(INIT_LEVEL_POST_KERNEL);

	soc_late_init_hook();

	board_late_init_hook();

#જો defined(CONFIG_ઢગલો_POINTER_RANDOM) && (CONFIG_ઢગલો_POINTER_RANDOM != 0)
	z_ઢગલો_adjust_initialized = 1;
#endif /* CONFIG_STACK_POINTER_RANDOM */

#ifdef CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#endif /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_run_level(INIT_LEVEL_APPLICATION);

	z_init_static_threads();

#ifdef CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#endif /* CONFIG_KERNEL_COHERENCE */

#ifdef CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_run_level(INIT_LEVEL_SMP);
#endif /* CONFIG_SMP */

#ifdef CONFIG_MMU
	z_mem_manage_boot_finish();
#endif /* CONFIG_MMU */

#ifdef CONFIG_BOOTARGS
	બાહ્ય પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv);
	બાહ્ય અક્ષર **sys_boot_prepare_main_args(પૂર્ણાંક *argc);

	પૂર્ણાંક argc = 0;
	અક્ષર **argv = sys_boot_prepare_main_args(&argc);
	(શૂન્ય)મુખ્ય(argc, argv);
#નહીંતર
	બાહ્ય પૂર્ણાંક મુખ્ય(શૂન્ય);

	(શૂન્ય)મુખ્ય();
#endif /* CONFIG_BOOTARGS */

#ifdef CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#endif

#ifdef CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#અથવાજો defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#endif /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#જો defined(CONFIG_MULTITHREADING)
__boot_func
સ્થિર શૂન્ય init_idle_thread(પૂર્ણાંક i)
{
	સંરચના k_thread *thread = &z_idle_threads[i];
	k_thread_ઢગલો_t *ઢગલો = z_idle_stacks[i];
	size_t ઢગલો_size = K_KERNEL_ઢગલો_SIZEOF(z_idle_stacks[i]);

#ifdef CONFIG_THREAD_NAME

#જો CONFIG_MP_MAX_અંક_CPUS > 1
	અક્ષર tname[8];
	snprintk(tname, 8, "idle %02d", i);
#નહીંતર
	અક્ષર *tname = "idle";
#endif /* CONFIG_MP_MAX_NUM_CPUS */

#નહીંતર
	અક્ષર *tname = ખાલી;
#endif /* CONFIG_THREAD_NAME */

	z_setup_new_thread(thread, ઢગલો,
			  ઢગલો_size, idle, &_kernel.cpus[i],
			  ખાલી, ખાલી, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#ifdef CONFIG_SMP
	thread->base.is_idle = 1U;
#endif /* CONFIG_SMP */
}

શૂન્ય z_init_cpu(પૂર્ણાંક id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_threads[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ઢગલો =
		(K_KERNEL_ઢગલો_BUFFER(z_interrupt_stacks[id]) +
		 K_KERNEL_ઢગલો_SIZEOF(z_interrupt_stacks[id]));
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#endif

#ifdef CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&_cpus_active);
#endif

#ifdef CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  કદ(સંરચના k_cycle_stats));
#endif
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#endif
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
સ્થિર અક્ષર *prepare_multithreading(શૂન્ય)
{
	અક્ષર *ઢગલો_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#ifndef CONFIG_SMP
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
#endif /* CONFIG_SMP */
	ઢગલો_ptr = z_setup_new_thread(&z_main_thread, z_main_ઢગલો,
				       K_THREAD_ઢગલો_SIZEOF(z_main_ઢગલો),
				       bg_thread_main,
				       ખાલી, ખાલી, ખાલી,
				       CONFIG_MAIN_THREAD_PRIORITY,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	પાછા ઢગલો_ptr;
}

__boot_func
સ્થિર FUNC_NORETURN શૂન્ય switch_to_main_thread(અક્ષર *ઢગલો_ptr)
{
#ifdef CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	arch_switch_to_main_thread(&z_main_thread, ઢગલો_ptr, bg_thread_main);
#નહીંતર
	ARG_UNUSED(ઢગલો_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_swap_unlocked();
#endif /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#endif /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ઢગલો_PROTECTOR
શૂન્ય __weak z_early_rand_get(uint8_t *buf, size_t length)
{
	સ્થિર uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	પૂર્ણાંક rc;

#ifdef CONFIG_ENTROPY_HAS_DRIVER
	અચલ સંરચના device *અચલ entropy = entropy_get_default_device();

	જો ((entropy != ખાલી) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, length, ENTROPY_BUSYWAIT);
		જો (rc > 0) {
			length -= rc;
			buf += rc;
		}
	}
#endif /* CONFIG_ENTROPY_HAS_DRIVER */

	જ્યારે (length > 0) {
		uint32_t val;

		state = state + k_cycle_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(length, કદ(val));
		arch_early_memcpy((શૂન્ય *)buf, &val, rc);

		length -= rc;
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
FUNC_NO_ઢગલો_PROTECTOR
FUNC_NORETURN શૂન્ય z_cstart(શૂન્ય)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_run_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#જો defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#endif /* CONFIG_MULTITHREADING */
	/* do any necessary initialization of static devices */
	z_device_state_init();

	soc_early_init_hook();

	board_early_init_hook();

	/* Run kernel pre-init hooks before PRE_KERNEL device init, so those
	 * drivers may use the kernel objects initialized here. Entries are
	 * present only for subsystems that are linked in (pay-per-use); the
	 * walk references only the section bounds.
	 */
	STRUCT_SECTION_FOREACH(k_kernel_init_pre_entry, entry) {
		entry->init_fn();
	}

	/* perform basic hardware initialization */
	z_sys_init_run_level(INIT_LEVEL_PRE_KERNEL_1);
#જો defined(CONFIG_SMP)
	arch_smp_init();
#endif
	z_sys_init_run_level(INIT_LEVEL_PRE_KERNEL_2);

#ifdef CONFIG_REQUIRES_ઢગલો_CANARIES
	uintptr_t ઢગલો_guard;

	z_early_rand_get((uint8_t *)&ઢગલો_guard, કદ(ઢગલો_guard));
	__stack_chk_guard = ઢગલો_guard;
	__stack_chk_guard <<= 8;
#endif	/* CONFIG_REQUIRES_STACK_CANARIES */

#ifdef CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_start();
#endif /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#ifdef CONFIG_MULTITHREADING
	switch_to_main_thread(prepare_multithreading());
#નહીંતર
#ifdef ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING(bg_thread_main,
		ખાલી, ખાલી, ખાલી);
#નહીંતર
	bg_thread_main(ખાલી, ખાલી, ખાલી);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_lock();
	જ્યારે (true) {
	}
	/* LCOV_EXCL_STOP */
#endif /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#endif /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}
