/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <kswap.h>
#ضمن <ksched.h>
#ضمن <ipi.h>

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#ضمن <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ساكن بنية k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
خارجي فراغ z_trace_sched_ipi(فراغ);
#endif

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
ساكن bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#إذا defined(CONFIG_PM)
	أرجع pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#وإلا
	ARG_UNUSED(cpu);
	أرجع true;
#endif
}

ساكن مضمن صحيح sched_ipi_thread_reservation(ثابت بنية k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	طالما (reservations != 0U) {
		unsigned صحيح cpu = (unsigned صحيح)u32_گنتی_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		إذا (_kernel.sched_ipi_target[cpu] == thread) {
			أرجع (صحيح)cpu;
		}
	}

	أرجع -1;
}

ساكن فراغ sched_ipi_idle_reserve(uint32_t cpu, بنية k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == عدم);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

ساكن فراغ sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = عدم;
	_kernel.sched_ipi_reserved &= ~bit;
}

فراغ ipi_idle_thread_unreserve(بنية k_thread *thread)
{
	صحيح cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	إذا (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(بنية k_thread *old_thread,
			    بنية k_thread *new_thread)
{
	صحيح old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	إذا (old_cpu < 0) {
		أرجع false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#إذا defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	إذا (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		أرجع false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	أرجع true;
}

بنية k_thread *ipi_idle_reserved_take(فراغ)
{
	uint32_t cpu = _current_cpu->id;
	بنية k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != عدم) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	إذا (thread != عدم) {
		sched_ipi_idle_unreserve(cpu);
	}

	أرجع thread;
}
#endif

فراغ flag_ipi(uint32_t ipi_mask)
{
#إذا defined(CONFIG_SCHED_IPI_SUPPORTED)
	إذا (arch_عدد_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(بنية k_thread *thread)
{
	إذا (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		أرجع (CONFIG_MP_MAX_عدد_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  عدد_cpus = (uint32_t)arch_عدد_cpus();
	uint32_t  id = _current_cpu->id;
	بنية k_thread *cpu_thread;

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	إذا (sched_ipi_thread_reservation(thread) >= 0) {
		أرجع 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	بنية k_thread *last_thread = _kernel.cpus[last].current;

	إذا (last != id &&
	    last_thread != عدم &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#إذا defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		أرجع IPI_CPU_MASK(last);
	}

#endif

	لكل (uint32_t i = 0; i < عدد_cpus; i++) {
		إذا (id == i) {
			استمر;
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

#إذا defined(CONFIG_SCHED_CPU_MASK)
		إذا ((thread->base.cpu_mask & BIT(i)) == 0) {
			استمر;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		إذا (cpu_thread == عدم) {
			استمر;
		}

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
		إذا ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			استمر;
		}
		إذا (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			أرجع IPI_CPU_MASK(i);
		}
#endif

		إذا ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	أرجع (atomic_val_t)ipi_mask;
}

فراغ signal_pending_ipi(فراغ)
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

#إذا defined(CONFIG_SCHED_IPI_SUPPORTED)
	إذا (arch_عدد_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		إذا (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#وإلا
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ساكن بنية k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned صحيح cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	أرجع (work == عدم) ? عدم
			      : CONTAINER_OF(work, بنية k_ipi_work, گرہ[cpu_id]);
}

صحيح k_ipi_work_add(بنية k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != عدم, "");
	__ASSERT(func != عدم, "");

	k_spinlock_کلید_t کلید = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	إذا (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, کلید);
		أرجع -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned صحيح cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	لكل (unsigned صحيح id = 0; id < arch_عدد_cpus(); id++) {
		إذا ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->گرہ[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, کلید);

	أرجع 0;
}

صحيح k_ipi_work_wait(بنية k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	أرجع (rv == 0) ? -EAGAIN : 0;
}

فراغ k_ipi_work_signal(فراغ)
{
	signal_pending_ipi();
}

ساكن فراغ ipi_work_process(sys_dlist_t *list, unsigned صحيح cpu_id)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&ipi_lock);

	لكل (بنية k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != عدم; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->گرہ[cpu_id]);
		k_spin_unlock(&ipi_lock, کلید);

		work->func(work);

		کلید = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, کلید);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

فراغ z_sched_ipi(فراغ)
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
	unsigned صحيح cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
