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

#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_KERNEL_دننی_H
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_KERNEL_دننی_H

#ضمن <zephyr/kernel.h>
#ضمن <kernel_arch_interface.h>
#ضمن <kthread.h>
#ضمن <string.h>

#که_نه_تعریف_شوی _ASMLANGUAGE

#که_تعریف_شوی __cplusplus
خارجي "C" {
#د_که_پای

/* Initialize per-CPU kernel data */
فراغ z_init_cpu(صحيح id);

/* Initialize a thread */
فراغ z_init_thread_بنسټ(بنية thread_بنسټ *thread_بنسټ_2, صحيح لومړیتوب,
			uint32_t initial_state, بینښه صحيح options);


FUNC_NORETURN فراغ z_cstart(فراغ);

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
بنية k_kernel_init_pre_ثبت {
	فراغ (*init_fn)(فراغ);
};

بنية k_kernel_init_post_ثبت {
	فراغ (*init_fn)(فراغ);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#تعریف_کړه K_KERNEL_INIT_PRE(_fn)                                                 \
	ساكن ثابت STRUCT_SECTION_ITERABLE(k_kernel_init_pre_ثبت,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#تعریف_کړه K_KERNEL_INIT_POST(_fn)                                                \
	ساكن ثابت STRUCT_SECTION_ITERABLE(k_kernel_init_post_ثبت,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

خارجي FUNC_NORETURN فراغ z_thread_ثبت(k_thread_ثبت_t ثبت,
			  فراغ *p1, فراغ *p2, فراغ *p3);

خارجي حرف *z_تیاری_new_thread(بنية k_thread *new_thread,
				k_thread_ډېری_t *ډېری, size_t ډېری_اندازه,
				k_thread_ثبت_t ثبت,
				فراغ *p1, فراغ *p2, فراغ *p3,
				صحيح prio, uint32_t options, ثابت حرف *name);

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
فراغ *z_thread_aligned_alloc(size_t align, size_t اندازه);

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
فراغ *z_thread_malloc(size_t اندازه);


#که_تعریف_شوی CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
ساكن ALWAYS_INLINE فراغ
arch_thread_return_ارزښت_سېټ(بنية k_thread *thread, بینښه صحيح ارزښت)
{
	thread->بدلون_retval = ارزښت;
}
#د_که_پای

ساكن ALWAYS_INLINE فراغ
z_thread_return_ارزښت_سېټ_with_معلومات(بنية k_thread *thread,
				   بینښه صحيح ارزښت,
				   فراغ *معلومات)
{
	arch_thread_return_ارزښت_سېټ(thread, ارزښت);
	thread->بنسټ.بدلون_معلومات = معلومات;
}

#که_تعریف_شوی CONFIG_SMP
خارجي فراغ z_smp_init(فراغ);
#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS
خارجي فراغ smp_timer_init(فراغ);
#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */
#د_که_پای /* CONFIG_SMP */

خارجي فراغ z_early_rand_get(uint8_t *buf, size_t اوږدتیا);

#إذا defined(CONFIG_ډېری_اشاره_ګر_RANDOM) && (CONFIG_ډېری_اشاره_ګر_RANDOM != 0)
خارجي صحيح z_ډېری_adjust_initialized;
#د_که_پای /* CONFIG_STACK_POINTER_RANDOM */

#که_تعریف_شوی CONFIG_MULTITHREADING
خارجي بنية k_thread z_main_thread;
#د_که_پای /* CONFIG_MULTITHREADING */


K_KERNEL_ډېری_ARRAY_DECLARE(z_مداخله_stacks, CONFIG_MP_MAX_عدد_CPUS,
				    CONFIG_ISR_ډېری_اندازه);
K_THREAD_ډېری_DECLARE(z_main_ډېری, CONFIG_MAIN_ډېری_اندازه);

#که_تعریف_شوی CONFIG_GEN_PRIV_STACKS
خارجي uint8_t *z_priv_ډېری_پیدا_کړه(k_thread_ډېری_t *ډېری);
#د_که_پای /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
صحيح z_ډېری_space_get(ثابت uint8_t *ډېری_پیل, size_t اندازه, size_t *ناکارول_شوی_ptr);

#که_تعریف_شوی CONFIG_USERSPACE
bool z_ډېری_is_user_capable(k_thread_ډېری_t *ډېری);

/* Memory domain setup hook, called from z_setup_new_thread() */
فراغ z_mem_domain_init_thread(بنية k_thread *thread);

/* Memory domain teardown hook, called from z_thread_abort() */
فراغ z_mem_domain_exit_thread(بنية k_thread *thread);

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
خارجي بنية k_spinlock z_mem_domain_قلف;
#د_که_پای /* CONFIG_USERSPACE */

#که_تعریف_شوی CONFIG_GDBSTUB
بنية gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
خارجي صحيح z_gdb_main_loop(بنية gdb_ctx *ctx);
#د_که_پای /* CONFIG_GDBSTUB */

#که_تعریف_شوی CONFIG_INSTRUMENT_THREAD_SWITCHING
فراغ z_thread_mark_switched_دننه(فراغ);
فراغ z_thread_mark_switched_بهر(فراغ);
#وإلا

/**
 * @brief Called after a thread has been selected to run
 */
#تعریف_کړه z_thread_mark_switched_دننه()

/**
 * @brief Called before a thread has been selected to run
 */

#تعریف_کړه z_thread_mark_switched_بهر()

#د_که_پای /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

/* Init hook for page frame management, invoked immediately upon entry of
 * main thread, before POST_KERNEL tasks
 */
فراغ z_mem_manage_init(فراغ);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
فراغ z_mem_manage_boot_پای(فراغ);


bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state);

#که_تعریف_شوی CONFIG_PM

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

#د_که_پای /* CONFIG_PM */

#که_تعریف_شوی CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
/**
 * Initialize the timing histograms for demand paging.
 */
فراغ z_paging_histogram_init(فراغ);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
فراغ z_paging_histogram_inc(بنية k_mem_paging_histogram_t *hist,
			    uint32_t دورې);
#د_که_پای /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_THREAD
صحيح z_thread_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_thread_stats_query(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_thread_stats_reset(بنية k_obj_core *obj_core);
صحيح z_thread_stats_disable(بنية k_obj_core *obj_core);
صحيح z_thread_stats_enable(بنية k_obj_core *obj_core);
#د_که_پای /* CONFIG_OBJ_CORE_STATS_THREAD */

#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_SYSTEM
صحيح z_cpu_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_cpu_stats_query(بنية k_obj_core *obj_core, فراغ *stats);

صحيح z_kernel_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_kernel_stats_query(بنية k_obj_core *obj_core, فراغ *stats);
#د_که_پای /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#إذا defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
فراغ k_thread_abort_cleanup(بنية k_thread *thread);

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
فراغ k_thread_abort_cleanup_check_reuse(بنية k_thread *thread);
#د_که_پای /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#که_تعریف_شوی __cplusplus
}
#د_که_پای

#د_که_پای /* _ASMLANGUAGE */

#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
