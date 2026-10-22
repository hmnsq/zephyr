/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_IPI_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_IPI_H_

#समावेश <zephyr/kernel.h>
#समावेश <stdint.h>
#समावेश <zephyr/sys/atomic.h>

#घोषणा IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_अंक_CPUS))

#घोषणा IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


व्योम z_sched_ipi(व्योम);

/* defined in ipi.c when CONFIG_SMP=y */
#अगर_घोषित CONFIG_SMP
व्योम flag_ipi(uint32_t ipi_mask);
व्योम signal_pending_ipi(व्योम);
atomic_val_t ipi_mask_create(काष्ठा k_thread *thread);
#अन्यथा
#घोषणा flag_ipi(ipi_mask) करो { } जबतक (false)
#घोषणा signal_pending_ipi() करो { } जबतक (false)
#पूर्ण_अगर /* CONFIG_SMP */

#अगर_घोषित CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
काष्ठा k_thread *ipi_idle_reserved_take(व्योम);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
व्योम ipi_idle_thread_unreserve(काष्ठा k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(काष्ठा k_thread *old_thread,
			    काष्ठा k_thread *new_thread);
#अन्यथा
#घोषणा ipi_idle_reserved_take() शून्य
#घोषणा ipi_idle_thread_unreserve(thread) करो { \
	ARG_UNUSED(thread); \
} जबतक (false)
#घोषणा ipi_idle_thread_rebind(old_thread, new_thread) false
#पूर्ण_अगर /* CONFIG_IPI_OPTIMIZE_IDLE */

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
