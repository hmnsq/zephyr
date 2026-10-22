/*
 * Copyright (c) 2010-2012, 2014-2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Architecture-independent private kernel APIs
 *
 * This file contains private kernel APIs that are not architecture-specific.
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_KERNEL_అంతర్గత_H
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_KERNEL_అంతర్గత_H

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kernel_arch_interface.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <string.h>

#నిర్వచితం_కాకపోతే _ASMLANGUAGE

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

/* Initialize per-CPU kernel data */
శూన్య z_init_cpu(పూర్ణాంకం id);

/* Initialize a thread */
శూన్య z_init_thread_పునాది(నిర్మాణం thread_పునాది *thread_పునాది_2, పూర్ణాంకం ప్రాధాన్యత,
			uint32_t initial_state, గుర్తులేని పూర్ణాంకం options);


FUNC_NORETURN శూన్య z_cstart(శూన్య);

/*
 * Lean kernel-internal init model.
 *
 * A subsystem registers a parameterless init function that the boot path runs
 * at a fixed phase, without the SYS_INIT level/priority machinery (kernel init
 * order is fixed at compile time). Each entry is emitted into the subsystem's
 * own translation unit, so it -- and its init -- is linked only when the
 * subsystem itself is linked, preserving the pay-per-use linkage that SYS_INIT
 * provides. The entry holds a single function pointer (no device back-pointer
 * and no priority), so it is half the size of a SYS_INIT init_entry.
 */
నిర్మాణం k_kernel_init_pre_నమోదు {
	శూన్య (*init_fn)(శూన్య);
};

నిర్మాణం k_kernel_init_post_నమోదు {
	శూన్య (*init_fn)(శూన్య);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#నిర్వచించు K_KERNEL_INIT_PRE(_fn)                                                 \
	స్థిర స్థిరరాశి STRUCT_SECTION_ITERABLE(k_kernel_init_pre_నమోదు,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#నిర్వచించు K_KERNEL_INIT_POST(_fn)                                                \
	స్థిర స్థిరరాశి STRUCT_SECTION_ITERABLE(k_kernel_init_post_నమోదు,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

బాహ్య FUNC_NORETURN శూన్య z_thread_నమోదు(k_thread_నమోదు_t నమోదు_2,
			  శూన్య *p1, శూన్య *p2, శూన్య *p3);

బాహ్య అక్షరం *z_అమరిక_new_thread(నిర్మాణం k_thread *new_thread,
				k_thread_దొంతర_t *దొంతర, size_t దొంతర_కొలత,
				k_thread_నమోదు_t నమోదు_2,
				శూన్య *p1, శూన్య *p2, శూన్య *p3,
				పూర్ణాంకం prio, uint32_t options, స్థిరరాశి అక్షరం *name);

/**
 * @brief Allocate aligned memory from the current thread's resource pool
 *
 * Threads may be assigned a resource pool, which will be used to allocate
 * memory on behalf of certain kernel and driver APIs. Memory reserved
 * in this way should be freed with k_free().
 *
 * If called from an ISR, the k_malloc() system heap will be used if it exists.
 *
 * @param align Required memory alignment
 * @param size Memory allocation size
 * @return A pointer to the allocated memory, or NULL if there is insufficient
 * RAM in the pool or there is no pool to draw memory from
 */
శూన్య *z_thread_aligned_alloc(size_t align, size_t కొలత);

/**
 * @brief Allocate some memory from the current thread's resource pool
 *
 * Threads may be assigned a resource pool, which will be used to allocate
 * memory on behalf of certain kernel and driver APIs. Memory reserved
 * in this way should be freed with k_free().
 *
 * If called from an ISR, the k_malloc() system heap will be used if it exists.
 *
 * @param size Memory allocation size
 * @return A pointer to the allocated memory, or NULL if there is insufficient
 * RAM in the pool or there is no pool to draw memory from
 */
శూన్య *z_thread_malloc(size_t కొలత);


#నిర్వచితమైతే CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
స్థిర ALWAYS_INLINE శూన్య
arch_thread_return_విలువ_సమితి(నిర్మాణం k_thread *thread, గుర్తులేని పూర్ణాంకం విలువ)
{
	thread->మార్పిడి_retval = విలువ;
}
#అయితే_ముగింపు

స్థిర ALWAYS_INLINE శూన్య
z_thread_return_విలువ_సమితి_with_దత్తాంశం(నిర్మాణం k_thread *thread,
				   గుర్తులేని పూర్ణాంకం విలువ,
				   శూన్య *దత్తాంశం)
{
	arch_thread_return_విలువ_సమితి(thread, విలువ);
	thread->పునాది.మార్పిడి_దత్తాంశం = దత్తాంశం;
}

#నిర్వచితమైతే CONFIG_SMP
బాహ్య శూన్య z_smp_init(శూన్య);
#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
బాహ్య శూన్య smp_timer_init(శూన్య);
#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */
#అయితే_ముగింపు /* CONFIG_SMP */

బాహ్య శూన్య z_early_rand_get(uint8_t *buf, size_t నిడివి);

#ఒకవేళ defined(CONFIG_దొంతర_సూచిక_RANDOM) && (CONFIG_దొంతర_సూచిక_RANDOM != 0)
బాహ్య పూర్ణాంకం z_దొంతర_adjust_initialized;
#అయితే_ముగింపు /* CONFIG_STACK_POINTER_RANDOM */

#నిర్వచితమైతే CONFIG_MULTITHREADING
బాహ్య నిర్మాణం k_thread z_main_thread;
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */


K_KERNEL_దొంతర_ARRAY_DECLARE(z_అంతరాయం_stacks, CONFIG_MP_MAX_అంకె_CPUS,
				    CONFIG_ISR_దొంతర_కొలత);
K_THREAD_దొంతర_DECLARE(z_main_దొంతర, CONFIG_MAIN_దొంతర_కొలత);

#నిర్వచితమైతే CONFIG_GEN_PRIV_STACKS
బాహ్య uint8_t *z_priv_దొంతర_కనుగొను(k_thread_దొంతర_t *దొంతర);
#అయితే_ముగింపు /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
పూర్ణాంకం z_దొంతర_space_get(స్థిరరాశి uint8_t *దొంతర_ప్రారంభం, size_t కొలత, size_t *వాడని_ptr);

#నిర్వచితమైతే CONFIG_USERSPACE
bool z_దొంతర_is_user_capable(k_thread_దొంతర_t *దొంతర);

/* Memory domain setup hook, called from z_setup_new_thread() */
శూన్య z_mem_domain_init_thread(నిర్మాణం k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
శూన్య z_mem_domain_exit_thread(నిర్మాణం k_thread *thread);

/* This spinlock:
 *
 * - Protects the full set of active k_mem_domain objects and their contents
 * - Serializes calls to arch_mem_domain_* APIs
 *
 * If architecture code needs to access k_mem_domain structures or the
 * partitions they contain at any other point, this spinlock should be held.
 * Uniprocessor systems can get away with just locking interrupts but this is
 * not recommended.
 */
బాహ్య నిర్మాణం k_spinlock z_mem_domain_తాళం;
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#నిర్వచితమైతే CONFIG_GDBSTUB
నిర్మాణం gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
బాహ్య పూర్ణాంకం z_gdb_main_loop(నిర్మాణం gdb_ctx *ctx);
#అయితే_ముగింపు /* CONFIG_GDBSTUB */

#నిర్వచితమైతే CONFIG_INSTRUMENT_THREAD_SWITCHING
శూన్య z_thread_mark_switched_లోపల(శూన్య);
శూన్య z_thread_mark_switched_బహిర్గతం(శూన్య);
#లేకపోతే

/**
 * @brief Called after a thread has been selected to run
 */
#నిర్వచించు z_thread_mark_switched_లోపల()

/**
 * @brief Called before a thread has been selected to run
 */

#నిర్వచించు z_thread_mark_switched_బహిర్గతం()

#అయితే_ముగింపు /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

/* Init hook for page frame management, invoked immediately upon entry of
 * main thread, before POST_KERNEL tasks
 */
శూన్య z_mem_manage_init(శూన్య);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
శూన్య z_mem_manage_boot_ముగింపు(శూన్య);


bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state);

#నిర్వచితమైతే CONFIG_PM

/* When the kernel is about to go idle, it calls this function to notify the
 * power management subsystem, that the kernel is ready to enter the idle state.
 *
 * At this point, the kernel has disabled interrupts and computed the maximum
 * time the system can remain idle. The function passes the time that the system
 * can remain idle. The SOC interface performs power operations that can be done
 * in the available time. The power management operations must halt execution of
 * the CPU.
 *
 * This function assumes that a wake up event has already been set up by the
 * application.
 *
 * This function is entered with the idle thread's interrupt lock held. If it
 * enters a power state, it returns before the idle thread restores the saved
 * interrupt key. SoC PM code may use architecture helpers around the
 * low-power instruction, but normal IRQ dispatch must remain blocked until
 * that final idle-thread restore.
 *
 * @return True if the system suspended, otherwise return false
 */
bool pm_system_suspend(int32_t ticks);

#అయితే_ముగింపు /* CONFIG_PM */

#నిర్వచితమైతే CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
/**
 * Initialize the timing histograms for demand paging.
 */
శూన్య z_paging_histogram_init(శూన్య);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
శూన్య z_paging_histogram_inc(నిర్మాణం k_mem_paging_histogram_t *hist,
			    uint32_t చక్రాలు);
#అయితే_ముగింపు /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_THREAD
పూర్ణాంకం z_thread_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats);
పూర్ణాంకం z_thread_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats);
పూర్ణాంకం z_thread_stats_reset(నిర్మాణం k_obj_core *obj_core);
పూర్ణాంకం z_thread_stats_disable(నిర్మాణం k_obj_core *obj_core);
పూర్ణాంకం z_thread_stats_enable(నిర్మాణం k_obj_core *obj_core);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_THREAD */

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
పూర్ణాంకం z_cpu_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats);
పూర్ణాంకం z_cpu_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats);

పూర్ణాంకం z_kernel_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats);
పూర్ణాంకం z_kernel_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ఒకవేళ defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
/**
 * Perform cleanup at the end of k_thread_abort().
 *
 * This performs additional cleanup steps at the end of k_thread_abort()
 * where these steps require that the thread is no longer running.
 * If the target thread is not the current running thread, the cleanup
 * steps will be performed immediately. However, if the target thread is
 * the current running thread (e.g. k_thread_abort(_current)), it defers
 * the cleanup steps to later when the work will be finished in another
 * context.
 *
 * @param thread Pointer to thread to be cleaned up.
 */
శూన్య k_thread_abort_cleanup(నిర్మాణం k_thread *thread);

/**
 * Check if thread is the same as the one waiting for cleanup.
 *
 * This is used to guard against reusing the same thread object
 * before the previous cleanup has finished. This will perform
 * the necessary cleanups before the thread object can be
 * reused. Should mainly be used during thread creation.
 *
 * @param thread Pointer to thread to be checked.
 */
శూన్య k_thread_abort_cleanup_check_reuse(నిర్మాణం k_thread *thread);
#అయితే_ముగింపు /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* _ASMLANGUAGE */

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
