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

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_

#समावेश <zephyr/kernel.h>
#समावेश <kernel_arch_interface.h>
#समावेश <kthread.h>
#समावेश <string.h>

#अगर_अघोषित _ASMLANGUAGE

#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर

/* Initialize per-CPU kernel data */
व्योम z_init_cpu(पूर्णांक id);

/* Initialize a thread */
व्योम z_init_thread_base(काष्ठा _thread_base *thread_base, पूर्णांक priority,
			uint32_t initial_state, अचिन्हित पूर्णांक options);


FUNC_NORETURN व्योम z_cstart(व्योम);

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
काष्ठा k_kernel_init_pre_entry {
	व्योम (*init_fn)(व्योम);
};

काष्ठा k_kernel_init_post_entry {
	व्योम (*init_fn)(व्योम);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#घोषणा K_KERNEL_INIT_PRE(_fn)                                                 \
	अटल स्थिर STRUCT_SECTION_ITERABLE(k_kernel_init_pre_entry,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#घोषणा K_KERNEL_INIT_POST(_fn)                                                \
	अटल स्थिर STRUCT_SECTION_ITERABLE(k_kernel_init_post_entry,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

बाह्य FUNC_NORETURN व्योम z_thread_entry(k_thread_entry_t entry,
			  व्योम *p1, व्योम *p2, व्योम *p3);

बाह्य अक्षर *z_setup_new_thread(काष्ठा k_thread *new_thread,
				k_thread_ढेर_t *ढेर, माप_प्रकार ढेर_आकार,
				k_thread_entry_t entry,
				व्योम *p1, व्योम *p2, व्योम *p3,
				पूर्णांक prio, uint32_t options, स्थिर अक्षर *name);

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
व्योम *z_thread_aligned_alloc(माप_प्रकार align, माप_प्रकार आकार);

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
व्योम *z_thread_malloc(माप_प्रकार आकार);


#अगर_घोषित CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
अटल ALWAYS_INLINE व्योम
arch_thread_return_मूल्य_set(काष्ठा k_thread *thread, अचिन्हित पूर्णांक मूल्य)
{
	thread->swap_retval = मूल्य;
}
#पूर्ण_अगर

अटल ALWAYS_INLINE व्योम
z_thread_return_मूल्य_set_with_तथ्य(काष्ठा k_thread *thread,
				   अचिन्हित पूर्णांक मूल्य,
				   व्योम *तथ्य)
{
	arch_thread_return_मूल्य_set(thread, मूल्य);
	thread->base.swap_तथ्य = तथ्य;
}

#अगर_घोषित CONFIG_SMP
बाह्य व्योम z_smp_init(व्योम);
#अगर_घोषित CONFIG_SYS_CLOCK_EXISTS
बाह्य व्योम smp_timer_init(व्योम);
#पूर्ण_अगर /* CONFIG_SYS_CLOCK_EXISTS */
#पूर्ण_अगर /* CONFIG_SMP */

बाह्य व्योम z_early_rand_get(uint8_t *buf, माप_प्रकार लंबाई);

#अगर defined(CONFIG_ढेर_POINTER_RANDOM) && (CONFIG_ढेर_POINTER_RANDOM != 0)
बाह्य पूर्णांक z_ढेर_adjust_initialized;
#पूर्ण_अगर /* CONFIG_STACK_POINTER_RANDOM */

#अगर_घोषित CONFIG_MULTITHREADING
बाह्य काष्ठा k_thread z_main_thread;
#पूर्ण_अगर /* CONFIG_MULTITHREADING */


K_KERNEL_ढेर_ARRAY_DECLARE(z_interrupt_stacks, CONFIG_MP_MAX_अंक_CPUS,
				    CONFIG_ISR_ढेर_आकार);
K_THREAD_ढेर_DECLARE(z_main_ढेर, CONFIG_MAIN_ढेर_आकार);

#अगर_घोषित CONFIG_GEN_PRIV_STACKS
बाह्य uint8_t *z_priv_ढेर_find(k_thread_ढेर_t *ढेर);
#पूर्ण_अगर /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
पूर्णांक z_ढेर_space_get(स्थिर uint8_t *ढेर_start, माप_प्रकार आकार, माप_प्रकार *unused_ptr);

#अगर_घोषित CONFIG_USERSPACE
bool z_ढेर_is_user_capable(k_thread_ढेर_t *ढेर);

/* Memory domain setup hook, called from z_setup_new_thread() */
व्योम z_mem_domain_init_thread(काष्ठा k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
व्योम z_mem_domain_exit_thread(काष्ठा k_thread *thread);

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
बाह्य काष्ठा k_spinlock z_mem_domain_lock;
#पूर्ण_अगर /* CONFIG_USERSPACE */

#अगर_घोषित CONFIG_GDBSTUB
काष्ठा gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
बाह्य पूर्णांक z_gdb_main_loop(काष्ठा gdb_ctx *ctx);
#पूर्ण_अगर /* CONFIG_GDBSTUB */

#अगर_घोषित CONFIG_INSTRUMENT_THREAD_SWITCHING
व्योम z_thread_mark_switched_in(व्योम);
व्योम z_thread_mark_switched_out(व्योम);
#अन्यथा

/**
 * @brief Called after a thread has been selected to run
 */
#घोषणा z_thread_mark_switched_in()

/**
 * @brief Called before a thread has been selected to run
 */

#घोषणा z_thread_mark_switched_out()

#पूर्ण_अगर /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

/* Init hook for page frame management, invoked immediately upon entry of
 * main thread, before POST_KERNEL tasks
 */
व्योम z_mem_manage_init(व्योम);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
व्योम z_mem_manage_boot_finish(व्योम);


bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state);

#अगर_घोषित CONFIG_PM

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

#पूर्ण_अगर /* CONFIG_PM */

#अगर_घोषित CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
/**
 * Initialize the timing histograms for demand paging.
 */
व्योम z_paging_histogram_init(व्योम);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
व्योम z_paging_histogram_inc(काष्ठा k_mem_paging_histogram_t *hist,
			    uint32_t cycles);
#पूर्ण_अगर /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#अगर_घोषित CONFIG_OBJ_CORE_STATS_THREAD
पूर्णांक z_thread_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats);
पूर्णांक z_thread_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats);
पूर्णांक z_thread_stats_reset(काष्ठा k_obj_core *obj_core);
पूर्णांक z_thread_stats_disable(काष्ठा k_obj_core *obj_core);
पूर्णांक z_thread_stats_enable(काष्ठा k_obj_core *obj_core);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_THREAD */

#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णांक z_cpu_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats);
पूर्णांक z_cpu_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats);

पूर्णांक z_kernel_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats);
पूर्णांक z_kernel_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#अगर defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
व्योम k_thread_abort_cleanup(काष्ठा k_thread *thread);

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
व्योम k_thread_abort_cleanup_check_reuse(काष्ठा k_thread *thread);
#पूर्ण_अगर /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#अगर_घोषित __cplusplus
}
#पूर्ण_अगर

#पूर्ण_अगर /* _ASMLANGUAGE */

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
