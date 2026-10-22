/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <kswap.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <ipi.h>

#જો defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#અન્તર્ભાવ <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
સ્થિર સંરચના k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
બાહ્ય શૂન્ય z_trace_sched_ipi(શૂન્ય);
#endif

#જો defined(CONFIG_IPI_OPTIMIZE_IDLE)
સ્થિર bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#જો defined(CONFIG_PM)
	પાછા pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#નહીંતર
	ARG_UNUSED(cpu);
	પાછા true;
#endif
}

સ્થિર અંતઃરેખ પૂર્ણાંક sched_ipi_thread_reservation(અચલ સંરચના k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	જ્યારે (reservations != 0U) {
		unsigned પૂર્ણાંક cpu = (unsigned પૂર્ણાંક)u32_ગણતરી_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		જો (_kernel.sched_ipi_target[cpu] == thread) {
			પાછા (પૂર્ણાંક)cpu;
		}
	}

	પાછા -1;
}

સ્થિર શૂન્ય sched_ipi_idle_reserve(uint32_t cpu, સંરચના k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == ખાલી);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

સ્થિર શૂન્ય sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = ખાલી;
	_kernel.sched_ipi_reserved &= ~bit;
}

શૂન્ય ipi_idle_thread_unreserve(સંરચના k_thread *thread)
{
	પૂર્ણાંક cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	જો (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(સંરચના k_thread *old_thread,
			    સંરચના k_thread *new_thread)
{
	પૂર્ણાંક old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	જો (old_cpu < 0) {
		પાછા false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#જો defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	જો (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		પાછા false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	પાછા true;
}

સંરચના k_thread *ipi_idle_reserved_take(શૂન્ય)
{
	uint32_t cpu = _current_cpu->id;
	સંરચના k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != ખાલી) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	જો (thread != ખાલી) {
		sched_ipi_idle_unreserve(cpu);
	}

	પાછા thread;
}
#endif

શૂન્ય flag_ipi(uint32_t ipi_mask)
{
#જો defined(CONFIG_SCHED_IPI_SUPPORTED)
	જો (arch_અંક_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(સંરચના k_thread *thread)
{
	જો (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		પાછા (CONFIG_MP_MAX_અંક_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  અંક_cpus = (uint32_t)arch_અંક_cpus();
	uint32_t  id = _current_cpu->id;
	સંરચના k_thread *cpu_thread;

#જો defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	જો (sched_ipi_thread_reservation(thread) >= 0) {
		પાછા 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	સંરચના k_thread *last_thread = _kernel.cpus[last].current;

	જો (last != id &&
	    last_thread != ખાલી &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#જો defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		પાછા IPI_CPU_MASK(last);
	}

#endif

	માટે (uint32_t i = 0; i < અંક_cpus; i++) {
		જો (id == i) {
			ચાલુ;
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

#જો defined(CONFIG_SCHED_CPU_MASK)
		જો ((thread->base.cpu_mask & BIT(i)) == 0) {
			ચાલુ;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		જો (cpu_thread == ખાલી) {
			ચાલુ;
		}

#જો defined(CONFIG_IPI_OPTIMIZE_IDLE)
		જો ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			ચાલુ;
		}
		જો (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			પાછા IPI_CPU_MASK(i);
		}
#endif

		જો ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	પાછા (atomic_val_t)ipi_mask;
}

શૂન્ય signal_pending_ipi(શૂન્ય)
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

#જો defined(CONFIG_SCHED_IPI_SUPPORTED)
	જો (arch_અંક_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		જો (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#નહીંતર
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
સ્થિર સંરચના k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned પૂર્ણાંક cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	પાછા (work == ખાલી) ? ખાલી
			      : CONTAINER_OF(work, સંરચના k_ipi_work, ગાંઠ[cpu_id]);
}

પૂર્ણાંક k_ipi_work_add(સંરચના k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ખાલી, "");
	__ASSERT(func != ખાલી, "");

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	જો (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, ચાવી);
		પાછા -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned પૂર્ણાંક cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	માટે (unsigned પૂર્ણાંક id = 0; id < arch_અંક_cpus(); id++) {
		જો ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->ગાંઠ[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, ચાવી);

	પાછા 0;
}

પૂર્ણાંક k_ipi_work_wait(સંરચના k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	પાછા (rv == 0) ? -EAGAIN : 0;
}

શૂન્ય k_ipi_work_signal(શૂન્ય)
{
	signal_pending_ipi();
}

સ્થિર શૂન્ય ipi_work_process(sys_dlist_t *list, unsigned પૂર્ણાંક cpu_id)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&ipi_lock);

	માટે (સંરચના k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != ખાલી; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->ગાંઠ[cpu_id]);
		k_spin_unlock(&ipi_lock, ચાવી);

		work->func(work);

		ચાવી = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, ચાવી);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

શૂન્ય z_sched_ipi(શૂન્ય)
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
	unsigned પૂર્ણાંક cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
