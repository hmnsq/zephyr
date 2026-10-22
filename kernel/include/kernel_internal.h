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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_KERNEL_ਅੰਦਰੂਨੀ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_KERNEL_ਅੰਦਰੂਨੀ_H

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_arch_interface.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _ASMLANGUAGE

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

/* Initialize per-CPU kernel data */
ਖਾਲੀ z_init_cpu(ਪੂਰਨ id);

/* Initialize a thread */
ਖਾਲੀ z_init_thread_ਅਧਾਰ(ਬਣਤਰ thread_ਅਧਾਰ *thread_ਅਧਾਰ_2, ਪੂਰਨ ਤਰਜੀਹ,
			uint32_t initial_state, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ options);


FUNC_NORETURN ਖਾਲੀ z_cstart(ਖਾਲੀ);

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
ਬਣਤਰ k_kernel_init_pre_ਇੰਦਰਾਜ {
	ਖਾਲੀ (*init_fn)(ਖਾਲੀ);
};

ਬਣਤਰ k_kernel_init_post_ਇੰਦਰਾਜ {
	ਖਾਲੀ (*init_fn)(ਖਾਲੀ);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_KERNEL_INIT_PRE(_fn)                                                 \
	ਸਥਿਰ ਅਚਲ STRUCT_SECTION_ITERABLE(k_kernel_init_pre_ਇੰਦਰਾਜ,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ K_KERNEL_INIT_POST(_fn)                                                \
	ਸਥਿਰ ਅਚਲ STRUCT_SECTION_ITERABLE(k_kernel_init_post_ਇੰਦਰਾਜ,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

ਬਾਹਰੀ FUNC_NORETURN ਖਾਲੀ z_thread_ਇੰਦਰਾਜ(k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ,
			  ਖਾਲੀ *p1, ਖਾਲੀ *p2, ਖਾਲੀ *p3);

ਬਾਹਰੀ ਅੱਖਰ *z_ਤਿਆਰੀ_new_thread(ਬਣਤਰ k_thread *new_thread,
				k_thread_ਢੇਰ_t *ਢੇਰ, size_t ਢੇਰ_ਮਾਪ,
				k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ,
				ਖਾਲੀ *p1, ਖਾਲੀ *p2, ਖਾਲੀ *p3,
				ਪੂਰਨ prio, uint32_t options, ਅਚਲ ਅੱਖਰ *name);

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
ਖਾਲੀ *z_thread_aligned_alloc(size_t align, size_t ਮਾਪ);

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
ਖਾਲੀ *z_thread_malloc(size_t ਮਾਪ);


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ
arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(ਬਣਤਰ k_thread *thread, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮੁੱਲ)
{
	thread->ਅਦਲਾ_ਬਦਲੀ_retval = ਮੁੱਲ;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ
z_thread_return_ਮੁੱਲ_ਸੈੱਟ_with_ਅੰਕੜੇ(ਬਣਤਰ k_thread *thread,
				   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮੁੱਲ,
				   ਖਾਲੀ *ਅੰਕੜੇ)
{
	arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, ਮੁੱਲ);
	thread->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ = ਅੰਕੜੇ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
ਬਾਹਰੀ ਖਾਲੀ z_smp_init(ਖਾਲੀ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CLOCK_EXISTS
ਬਾਹਰੀ ਖਾਲੀ smp_timer_init(ਖਾਲੀ);
#ਜੇ_ਅੰਤ /* CONFIG_SYS_CLOCK_EXISTS */
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

ਬਾਹਰੀ ਖਾਲੀ z_early_rand_get(uint8_t *buf, size_t ਲੰਮਾਈ);

#ਜੇ defined(CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM) && (CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM != 0)
ਬਾਹਰੀ ਪੂਰਨ z_ਢੇਰ_adjust_initialized;
#ਜੇ_ਅੰਤ /* CONFIG_STACK_POINTER_RANDOM */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MULTITHREADING
ਬਾਹਰੀ ਬਣਤਰ k_thread z_main_thread;
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */


K_KERNEL_ਢੇਰ_ARRAY_DECLARE(z_ਰੁਕਾਵਟ_stacks, CONFIG_MP_MAX_ਅੰਕ_CPUS,
				    CONFIG_ISR_ਢੇਰ_ਮਾਪ);
K_THREAD_ਢੇਰ_DECLARE(z_main_ਢੇਰ, CONFIG_MAIN_ਢੇਰ_ਮਾਪ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_GEN_PRIV_STACKS
ਬਾਹਰੀ uint8_t *z_priv_ਢੇਰ_ਲੱਭੋ(k_thread_ਢੇਰ_t *ਢੇਰ);
#ਜੇ_ਅੰਤ /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
ਪੂਰਨ z_ਢੇਰ_space_get(ਅਚਲ uint8_t *ਢੇਰ_ਸ਼ੁਰੂ, size_t ਮਾਪ, size_t *ਅਣਵਰਤਿਆ_ptr);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
bool z_ਢੇਰ_is_user_capable(k_thread_ਢੇਰ_t *ਢੇਰ);

/* Memory domain setup hook, called from z_setup_new_thread() */
ਖਾਲੀ z_mem_domain_init_thread(ਬਣਤਰ k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
ਖਾਲੀ z_mem_domain_exit_thread(ਬਣਤਰ k_thread *thread);

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
ਬਾਹਰੀ ਬਣਤਰ k_spinlock z_mem_domain_ਤਾਲਾ;
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_GDBSTUB
ਬਣਤਰ gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
ਬਾਹਰੀ ਪੂਰਨ z_gdb_main_loop(ਬਣਤਰ gdb_ctx *ctx);
#ਜੇ_ਅੰਤ /* CONFIG_GDBSTUB */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_INSTRUMENT_THREAD_SWITCHING
ਖਾਲੀ z_thread_mark_switched_ਵਿੱਚ(ਖਾਲੀ);
ਖਾਲੀ z_thread_mark_switched_ਬਾਹਰਲਾ(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ

/**
 * @brief Called after a thread has been selected to run
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_thread_mark_switched_ਵਿੱਚ()

/**
 * @brief Called before a thread has been selected to run
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_thread_mark_switched_ਬਾਹਰਲਾ()

#ਜੇ_ਅੰਤ /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

/* Init hook for page frame management, invoked immediately upon entry of
 * main thread, before POST_KERNEL tasks
 */
ਖਾਲੀ z_mem_manage_init(ਖਾਲੀ);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
ਖਾਲੀ z_mem_manage_boot_ਅੰਤ(ਖਾਲੀ);


bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PM

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

#ਜੇ_ਅੰਤ /* CONFIG_PM */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
/**
 * Initialize the timing histograms for demand paging.
 */
ਖਾਲੀ z_paging_histogram_init(ਖਾਲੀ);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
ਖਾਲੀ z_paging_histogram_inc(ਬਣਤਰ k_mem_paging_histogram_t *hist,
			    uint32_t ਚੱਕਰਾਂ);
#ਜੇ_ਅੰਤ /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_THREAD
ਪੂਰਨ z_thread_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats);
ਪੂਰਨ z_thread_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats);
ਪੂਰਨ z_thread_stats_reset(ਬਣਤਰ k_obj_core *obj_core);
ਪੂਰਨ z_thread_stats_disable(ਬਣਤਰ k_obj_core *obj_core);
ਪੂਰਨ z_thread_stats_enable(ਬਣਤਰ k_obj_core *obj_core);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_THREAD */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
ਪੂਰਨ z_cpu_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats);
ਪੂਰਨ z_cpu_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats);

ਪੂਰਨ z_kernel_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats);
ਪੂਰਨ z_kernel_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ਜੇ defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
ਖਾਲੀ k_thread_abort_cleanup(ਬਣਤਰ k_thread *thread);

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
ਖਾਲੀ k_thread_abort_cleanup_check_reuse(ਬਣਤਰ k_thread *thread);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* _ASMLANGUAGE */

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
