/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <kswap.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <ipi.h>

#यदि defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#अन्तर्भाव <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
स्थिर संरचना k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
बाह्य शून्य z_trace_sched_ipi(शून्य);
#endif

#यदि defined(CONFIG_IPI_OPTIMIZE_IDLE)
स्थिर bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#यदि defined(CONFIG_PM)
	फर्काऊ pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#अन्यथा
	ARG_UNUSED(cpu);
	फर्काऊ true;
#endif
}

स्थिर अन्तःस्थ पूर्णाङ्क sched_ipi_thread_reservation(अचर संरचना k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	जबसम्म (reservations != 0U) {
		अचिन्हित पूर्णाङ्क cpu = (अचिन्हित पूर्णाङ्क)u32_गन्ती_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		यदि (_kernel.sched_ipi_target[cpu] == thread) {
			फर्काऊ (पूर्णाङ्क)cpu;
		}
	}

	फर्काऊ -1;
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
	पूर्णाङ्क cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	यदि (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(संरचना k_thread *old_thread,
			    संरचना k_thread *new_thread)
{
	पूर्णाङ्क old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	यदि (old_cpu < 0) {
		फर्काऊ false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#यदि defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	यदि (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		फर्काऊ false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	फर्काऊ true;
}

संरचना k_thread *ipi_idle_reserved_take(शून्य)
{
	uint32_t cpu = _current_cpu->id;
	संरचना k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != रिक्त) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	यदि (thread != रिक्त) {
		sched_ipi_idle_unreserve(cpu);
	}

	फर्काऊ thread;
}
#endif

शून्य flag_ipi(uint32_t ipi_mask)
{
#यदि defined(CONFIG_SCHED_IPI_SUPPORTED)
	यदि (arch_अङ्क_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(संरचना k_thread *thread)
{
	यदि (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		फर्काऊ (CONFIG_MP_MAX_अङ्क_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  अङ्क_cpus = (uint32_t)arch_अङ्क_cpus();
	uint32_t  id = _current_cpu->id;
	संरचना k_thread *cpu_thread;

#यदि defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	यदि (sched_ipi_thread_reservation(thread) >= 0) {
		फर्काऊ 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	संरचना k_thread *last_thread = _kernel.cpus[last].current;

	यदि (last != id &&
	    last_thread != रिक्त &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#यदि defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		फर्काऊ IPI_CPU_MASK(last);
	}

#endif

	लागि (uint32_t i = 0; i < अङ्क_cpus; i++) {
		यदि (id == i) {
			जारी;
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

#यदि defined(CONFIG_SCHED_CPU_MASK)
		यदि ((thread->base.cpu_mask & BIT(i)) == 0) {
			जारी;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		यदि (cpu_thread == रिक्त) {
			जारी;
		}

#यदि defined(CONFIG_IPI_OPTIMIZE_IDLE)
		यदि ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			जारी;
		}
		यदि (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			फर्काऊ IPI_CPU_MASK(i);
		}
#endif

		यदि ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	फर्काऊ (atomic_val_t)ipi_mask;
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

#यदि defined(CONFIG_SCHED_IPI_SUPPORTED)
	यदि (arch_अङ्क_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		यदि (cpu_bitmap != 0) {
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
स्थिर संरचना k_ipi_work *first_ipi_work(sys_dlist_t *list, अचिन्हित पूर्णाङ्क cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	फर्काऊ (work == रिक्त) ? रिक्त
			      : CONTAINER_OF(work, संरचना k_ipi_work, गाँठो[cpu_id]);
}

पूर्णाङ्क k_ipi_work_add(संरचना k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != रिक्त, "");
	__ASSERT(func != रिक्त, "");

	k_spinlock_साँचो_t साँचो = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	यदि (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, साँचो);
		फर्काऊ -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	अचिन्हित पूर्णाङ्क cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	लागि (अचिन्हित पूर्णाङ्क id = 0; id < arch_अङ्क_cpus(); id++) {
		यदि ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->गाँठो[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, साँचो);

	फर्काऊ 0;
}

पूर्णाङ्क k_ipi_work_wait(संरचना k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	फर्काऊ (rv == 0) ? -EAGAIN : 0;
}

शून्य k_ipi_work_signal(शून्य)
{
	signal_pending_ipi();
}

स्थिर शून्य ipi_work_process(sys_dlist_t *list, अचिन्हित पूर्णाङ्क cpu_id)
{
	k_spinlock_साँचो_t साँचो = k_spin_lock(&ipi_lock);

	लागि (संरचना k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != रिक्त; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->गाँठो[cpu_id]);
		k_spin_unlock(&ipi_lock, साँचो);

		work->func(work);

		साँचो = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, साँचो);
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
	अचिन्हित पूर्णाङ्क cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
