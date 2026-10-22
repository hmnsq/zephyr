/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <kswap.h>
#समावेश <ksched.h>
#समावेश <ipi.h>

#अगर defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#समावेश <zephyr/pm/pm.h>
#पूर्ण_अगर

#अगर_घोषित CONFIG_SCHED_IPI_SUPPORTED
अटल काष्ठा k_spinlock ipi_lock;
#पूर्ण_अगर

#अगर_घोषित CONFIG_TRACE_SCHED_IPI
बाह्य व्योम z_trace_sched_ipi(व्योम);
#पूर्ण_अगर

#अगर defined(CONFIG_IPI_OPTIMIZE_IDLE)
अटल bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#अगर defined(CONFIG_PM)
	वापस pm_state_अग्र_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#अन्यथा
	ARG_UNUSED(cpu);
	वापस true;
#पूर्ण_अगर
}

अटल अंतरभूत पूर्णांक sched_ipi_thread_reservation(स्थिर काष्ठा k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	जबतक (reservations != 0U) {
		अचिन्हित पूर्णांक cpu = (अचिन्हित पूर्णांक)u32_गिनती_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		अगर (_kernel.sched_ipi_target[cpu] == thread) {
			वापस (पूर्णांक)cpu;
		}
	}

	वापस -1;
}

अटल व्योम sched_ipi_idle_reserve(uint32_t cpu, काष्ठा k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == शून्य);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

अटल व्योम sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = शून्य;
	_kernel.sched_ipi_reserved &= ~bit;
}

व्योम ipi_idle_thread_unreserve(काष्ठा k_thread *thread)
{
	पूर्णांक cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	अगर (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(काष्ठा k_thread *old_thread,
			    काष्ठा k_thread *new_thread)
{
	पूर्णांक old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	अगर (old_cpu < 0) {
		वापस false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#अगर defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#पूर्ण_अगर

	अगर (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		वापस false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	वापस true;
}

काष्ठा k_thread *ipi_idle_reserved_take(व्योम)
{
	uint32_t cpu = _current_cpu->id;
	काष्ठा k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != शून्य) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	अगर (thread != शून्य) {
		sched_ipi_idle_unreserve(cpu);
	}

	वापस thread;
}
#पूर्ण_अगर

व्योम flag_ipi(uint32_t ipi_mask)
{
#अगर defined(CONFIG_SCHED_IPI_SUPPORTED)
	अगर (arch_अंक_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#पूर्ण_अगर /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(काष्ठा k_thread *thread)
{
	अगर (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		वापस (CONFIG_MP_MAX_अंक_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  अंक_cpus = (uint32_t)arch_अंक_cpus();
	uint32_t  id = _current_cpu->id;
	काष्ठा k_thread *cpu_thread;

#अगर defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	अगर (sched_ipi_thread_reservation(thread) >= 0) {
		वापस 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	काष्ठा k_thread *last_thread = _kernel.cpus[last].current;

	अगर (last != id &&
	    last_thread != शून्य &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#अगर defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#पूर्ण_अगर
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		वापस IPI_CPU_MASK(last);
	}

#पूर्ण_अगर

	क्रम (uint32_t i = 0; i < अंक_cpus; i++) {
		अगर (id == i) {
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

#अगर defined(CONFIG_SCHED_CPU_MASK)
		अगर ((thread->base.cpu_mask & BIT(i)) == 0) {
			जारी;
		}
#पूर्ण_अगर

		cpu_thread = _kernel.cpus[i].current;

		अगर (cpu_thread == शून्य) {
			जारी;
		}

#अगर defined(CONFIG_IPI_OPTIMIZE_IDLE)
		अगर ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			जारी;
		}
		अगर (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			वापस IPI_CPU_MASK(i);
		}
#पूर्ण_अगर

		अगर ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	वापस (atomic_val_t)ipi_mask;
}

व्योम signal_pending_ipi(व्योम)
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

#अगर defined(CONFIG_SCHED_IPI_SUPPORTED)
	अगर (arch_अंक_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		अगर (cpu_bitmap != 0) {
#अगर_घोषित CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#अन्यथा
			arch_sched_broadcast_ipi();
#पूर्ण_अगर
		}
	}
#पूर्ण_अगर /* CONFIG_SCHED_IPI_SUPPORTED */
}

#अगर_घोषित CONFIG_SCHED_IPI_SUPPORTED
अटल काष्ठा k_ipi_work *first_ipi_work(sys_dlist_t *list, अचिन्हित पूर्णांक cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_शीर्ष(list);

	वापस (work == शून्य) ? शून्य
			      : CONTAINER_OF(work, काष्ठा k_ipi_work, गाँठ[cpu_id]);
}

पूर्णांक k_ipi_work_add(काष्ठा k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != शून्य, "");
	__ASSERT(func != शून्य, "");

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	अगर (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, कुंजी);
		वापस -EBUSY;
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

	क्रम (अचिन्हित पूर्णांक id = 0; id < arch_अंक_cpus(); id++) {
		अगर ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->गाँठ[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, कुंजी);

	वापस 0;
}

पूर्णांक k_ipi_work_wait(काष्ठा k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	वापस (rv == 0) ? -EAGAIN : 0;
}

व्योम k_ipi_work_signal(व्योम)
{
	signal_pending_ipi();
}

अटल व्योम ipi_work_process(sys_dlist_t *list, अचिन्हित पूर्णांक cpu_id)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&ipi_lock);

	क्रम (काष्ठा k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != शून्य; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->गाँठ[cpu_id]);
		k_spin_unlock(&ipi_lock, कुंजी);

		work->func(work);

		कुंजी = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, कुंजी);
}
#पूर्ण_अगर /* CONFIG_SCHED_IPI_SUPPORTED */

व्योम z_sched_ipi(व्योम)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#अगर_घोषित CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#पूर्ण_अगर /* CONFIG_TRACE_SCHED_IPI */

#अगर_घोषित CONFIG_TIMESLICING
	z_time_slice();
#पूर्ण_अगर /* CONFIG_TIMESLICING */

#अगर_घोषित CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#पूर्ण_अगर

#अगर_घोषित CONFIG_SCHED_IPI_SUPPORTED
	अचिन्हित पूर्णांक cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#पूर्ण_अगर
}
