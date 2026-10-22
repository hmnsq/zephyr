/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ZEPHYR_KERNEL_INCLUDE_IPI_H_
#define ZEPHYR_KERNEL_INCLUDE_IPI_H_

#شامل <zephyr/kernel.h>
#شامل <stdint.h>
#شامل <zephyr/sys/atomic.h>

#define IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_رقم_CPUS))

#define IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


تهی z_sched_ipi(تهی);

/* defined in ipi.c when CONFIG_SMP=y */
#ifdef CONFIG_SMP
تهی flag_ipi(uint32_t ipi_mask);
تهی signal_pending_ipi(تهی);
atomic_val_t ipi_mask_create(ساختار k_thread *thread);
#وگرنه
#define flag_ipi(ipi_mask) انجامبده { } وقتی (false)
#define signal_pending_ipi() انجامبده { } وقتی (false)
#endif /* CONFIG_SMP */

#ifdef CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
ساختار k_thread *ipi_idle_reserved_take(تهی);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
تهی ipi_idle_thread_unreserve(ساختار k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(ساختار k_thread *old_thread,
			    ساختار k_thread *new_thread);
#وگرنه
#define ipi_idle_reserved_take() پوچ
#define ipi_idle_thread_unreserve(thread) انجامبده { \
	ARG_UNUSED(thread); \
} وقتی (false)
#define ipi_idle_thread_rebind(old_thread, new_thread) false
#endif /* CONFIG_IPI_OPTIMIZE_IDLE */

#endif /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
