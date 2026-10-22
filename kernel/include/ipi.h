/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_IPI_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_IPI_H_

#شامل <zephyr/kernel.h>
#شامل <stdint.h>
#شامل <zephyr/sys/atomic.h>

#تعریف_کرو IPI_ALL_CPUS_نقاب  (UINT32_MAX >> (32 - CONFIG_MP_MAX_عدد_CPUS))

#تعریف_کرو IPI_CPU_نقاب(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? ثنائی_ہندسہ(cpu_id) : IPI_ALL_CPUS_نقاب)


خالی z_sched_ipi(خالی);

/* defined in ipi.c when CONFIG_SMP=y */
#اگر_تعریف_شدہ CONFIG_SMP
خالی flag_ipi(uint32_t ipi_نقاب);
خالی signal_pending_ipi(خالی);
atomic_val_t ipi_نقاب_create(ساخت k_thread *thread);
#ورنہ
#تعریف_کرو flag_ipi(ipi_نقاب) کرو { } جب (false)
#تعریف_کرو signal_pending_ipi() کرو { } جب (false)
#اگر_ختم /* CONFIG_SMP */

#اگر_تعریف_شدہ CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
ساخت k_thread *ipi_idle_reserved_take(خالی);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
خالی ipi_idle_thread_unreserve(ساخت k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(ساخت k_thread *old_thread,
			    ساخت k_thread *new_thread);
#ورنہ
#تعریف_کرو ipi_idle_reserved_take() تہی
#تعریف_کرو ipi_idle_thread_unreserve(thread) کرو { \
	ARG_غیر_مستعمل(thread); \
} جب (false)
#تعریف_کرو ipi_idle_thread_rebind(old_thread, new_thread) false
#اگر_ختم /* CONFIG_IPI_OPTIMIZE_IDLE */

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
