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

#ifndef ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_
#define ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_arch_interface.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
ಬಾಹ್ಯ "C" {
#endif

/* Initialize per-CPU kernel data */
ಶೂನ್ಯ z_init_cpu(ಪೂರ್ಣಾಙ್ಕ id);

/* Initialize a thread */
ಶೂನ್ಯ z_init_thread_base(ಸಂರಚನಾ _thread_base *thread_base, ಪೂರ್ಣಾಙ್ಕ priority,
			uint32_t initial_state, unsigned ಪೂರ್ಣಾಙ್ಕ options);


FUNC_NORETURN ಶೂನ್ಯ z_cstart(ಶೂನ್ಯ);

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
ಸಂರಚನಾ k_kernel_init_pre_entry {
	ಶೂನ್ಯ (*init_fn)(ಶೂನ್ಯ);
};

ಸಂರಚನಾ k_kernel_init_post_entry {
	ಶೂನ್ಯ (*init_fn)(ಶೂನ್ಯ);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#define K_KERNEL_INIT_PRE(_fn)                                                 \
	ಸ್ಥಿರ ಧ್ರುವ STRUCT_SECTION_ITERABLE(k_kernel_init_pre_entry,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#define K_KERNEL_INIT_POST(_fn)                                                \
	ಸ್ಥಿರ ಧ್ರುವ STRUCT_SECTION_ITERABLE(k_kernel_init_post_entry,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

ಬಾಹ್ಯ FUNC_NORETURN ಶೂನ್ಯ z_thread_entry(k_thread_entry_t entry,
			  ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3);

ಬಾಹ್ಯ ವರ್ಣ *z_setup_new_thread(ಸಂರಚನಾ k_thread *new_thread,
				k_thread_ರಾಶಿ_t *ರಾಶಿ, size_t ರಾಶಿ_size,
				k_thread_entry_t entry,
				ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3,
				ಪೂರ್ಣಾಙ್ಕ prio, uint32_t options, ಧ್ರುವ ವರ್ಣ *name);

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
ಶೂನ್ಯ *z_thread_aligned_alloc(size_t align, size_t size);

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
ಶೂನ್ಯ *z_thread_malloc(size_t size);


#ifdef CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ
arch_thread_return_ಮೌಲ್ಯ_set(ಸಂರಚನಾ k_thread *thread, unsigned ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ)
{
	thread->swap_retval = ಮೌಲ್ಯ;
}
#endif

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ
z_thread_return_ಮೌಲ್ಯ_set_with_data(ಸಂರಚನಾ k_thread *thread,
				   unsigned ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ,
				   ಶೂನ್ಯ *data)
{
	arch_thread_return_ಮೌಲ್ಯ_set(thread, ಮೌಲ್ಯ);
	thread->base.swap_data = data;
}

#ifdef CONFIG_SMP
ಬಾಹ್ಯ ಶೂನ್ಯ z_smp_init(ಶೂನ್ಯ);
#ifdef CONFIG_SYS_CLOCK_EXISTS
ಬಾಹ್ಯ ಶೂನ್ಯ smp_timer_init(ಶೂನ್ಯ);
#endif /* CONFIG_SYS_CLOCK_EXISTS */
#endif /* CONFIG_SMP */

ಬಾಹ್ಯ ಶೂನ್ಯ z_early_rand_get(uint8_t *buf, size_t length);

#ಯದಿ defined(CONFIG_ರಾಶಿ_POINTER_RANDOM) && (CONFIG_ರಾಶಿ_POINTER_RANDOM != 0)
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ z_ರಾಶಿ_adjust_initialized;
#endif /* CONFIG_STACK_POINTER_RANDOM */

#ifdef CONFIG_MULTITHREADING
ಬಾಹ್ಯ ಸಂರಚನಾ k_thread z_main_thread;
#endif /* CONFIG_MULTITHREADING */


K_KERNEL_ರಾಶಿ_ARRAY_DECLARE(z_interrupt_stacks, CONFIG_MP_MAX_ಅಂಕೆ_CPUS,
				    CONFIG_ISR_ರಾಶಿ_SIZE);
K_THREAD_ರಾಶಿ_DECLARE(z_main_ರಾಶಿ, CONFIG_MAIN_ರಾಶಿ_SIZE);

#ifdef CONFIG_GEN_PRIV_STACKS
ಬಾಹ್ಯ uint8_t *z_priv_ರಾಶಿ_find(k_thread_ರಾಶಿ_t *ರಾಶಿ);
#endif /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
ಪೂರ್ಣಾಙ್ಕ z_ರಾಶಿ_space_get(ಧ್ರುವ uint8_t *ರಾಶಿ_start, size_t size, size_t *unused_ptr);

#ifdef CONFIG_USERSPACE
bool z_ರಾಶಿ_is_user_capable(k_thread_ರಾಶಿ_t *ರಾಶಿ);

/* Memory domain setup hook, called from z_setup_new_thread() */
ಶೂನ್ಯ z_mem_domain_init_thread(ಸಂರಚನಾ k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
ಶೂನ್ಯ z_mem_domain_exit_thread(ಸಂರಚನಾ k_thread *thread);

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
ಬಾಹ್ಯ ಸಂರಚನಾ k_spinlock z_mem_domain_lock;
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_GDBSTUB
ಸಂರಚನಾ gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ z_gdb_main_loop(ಸಂರಚನಾ gdb_ctx *ctx);
#endif /* CONFIG_GDBSTUB */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
ಶೂನ್ಯ z_thread_mark_switched_in(ಶೂನ್ಯ);
ಶೂನ್ಯ z_thread_mark_switched_out(ಶೂನ್ಯ);
#ಅನ್ಯಥಾ

/**
 * @brief Called after a thread has been selected to run
 */
#define z_thread_mark_switched_in()

/**
 * @brief Called before a thread has been selected to run
 */

#define z_thread_mark_switched_out()

#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

/* Init hook for page frame management, invoked immediately upon entry of
 * main thread, before POST_KERNEL tasks
 */
ಶೂನ್ಯ z_mem_manage_init(ಶೂನ್ಯ);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
ಶೂನ್ಯ z_mem_manage_boot_finish(ಶೂನ್ಯ);


bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state);

#ifdef CONFIG_PM

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

#endif /* CONFIG_PM */

#ifdef CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
/**
 * Initialize the timing histograms for demand paging.
 */
ಶೂನ್ಯ z_paging_histogram_init(ಶೂನ್ಯ);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ಶೂನ್ಯ z_paging_histogram_inc(ಸಂರಚನಾ k_mem_paging_histogram_t *hist,
			    uint32_t cycles);
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
ಪೂರ್ಣಾಙ್ಕ z_thread_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats);
ಪೂರ್ಣಾಙ್ಕ z_thread_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats);
ಪೂರ್ಣಾಙ್ಕ z_thread_stats_reset(ಸಂರಚನಾ k_obj_core *obj_core);
ಪೂರ್ಣಾಙ್ಕ z_thread_stats_disable(ಸಂರಚನಾ k_obj_core *obj_core);
ಪೂರ್ಣಾಙ್ಕ z_thread_stats_enable(ಸಂರಚನಾ k_obj_core *obj_core);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ಪೂರ್ಣಾಙ್ಕ z_cpu_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats);
ಪೂರ್ಣಾಙ್ಕ z_cpu_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats);

ಪೂರ್ಣಾಙ್ಕ z_kernel_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats);
ಪೂರ್ಣಾಙ್ಕ z_kernel_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ಯದಿ defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
ಶೂನ್ಯ k_thread_abort_cleanup(ಸಂರಚನಾ k_thread *thread);

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
ಶೂನ್ಯ k_thread_abort_cleanup_check_reuse(ಸಂರಚನಾ k_thread *thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
