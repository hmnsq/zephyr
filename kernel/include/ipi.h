/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ZEPHYR_KERNEL_INCLUDE_IPI_H_
#define ZEPHYR_KERNEL_INCLUDE_IPI_H_

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <zephyr/sys/atomic.h>

#define IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_अङ्क_CPUS))

#define IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


शून्य z_sched_ipi(शून्य);

/* defined in ipi.c when CONFIG_SMP=y */
#ifdef CONFIG_SMP
शून्य flag_ipi(uint32_t ipi_mask);
शून्य signal_pending_ipi(शून्य);
atomic_val_t ipi_mask_create(संरचना k_thread *thread);
#अन्यथा
#define flag_ipi(ipi_mask) गर { } जबसम्म (false)
#define signal_pending_ipi() गर { } जबसम्म (false)
#endif /* CONFIG_SMP */

#ifdef CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
संरचना k_thread *ipi_idle_reserved_take(शून्य);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
शून्य ipi_idle_thread_unreserve(संरचना k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(संरचना k_thread *old_thread,
			    संरचना k_thread *new_thread);
#अन्यथा
#define ipi_idle_reserved_take() रिक्त
#define ipi_idle_thread_unreserve(thread) गर { \
	ARG_UNUSED(thread); \
} जबसम्म (false)
#define ipi_idle_thread_rebind(old_thread, new_thread) false
#endif /* CONFIG_IPI_OPTIMIZE_IDLE */

#endif /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
