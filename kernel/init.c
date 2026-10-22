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

#समावेश <ctype.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <offsets_short.h>
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/sys/minmax.h>
#समावेश <zephyr/sys/printk.h>
#समावेश <zephyr/debug/stack.h>
#समावेश <zephyr/random/random.h>
#समावेश <zephyr/linker/sections.h>
#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/device.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/linker/linker-defs.h>
#समावेश <zephyr/platform/hooks.h>
#समावेश <ksched.h>
#समावेश <kthread.h>
#समावेश <ipi.h>
#समावेश <zephyr/sys/dlist.h>
#समावेश <kernel_internal.h>
#समावेश <zephyr/drivers/entropy.h>
#समावेश <zephyr/logging/log_ctrl.h>
#समावेश <zephyr/tracing/tracing.h>
#समावेश <zephyr/debug/gcov.h>
#समावेश <kswap.h>
#समावेश <zephyr/timing/timing.h>
#समावेश <zephyr/logging/log.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/arch/common/init.h>
#समावेश <scheduler.h>

LOG_MODULE_REGISTER(os, CONFIG_KERNEL_LOG_LEVEL);

/* the only struct z_kernel instance */
काष्ठा z_kernel _kernel;

#अगर_घोषित CONFIG_PM
atomic_t _cpus_active;
#पूर्ण_अगर

/* init/main and idle threads */
K_THREAD_ढेर_DEFINE(z_main_ढेर, CONFIG_MAIN_ढेर_आकार);

#अगर_घोषित CONFIG_MULTITHREADING
काष्ठा k_thread z_main_thread;
काष्ठा k_thread z_idle_threads[CONFIG_MP_MAX_अंक_CPUS];

अटल K_KERNEL_ढेर_ARRAY_DEFINE(z_idle_stacks,
					  CONFIG_MP_MAX_अंक_CPUS,
					  CONFIG_IDLE_ढेर_आकार);

अटल व्योम z_init_static_threads(व्योम)
{
	STRUCT_SECTION_FOREACH(static_thread_तथ्य, thread_तथ्य) {
		z_setup_new_thread(
			thread_तथ्य->init_thread,
			thread_तथ्य->init_ढेर,
			thread_तथ्य->init_ढेर_आकार,
			thread_तथ्य->init_entry,
			thread_तथ्य->init_p1,
			thread_तथ्य->init_p2,
			thread_तथ्य->init_p3,
			thread_तथ्य->init_prio,
			thread_तथ्य->init_options,
			thread_तथ्य->init_name);

		thread_तथ्य->init_thread->init_तथ्य = thread_तथ्य;
	}

#अगर_घोषित CONFIG_USERSPACE
	STRUCT_SECTION_FOREACH(k_object_assignment, pos) {
		क्रम (पूर्णांक i = 0; pos->objects[i] != शून्य; i++) {
			k_object_access_grant(pos->objects[i],
					      pos->thread);
		}
	}
#पूर्ण_अगर /* CONFIG_USERSPACE */

	/*
	 * Static threads may be started immediately or
	 * after a previously specified delay. Even though the
	 * scheduler is locked, ticks can still be delivered and
	 * processed. Take a sched lock to prevent them from running
	 * until they are all started.
	 */
	k_sched_lock();
	STRUCT_SECTION_FOREACH(static_thread_तथ्य, thread_तथ्य) {
		k_timeout_t init_delay = Z_THREAD_INIT_DELAY(thread_तथ्य);

		thread_schedule_new(thread_तथ्य->init_thread, init_delay);
	}
	k_sched_unlock();
}
#अन्यथा
#घोषणा z_init_static_threads() करो { } जबतक (false)
#पूर्ण_अगर /* CONFIG_MULTITHREADING */

बाह्य स्थिर काष्ठा init_entry __init_start[];
बाह्य स्थिर काष्ठा init_entry __init_EARLY_start[];
बाह्य स्थिर काष्ठा init_entry __init_PRE_KERNEL_1_start[];
बाह्य स्थिर काष्ठा init_entry __init_PRE_KERNEL_2_start[];
बाह्य स्थिर काष्ठा init_entry __init_POST_KERNEL_start[];
बाह्य स्थिर काष्ठा init_entry __init_APPLICATION_start[];
बाह्य स्थिर काष्ठा init_entry __init_end[];

क्रमागत init_level {
	INIT_LEVEL_EARLY = 0,
	INIT_LEVEL_PRE_KERNEL_1,
	INIT_LEVEL_PRE_KERNEL_2,
	INIT_LEVEL_POST_KERNEL,
	INIT_LEVEL_APPLICATION,
#अगर_घोषित CONFIG_SMP
	INIT_LEVEL_SMP,
#पूर्ण_अगर /* CONFIG_SMP */
};

#अगर_घोषित CONFIG_SMP
बाह्य स्थिर काष्ठा init_entry __init_SMP_start[];
#पूर्ण_अगर /* CONFIG_SMP */

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_KERNEL_ढेर_ARRAY_DEFINE(z_interrupt_stacks,
				   CONFIG_MP_MAX_अंक_CPUS,
				   CONFIG_ISR_ढेर_आकार);

बाह्य व्योम idle(व्योम *unused1, व्योम *unused2, व्योम *unused3);

#अगर_घोषित CONFIG_OBJ_CORE_SYSTEM
#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
अटल काष्ठा k_obj_core_stats_desc  cpu_stats_desc = {
	.raw_आकार = माप(काष्ठा k_cycle_stats),
	.query_आकार = माप(काष्ठा k_thread_runtime_stats),
	.raw   = z_cpu_stats_raw,
	.query = z_cpu_stats_query,
	.reset = शून्य,
	.disable = शून्य,
	.enable  = शून्य,
};

/* CPU object cores have no static instances to walk; each CPU links its own
 * object core in z_init_cpu(). Register the type only.
 */
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID,
			    &cpu_stats_desc);
#अन्यथा
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_cpu, _cpu, K_OBJ_TYPE_CPU_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_SYSTEM */
#पूर्ण_अगर /* CONFIG_OBJ_CORE_SYSTEM */

#अगर_घोषित CONFIG_REQUIRES_ढेर_CANARIES
#अगर_घोषित CONFIG_ढेर_CANARIES_TLS
बाह्य Z_THREAD_LOCAL अस्थिर uintptr_t __stack_chk_guard;
#अन्यथा
बाह्य अस्थिर uintptr_t __stack_chk_guard;
#पूर्ण_अगर /* CONFIG_STACK_CANARIES_TLS */
#पूर्ण_अगर /* CONFIG_REQUIRES_STACK_CANARIES */

bool z_sys_post_kernel;

/* defined in device.c */
बाह्य पूर्णांक do_device_init(स्थिर काष्ठा device *dev);

/**
 * @brief Initialize state for all static devices.
 *
 * The state object is always zero-initialized, but this may not be
 * sufficient.
 */
अटल व्योम z_device_state_init(व्योम)
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
अटल व्योम z_sys_init_run_level(क्रमागत init_level level)
{
	अटल स्थिर काष्ठा init_entry *levels[] = {
		__init_EARLY_start,
		__init_PRE_KERNEL_1_start,
		__init_PRE_KERNEL_2_start,
		__init_POST_KERNEL_start,
		__init_APPLICATION_start,
#अगर_घोषित CONFIG_SMP
		__init_SMP_start,
#पूर्ण_अगर /* CONFIG_SMP */
		/* End marker */
		__init_end,
	};
	स्थिर काष्ठा init_entry *entry;

	क्रम (entry = levels[level]; entry < levels[level+1]; entry++) {
		स्थिर काष्ठा device *dev = entry->dev;
		पूर्णांक परिणाम = 0;

		sys_trace_sys_init_enter(entry, level);
		अगर (dev != शून्य) {
			अगर ((dev->flags & DEVICE_FLAG_INIT_DEFERRED) == 0U) {
				परिणाम = do_device_init(dev);
			}
		} अन्यथा {
			परिणाम = entry->init_fn();
		}
		sys_trace_sys_init_exit(entry, level, परिणाम);
	}
}

#अगर_घोषित CONFIG_STATIC_INIT_GNU

बाह्य व्योम (*__zephyr_init_array_start[])();
बाह्य व्योम (*__zephyr_init_array_end[])();

अटल व्योम z_static_init_gnu(व्योम)
{
	व्योम	(**fn)();

	क्रम (fn = __zephyr_init_array_start; fn != __zephyr_init_array_end; fn++) {
		/* MWDT toolchain sticks a NULL at the end of the array */
		अगर (*fn == शून्य) {
			अवरोध;
		}
		(**fn)();
	}
}

#पूर्ण_अगर

/**
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 */
__boot_func
अटल व्योम bg_thread_main(व्योम *unused1, व्योम *unused2, व्योम *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

#अगर_घोषित CONFIG_MMU
	/* Invoked here such that backing store or eviction algorithms may
	 * initialize kernel objects, and that all POST_KERNEL and later tasks
	 * may perform memory management tasks (except for
	 * k_mem_map_phys_bare() which is allowed at any time)
	 */
	z_mem_manage_init();
#पूर्ण_अगर /* CONFIG_MMU */
	z_sys_post_kernel = true;

#अगर CONFIG_IRQ_OFFLOAD
	arch_irq_offload_init();
#पूर्ण_अगर

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

#अगर defined(CONFIG_ढेर_POINTER_RANDOM) && (CONFIG_ढेर_POINTER_RANDOM != 0)
	z_ढेर_adjust_initialized = 1;
#पूर्ण_अगर /* CONFIG_STACK_POINTER_RANDOM */

#अगर_घोषित CONFIG_STATIC_INIT_GNU
	z_static_init_gnu();
#पूर्ण_अगर /* CONFIG_STATIC_INIT_GNU */

	/* Final init level before app starts */
	z_sys_init_run_level(INIT_LEVEL_APPLICATION);

	z_init_static_threads();

#अगर_घोषित CONFIG_KERNEL_COHERENCE
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(&_kernel));
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */

#अगर_घोषित CONFIG_SMP
	/* Start the secondary CPUs; CPUs whose devicetree node carries
	 * zephyr,deferred-start are skipped and left for a run-time
	 * k_smp_cpu_start()/k_smp_cpu_resume().
	 */
	z_smp_init();
	z_sys_init_run_level(INIT_LEVEL_SMP);
#पूर्ण_अगर /* CONFIG_SMP */

#अगर_घोषित CONFIG_MMU
	z_mem_manage_boot_finish();
#पूर्ण_अगर /* CONFIG_MMU */

#अगर_घोषित CONFIG_BOOTARGS
	बाह्य पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv);
	बाह्य अक्षर **sys_boot_prepare_main_args(पूर्णांक *argc);

	पूर्णांक argc = 0;
	अक्षर **argv = sys_boot_prepare_main_args(&argc);
	(व्योम)मुख्य(argc, argv);
#अन्यथा
	बाह्य पूर्णांक मुख्य(व्योम);

	(व्योम)मुख्य();
#पूर्ण_अगर /* CONFIG_BOOTARGS */

#अगर_घोषित CONFIG_MULTITHREADING
	/* Mark non-essential since main() has no more work to do */
	z_thread_essential_clear(&z_main_thread);
#पूर्ण_अगर

#अगर_घोषित CONFIG_COVERAGE_DUMP
	/* Dump coverage data once the main() has exited. */
	gcov_coverage_dump();
#या_अगर defined(CONFIG_COVERAGE_SEMIHOST)
	gcov_coverage_semihost();
#पूर्ण_अगर /* CONFIG_COVERAGE_DUMP */
} /* LCOV_EXCL_LINE ... because we just dumped final coverage data */

#अगर defined(CONFIG_MULTITHREADING)
__boot_func
अटल व्योम init_idle_thread(पूर्णांक i)
{
	काष्ठा k_thread *thread = &z_idle_threads[i];
	k_thread_ढेर_t *ढेर = z_idle_stacks[i];
	माप_प्रकार ढेर_आकार = K_KERNEL_ढेर_SIZEOF(z_idle_stacks[i]);

#अगर_घोषित CONFIG_THREAD_NAME

#अगर CONFIG_MP_MAX_अंक_CPUS > 1
	अक्षर tname[8];
	snprintk(tname, 8, "idle %02d", i);
#अन्यथा
	अक्षर *tname = "idle";
#पूर्ण_अगर /* CONFIG_MP_MAX_NUM_CPUS */

#अन्यथा
	अक्षर *tname = शून्य;
#पूर्ण_अगर /* CONFIG_THREAD_NAME */

	z_setup_new_thread(thread, ढेर,
			  ढेर_आकार, idle, &_kernel.cpus[i],
			  शून्य, शून्य, K_IDLE_PRIO, K_ESSENTIAL,
			  tname);
	z_mark_thread_as_not_sleeping(thread);

#अगर_घोषित CONFIG_SMP
	thread->base.is_idle = 1U;
#पूर्ण_अगर /* CONFIG_SMP */
}

व्योम z_init_cpu(पूर्णांक id)
{
	init_idle_thread(id);
	_kernel.cpus[id].idle_thread = &z_idle_threads[id];
	_kernel.cpus[id].id = id;
	_kernel.cpus[id].irq_ढेर =
		(K_KERNEL_ढेर_बफर(z_interrupt_stacks[id]) +
		 K_KERNEL_ढेर_SIZEOF(z_interrupt_stacks[id]));
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
	_kernel.cpus[id].usage = &_kernel.usage[id];
	_kernel.cpus[id].usage->track_usage =
		CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#पूर्ण_अगर

#अगर_घोषित CONFIG_PM
	/*
	 * Increment number of CPUs active. The pm subsystem
	 * will keep track of this from here.
	 */
	atomic_inc(&_cpus_active);
#पूर्ण_अगर

#अगर_घोषित CONFIG_OBJ_CORE_SYSTEM
	k_obj_core_init_and_link(K_OBJ_CORE(&_kernel.cpus[id]), &obj_type_cpu);
#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
	k_obj_core_stats_register(K_OBJ_CORE(&_kernel.cpus[id]),
				  _kernel.cpus[id].usage,
				  माप(काष्ठा k_cycle_stats));
#पूर्ण_अगर
#पूर्ण_अगर

#अगर_घोषित CONFIG_SCHED_IPI_SUPPORTED
	sys_dlist_init(&_kernel.cpus[id].ipi_workq);
#पूर्ण_अगर
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
अटल अक्षर *prepare_multithreading(व्योम)
{
	अक्षर *ढेर_ptr;

	/* _kernel.ready_q is all zeroes */
	z_sched_init();

#अगर_अघोषित CONFIG_SMP
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
#पूर्ण_अगर /* CONFIG_SMP */
	ढेर_ptr = z_setup_new_thread(&z_main_thread, z_main_ढेर,
				       K_THREAD_ढेर_SIZEOF(z_main_ढेर),
				       bg_thread_main,
				       शून्य, शून्य, शून्य,
				       CONFIG_MAIN_THREAD_PRIORITY,
				       K_ESSENTIAL, "main");
	z_mark_thread_as_not_sleeping(&z_main_thread);
	z_ready_thread(&z_main_thread);

	z_init_cpu(0);

	वापस ढेर_ptr;
}

__boot_func
अटल FUNC_NORETURN व्योम switch_to_main_thread(अक्षर *ढेर_ptr)
{
#अगर_घोषित CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	arch_switch_to_main_thread(&z_main_thread, ढेर_ptr, bg_thread_main);
#अन्यथा
	ARG_UNUSED(ढेर_ptr);
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */
	z_swap_unlocked();
#पूर्ण_अगर /* CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN */
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}
#पूर्ण_अगर /* CONFIG_MULTITHREADING */

__boot_func
FUNC_NO_ढेर_PROTECTOR
व्योम __weak z_early_rand_get(uint8_t *buf, माप_प्रकार लंबाई)
{
	अटल uint64_t state = (uint64_t)CONFIG_TIMER_RANDOM_INITIAL_STATE;
	पूर्णांक rc;

#अगर_घोषित CONFIG_ENTROPY_HAS_DRIVER
	स्थिर काष्ठा device *स्थिर entropy = entropy_get_default_device();

	अगर ((entropy != शून्य) && device_is_ready(entropy)) {
		/* Try to see if driver provides an ISR-specific API */
		rc = entropy_get_entropy_isr(entropy, buf, लंबाई, ENTROPY_BUSYWAIT);
		अगर (rc > 0) {
			लंबाई -= rc;
			buf += rc;
		}
	}
#पूर्ण_अगर /* CONFIG_ENTROPY_HAS_DRIVER */

	जबतक (लंबाई > 0) {
		uint32_t val;

		state = state + k_cycle_get_32();
		state = state * 2862933555777941757ULL + 3037000493ULL;
		val = (uint32_t)(state >> 32);
		rc = min(लंबाई, माप(val));
		arch_early_memcpy((व्योम *)buf, &val, rc);

		लंबाई -= rc;
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
FUNC_NO_ढेर_PROTECTOR
FUNC_NORETURN व्योम z_cstart(व्योम)
{
	/* gcov hook needed to get the coverage report.*/
	gcov_static_init();

	/* initialize early init calls */
	z_sys_init_run_level(INIT_LEVEL_EARLY);

	/* perform any architecture-specific initialization */
	arch_kernel_init();

	LOG_CORE_INIT();

#अगर defined(CONFIG_MULTITHREADING)
	z_dummy_thread_init(&_thread_dummy);
#पूर्ण_अगर /* CONFIG_MULTITHREADING */
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
#अगर defined(CONFIG_SMP)
	arch_smp_init();
#पूर्ण_अगर
	z_sys_init_run_level(INIT_LEVEL_PRE_KERNEL_2);

#अगर_घोषित CONFIG_REQUIRES_ढेर_CANARIES
	uintptr_t ढेर_guard;

	z_early_rand_get((uint8_t *)&ढेर_guard, माप(ढेर_guard));
	__stack_chk_guard = ढेर_guard;
	__stack_chk_guard <<= 8;
#पूर्ण_अगर	/* CONFIG_REQUIRES_STACK_CANARIES */

#अगर_घोषित CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT
	timing_init();
	timing_start();
#पूर्ण_अगर /* CONFIG_TIMING_FUNCTIONS_NEED_AT_BOOT */

#अगर_घोषित CONFIG_MULTITHREADING
	switch_to_main_thread(prepare_multithreading());
#अन्यथा
#अगर_घोषित ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING
	/* Custom ARCH-specific routine to switch to main()
	 * in the case of no multi-threading.
	 */
	ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING(bg_thread_main,
		शून्य, शून्य, शून्य);
#अन्यथा
	bg_thread_main(शून्य, शून्य, शून्य);

	/* LCOV_EXCL_START
	 * We've already dumped coverage data at this point.
	 */
	irq_lock();
	जबतक (true) {
	}
	/* LCOV_EXCL_STOP */
#पूर्ण_अगर /* ARCH_SWITCH_TO_MAIN_NO_MULTITHREADING */
#पूर्ण_अगर /* CONFIG_MULTITHREADING */

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}
