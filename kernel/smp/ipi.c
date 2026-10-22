/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kswap.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ipi.h>

#ਜੇ defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/pm/pm.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_IPI_SUPPORTED
ਸਥਿਰ ਬਣਤਰ k_spinlock ipi_ਤਾਲਾ;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TRACE_SCHED_IPI
ਬਾਹਰੀ ਖਾਲੀ z_trace_sched_ipi(ਖਾਲੀ);
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_IPI_OPTIMIZE_IDLE)
ਸਥਿਰ bool sched_ipi_idle_cpu_ਸਰਗਰਮ(uint32_t cpu)
{
#ਜੇ defined(CONFIG_PM)
	ਵਾਪਸ pm_state_ਅਗਲਾ_get((uint8_t)cpu)->state == PM_STATE_ਸਰਗਰਮ;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(cpu);
	ਵਾਪਸ true;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ sched_ipi_thread_reservation(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	ਜਦੋਂ (reservations != 0U) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)u32_ਗਿਣਤੀ_trailing_zeros(reservations);

		reservations &= ~ਦੋਆਧਾਰੀ_ਅੰਕ(cpu);
		ਜੇ (_kernel.sched_ipi_ਨਿਸ਼ਾਨਾ[cpu] == thread) {
			ਵਾਪਸ (ਪੂਰਨ)cpu;
		}
	}

	ਵਾਪਸ -1;
}

ਸਥਿਰ ਖਾਲੀ sched_ipi_idle_reserve(uint32_t cpu, ਬਣਤਰ k_thread *thread)
{
	uint32_t ਦੋਆਧਾਰੀ_ਅੰਕ_2 = ਦੋਆਧਾਰੀ_ਅੰਕ(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & ਦੋਆਧਾਰੀ_ਅੰਕ_2) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_ਨਿਸ਼ਾਨਾ[cpu] == ਨਿਰਾ);

	_kernel.sched_ipi_ਨਿਸ਼ਾਨਾ[cpu] = thread;
	_kernel.sched_ipi_reserved |= ਦੋਆਧਾਰੀ_ਅੰਕ_2;
}

ਸਥਿਰ ਖਾਲੀ sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t ਦੋਆਧਾਰੀ_ਅੰਕ_2 = ਦੋਆਧਾਰੀ_ਅੰਕ(cpu);

	_kernel.sched_ipi_ਨਿਸ਼ਾਨਾ[cpu] = ਨਿਰਾ;
	_kernel.sched_ipi_reserved &= ~ਦੋਆਧਾਰੀ_ਅੰਕ_2;
}

ਖਾਲੀ ipi_idle_thread_unreserve(ਬਣਤਰ k_thread *thread)
{
	ਪੂਰਨ cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	ਜੇ (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(ਬਣਤਰ k_thread *old_thread,
			    ਬਣਤਰ k_thread *new_thread)
{
	ਪੂਰਨ old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	ਜੇ (old_cpu < 0) {
		ਵਾਪਸ false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_ਸਰਗਰਮ(cpu);

#ਜੇ defined(CONFIG_SCHED_CPU_ਨਕਾਬ)
	eligible = eligible && ((new_thread->ਅਧਾਰ.cpu_ਨਕਾਬ & ਦੋਆਧਾਰੀ_ਅੰਕ(cpu)) != 0U);
#ਜੇ_ਅੰਤ

	ਜੇ (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		ਵਾਪਸ false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_ਨਿਸ਼ਾਨਾ[cpu] = new_thread;
	ਵਾਪਸ true;
}

ਬਣਤਰ k_thread *ipi_idle_reserved_take(ਖਾਲੀ)
{
	uint32_t cpu = _current_cpu->id;
	ਬਣਤਰ k_thread *thread = _kernel.sched_ipi_ਨਿਸ਼ਾਨਾ[cpu];

	__ASSERT_NO_MSG((thread != ਨਿਰਾ) ==
			((_kernel.sched_ipi_reserved & ਦੋਆਧਾਰੀ_ਅੰਕ(cpu)) != 0U));

	ਜੇ (thread != ਨਿਰਾ) {
		sched_ipi_idle_unreserve(cpu);
	}

	ਵਾਪਸ thread;
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ flag_ipi(uint32_t ipi_ਨਕਾਬ)
{
#ਜੇ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ਜੇ (arch_ਅੰਕ_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_ਨਕਾਬ);
	}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_ਨਕਾਬ_create(ਬਣਤਰ k_thread *thread)
{
	ਜੇ (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		ਵਾਪਸ (CONFIG_MP_MAX_ਅੰਕ_CPUS > 1) ? IPI_ALL_CPUS_ਨਕਾਬ : 0;
	}

	uint32_t  ipi_ਨਕਾਬ = 0;
	uint32_t  ਅੰਕ_cpus = (uint32_t)arch_ਅੰਕ_cpus();
	uint32_t  id = _current_cpu->id;
	ਬਣਤਰ k_thread *cpu_thread;

#ਜੇ defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	ਜੇ (sched_ipi_thread_reservation(thread) >= 0) {
		ਵਾਪਸ 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t ਆਖਰੀ = thread->ਅਧਾਰ.cpu;
	ਬਣਤਰ k_thread *ਆਖਰੀ_thread = _kernel.cpus[ਆਖਰੀ].current;

	ਜੇ (ਆਖਰੀ != id &&
	    ਆਖਰੀ_thread != ਨਿਰਾ &&
	    z_is_idle_thread_object(ਆਖਰੀ_thread) &&
	    sched_ipi_idle_cpu_ਸਰਗਰਮ(ਆਖਰੀ) &&
#ਜੇ defined(CONFIG_SCHED_CPU_ਨਕਾਬ)
	    (thread->ਅਧਾਰ.cpu_ਨਕਾਬ & ਦੋਆਧਾਰੀ_ਅੰਕ(ਆਖਰੀ)) != 0 &&
#ਜੇ_ਅੰਤ
	    (_kernel.sched_ipi_reserved & ਦੋਆਧਾਰੀ_ਅੰਕ(ਆਖਰੀ)) == 0U) {
		sched_ipi_idle_reserve(ਆਖਰੀ, thread);
		ਵਾਪਸ IPI_CPU_ਨਕਾਬ(ਆਖਰੀ);
	}

#ਜੇ_ਅੰਤ

	ਲਈ (uint32_t i = 0; i < ਅੰਕ_cpus; i++) {
		ਜੇ (id == i) {
			ਜਾਰੀ;
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

#ਜੇ defined(CONFIG_SCHED_CPU_ਨਕਾਬ)
		ਜੇ ((thread->ਅਧਾਰ.cpu_ਨਕਾਬ & ਦੋਆਧਾਰੀ_ਅੰਕ(i)) == 0) {
			ਜਾਰੀ;
		}
#ਜੇ_ਅੰਤ

		cpu_thread = _kernel.cpus[i].current;

		ਜੇ (cpu_thread == ਨਿਰਾ) {
			ਜਾਰੀ;
		}

#ਜੇ defined(CONFIG_IPI_OPTIMIZE_IDLE)
		ਜੇ ((_kernel.sched_ipi_reserved & ਦੋਆਧਾਰੀ_ਅੰਕ(i)) != 0U) {
			ਜਾਰੀ;
		}
		ਜੇ (i != (uint32_t)thread->ਅਧਾਰ.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_ਸਰਗਰਮ(i)) {
			sched_ipi_idle_reserve(i, thread);
			ਵਾਪਸ IPI_CPU_ਨਕਾਬ(i);
		}
#ਜੇ_ਅੰਤ

		ਜੇ ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_ਨਕਾਬ |= ਦੋਆਧਾਰੀ_ਅੰਕ(i);
		}
	}

	ਵਾਪਸ (atomic_val_t)ipi_ਨਕਾਬ;
}

ਖਾਲੀ signal_pending_ipi(ਖਾਲੀ)
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

#ਜੇ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ਜੇ (arch_ਅੰਕ_cpus() > 1) {
		uint32_t self_ਦੋਆਧਾਰੀ_ਅੰਕ = ਦੋਆਧਾਰੀ_ਅੰਕ(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_ਦੋਆਧਾਰੀ_ਅੰਕ);
		cpu_bitmap &= ~self_ਦੋਆਧਾਰੀ_ਅੰਕ;
		ਜੇ (cpu_bitmap != 0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#ਨਹੀਂਤਾਂ
			arch_sched_ਪ੍ਰਸਾਰਣ_ipi();
#ਜੇ_ਅੰਤ
		}
	}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_IPI_SUPPORTED
ਸਥਿਰ ਬਣਤਰ k_ipi_work *ਪਹਿਲਾ_ipi_work(sys_dlist_t *list, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_ਸਿਰ(list);

	ਵਾਪਸ (work == ਨਿਰਾ) ? ਨਿਰਾ
			      : CONTAINER_ਦਾ(work, ਬਣਤਰ k_ipi_work, ਗੰਢ[cpu_id]);
}

ਪੂਰਨ k_ipi_work_ਜੋੜੋ(ਬਣਤਰ k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ਨਿਰਾ, "");
	__ASSERT(func != ਨਿਰਾ, "");

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ipi_ਤਾਲਾ);

	/* Verify the IPI work item is not currently in use */

	ਜੇ (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_ਨਕਾਬ & ~ਦੋਆਧਾਰੀ_ਅੰਕ(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_ਨਕਾਬ);
	work->func = func;
	work->bitmask = cpu_bitmask;

	ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ id = 0; id < arch_ਅੰਕ_cpus(); id++) {
		ਜੇ ((cpu_bitmask & ਦੋਆਧਾਰੀ_ਅੰਕ(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->ਗੰਢ[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_ipi_work_wait(ਬਣਤਰ k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	ਵਾਪਸ (rv == 0) ? -EAGAIN : 0;
}

ਖਾਲੀ k_ipi_work_signal(ਖਾਲੀ)
{
	signal_pending_ipi();
}

ਸਥਿਰ ਖਾਲੀ ipi_work_process(sys_dlist_t *list, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu_id)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ipi_ਤਾਲਾ);

	ਲਈ (ਬਣਤਰ k_ipi_work *work = ਪਹਿਲਾ_ipi_work(list, cpu_id);
	     work != ਨਿਰਾ; work = ਪਹਿਲਾ_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->ਗੰਢ[cpu_id]);
		k_spin_unlock(&ipi_ਤਾਲਾ, ਕੁੰਜੀ);

		work->func(work);

		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&ipi_ਤਾਲਾ);
		k_event_post(&work->event, ਦੋਆਧਾਰੀ_ਅੰਕ(cpu_id));
	}

	k_spin_unlock(&ipi_ਤਾਲਾ, ਕੁੰਜੀ);
}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_IPI_SUPPORTED */

ਖਾਲੀ z_sched_ipi(ਖਾਲੀ)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#ਜੇ_ਅੰਤ /* CONFIG_TRACE_SCHED_IPI */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICING
	z_ਸਮਾਂ_ਟੁਕੜਾ();
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICING */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_IPI_SUPPORTED
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#ਜੇ_ਅੰਤ
}
