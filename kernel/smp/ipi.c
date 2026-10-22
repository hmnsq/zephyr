/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/sys/math_extras.h>
#অন্তর্ভাব <kswap.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <ipi.h>

#যদি defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#অন্তর্ভাব <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
স্থির কাঠামো k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
বাহ্যিক শূন্য z_trace_sched_ipi(শূন্য);
#endif

#যদি defined(CONFIG_IPI_OPTIMIZE_IDLE)
স্থির bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#যদি defined(CONFIG_PM)
	ফেরত pm_state_পরবর্তী_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#অন্যথা
	ARG_UNUSED(cpu);
	ফেরত true;
#endif
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা sched_ipi_thread_reservation(ধ্রুবক কাঠামো k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	যখন (reservations != 0U) {
		unsigned পূর্ণসংখ্যা cpu = (unsigned পূর্ণসংখ্যা)u32_গুনতি_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		যদি (_kernel.sched_ipi_target[cpu] == thread) {
			ফেরত (পূর্ণসংখ্যা)cpu;
		}
	}

	ফেরত -1;
}

স্থির শূন্য sched_ipi_idle_reserve(uint32_t cpu, কাঠামো k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == ফাঁকা);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

স্থির শূন্য sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = ফাঁকা;
	_kernel.sched_ipi_reserved &= ~bit;
}

শূন্য ipi_idle_thread_unreserve(কাঠামো k_thread *thread)
{
	পূর্ণসংখ্যা cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	যদি (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(কাঠামো k_thread *old_thread,
			    কাঠামো k_thread *new_thread)
{
	পূর্ণসংখ্যা old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	যদি (old_cpu < 0) {
		ফেরত false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#যদি defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	যদি (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		ফেরত false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	ফেরত true;
}

কাঠামো k_thread *ipi_idle_reserved_take(শূন্য)
{
	uint32_t cpu = _current_cpu->id;
	কাঠামো k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != ফাঁকা) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	যদি (thread != ফাঁকা) {
		sched_ipi_idle_unreserve(cpu);
	}

	ফেরত thread;
}
#endif

শূন্য flag_ipi(uint32_t ipi_mask)
{
#যদি defined(CONFIG_SCHED_IPI_SUPPORTED)
	যদি (arch_অঙ্ক_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(কাঠামো k_thread *thread)
{
	যদি (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		ফেরত (CONFIG_MP_MAX_অঙ্ক_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  অঙ্ক_cpus = (uint32_t)arch_অঙ্ক_cpus();
	uint32_t  id = _current_cpu->id;
	কাঠামো k_thread *cpu_thread;

#যদি defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	যদি (sched_ipi_thread_reservation(thread) >= 0) {
		ফেরত 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	কাঠামো k_thread *last_thread = _kernel.cpus[last].current;

	যদি (last != id &&
	    last_thread != ফাঁকা &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#যদি defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		ফেরত IPI_CPU_MASK(last);
	}

#endif

	জন্য (uint32_t i = 0; i < অঙ্ক_cpus; i++) {
		যদি (id == i) {
			চালু;
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

#যদি defined(CONFIG_SCHED_CPU_MASK)
		যদি ((thread->base.cpu_mask & BIT(i)) == 0) {
			চালু;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		যদি (cpu_thread == ফাঁকা) {
			চালু;
		}

#যদি defined(CONFIG_IPI_OPTIMIZE_IDLE)
		যদি ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			চালু;
		}
		যদি (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			ফেরত IPI_CPU_MASK(i);
		}
#endif

		যদি ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	ফেরত (atomic_val_t)ipi_mask;
}

শূন্য signal_pending_ipi(শূন্য)
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

#যদি defined(CONFIG_SCHED_IPI_SUPPORTED)
	যদি (arch_অঙ্ক_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		যদি (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#অন্যথা
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
স্থির কাঠামো k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned পূর্ণসংখ্যা cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_শীর্ষ(list);

	ফেরত (work == ফাঁকা) ? ফাঁকা
			      : CONTAINER_OF(work, কাঠামো k_ipi_work, গ্রন্থি[cpu_id]);
}

পূর্ণসংখ্যা k_ipi_work_add(কাঠামো k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ফাঁকা, "");
	__ASSERT(func != ফাঁকা, "");

	k_spinlock_চাবি_t চাবি = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	যদি (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, চাবি);
		ফেরত -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned পূর্ণসংখ্যা cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	জন্য (unsigned পূর্ণসংখ্যা id = 0; id < arch_অঙ্ক_cpus(); id++) {
		যদি ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->গ্রন্থি[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, চাবি);

	ফেরত 0;
}

পূর্ণসংখ্যা k_ipi_work_wait(কাঠামো k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	ফেরত (rv == 0) ? -EAGAIN : 0;
}

শূন্য k_ipi_work_signal(শূন্য)
{
	signal_pending_ipi();
}

স্থির শূন্য ipi_work_process(sys_dlist_t *list, unsigned পূর্ণসংখ্যা cpu_id)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&ipi_lock);

	জন্য (কাঠামো k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != ফাঁকা; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->গ্রন্থি[cpu_id]);
		k_spin_unlock(&ipi_lock, চাবি);

		work->func(work);

		চাবি = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, চাবি);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

শূন্য z_sched_ipi(শূন্য)
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
	unsigned পূর্ণসংখ্যা cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
