/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_IPI_H_
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_IPI_H_

#ضمن <zephyr/kernel.h>
#ضمن <stdint.h>
#ضمن <zephyr/sys/atomic.h>

#وصف_ڪريو IPI_ALL_CPUS_نقاب  (UINT32_MAX >> (32 - CONFIG_MP_MAX_انگ_CPUS))

#وصف_ڪريو IPI_CPU_نقاب(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? ٻٽو_انگ(cpu_id) : IPI_ALL_CPUS_نقاب)


فراغ z_sched_ipi(فراغ);

/* defined in ipi.c when CONFIG_SMP=y */
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
فراغ flag_ipi(uint32_t ipi_نقاب);
فراغ signal_pending_ipi(فراغ);
atomic_val_t ipi_نقاب_create(بنية k_thread *thread);
#وإلا
#وصف_ڪريو flag_ipi(ipi_نقاب) افعل { } طالما (false)
#وصف_ڪريو signal_pending_ipi() افعل { } طالما (false)
#جيڪڏهن_پورو /* CONFIG_SMP */

#جيڪڏهن_وصف_ٿيل CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
بنية k_thread *ipi_idle_reserved_take(فراغ);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
فراغ ipi_idle_thread_unreserve(بنية k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(بنية k_thread *old_thread,
			    بنية k_thread *new_thread);
#وإلا
#وصف_ڪريو ipi_idle_reserved_take() عدم
#وصف_ڪريو ipi_idle_thread_unreserve(thread) افعل { \
	ARG_اڻ_استعمال_ٿيل(thread); \
} طالما (false)
#وصف_ڪريو ipi_idle_thread_rebind(old_thread, new_thread) false
#جيڪڏهن_پورو /* CONFIG_IPI_OPTIMIZE_IDLE */

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
