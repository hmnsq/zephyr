/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_IPI_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_IPI_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/atomic.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPI_ALL_CPUS_ਨਕਾਬ  (UINT32_MAX >> (32 - CONFIG_MP_MAX_ਅੰਕ_CPUS))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IPI_CPU_ਨਕਾਬ(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? ਦੋਆਧਾਰੀ_ਅੰਕ(cpu_id) : IPI_ALL_CPUS_ਨਕਾਬ)


ਖਾਲੀ z_sched_ipi(ਖਾਲੀ);

/* defined in ipi.c when CONFIG_SMP=y */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
ਖਾਲੀ flag_ipi(uint32_t ipi_ਨਕਾਬ);
ਖਾਲੀ signal_pending_ipi(ਖਾਲੀ);
atomic_val_t ipi_ਨਕਾਬ_create(ਬਣਤਰ k_thread *thread);
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ flag_ipi(ipi_ਨਕਾਬ) ਕਰੋ { } ਜਦੋਂ (false)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ signal_pending_ipi() ਕਰੋ { } ਜਦੋਂ (false)
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
ਬਣਤਰ k_thread *ipi_idle_reserved_take(ਖਾਲੀ);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
ਖਾਲੀ ipi_idle_thread_unreserve(ਬਣਤਰ k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(ਬਣਤਰ k_thread *old_thread,
			    ਬਣਤਰ k_thread *new_thread);
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ipi_idle_reserved_take() ਨਿਰਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ipi_idle_thread_unreserve(thread) ਕਰੋ { \
	ARG_ਅਣਵਰਤਿਆ(thread); \
} ਜਦੋਂ (false)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ipi_idle_thread_rebind(old_thread, new_thread) false
#ਜੇ_ਅੰਤ /* CONFIG_IPI_OPTIMIZE_IDLE */

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
