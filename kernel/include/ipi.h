/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_IPI_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_IPI_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>

#ವ್ಯಾಖ್ಯಾನಿಸು IPI_ALL_CPUS_ಮುಖವಾಡ  (UINT32_MAX >> (32 - CONFIG_MP_MAX_ಅಂಕೆ_CPUS))

#ವ್ಯಾಖ್ಯಾನಿಸು IPI_CPU_ಮುಖವಾಡ(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? ಬಿಟ್(cpu_id) : IPI_ALL_CPUS_ಮುಖವಾಡ)


ಶೂನ್ಯ z_sched_ipi(ಶೂನ್ಯ);

/* defined in ipi.c when CONFIG_SMP=y */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
ಶೂನ್ಯ flag_ipi(uint32_t ipi_ಮುಖವಾಡ);
ಶೂನ್ಯ signal_pending_ipi(ಶೂನ್ಯ);
atomic_val_t ipi_ಮುಖವಾಡ_create(ರಚನೆ k_thread *thread);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು flag_ipi(ipi_ಮುಖವಾಡ) ಮಾಡು { } ಆಗ (false)
#ವ್ಯಾಖ್ಯಾನಿಸು signal_pending_ipi() ಮಾಡು { } ಆಗ (false)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
ರಚನೆ k_thread *ipi_idle_reserved_take(ಶೂನ್ಯ);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
ಶೂನ್ಯ ipi_idle_thread_unreserve(ರಚನೆ k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(ರಚನೆ k_thread *old_thread,
			    ರಚನೆ k_thread *new_thread);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ipi_idle_reserved_take() ಖಾಲಿ
#ವ್ಯಾಖ್ಯಾನಿಸು ipi_idle_thread_unreserve(thread) ಮಾಡು { \
	ARG_ಬಳಸದ(thread); \
} ಆಗ (false)
#ವ್ಯಾಖ್ಯಾನಿಸು ipi_idle_thread_rebind(old_thread, new_thread) false
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_IPI_OPTIMIZE_IDLE */

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
