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

#ضمن <zephyr/kernel.h>
#ضمن <kernel_arch_interface.h>
#ضمن <kthread.h>
#ضمن <string.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
خارجي "C" {
#endif

/* Initialize per-CPU kernel data */
فراغ z_init_cpu(صحيح id);

/* Initialize a thread */
فراغ z_init_thread_base(بنية _thread_base *thread_base, صحيح priority,
			uint32_t initial_state, unsigned صحيح options);


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
بنية k_kernel_init_pre_entry {
	فراغ (*init_fn)(فراغ);
};

بنية k_kernel_init_post_entry {
	فراغ (*init_fn)(فراغ);
};

/* Register a kernel init function to run before multithreading starts (the
 * PRE_KERNEL phase), ahead of PRE_KERNEL device init.
 */
#define K_KERNEL_INIT_PRE(_fn)                                                 \
	ساكن ثابت STRUCT_SECTION_ITERABLE(k_kernel_init_pre_entry,          \
					     _kernel_init_pre_##_fn) = {       \
		.init_fn = (_fn),                                              \
	}

/* Register a kernel init function to run after the kernel is up (the
 * POST_KERNEL phase), before POST_KERNEL device init.
 */
#define K_KERNEL_INIT_POST(_fn)                                                \
	ساكن ثابت STRUCT_SECTION_ITERABLE(k_kernel_init_post_entry,         \
					     _kernel_init_post_##_fn) = {      \
		.init_fn = (_fn),                                              \
	}

خارجي FUNC_NORETURN فراغ z_thread_entry(k_thread_entry_t entry,
			  فراغ *p1, فراغ *p2, فراغ *p3);

خارجي حرف *z_setup_new_thread(بنية k_thread *new_thread,
				k_thread_stack_t *stack, size_t stack_size,
				k_thread_entry_t entry,
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
فراغ *z_thread_aligned_alloc(size_t align, size_t size);

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
فراغ *z_thread_malloc(size_t size);


#ifdef CONFIG_USE_SWITCH
/* This is a arch function traditionally, but when the switch-based
 * z_swap() is in use it's a simple inline provided by the kernel.
 */
ساكن ALWAYS_INLINE فراغ
arch_thread_return_value_set(بنية k_thread *thread, unsigned صحيح value)
{
	thread->swap_retval = value;
}
#endif

ساكن ALWAYS_INLINE فراغ
z_thread_return_value_set_with_data(بنية k_thread *thread,
				   unsigned صحيح value,
				   فراغ *data)
{
	arch_thread_return_value_set(thread, value);
	thread->base.swap_data = data;
}

#ifdef CONFIG_SMP
خارجي فراغ z_smp_init(فراغ);
#ifdef CONFIG_SYS_CLOCK_EXISTS
خارجي فراغ smp_timer_init(فراغ);
#endif /* CONFIG_SYS_CLOCK_EXISTS */
#endif /* CONFIG_SMP */

خارجي فراغ z_early_rand_get(uint8_t *buf, size_t length);

#אם defined(CONFIG_STACK_POINTER_RANDOM) && (CONFIG_STACK_POINTER_RANDOM != 0)
خارجي صحيح z_stack_adjust_initialized;
#endif /* CONFIG_STACK_POINTER_RANDOM */

#ifdef CONFIG_MULTITHREADING
خارجي بنية k_thread z_main_thread;
#endif /* CONFIG_MULTITHREADING */


K_KERNEL_STACK_ARRAY_DECLARE(z_interrupt_stacks, CONFIG_MP_MAX_NUM_CPUS,
				    CONFIG_ISR_STACK_SIZE);
K_THREAD_STACK_DECLARE(z_main_stack, CONFIG_MAIN_STACK_SIZE);

#ifdef CONFIG_GEN_PRIV_STACKS
خارجي uint8_t *z_priv_stack_find(k_thread_stack_t *stack);
#endif /* CONFIG_GEN_PRIV_STACKS */

/* Calculate stack usage. */
صحيح z_stack_space_get(ثابت uint8_t *stack_start, size_t size, size_t *unused_ptr);

#ifdef CONFIG_USERSPACE
bool z_stack_is_user_capable(k_thread_stack_t *stack);

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
خارجي بنية k_spinlock z_mem_domain_lock;
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_GDBSTUB
بنية gdb_ctx;

/* Should be called by the arch layer. This is the gdbstub main loop
 * and synchronously communicate with gdb on host.
 */
خارجي صحيح z_gdb_main_loop(بنية gdb_ctx *ctx);
#endif /* CONFIG_GDBSTUB */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
فراغ z_thread_mark_switched_in(فراغ);
فراغ z_thread_mark_switched_out(فراغ);
#אחרת

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
فراغ z_mem_manage_init(فراغ);

/**
 * @brief Finalize page frame management at the end of boot process.
 */
فراغ z_mem_manage_boot_finish(فراغ);


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
فراغ z_paging_histogram_init(فراغ);

/**
 * Increment the counter in the timing histogram.
 *
 * @param hist The timing histogram to be updated.
 * @param cycles Time spent in measured operation.
 */
فراغ z_paging_histogram_inc(بنية k_mem_paging_histogram_t *hist,
			    uint32_t cycles);
#endif /* CONFIG_DEMAND_PAGING_TIMING_HISTOGRAM */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
صحيح z_thread_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_thread_stats_query(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_thread_stats_reset(بنية k_obj_core *obj_core);
صحيح z_thread_stats_disable(بنية k_obj_core *obj_core);
صحيح z_thread_stats_enable(بنية k_obj_core *obj_core);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
صحيح z_cpu_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_cpu_stats_query(بنية k_obj_core *obj_core, فراغ *stats);

صحيح z_kernel_stats_raw(بنية k_obj_core *obj_core, فراغ *stats);
صحيح z_kernel_stats_query(بنية k_obj_core *obj_core, فراغ *stats);
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#אם defined(CONFIG_THREAD_ABORT_NEED_CLEANUP)
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
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_KERNEL_INCLUDE_KERNEL_INTERNAL_H_ */
