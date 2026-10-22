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

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_KERNEL_आन्तरिकम्_H
#परिभाषय ZEPHYR_KERNEL_INCLUDE_KERNEL_आन्तरिकम्_H

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kernel_arch_interface.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <string.h>

#यदि_अपरिभाषितम् _ASMLANGUAGE

#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः

/* Initialize per-CPU kernel data */
शून्य z_init_cpu(पूर्णाङ्क id);

/* Initialize a thread */
शून्य z_init_thread_आधारः(संरचना thread_आधारः *thread_आधारः_2, पूर्णाङ्क प्राथमिकता,
			uint32_t initial_state, अचिह्नित पूर्णाङ्क options);


FUNC_NORETURN शून्य z_cstart(शून्य);

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
संरचना k_kernel_init_pre_प्रविष्टिः {
	शून्य (*init_fn)(शून्य);
};

संरचना k_kernel_init_post_प्रविष्टिः {
	शून्य (*init_fn)(शून्य);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#परिभाषय K_KERNEL_INIT_PRE(_fn)                                                 \
	स्थिर ध्रुव STRUCT_SECTION_ITERABLE(k_kernel_init_pre_प्रविष्टिः,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#परिभाषय K_KERNEL_INIT_POST(_fn)                                                \
	स्थिर ध्रुव STRUCT_SECTION_ITERABLE(k_kernel_init_post_प्रविष्टिः,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

बाह्य FUNC_NORETURN शून्य z_thread_प्रविष्टिः(k_thread_प्रविष्टिः_t प्रविष्टिः,
			  शून्य *p1, शून्य *p2, शून्य *p3);

बाह्य वर्ण *z_सज्जता_new_thread(संरचना k_thread *new_thread,
				k_thread_राशिः_t *राशिः, size_t राशिः_आकार,
				k_thread_प्रविष्टिः_t प्रविष्टिः,
				शून्य *p1, शून्य *p2, शून्य *p3,
				पूर्णाङ्क prio, uint32_t options, ध्रुव वर्ण *name);

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
शून्य *z_thread_aligned_alloc(size_t align, size_t आकार);

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
शून्य *z_thread_malloc(size_t आकार);


#यदि_परिभाषितम् CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
स्थिर ALWAYS_INLINE शून्य
arch_thread_return_मानम्_समुच्चयः(संरचना k_thread *thread, अचिह्नित पूर्णाङ्क मानम्)
{
	thread->swap_retval = मानम्;
}
#यदि_अन्तः

स्थिर ALWAYS_INLINE शून्य
z_thread_return_मानम्_समुच्चयः_with_दत्त(संरचना k_thread *thread,
				   अचिह्नित पूर्णाङ्क मानम्,
				   शून्य *दत्त)
{
	arch_thread_return_मानम्_समुच्चयः(thread, मानम्);
	thread->आधारः.swap_दत्त = दत्त;
}

#यदि_परिभाषितम् CONFIG_SMP
बाह्य शून्य z_smp_init(शून्य);
#यदि_परिभाषितम् CONFIG_SYS_CLOCK_EXISTS
बाह्य शून्य smp_timer_init(शून्य);
#यदि_अन्तः /* CONFIG_SYS_CLOCK_EXISTS */
#यदि_अन्तः /* CONFIG_SMP */

बाह्य शून्य z_early_rand_get(uint8_t *buf, size_t दीर्घता);

#यदि defined(CONFIG_राशिः_POINTER_RANDOM) && (CONFIG_राशिः_POINTER_RANDOM != 0)
बाह्य पूर्णाङ्क z_राशिः_adjust_initialized;
#यदि_अन्तः /* CONFIG_STACK_POINTER_RANDOM */

#यदि_परिभाषितम् CONFIG_MULTITHREADING
बाह्य संरचना k_thread z_main_thread;
#यदि_अन्तः /* CONFIG_MULTITHREADING */


K_KERNEL_राशिः_ARRAY_DECLARE(z_व्यवधानम्_stacks, CONFIG_MP_MAX_अङ्कः_CPUS,
				    CONFIG_ISR_राशिः_आकार);
K_THREAD_राशिः_DECLARE(z_main_राशिः, CONFIG_MAIN_राशिः_आकार);

#यदि_परिभाषितम् CONFIG_GEN_PRIV_STACKS
बाह्य uint8_t *z_priv_राशिः_find(k_thread_राशिः_t *राशिः);
#यदि_अन्तः /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
पूर्णाङ्क z_राशिः_space_get(ध्रुव uint8_t *राशिः_start, size_t आकार, size_t *अप्रयुक्तम्_ptr);

#यदि_परिभाषितम् CONFIG_USERSPACE
bool z_राशिः_is_user_capable(k_thread_राशिः_t *राशिः);

/* Memory domain setup hook, called from z_setup_new_thread() */
शून्य z_mem_domain_init_thread(संरचना k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
शून्य z_mem_domain_exit_thread(संरचना k_thread *thread);

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
बाह्य संरचना k_spinlock z_mem_domain_तालकम्;
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि_परिभाषितम् CONFIG_GDBSTUB
संरचना gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
बाह्य पूर्णाङ्क z_gdb_main_loop(संरचना gdb_ctx *ctx);
#यदि_अन्तः /* CONFIG_GDBSTUB */

#यदि_परिभाषितम् CONFIG_INSTRUMENT_THREAD_SWITCHING
शून्य z_thread_mark_switched_अन्तर्(शून्य);
शून्य z_thread_mark_switched_बहिर्गमनम्(शून्य);
#अन्यथा

/**
 * @brief Called after a thread has been selected to run
 */
#परिभाषय z_thread_mark_switched_अन्तर्()

/**
 * @brief Called before a thread has been selected to run
 */

#परिभाषय z_thread_mark_switched_बहिर्गमनम्()

#यदि_अन्तः /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

/* Init hook for page frame management, invoked immediately upon entry of
 * main thread, before POST_KERNEL tasks
 */
शून्य z_mem_manage_init(शून्य);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
शून्य z_mem_manage_boot_finish(शून्य);


bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state);

#यदि_परिभाषितम् CONFIG_PM

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

#यदि_अन्तः /* CONFIG_PM */

#यदि_परिभाषितम् CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
/**
 * Initialize the timing histograms for demand paging.
 */
शून्य z_paging_histogram_init(शून्य);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
शून्य z_paging_histogram_inc(संरचना k_mem_paging_histogram_t *hist,
			    uint32_t आवृत्तयः);
#यदि_अन्तः /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_THREAD
पूर्णाङ्क z_thread_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats);
पूर्णाङ्क z_thread_stats_query(संरचना k_obj_core *obj_core, शून्य *stats);
पूर्णाङ्क z_thread_stats_reset(संरचना k_obj_core *obj_core);
पूर्णाङ्क z_thread_stats_disable(संरचना k_obj_core *obj_core);
पूर्णाङ्क z_thread_stats_enable(संरचना k_obj_core *obj_core);
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_THREAD */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णाङ्क z_cpu_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats);
पूर्णाङ्क z_cpu_stats_query(संरचना k_obj_core *obj_core, शून्य *stats);

पूर्णाङ्क z_kernel_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats);
पूर्णाङ्क z_kernel_stats_query(संरचना k_obj_core *obj_core, शून्य *stats);
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#यदि defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
शून्य k_thread_abort_cleanup(संरचना k_thread *thread);

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
शून्य k_thread_abort_cleanup_check_reuse(संरचना k_thread *thread);
#यदि_अन्तः /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः

#यदि_अन्तः /* _ASMLANGUAGE */

#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
