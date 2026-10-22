/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <kswap.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <ipi.h>

#जर defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#अन्तर्भाव <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
स्थिर संरचना k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
बाह्य शून्य z_trace_sched_ipi(शून्य);
#endif

#जर defined(CONFIG_IPI_OPTIMIZE_IDLE)
स्थिर bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#जर defined(CONFIG_PM)
	परत pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#अन्यथा
	ARG_UNUSED(cpu);
	परत true;
#endif
}

स्थिर ओळीत पूर्णांक sched_ipi_thread_reservation(स्थिरांक संरचना k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	पर्यंत (reservations != 0U) {
		अचिन्हित पूर्णांक cpu = (अचिन्हित पूर्णांक)u32_मोजणी_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		जर (_kernel.sched_ipi_target[cpu] == thread) {
			परत (पूर्णांक)cpu;
		}
	}

	परत -1;
}

स्थिर शून्य sched_ipi_idle_reserve(uint32_t cpu, संरचना k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == रिक्त);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

स्थिर शून्य sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = रिक्त;
	_kernel.sched_ipi_reserved &= ~bit;
}

शून्य ipi_idle_thread_unreserve(संरचना k_thread *thread)
{
	पूर्णांक cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	जर (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(संरचना k_thread *old_thread,
			    संरचना k_thread *new_thread)
{
	पूर्णांक old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	जर (old_cpu < 0) {
		परत false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#जर defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	जर (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		परत false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	परत true;
}

संरचना k_thread *ipi_idle_reserved_take(शून्य)
{
	uint32_t cpu = _current_cpu->id;
	संरचना k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != रिक्त) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	जर (thread != रिक्त) {
		sched_ipi_idle_unreserve(cpu);
	}

	परत thread;
}
#endif

शून्य flag_ipi(uint32_t ipi_mask)
{
#जर defined(CONFIG_SCHED_IPI_SUPPORTED)
	जर (arch_अंक_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(संरचना k_thread *thread)
{
	जर (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		परत (CONFIG_MP_MAX_अंक_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  अंक_cpus = (uint32_t)arch_अंक_cpus();
	uint32_t  id = _current_cpu->id;
	संरचना k_thread *cpu_thread;

#जर defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	जर (sched_ipi_thread_reservation(thread) >= 0) {
		परत 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	संरचना k_thread *last_thread = _kernel.cpus[last].current;

	जर (last != id &&
	    last_thread != रिक्त &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#जर defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		परत IPI_CPU_MASK(last);
	}

#endif

	साठी (uint32_t i = 0; i < अंक_cpus; i++) {
		जर (id == i) {
			सुरूठेवा;
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

#जर defined(CONFIG_SCHED_CPU_MASK)
		जर ((thread->base.cpu_mask & BIT(i)) == 0) {
			सुरूठेवा;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		जर (cpu_thread == रिक्त) {
			सुरूठेवा;
		}

#जर defined(CONFIG_IPI_OPTIMIZE_IDLE)
		जर ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			सुरूठेवा;
		}
		जर (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			परत IPI_CPU_MASK(i);
		}
#endif

		जर ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	परत (atomic_val_t)ipi_mask;
}

शून्य signal_pending_ipi(शून्य)
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

#जर defined(CONFIG_SCHED_IPI_SUPPORTED)
	जर (arch_अंक_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		जर (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#अन्यथा
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
स्थिर संरचना k_ipi_work *first_ipi_work(sys_dlist_t *list, अचिन्हित पूर्णांक cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	परत (work == रिक्त) ? रिक्त
			      : CONTAINER_OF(work, संरचना k_ipi_work, गाठ[cpu_id]);
}

पूर्णांक k_ipi_work_add(संरचना k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != रिक्त, "");
	__ASSERT(func != रिक्त, "");

	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	जर (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, किल्ली);
		परत -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	अचिन्हित पूर्णांक cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	साठी (अचिन्हित पूर्णांक id = 0; id < arch_अंक_cpus(); id++) {
		जर ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->गाठ[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, किल्ली);

	परत 0;
}

पूर्णांक k_ipi_work_wait(संरचना k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	परत (rv == 0) ? -EAGAIN : 0;
}

शून्य k_ipi_work_signal(शून्य)
{
	signal_pending_ipi();
}

स्थिर शून्य ipi_work_process(sys_dlist_t *list, अचिन्हित पूर्णांक cpu_id)
{
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&ipi_lock);

	साठी (संरचना k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != रिक्त; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->गाठ[cpu_id]);
		k_spin_unlock(&ipi_lock, किल्ली);

		work->func(work);

		किल्ली = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, किल्ली);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

शून्य z_sched_ipi(शून्य)
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
	अचिन्हित पूर्णांक cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
