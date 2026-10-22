/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ZEPHYR_KERNEL_INCLUDE_IPI_H_
#define ZEPHYR_KERNEL_INCLUDE_IPI_H_

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <zephyr/sys/atomic.h>

#define IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_இலக்கம்_CPUS))

#define IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


வெறுமை z_sched_ipi(வெறுமை);

/* defined in ipi.c when CONFIG_SMP=y */
#ifdef CONFIG_SMP
வெறுமை flag_ipi(uint32_t ipi_mask);
வெறுமை signal_pending_ipi(வெறுமை);
atomic_val_t ipi_mask_create(கட்டமைப்பு k_thread *thread);
#இல்லையெனில்
#define flag_ipi(ipi_mask) செய் { } போது (false)
#define signal_pending_ipi() செய் { } போது (false)
#endif /* CONFIG_SMP */

#ifdef CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
கட்டமைப்பு k_thread *ipi_idle_reserved_take(வெறுமை);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
வெறுமை ipi_idle_thread_unreserve(கட்டமைப்பு k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(கட்டமைப்பு k_thread *old_thread,
			    கட்டமைப்பு k_thread *new_thread);
#இல்லையெனில்
#define ipi_idle_reserved_take() வெற்று
#define ipi_idle_thread_unreserve(thread) செய் { \
	ARG_UNUSED(thread); \
} போது (false)
#define ipi_idle_thread_rebind(old_thread, new_thread) false
#endif /* CONFIG_IPI_OPTIMIZE_IDLE */

#endif /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
