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

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kernel_arch_interface.h>
#ଅନ୍ତର୍ଭାଵ <kthread.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
ବାହ୍ୟ "C" {
#endif

/* Initialize per-CPU kernel data */
ଶୂନ୍ୟ z_init_cpu(ପୂର୍ଣ୍ଣସଂଖ୍ୟା id);

/* Initialize a thread */
ଶୂନ୍ୟ z_init_thread_base(ସଂରଚନା _thread_base *thread_base, ପୂର୍ଣ୍ଣସଂଖ୍ୟା priority,
			uint32_t initial_state, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା options);


FUNC_NORETURN ଶୂନ୍ୟ z_cstart(ଶୂନ୍ୟ);

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
ସଂରଚନା k_kernel_init_pre_entry {
	ଶୂନ୍ୟ (*init_fn)(ଶୂନ୍ୟ);
};

ସଂରଚନା k_kernel_init_post_entry {
	ଶୂନ୍ୟ (*init_fn)(ଶୂନ୍ୟ);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#define K_KERNEL_INIT_PRE(_fn)                                                 \
	ସ୍ଥିର ଅଚଳ STRUCT_SECTION_ITERABLE(k_kernel_init_pre_entry,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#define K_KERNEL_INIT_POST(_fn)                                                \
	ସ୍ଥିର ଅଚଳ STRUCT_SECTION_ITERABLE(k_kernel_init_post_entry,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

ବାହ୍ୟ FUNC_NORETURN ଶୂନ୍ୟ z_thread_entry(k_thread_entry_t entry,
			  ଶୂନ୍ୟ *p1, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3);

ବାହ୍ୟ ଅକ୍ଷର *z_setup_new_thread(ସଂରଚନା k_thread *new_thread,
				k_thread_ଗଦା_t *ଗଦା, size_t ଗଦା_size,
				k_thread_entry_t entry,
				ଶୂନ୍ୟ *p1, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3,
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio, uint32_t options, ଅଚଳ ଅକ୍ଷର *name);

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
ଶୂନ୍ୟ *z_thread_aligned_alloc(size_t align, size_t size);

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
ଶୂନ୍ୟ *z_thread_malloc(size_t size);


#ifdef CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ
arch_thread_return_ମୂଲ୍ୟ_set(ସଂରଚନା k_thread *thread, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ)
{
	thread->swap_retval = ମୂଲ୍ୟ;
}
#endif

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ
z_thread_return_ମୂଲ୍ୟ_set_with_data(ସଂରଚନା k_thread *thread,
				   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ,
				   ଶୂନ୍ୟ *data)
{
	arch_thread_return_ମୂଲ୍ୟ_set(thread, ମୂଲ୍ୟ);
	thread->base.swap_data = data;
}

#ifdef CONFIG_SMP
ବାହ୍ୟ ଶୂନ୍ୟ z_smp_init(ଶୂନ୍ୟ);
#ifdef CONFIG_SYS_CLOCK_EXISTS
ବାହ୍ୟ ଶୂନ୍ୟ smp_timer_init(ଶୂନ୍ୟ);
#endif /* CONFIG_SYS_CLOCK_EXISTS */
#endif /* CONFIG_SMP */

ବାହ୍ୟ ଶୂନ୍ୟ z_early_rand_get(uint8_t *buf, size_t length);

#ଯଦି defined(CONFIG_ଗଦା_POINTER_RANDOM) && (CONFIG_ଗଦା_POINTER_RANDOM != 0)
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_ଗଦା_adjust_initialized;
#endif /* CONFIG_STACK_POINTER_RANDOM */

#ifdef CONFIG_MULTITHREADING
ବାହ୍ୟ ସଂରଚନା k_thread z_main_thread;
#endif /* CONFIG_MULTITHREADING */


K_KERNEL_ଗଦା_ARRAY_DECLARE(z_interrupt_stacks, CONFIG_MP_MAX_ଅଙ୍କ_CPUS,
				    CONFIG_ISR_ଗଦା_SIZE);
K_THREAD_ଗଦା_DECLARE(z_main_ଗଦା, CONFIG_MAIN_ଗଦା_SIZE);

#ifdef CONFIG_GEN_PRIV_STACKS
ବାହ୍ୟ uint8_t *z_priv_ଗଦା_find(k_thread_ଗଦା_t *ଗଦା);
#endif /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_ଗଦା_space_get(ଅଚଳ uint8_t *ଗଦା_start, size_t size, size_t *unused_ptr);

#ifdef CONFIG_USERSPACE
bool z_ଗଦା_is_user_capable(k_thread_ଗଦା_t *ଗଦା);

/* Memory domain setup hook, called from z_setup_new_thread() */
ଶୂନ୍ୟ z_mem_domain_init_thread(ସଂରଚନା k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
ଶୂନ୍ୟ z_mem_domain_exit_thread(ସଂରଚନା k_thread *thread);

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
ବାହ୍ୟ ସଂରଚନା k_spinlock z_mem_domain_lock;
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_GDBSTUB
ସଂରଚନା gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_gdb_main_loop(ସଂରଚନା gdb_ctx *ctx);
#endif /* CONFIG_GDBSTUB */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
ଶୂନ୍ୟ z_thread_mark_switched_in(ଶୂନ୍ୟ);
ଶୂନ୍ୟ z_thread_mark_switched_out(ଶୂନ୍ୟ);
#ଅନ୍ୟଥା

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
ଶୂନ୍ୟ z_mem_manage_init(ଶୂନ୍ୟ);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
ଶୂନ୍ୟ z_mem_manage_boot_finish(ଶୂନ୍ୟ);


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
ଶୂନ୍ୟ z_paging_histogram_init(ଶୂନ୍ୟ);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ଶୂନ୍ୟ z_paging_histogram_inc(ସଂରଚନା k_mem_paging_histogram_t *hist,
			    uint32_t cycles);
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_reset(ସଂରଚନା k_obj_core *obj_core);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_disable(ସଂରଚନା k_obj_core *obj_core);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_enable(ସଂରଚନା k_obj_core *obj_core);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_cpu_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_cpu_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats);

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_kernel_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_kernel_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ଯଦି defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
ଶୂନ୍ୟ k_thread_abort_cleanup(ସଂରଚନା k_thread *thread);

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
ଶୂନ୍ୟ k_thread_abort_cleanup_check_reuse(ସଂରଚନା k_thread *thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
