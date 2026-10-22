/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <kswap.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <ipi.h>

#എങ്കിൽ defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#അന്തര്ഭാവ <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
സ്ഥിതം ഘടന k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
ബാഹ്യ ശൂന്യം z_trace_sched_ipi(ശൂന്യം);
#endif

#എങ്കിൽ defined(CONFIG_IPI_OPTIMIZE_IDLE)
സ്ഥിതം bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#എങ്കിൽ defined(CONFIG_PM)
	തിരികെ pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#അല്ലെങ്കിൽ
	ARG_UNUSED(cpu);
	തിരികെ true;
#endif
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ sched_ipi_thread_reservation(സ്ഥിരം ഘടന k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	ആകുമ്പോൾ (reservations != 0U) {
		unsigned പൂർണ്ണസംഖ്യ cpu = (unsigned പൂർണ്ണസംഖ്യ)u32_എണ്ണം_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		എങ്കിൽ (_kernel.sched_ipi_target[cpu] == thread) {
			തിരികെ (പൂർണ്ണസംഖ്യ)cpu;
		}
	}

	തിരികെ -1;
}

സ്ഥിതം ശൂന്യം sched_ipi_idle_reserve(uint32_t cpu, ഘടന k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == ശൂന്യ);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

സ്ഥിതം ശൂന്യം sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = ശൂന്യ;
	_kernel.sched_ipi_reserved &= ~bit;
}

ശൂന്യം ipi_idle_thread_unreserve(ഘടന k_thread *thread)
{
	പൂർണ്ണസംഖ്യ cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	എങ്കിൽ (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(ഘടന k_thread *old_thread,
			    ഘടന k_thread *new_thread)
{
	പൂർണ്ണസംഖ്യ old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	എങ്കിൽ (old_cpu < 0) {
		തിരികെ false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#എങ്കിൽ defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	എങ്കിൽ (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		തിരികെ false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	തിരികെ true;
}

ഘടന k_thread *ipi_idle_reserved_take(ശൂന്യം)
{
	uint32_t cpu = _current_cpu->id;
	ഘടന k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != ശൂന്യ) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	എങ്കിൽ (thread != ശൂന്യ) {
		sched_ipi_idle_unreserve(cpu);
	}

	തിരികെ thread;
}
#endif

ശൂന്യം flag_ipi(uint32_t ipi_mask)
{
#എങ്കിൽ defined(CONFIG_SCHED_IPI_SUPPORTED)
	എങ്കിൽ (arch_അക്കം_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(ഘടന k_thread *thread)
{
	എങ്കിൽ (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		തിരികെ (CONFIG_MP_MAX_അക്കം_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  അക്കം_cpus = (uint32_t)arch_അക്കം_cpus();
	uint32_t  id = _current_cpu->id;
	ഘടന k_thread *cpu_thread;

#എങ്കിൽ defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	എങ്കിൽ (sched_ipi_thread_reservation(thread) >= 0) {
		തിരികെ 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	ഘടന k_thread *last_thread = _kernel.cpus[last].current;

	എങ്കിൽ (last != id &&
	    last_thread != ശൂന്യ &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#എങ്കിൽ defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		തിരികെ IPI_CPU_MASK(last);
	}

#endif

	വേണ്ടി (uint32_t i = 0; i < അക്കം_cpus; i++) {
		എങ്കിൽ (id == i) {
			തുടരുക;
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

#എങ്കിൽ defined(CONFIG_SCHED_CPU_MASK)
		എങ്കിൽ ((thread->base.cpu_mask & BIT(i)) == 0) {
			തുടരുക;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		എങ്കിൽ (cpu_thread == ശൂന്യ) {
			തുടരുക;
		}

#എങ്കിൽ defined(CONFIG_IPI_OPTIMIZE_IDLE)
		എങ്കിൽ ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			തുടരുക;
		}
		എങ്കിൽ (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			തിരികെ IPI_CPU_MASK(i);
		}
#endif

		എങ്കിൽ ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	തിരികെ (atomic_val_t)ipi_mask;
}

ശൂന്യം signal_pending_ipi(ശൂന്യം)
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

#എങ്കിൽ defined(CONFIG_SCHED_IPI_SUPPORTED)
	എങ്കിൽ (arch_അക്കം_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		എങ്കിൽ (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#അല്ലെങ്കിൽ
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
സ്ഥിതം ഘടന k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned പൂർണ്ണസംഖ്യ cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	തിരികെ (work == ശൂന്യ) ? ശൂന്യ
			      : CONTAINER_OF(work, ഘടന k_ipi_work, കണ്ണി[cpu_id]);
}

പൂർണ്ണസംഖ്യ k_ipi_work_add(ഘടന k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ശൂന്യ, "");
	__ASSERT(func != ശൂന്യ, "");

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	എങ്കിൽ (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, താക്കോൽ);
		തിരികെ -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned പൂർണ്ണസംഖ്യ cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	വേണ്ടി (unsigned പൂർണ്ണസംഖ്യ id = 0; id < arch_അക്കം_cpus(); id++) {
		എങ്കിൽ ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->കണ്ണി[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, താക്കോൽ);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ k_ipi_work_wait(ഘടന k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	തിരികെ (rv == 0) ? -EAGAIN : 0;
}

ശൂന്യം k_ipi_work_signal(ശൂന്യം)
{
	signal_pending_ipi();
}

സ്ഥിതം ശൂന്യം ipi_work_process(sys_dlist_t *list, unsigned പൂർണ്ണസംഖ്യ cpu_id)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&ipi_lock);

	വേണ്ടി (ഘടന k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != ശൂന്യ; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->കണ്ണി[cpu_id]);
		k_spin_unlock(&ipi_lock, താക്കോൽ);

		work->func(work);

		താക്കോൽ = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, താക്കോൽ);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

ശൂന്യം z_sched_ipi(ശൂന്യം)
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
	unsigned പൂർണ്ണസംഖ്യ cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
