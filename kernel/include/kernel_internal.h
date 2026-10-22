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

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kernel_arch_interface.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <string.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
બાહ્ય "C" {
#endif

/* Initialize per-CPU kernel data */
શૂન્ય z_init_cpu(પૂર્ણાંક id);

/* Initialize a thread */
શૂન્ય z_init_thread_base(સંરચના _thread_base *thread_base, પૂર્ણાંક priority,
			uint32_t initial_state, unsigned પૂર્ણાંક options);


FUNC_NORETURN શૂન્ય z_cstart(શૂન્ય);

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
સંરચના k_kernel_init_pre_entry {
	શૂન્ય (*init_fn)(શૂન્ય);
};

સંરચના k_kernel_init_post_entry {
	શૂન્ય (*init_fn)(શૂન્ય);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#define K_KERNEL_INIT_PRE(_fn)                                                 \
	સ્થિર અચલ STRUCT_SECTION_ITERABLE(k_kernel_init_pre_entry,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#define K_KERNEL_INIT_POST(_fn)                                                \
	સ્થિર અચલ STRUCT_SECTION_ITERABLE(k_kernel_init_post_entry,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

બાહ્ય FUNC_NORETURN શૂન્ય z_thread_entry(k_thread_entry_t entry,
			  શૂન્ય *p1, શૂન્ય *p2, શૂન્ય *p3);

બાહ્ય અક્ષર *z_setup_new_thread(સંરચના k_thread *new_thread,
				k_thread_ઢગલો_t *ઢગલો, size_t ઢગલો_size,
				k_thread_entry_t entry,
				શૂન્ય *p1, શૂન્ય *p2, શૂન્ય *p3,
				પૂર્ણાંક prio, uint32_t options, અચલ અક્ષર *name);

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
શૂન્ય *z_thread_aligned_alloc(size_t align, size_t size);

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
શૂન્ય *z_thread_malloc(size_t size);


#ifdef CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
સ્થિર ALWAYS_INLINE શૂન્ય
arch_thread_return_કિંમત_set(સંરચના k_thread *thread, unsigned પૂર્ણાંક કિંમત)
{
	thread->swap_retval = કિંમત;
}
#endif

સ્થિર ALWAYS_INLINE શૂન્ય
z_thread_return_કિંમત_set_with_data(સંરચના k_thread *thread,
				   unsigned પૂર્ણાંક કિંમત,
				   શૂન્ય *data)
{
	arch_thread_return_કિંમત_set(thread, કિંમત);
	thread->base.swap_data = data;
}

#ifdef CONFIG_SMP
બાહ્ય શૂન્ય z_smp_init(શૂન્ય);
#ifdef CONFIG_SYS_CLOCK_EXISTS
બાહ્ય શૂન્ય smp_timer_init(શૂન્ય);
#endif /* CONFIG_SYS_CLOCK_EXISTS */
#endif /* CONFIG_SMP */

બાહ્ય શૂન્ય z_early_rand_get(uint8_t *buf, size_t length);

#જો defined(CONFIG_ઢગલો_POINTER_RANDOM) && (CONFIG_ઢગલો_POINTER_RANDOM != 0)
બાહ્ય પૂર્ણાંક z_ઢગલો_adjust_initialized;
#endif /* CONFIG_STACK_POINTER_RANDOM */

#ifdef CONFIG_MULTITHREADING
બાહ્ય સંરચના k_thread z_main_thread;
#endif /* CONFIG_MULTITHREADING */


K_KERNEL_ઢગલો_ARRAY_DECLARE(z_interrupt_stacks, CONFIG_MP_MAX_અંક_CPUS,
				    CONFIG_ISR_ઢગલો_SIZE);
K_THREAD_ઢગલો_DECLARE(z_main_ઢગલો, CONFIG_MAIN_ઢગલો_SIZE);

#ifdef CONFIG_GEN_PRIV_STACKS
બાહ્ય uint8_t *z_priv_ઢગલો_find(k_thread_ઢગલો_t *ઢગલો);
#endif /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
પૂર્ણાંક z_ઢગલો_space_get(અચલ uint8_t *ઢગલો_start, size_t size, size_t *unused_ptr);

#ifdef CONFIG_USERSPACE
bool z_ઢગલો_is_user_capable(k_thread_ઢગલો_t *ઢગલો);

/* Memory domain setup hook, called from z_setup_new_thread() */
શૂન્ય z_mem_domain_init_thread(સંરચના k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
શૂન્ય z_mem_domain_exit_thread(સંરચના k_thread *thread);

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
બાહ્ય સંરચના k_spinlock z_mem_domain_lock;
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_GDBSTUB
સંરચના gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
બાહ્ય પૂર્ણાંક z_gdb_main_loop(સંરચના gdb_ctx *ctx);
#endif /* CONFIG_GDBSTUB */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
શૂન્ય z_thread_mark_switched_in(શૂન્ય);
શૂન્ય z_thread_mark_switched_out(શૂન્ય);
#નહીંતર

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
શૂન્ય z_mem_manage_init(શૂન્ય);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
શૂન્ય z_mem_manage_boot_finish(શૂન્ય);


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
શૂન્ય z_paging_histogram_init(શૂન્ય);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
શૂન્ય z_paging_histogram_inc(સંરચના k_mem_paging_histogram_t *hist,
			    uint32_t cycles);
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
પૂર્ણાંક z_thread_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats);
પૂર્ણાંક z_thread_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats);
પૂર્ણાંક z_thread_stats_reset(સંરચના k_obj_core *obj_core);
પૂર્ણાંક z_thread_stats_disable(સંરચના k_obj_core *obj_core);
પૂર્ણાંક z_thread_stats_enable(સંરચના k_obj_core *obj_core);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
પૂર્ણાંક z_cpu_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats);
પૂર્ણાંક z_cpu_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats);

પૂર્ણાંક z_kernel_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats);
પૂર્ણાંક z_kernel_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#જો defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
શૂન્ય k_thread_abort_cleanup(સંરચના k_thread *thread);

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
શૂન્ય k_thread_abort_cleanup_check_reuse(સંરચના k_thread *thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
