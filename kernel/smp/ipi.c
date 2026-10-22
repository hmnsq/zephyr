/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>

#ಆದರೆ defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#ಅನ್ತರ್ಭಾವ <zephyr/pm/pm.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_IPI_SUPPORTED
ಸ್ಥಿರ ರಚನೆ k_spinlock ipi_ಬೀಗ;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TRACE_SCHED_IPI
ಬಾಹ್ಯ ಶೂನ್ಯ z_trace_sched_ipi(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_IPI_OPTIMIZE_IDLE)
ಸ್ಥಿರ bool sched_ipi_idle_cpu_ಸಕ್ರಿಯ(uint32_t cpu)
{
#ಆದರೆ defined(CONFIG_PM)
	ಹಿಂತಿರುಗಿ pm_state_ಮುಂದಿನ_get((uint8_t)cpu)->state == PM_STATE_ಸಕ್ರಿಯ;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(cpu);
	ಹಿಂತಿರುಗಿ true;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ sched_ipi_thread_reservation(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	ಆಗ (reservations != 0U) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)u32_ಎಣಿಕೆ_trailing_zeros(reservations);

		reservations &= ~ಬಿಟ್(cpu);
		ಆದರೆ (_kernel.sched_ipi_ಗುರಿ[cpu] == thread) {
			ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)cpu;
		}
	}

	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ sched_ipi_idle_reserve(uint32_t cpu, ರಚನೆ k_thread *thread)
{
	uint32_t ಬಿಟ್_2 = ಬಿಟ್(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & ಬಿಟ್_2) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_ಗುರಿ[cpu] == ಖಾಲಿ);

	_kernel.sched_ipi_ಗುರಿ[cpu] = thread;
	_kernel.sched_ipi_reserved |= ಬಿಟ್_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t ಬಿಟ್_2 = ಬಿಟ್(cpu);

	_kernel.sched_ipi_ಗುರಿ[cpu] = ಖಾಲಿ;
	_kernel.sched_ipi_reserved &= ~ಬಿಟ್_2;
}

ಶೂನ್ಯ ipi_idle_thread_unreserve(ರಚನೆ k_thread *thread)
{
	ಪೂರ್ಣಾಂಕ cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	ಆದರೆ (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(ರಚನೆ k_thread *old_thread,
			    ರಚನೆ k_thread *new_thread)
{
	ಪೂರ್ಣಾಂಕ old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	ಆದರೆ (old_cpu < 0) {
		ಹಿಂತಿರುಗಿ false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_ಸಕ್ರಿಯ(cpu);

#ಆದರೆ defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ)
	eligible = eligible && ((new_thread->ತಳ.cpu_ಮುಖವಾಡ & ಬಿಟ್(cpu)) != 0U);
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		ಹಿಂತಿರುಗಿ false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_ಗುರಿ[cpu] = new_thread;
	ಹಿಂತಿರುಗಿ true;
}

ರಚನೆ k_thread *ipi_idle_reserved_take(ಶೂನ್ಯ)
{
	uint32_t cpu = _current_cpu->id;
	ರಚನೆ k_thread *thread = _kernel.sched_ipi_ಗುರಿ[cpu];

	__ASSERT_NO_MSG((thread != ಖಾಲಿ) ==
			((_kernel.sched_ipi_reserved & ಬಿಟ್(cpu)) != 0U));

	ಆದರೆ (thread != ಖಾಲಿ) {
		sched_ipi_idle_unreserve(cpu);
	}

	ಹಿಂತಿರುಗಿ thread;
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ flag_ipi(uint32_t ipi_ಮುಖವಾಡ)
{
#ಆದರೆ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ಆದರೆ (arch_ಅಂಕೆ_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_ಮುಖವಾಡ);
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_ಮುಖವಾಡ_create(ರಚನೆ k_thread *thread)
{
	ಆದರೆ (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		ಹಿಂತಿರುಗಿ (CONFIG_MP_MAX_ಅಂಕೆ_CPUS > 1) ? IPI_ALL_CPUS_ಮುಖವಾಡ : 0;
	}

	uint32_t  ipi_ಮುಖವಾಡ = 0;
	uint32_t  ಅಂಕೆ_cpus = (uint32_t)arch_ಅಂಕೆ_cpus();
	uint32_t  id = _current_cpu->id;
	ರಚನೆ k_thread *cpu_thread;

#ಆದರೆ defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	ಆದರೆ (sched_ipi_thread_reservation(thread) >= 0) {
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t ಕೊನೆಯ = thread->ತಳ.cpu;
	ರಚನೆ k_thread *ಕೊನೆಯ_thread = _kernel.cpus[ಕೊನೆಯ].current;

	ಆದರೆ (ಕೊನೆಯ != id &&
	    ಕೊನೆಯ_thread != ಖಾಲಿ &&
	    z_is_idle_thread_object(ಕೊನೆಯ_thread) &&
	    sched_ipi_idle_cpu_ಸಕ್ರಿಯ(ಕೊನೆಯ) &&
#ಆದರೆ defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ)
	    (thread->ತಳ.cpu_ಮುಖವಾಡ & ಬಿಟ್(ಕೊನೆಯ)) != 0 &&
#ಆದರೆ_ಅಂತ್ಯ
	    (_kernel.sched_ipi_reserved & ಬಿಟ್(ಕೊನೆಯ)) == 0U) {
		sched_ipi_idle_reserve(ಕೊನೆಯ, thread);
		ಹಿಂತಿರುಗಿ IPI_CPU_ಮುಖವಾಡ(ಕೊನೆಯ);
	}

#ಆದರೆ_ಅಂತ್ಯ

	ಗಾಗಿ (uint32_t i = 0; i < ಅಂಕೆ_cpus; i++) {
		ಆದರೆ (id == i) {
			ಮುಂದುವರಿ;
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

#ಆದರೆ defined(CONFIG_SCHED_CPU_ಮುಖವಾಡ)
		ಆದರೆ ((thread->ತಳ.cpu_ಮುಖವಾಡ & ಬಿಟ್(i)) == 0) {
			ಮುಂದುವರಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ

		cpu_thread = _kernel.cpus[i].current;

		ಆದರೆ (cpu_thread == ಖಾಲಿ) {
			ಮುಂದುವರಿ;
		}

#ಆದರೆ defined(CONFIG_IPI_OPTIMIZE_IDLE)
		ಆದರೆ ((_kernel.sched_ipi_reserved & ಬಿಟ್(i)) != 0U) {
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (i != (uint32_t)thread->ತಳ.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_ಸಕ್ರಿಯ(i)) {
			sched_ipi_idle_reserve(i, thread);
			ಹಿಂತಿರುಗಿ IPI_CPU_ಮುಖವಾಡ(i);
		}
#ಆದರೆ_ಅಂತ್ಯ

		ಆದರೆ ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_ಮುಖವಾಡ |= ಬಿಟ್(i);
		}
	}

	ಹಿಂತಿರುಗಿ (atomic_val_t)ipi_ಮುಖವಾಡ;
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

#ಆದರೆ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ಆದರೆ (arch_ಅಂಕೆ_cpus() > 1) {
		uint32_t self_ಬಿಟ್ = ಬಿಟ್(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_ಬಿಟ್);
		cpu_bitmap &= ~self_ಬಿಟ್;
		ಆದರೆ (cpu_bitmap != 0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#ಇಲ್ಲದಿದ್ದರೆ
			arch_sched_ಪ್ರಸಾರ_ipi();
#ಆದರೆ_ಅಂತ್ಯ
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_IPI_SUPPORTED
ಸ್ಥಿರ ರಚನೆ k_ipi_work *ಮೊದಲ_ipi_work(sys_dlist_t *list, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_ತಲೆ(list);

	ಹಿಂತಿರುಗಿ (work == ಖಾಲಿ) ? ಖಾಲಿ
			      : CONTAINER_ದ(work, ರಚನೆ k_ipi_work, ಗಂಟು[cpu_id]);
}

ಪೂರ್ಣಾಂಕ k_ipi_work_ಕೂಡಿಸು(ರಚನೆ k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ಖಾಲಿ, "");
	__ASSERT(func != ಖಾಲಿ, "");

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ipi_ಬೀಗ);

	/* Verify the IPI work item is not currently in use */

	ಆದರೆ (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_ಮುಖವಾಡ & ~ಬಿಟ್(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_ಮುಖವಾಡ);
	work->func = func;
	work->bitmask = cpu_bitmask;

	ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ id = 0; id < arch_ಅಂಕೆ_cpus(); id++) {
		ಆದರೆ ((cpu_bitmask & ಬಿಟ್(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->ಗಂಟು[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_ipi_work_wait(ರಚನೆ k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	ಹಿಂತಿರುಗಿ (rv == 0) ? -EAGAIN : 0;
}

ಶೂನ್ಯ k_ipi_work_signal(ಶೂನ್ಯ)
{
	signal_pending_ipi();
}

ಸ್ಥಿರ ಶೂನ್ಯ ipi_work_process(sys_dlist_t *list, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu_id)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&ipi_ಬೀಗ);

	ಗಾಗಿ (ರಚನೆ k_ipi_work *work = ಮೊದಲ_ipi_work(list, cpu_id);
	     work != ಖಾಲಿ; work = ಮೊದಲ_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->ಗಂಟು[cpu_id]);
		k_spin_unlock(&ipi_ಬೀಗ, ಕೀಲಿ);

		work->func(work);

		ಕೀಲಿ = k_spin_ಬೀಗ(&ipi_ಬೀಗ);
		k_event_post(&work->event, ಬಿಟ್(cpu_id));
	}

	k_spin_unlock(&ipi_ಬೀಗ, ಕೀಲಿ);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_IPI_SUPPORTED */

ಶೂನ್ಯ z_sched_ipi(ಶೂನ್ಯ)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TRACE_SCHED_IPI */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICING
	z_ಸಮಯ_ತುಂಡು();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICING */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_IPI_SUPPORTED
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#ಆದರೆ_ಅಂತ್ಯ
}
