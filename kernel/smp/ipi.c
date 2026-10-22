/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/math_extras.h>
#include <kswap.h>
#include <ksched.h>
#include <ipi.h>

#nếu defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#include <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
static struct k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
extern void z_trace_sched_ipi(void);
#endif

#nếu defined(CONFIG_IPI_OPTIMIZE_IDLE)
static bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#nếu defined(CONFIG_PM)
	trả_về pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#không_thì
	ARG_UNUSED(cpu);
	trả_về true;
#endif
}

static inline số_nguyên sched_ipi_thread_reservation(const struct k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	trong_khi (reservations != 0U) {
		unsigned số_nguyên cpu = (unsigned số_nguyên)u32_count_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		nếu (_kernel.sched_ipi_target[cpu] == thread) {
			trả_về (số_nguyên)cpu;
		}
	}

	trả_về -1;
}

static void sched_ipi_idle_reserve(uint32_t cpu, struct k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == NULL);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

static void sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = NULL;
	_kernel.sched_ipi_reserved &= ~bit;
}

void ipi_idle_thread_unreserve(struct k_thread *thread)
{
	số_nguyên cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	nếu (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(struct k_thread *old_thread,
			    struct k_thread *new_thread)
{
	số_nguyên old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	nếu (old_cpu < 0) {
		trả_về false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#nếu defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	nếu (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		trả_về false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	trả_về true;
}

struct k_thread *ipi_idle_reserved_take(void)
{
	uint32_t cpu = _current_cpu->id;
	struct k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != NULL) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	nếu (thread != NULL) {
		sched_ipi_idle_unreserve(cpu);
	}

	trả_về thread;
}
#endif

void flag_ipi(uint32_t ipi_mask)
{
#nếu defined(CONFIG_SCHED_IPI_SUPPORTED)
	nếu (arch_num_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(struct k_thread *thread)
{
	nếu (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		trả_về (CONFIG_MP_MAX_NUM_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  num_cpus = (uint32_t)arch_num_cpus();
	uint32_t  id = _current_cpu->id;
	struct k_thread *cpu_thread;

#nếu defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	nếu (sched_ipi_thread_reservation(thread) >= 0) {
		trả_về 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	struct k_thread *last_thread = _kernel.cpus[last].current;

	nếu (last != id &&
	    last_thread != NULL &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#nếu defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		trả_về IPI_CPU_MASK(last);
	}

#endif

	cho (uint32_t i = 0; i < num_cpus; i++) {
		nếu (id == i) {
			continue;
		}

		/*
		 * An IPI absolutely does not need to be sent if ...
		 * 1. the CPU is not active, or
		 * 2. <thread> can not execute on the target CPU
		 * ... and might not need to be sent if ...
		 * 3. the target CPU's active thread is not preemptible, or
		 * 4. the target CPU's active thread has a higher priority
		 *    (Items 3 & 4 may be overridden by a metaIRQ thread)
		 */

#nếu defined(CONFIG_SCHED_CPU_MASK)
		nếu ((thread->base.cpu_mask & BIT(i)) == 0) {
			continue;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		nếu (cpu_thread == NULL) {
			continue;
		}

#nếu defined(CONFIG_IPI_OPTIMIZE_IDLE)
		nếu ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			continue;
		}
		nếu (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			trả_về IPI_CPU_MASK(i);
		}
#endif

		nếu ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	trả_về (atomic_val_t)ipi_mask;
}

void signal_pending_ipi(void)
{
	/* Drain only the bits for other CPUs. Clearing our own bit here
	 * would silently drop an IPI destined for us, because
	 * arch_sched_directed_ipi() skips the calling CPU. Our own bit
	 * is left set so the next interrupt entry on this CPU will see
	 * it and run the scheduler.
	 *
	 * When rescheduling, callers must ensure that signal_pending_ipi()
	 * is invoked while the scheduler lock is still held. Holding the
	 * lock ensures the scheduling decision and IPI dispatch are atomic:
	 * either a concurrent flag_ipi() lands before the lock is acquired
	 * (and the CPU sees the new thread), or it lands after the lock is
	 * released (and the other CPU dispatches the IPI).
	 */

#nếu defined(CONFIG_SCHED_IPI_SUPPORTED)
	nếu (arch_num_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		nếu (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#không_thì
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
static struct k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned số_nguyên cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	trả_về (work == NULL) ? NULL
			      : CONTAINER_OF(work, struct k_ipi_work, node[cpu_id]);
}

số_nguyên k_ipi_work_add(struct k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != NULL, "");
	__ASSERT(func != NULL, "");

	k_spinlock_key_t key = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	nếu (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, key);
		trả_về -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned số_nguyên cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	cho (unsigned số_nguyên id = 0; id < arch_num_cpus(); id++) {
		nếu ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->node[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, key);

	trả_về 0;
}

số_nguyên k_ipi_work_wait(struct k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	trả_về (rv == 0) ? -EAGAIN : 0;
}

void k_ipi_work_signal(void)
{
	signal_pending_ipi();
}

static void ipi_work_process(sys_dlist_t *list, unsigned số_nguyên cpu_id)
{
	k_spinlock_key_t key = k_spin_lock(&ipi_lock);

	cho (struct k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != NULL; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->node[cpu_id]);
		k_spin_unlock(&ipi_lock, key);

		work->func(work);

		key = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, key);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

void z_sched_ipi(void)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#ifdef CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#endif /* CONFIG_TRACE_SCHED_IPI */

#ifdef CONFIG_TIMESLICING
	z_time_slice();
#endif /* CONFIG_TIMESLICING */

#ifdef CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
	unsigned số_nguyên cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
