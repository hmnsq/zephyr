/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_IPI_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_IPI_H_

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <zephyr/sys/atomic.h>

#నిర్వచించు IPI_ALL_CPUS_ముసుగు  (UINT32_MAX >> (32 - CONFIG_MP_MAX_అంకె_CPUS))

#నిర్వచించు IPI_CPU_ముసుగు(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? బిట్టు(cpu_id) : IPI_ALL_CPUS_ముసుగు)


శూన్య z_sched_ipi(శూన్య);

/* defined in ipi.c when CONFIG_SMP=y */
#నిర్వచితమైతే CONFIG_SMP
శూన్య flag_ipi(uint32_t ipi_ముసుగు);
శూన్య signal_pending_ipi(శూన్య);
atomic_val_t ipi_ముసుగు_create(నిర్మాణం k_thread *thread);
#లేకపోతే
#నిర్వచించు flag_ipi(ipi_ముసుగు) చేయి { } అయితే (false)
#నిర్వచించు signal_pending_ipi() చేయి { } అయితే (false)
#అయితే_ముగింపు /* CONFIG_SMP */

#నిర్వచితమైతే CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
నిర్మాణం k_thread *ipi_idle_reserved_take(శూన్య);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
శూన్య ipi_idle_thread_unreserve(నిర్మాణం k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(నిర్మాణం k_thread *old_thread,
			    నిర్మాణం k_thread *new_thread);
#లేకపోతే
#నిర్వచించు ipi_idle_reserved_take() ఖాళీ
#నిర్వచించు ipi_idle_thread_unreserve(thread) చేయి { \
	ARG_వాడని(thread); \
} అయితే (false)
#నిర్వచించు ipi_idle_thread_rebind(old_thread, new_thread) false
#అయితే_ముగింపు /* CONFIG_IPI_OPTIMIZE_IDLE */

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
