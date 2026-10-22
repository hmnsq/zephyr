/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>

#ಯದಿ defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#ಅನ್ತರ್ಭಾವ <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
ಬಾಹ್ಯ ಶೂನ್ಯ z_trace_sched_ipi(ಶೂನ್ಯ);
#endif

#ಯದಿ defined(CONFIG_IPI_OPTIMIZE_IDLE)
ಸ್ಥಿರ bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#ಯದಿ defined(CONFIG_PM)
	ಪ್ರತ್ಯಾಗಮ pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#ಅನ್ಯಥಾ
	ARG_UNUSED(cpu);
	ಪ್ರತ್ಯಾಗಮ true;
#endif
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ sched_ipi_thread_reservation(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	ಯಾವತ್ (reservations != 0U) {
		unsigned ಪೂರ್ಣಾಙ್ಕ cpu = (unsigned ಪೂರ್ಣಾಙ್ಕ)u32_ಎಣಿಕೆ_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		ಯದಿ (_kernel.sched_ipi_target[cpu] == thread) {
			ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)cpu;
		}
	}

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ sched_ipi_idle_reserve(uint32_t cpu, ಸಂರಚನಾ k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == ರಿಕ್ತ);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

ಸ್ಥಿರ ಶೂನ್ಯ sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = ರಿಕ್ತ;
	_kernel.sched_ipi_reserved &= ~bit;
}

ಶೂನ್ಯ ipi_idle_thread_unreserve(ಸಂರಚನಾ k_thread *thread)
{
	ಪೂರ್ಣಾಙ್ಕ cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	ಯದಿ (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(ಸಂರಚನಾ k_thread *old_thread,
			    ಸಂರಚನಾ k_thread *new_thread)
{
	ಪೂರ್ಣಾಙ್ಕ old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	ಯದಿ (old_cpu < 0) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#ಯದಿ defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	ಯದಿ (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸಂರಚನಾ k_thread *ipi_idle_reserved_take(ಶೂನ್ಯ)
{
	uint32_t cpu = _current_cpu->id;
	ಸಂರಚನಾ k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != ರಿಕ್ತ) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	ಯದಿ (thread != ರಿಕ್ತ) {
		sched_ipi_idle_unreserve(cpu);
	}

	ಪ್ರತ್ಯಾಗಮ thread;
}
#endif

ಶೂನ್ಯ flag_ipi(uint32_t ipi_mask)
{
#ಯದಿ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ಯದಿ (arch_ಅಂಕೆ_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(ಸಂರಚನಾ k_thread *thread)
{
	ಯದಿ (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		ಪ್ರತ್ಯಾಗಮ (CONFIG_MP_MAX_ಅಂಕೆ_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  ಅಂಕೆ_cpus = (uint32_t)arch_ಅಂಕೆ_cpus();
	uint32_t  id = _current_cpu->id;
	ಸಂರಚನಾ k_thread *cpu_thread;

#ಯದಿ defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	ಯದಿ (sched_ipi_thread_reservation(thread) >= 0) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	ಸಂರಚನಾ k_thread *last_thread = _kernel.cpus[last].current;

	ಯದಿ (last != id &&
	    last_thread != ರಿಕ್ತ &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#ಯದಿ defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		ಪ್ರತ್ಯಾಗಮ IPI_CPU_MASK(last);
	}

#endif

	ಪ್ರತಿ (uint32_t i = 0; i < ಅಂಕೆ_cpus; i++) {
		ಯದಿ (id == i) {
			ಅನುವರ್ತ;
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

#ಯದಿ defined(CONFIG_SCHED_CPU_MASK)
		ಯದಿ ((thread->base.cpu_mask & BIT(i)) == 0) {
			ಅನುವರ್ತ;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		ಯದಿ (cpu_thread == ರಿಕ್ತ) {
			ಅನುವರ್ತ;
		}

#ಯದಿ defined(CONFIG_IPI_OPTIMIZE_IDLE)
		ಯದಿ ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			ಅನುವರ್ತ;
		}
		ಯದಿ (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			ಪ್ರತ್ಯಾಗಮ IPI_CPU_MASK(i);
		}
#endif

		ಯದಿ ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	ಪ್ರತ್ಯಾಗಮ (atomic_val_t)ipi_mask;
}

ಶೂನ್ಯ signal_pending_ipi(ಶೂನ್ಯ)
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

#ಯದಿ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ಯದಿ (arch_ಅಂಕೆ_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		ಯದಿ (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#ಅನ್ಯಥಾ
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ಸ್ಥಿರ ಸಂರಚನಾ k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned ಪೂರ್ಣಾಙ್ಕ cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	ಪ್ರತ್ಯಾಗಮ (work == ರಿಕ್ತ) ? ರಿಕ್ತ
			      : CONTAINER_OF(work, ಸಂರಚನಾ k_ipi_work, ಗಂಟು[cpu_id]);
}

ಪೂರ್ಣಾಙ್ಕ k_ipi_work_add(ಸಂರಚನಾ k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ರಿಕ್ತ, "");
	__ASSERT(func != ರಿಕ್ತ, "");

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	ಯದಿ (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned ಪೂರ್ಣಾಙ್ಕ cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ id = 0; id < arch_ಅಂಕೆ_cpus(); id++) {
		ಯದಿ ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->ಗಂಟು[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ k_ipi_work_wait(ಸಂರಚನಾ k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	ಪ್ರತ್ಯಾಗಮ (rv == 0) ? -EAGAIN : 0;
}

ಶೂನ್ಯ k_ipi_work_signal(ಶೂನ್ಯ)
{
	signal_pending_ipi();
}

ಸ್ಥಿರ ಶೂನ್ಯ ipi_work_process(sys_dlist_t *list, unsigned ಪೂರ್ಣಾಙ್ಕ cpu_id)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&ipi_lock);

	ಪ್ರತಿ (ಸಂರಚನಾ k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != ರಿಕ್ತ; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->ಗಂಟು[cpu_id]);
		k_spin_unlock(&ipi_lock, ಕೀಲಿ);

		work->func(work);

		ಕೀಲಿ = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, ಕೀಲಿ);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

ಶೂನ್ಯ z_sched_ipi(ಶೂನ್ಯ)
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
	unsigned ಪೂರ್ಣಾಙ್ಕ cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
