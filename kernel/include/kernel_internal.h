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

#اگر_تعریف_نشده ZEPHYR_KERNEL_INCLUDE_KERNEL_داخلی_H
#تعریف_کن ZEPHYR_KERNEL_INCLUDE_KERNEL_داخلی_H

#ضمن <zephyr/kernel.h>
#ضمن <kernel_arch_interface.h>
#ضمن <kthread.h>
#ضمن <string.h>

#اگر_تعریف_نشده _ASMLANGUAGE

#اگر_تعریف_شده __cplusplus
خارجي "C" {
#پایان_اگر

/* Initialize per-CPU kernel data */
فراغ z_init_cpu(صحيح id);

/* Initialize a thread */
فراغ z_init_thread_پایه(بنية thread_پایه *thread_پایه_2, صحيح اولویت,
			uint32_t initial_state, بیعلامت صحيح options);


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
بنية k_kernel_init_pre_مدخل {
	فراغ (*init_fn)(فراغ);
};

بنية k_kernel_init_post_مدخل {
	فراغ (*init_fn)(فراغ);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#تعریف_کن K_KERNEL_INIT_PRE(_fn)                                                 \
	ساكن ثابت STRUCT_SECTION_ITERABLE(k_kernel_init_pre_مدخل,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#تعریف_کن K_KERNEL_INIT_POST(_fn)                                                \
	ساكن ثابت STRUCT_SECTION_ITERABLE(k_kernel_init_post_مدخل,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

خارجي FUNC_NORETURN فراغ z_thread_مدخل(k_thread_مدخل_t مدخل,
			  فراغ *p1, فراغ *p2, فراغ *p3);

خارجي حرف *z_راهاندازی_new_thread(بنية k_thread *new_thread,
				k_thread_پشته_t *پشته, size_t پشته_اندازه,
				k_thread_مدخل_t مدخل,
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


#اگر_تعریف_شده CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
ساكن ALWAYS_INLINE فراغ
arch_thread_return_مقدار_مجموعه(بنية k_thread *thread, بیعلامت صحيح مقدار)
{
	thread->جابجا_retval = مقدار;
}
#پایان_اگر

ساكن ALWAYS_INLINE فراغ
z_thread_return_مقدار_مجموعه_with_داده(بنية k_thread *thread,
				   بیعلامت صحيح مقدار,
				   فراغ *داده)
{
	arch_thread_return_مقدار_مجموعه(thread, مقدار);
	thread->پایه.جابجا_داده = داده;
}

#اگر_تعریف_شده CONFIG_SMP
خارجي فراغ z_smp_init(فراغ);
#اگر_تعریف_شده CONFIG_SYS_CLOCK_EXISTS
خارجي فراغ smp_timer_init(فراغ);
#پایان_اگر /* CONFIG_SYS_CLOCK_EXISTS */
#پایان_اگر /* CONFIG_SMP */

خارجي فراغ z_early_rand_get(uint8_t *buf, size_t درازی);

#إذا defined(CONFIG_پشته_اشاره_گر_RANDOM) && (CONFIG_پشته_اشاره_گر_RANDOM != 0)
خارجي صحيح z_پشته_adjust_initialized;
#پایان_اگر /* CONFIG_STACK_POINTER_RANDOM */

#اگر_تعریف_شده CONFIG_MULTITHREADING
خارجي بنية k_thread z_main_thread;
#پایان_اگر /* CONFIG_MULTITHREADING */


K_KERNEL_پشته_ARRAY_DECLARE(z_وقفه_stacks, CONFIG_MP_MAX_رقم_CPUS,
				    CONFIG_ISR_پشته_اندازه);
K_THREAD_پشته_DECLARE(z_main_پشته, CONFIG_MAIN_پشته_اندازه);

#اگر_تعریف_شده CONFIG_GEN_PRIV_STACKS
خارجي uint8_t *z_priv_پشته_پیدا_کن(k_thread_پشته_t *پشته);
#پایان_اگر /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
صحيح z_پشته_space_get(ثابت uint8_t *پشته_شروع, size_t اندازه, size_t *استفادهنشده_ptr);

#اگر_تعریف_شده CONFIG_USERSPACE
bool z_پشته_is_user_capable(k_thread_پشته_t *پشته);

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
خارجي بنية k_spinlock z_mem_domain_قفل;
#پایان_اگر /* CONFIG_USERSPACE */

#اگر_تعریف_شده CONFIG_GDBSTUB
بنية gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
خارجي صحيح z_gdb_main_loop(بنية gdb_ctx *ctx);
#پایان_اگر /* CONFIG_GDBSTUB */

#اگر_تعریف_شده CONFIG_INSTRUMENT_THREAD_SWITCHING
فراغ z_thread_mark_switched_درون(فراغ);
فراغ z_thread_mark_switched_بیرون(فراغ);
#وإلا

/**
 * @brief Called after a thread has been selected to run
 */
#تعریف_کن z_thread_mark_switched_درون()

/**
 * @brief Called before a thread has been selected to run
 */

#تعریف_کن z_thread_mark_switched_بیرون()

#پایان_اگر /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

/* Init hook for page frame management, invoked immediately upon entry of
 * main thread, before POST_KERNEL tasks
 */
فراغ z_mem_manage_init(فراغ);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
فراغ z_mem_manage_boot_پایان(فراغ);


bool z_handle_obj_poll_events(sys_dlist_t *events, uint32_t state);

#اگر_تعریف_شده CONFIG_PM

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

#پایان_اگر /* CONFIG_PM */

#اگر_تعریف_شده CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM
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
			    uint32_t چرخهها);
#پایان_اگر /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_THREAD
صحيح z_thread_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_thread_stats_query(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_thread_stats_reset(بنية k_obj_core *obj_core);
صحيح z_thread_stats_disable(بنية k_obj_core *obj_core);
صحيح z_thread_stats_enable(بنية k_obj_core *obj_core);
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_THREAD */

#اگر_تعریف_شده CONFIG_OBJ_CORE_STATS_SYSTEM
صحيح z_cpu_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_cpu_stats_query(بنية k_obj_core *obj_core, فراغ *stats);

صحيح z_kernel_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_kernel_stats_query(بنية k_obj_core *obj_core, فراغ *stats);
#پایان_اگر /* CONFIG_OBJ_CORE_STATS_SYSTEM */

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
#پایان_اگر /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#اگر_تعریف_شده __cplusplus
}
#پایان_اگر

#پایان_اگر /* _ASMLANGUAGE */

#پایان_اگر /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
