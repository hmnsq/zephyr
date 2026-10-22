/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <kswap.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <ipi.h>

#ଯଦି defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#ଅନ୍ତର୍ଭାଵ <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ସ୍ଥିର ସଂରଚନା k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
ବାହ୍ୟ ଶୂନ୍ୟ z_trace_sched_ipi(ଶୂନ୍ୟ);
#endif

#ଯଦି defined(CONFIG_IPI_OPTIMIZE_IDLE)
ସ୍ଥିର bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#ଯଦି defined(CONFIG_PM)
	ଫେରନ୍ତୁ pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#ଅନ୍ୟଥା
	ARG_UNUSED(cpu);
	ଫେରନ୍ତୁ true;
#endif
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା sched_ipi_thread_reservation(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	ଯେତେବେଳେ (reservations != 0U) {
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)u32_ଗଣନା_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		ଯଦି (_kernel.sched_ipi_target[cpu] == thread) {
			ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)cpu;
		}
	}

	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଶୂନ୍ୟ sched_ipi_idle_reserve(uint32_t cpu, ସଂରଚନା k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == ଖାଲି);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

ସ୍ଥିର ଶୂନ୍ୟ sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = ଖାଲି;
	_kernel.sched_ipi_reserved &= ~bit;
}

ଶୂନ୍ୟ ipi_idle_thread_unreserve(ସଂରଚନା k_thread *thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	ଯଦି (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(ସଂରଚନା k_thread *old_thread,
			    ସଂରଚନା k_thread *new_thread)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	ଯଦି (old_cpu < 0) {
		ଫେରନ୍ତୁ false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#ଯଦି defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	ଯଦି (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		ଫେରନ୍ତୁ false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	ଫେରନ୍ତୁ true;
}

ସଂରଚନା k_thread *ipi_idle_reserved_take(ଶୂନ୍ୟ)
{
	uint32_t cpu = _current_cpu->id;
	ସଂରଚନା k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != ଖାଲି) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	ଯଦି (thread != ଖାଲି) {
		sched_ipi_idle_unreserve(cpu);
	}

	ଫେରନ୍ତୁ thread;
}
#endif

ଶୂନ୍ୟ flag_ipi(uint32_t ipi_mask)
{
#ଯଦି defined(CONFIG_SCHED_IPI_SUPPORTED)
	ଯଦି (arch_ଅଙ୍କ_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(ସଂରଚନା k_thread *thread)
{
	ଯଦି (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		ଫେରନ୍ତୁ (CONFIG_MP_MAX_ଅଙ୍କ_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  ଅଙ୍କ_cpus = (uint32_t)arch_ଅଙ୍କ_cpus();
	uint32_t  id = _current_cpu->id;
	ସଂରଚନା k_thread *cpu_thread;

#ଯଦି defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	ଯଦି (sched_ipi_thread_reservation(thread) >= 0) {
		ଫେରନ୍ତୁ 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	ସଂରଚନା k_thread *last_thread = _kernel.cpus[last].current;

	ଯଦି (last != id &&
	    last_thread != ଖାଲି &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#ଯଦି defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		ଫେରନ୍ତୁ IPI_CPU_MASK(last);
	}

#endif

	ପାଇଁ (uint32_t i = 0; i < ଅଙ୍କ_cpus; i++) {
		ଯଦି (id == i) {
			ଜାରି;
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

#ଯଦି defined(CONFIG_SCHED_CPU_MASK)
		ଯଦି ((thread->base.cpu_mask & BIT(i)) == 0) {
			ଜାରି;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		ଯଦି (cpu_thread == ଖାଲି) {
			ଜାରି;
		}

#ଯଦି defined(CONFIG_IPI_OPTIMIZE_IDLE)
		ଯଦି ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			ଜାରି;
		}
		ଯଦି (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			ଫେରନ୍ତୁ IPI_CPU_MASK(i);
		}
#endif

		ଯଦି ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	ଫେରନ୍ତୁ (atomic_val_t)ipi_mask;
}

ଶୂନ୍ୟ signal_pending_ipi(ଶୂନ୍ୟ)
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

#ଯଦି defined(CONFIG_SCHED_IPI_SUPPORTED)
	ଯଦି (arch_ଅଙ୍କ_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		ଯଦି (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#ଅନ୍ୟଥା
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ସ୍ଥିର ସଂରଚନା k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	ଫେରନ୍ତୁ (work == ଖାଲି) ? ଖାଲି
			      : CONTAINER_OF(work, ସଂରଚନା k_ipi_work, ଗଣ୍ଠି[cpu_id]);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_ipi_work_add(ସଂରଚନା k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ଖାଲି, "");
	__ASSERT(func != ଖାଲି, "");

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	ଯଦି (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, ଚାବି);
		ଫେରନ୍ତୁ -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	ପାଇଁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା id = 0; id < arch_ଅଙ୍କ_cpus(); id++) {
		ଯଦି ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->ଗଣ୍ଠି[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_ipi_work_wait(ସଂରଚନା k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	ଫେରନ୍ତୁ (rv == 0) ? -EAGAIN : 0;
}

ଶୂନ୍ୟ k_ipi_work_signal(ଶୂନ୍ୟ)
{
	signal_pending_ipi();
}

ସ୍ଥିର ଶୂନ୍ୟ ipi_work_process(sys_dlist_t *list, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_id)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&ipi_lock);

	ପାଇଁ (ସଂରଚନା k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != ଖାଲି; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->ଗଣ୍ଠି[cpu_id]);
		k_spin_unlock(&ipi_lock, ଚାବି);

		work->func(work);

		ଚାବି = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, ଚାବି);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

ଶୂନ୍ୟ z_sched_ipi(ଶୂନ୍ୟ)
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
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
