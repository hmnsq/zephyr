/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <kswap.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <ipi.h>

#எனில் defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#அந்தர்பாவ <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
நிலையான கட்டமைப்பு k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
வெளி வெறுமை z_trace_sched_ipi(வெறுமை);
#endif

#எனில் defined(CONFIG_IPI_OPTIMIZE_IDLE)
நிலையான bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#எனில் defined(CONFIG_PM)
	திரும்பு pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#இல்லையெனில்
	ARG_UNUSED(cpu);
	திரும்பு true;
#endif
}

நிலையான உள்வரி முழு sched_ipi_thread_reservation(மாறிலி கட்டமைப்பு k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	போது (reservations != 0U) {
		unsigned முழு cpu = (unsigned முழு)u32_மொத்தம்_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		எனில் (_kernel.sched_ipi_target[cpu] == thread) {
			திரும்பு (முழு)cpu;
		}
	}

	திரும்பு -1;
}

நிலையான வெறுமை sched_ipi_idle_reserve(uint32_t cpu, கட்டமைப்பு k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == வெற்று);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

நிலையான வெறுமை sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = வெற்று;
	_kernel.sched_ipi_reserved &= ~bit;
}

வெறுமை ipi_idle_thread_unreserve(கட்டமைப்பு k_thread *thread)
{
	முழு cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	எனில் (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(கட்டமைப்பு k_thread *old_thread,
			    கட்டமைப்பு k_thread *new_thread)
{
	முழு old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	எனில் (old_cpu < 0) {
		திரும்பு false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#எனில் defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	எனில் (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		திரும்பு false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	திரும்பு true;
}

கட்டமைப்பு k_thread *ipi_idle_reserved_take(வெறுமை)
{
	uint32_t cpu = _current_cpu->id;
	கட்டமைப்பு k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != வெற்று) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	எனில் (thread != வெற்று) {
		sched_ipi_idle_unreserve(cpu);
	}

	திரும்பு thread;
}
#endif

வெறுமை flag_ipi(uint32_t ipi_mask)
{
#எனில் defined(CONFIG_SCHED_IPI_SUPPORTED)
	எனில் (arch_இலக்கம்_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(கட்டமைப்பு k_thread *thread)
{
	எனில் (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		திரும்பு (CONFIG_MP_MAX_இலக்கம்_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  இலக்கம்_cpus = (uint32_t)arch_இலக்கம்_cpus();
	uint32_t  id = _current_cpu->id;
	கட்டமைப்பு k_thread *cpu_thread;

#எனில் defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	எனில் (sched_ipi_thread_reservation(thread) >= 0) {
		திரும்பு 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	கட்டமைப்பு k_thread *last_thread = _kernel.cpus[last].current;

	எனில் (last != id &&
	    last_thread != வெற்று &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#எனில் defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		திரும்பு IPI_CPU_MASK(last);
	}

#endif

	க்கு (uint32_t i = 0; i < இலக்கம்_cpus; i++) {
		எனில் (id == i) {
			தொடர்;
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

#எனில் defined(CONFIG_SCHED_CPU_MASK)
		எனில் ((thread->base.cpu_mask & BIT(i)) == 0) {
			தொடர்;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		எனில் (cpu_thread == வெற்று) {
			தொடர்;
		}

#எனில் defined(CONFIG_IPI_OPTIMIZE_IDLE)
		எனில் ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			தொடர்;
		}
		எனில் (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			திரும்பு IPI_CPU_MASK(i);
		}
#endif

		எனில் ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	திரும்பு (atomic_val_t)ipi_mask;
}

வெறுமை signal_pending_ipi(வெறுமை)
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

#எனில் defined(CONFIG_SCHED_IPI_SUPPORTED)
	எனில் (arch_இலக்கம்_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		எனில் (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#இல்லையெனில்
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
நிலையான கட்டமைப்பு k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned முழு cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	திரும்பு (work == வெற்று) ? வெற்று
			      : CONTAINER_OF(work, கட்டமைப்பு k_ipi_work, முனை[cpu_id]);
}

முழு k_ipi_work_add(கட்டமைப்பு k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != வெற்று, "");
	__ASSERT(func != வெற்று, "");

	k_spinlock_சாவி_t சாவி = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	எனில் (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, சாவி);
		திரும்பு -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned முழு cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	க்கு (unsigned முழு id = 0; id < arch_இலக்கம்_cpus(); id++) {
		எனில் ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->முனை[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, சாவி);

	திரும்பு 0;
}

முழு k_ipi_work_wait(கட்டமைப்பு k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	திரும்பு (rv == 0) ? -EAGAIN : 0;
}

வெறுமை k_ipi_work_signal(வெறுமை)
{
	signal_pending_ipi();
}

நிலையான வெறுமை ipi_work_process(sys_dlist_t *list, unsigned முழு cpu_id)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&ipi_lock);

	க்கு (கட்டமைப்பு k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != வெற்று; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->முனை[cpu_id]);
		k_spin_unlock(&ipi_lock, சாவி);

		work->func(work);

		சாவி = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, சாவி);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

வெறுமை z_sched_ipi(வெறுமை)
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
	unsigned முழு cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
