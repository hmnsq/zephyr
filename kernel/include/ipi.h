/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_IPI_H_
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_IPI_H_

#ضمن <zephyr/kernel.h>
#ضمن <stdint.h>
#ضمن <zephyr/sys/atomic.h>

#تعریف_کړه IPI_ALL_CPUS_ماسک  (UINT32_MAX >> (32 - CONFIG_MP_MAX_عدد_CPUS))

#تعریف_کړه IPI_CPU_ماسک(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? بټ(cpu_id) : IPI_ALL_CPUS_ماسک)


فراغ z_sched_ipi(فراغ);

/* defined in ipi.c when CONFIG_SMP=y */
#که_تعریف_شوی CONFIG_SMP
فراغ flag_ipi(uint32_t ipi_ماسک);
فراغ signal_pending_ipi(فراغ);
atomic_val_t ipi_ماسک_create(بنية k_thread *thread);
#وإلا
#تعریف_کړه flag_ipi(ipi_ماسک) افعل { } طالما (false)
#تعریف_کړه signal_pending_ipi() افعل { } طالما (false)
#د_که_پای /* CONFIG_SMP */

#که_تعریف_شوی CONFIG_IPI_OPTIMIZE_IDLE
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
#تعریف_کړه ipi_idle_reserved_take() عدم
#تعریف_کړه ipi_idle_thread_unreserve(thread) افعل { \
	ARG_ناکارول_شوی(thread); \
} طالما (false)
#تعریف_کړه ipi_idle_thread_rebind(old_thread, new_thread) false
#د_که_پای /* CONFIG_IPI_OPTIMIZE_IDLE */

#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
