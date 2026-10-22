/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <kswap.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <ipi.h>

#ఒకవేళ defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#అన్తర్భావ <zephyr/pm/pm.h>
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SCHED_IPI_SUPPORTED
స్థిర నిర్మాణం k_spinlock ipi_తాళం;
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_TRACE_SCHED_IPI
బాహ్య శూన్య z_trace_sched_ipi(శూన్య);
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_IPI_OPTIMIZE_IDLE)
స్థిర bool sched_ipi_idle_cpu_చురుకైన(uint32_t cpu)
{
#ఒకవేళ defined(CONFIG_PM)
	తిరిగి pm_state_తదుపరి_get((uint8_t)cpu)->state == PM_STATE_చురుకైన;
#లేకపోతే
	ARG_వాడని(cpu);
	తిరిగి true;
#అయితే_ముగింపు
}

స్థిర అంతఃవరుస పూర్ణాంకం sched_ipi_thread_reservation(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	అయితే (reservations != 0U) {
		గుర్తులేని పూర్ణాంకం cpu = (గుర్తులేని పూర్ణాంకం)u32_లెక్క_trailing_zeros(reservations);

		reservations &= ~బిట్టు(cpu);
		ఒకవేళ (_kernel.sched_ipi_లక్ష్యం[cpu] == thread) {
			తిరిగి (పూర్ణాంకం)cpu;
		}
	}

	తిరిగి -1;
}

స్థిర శూన్య sched_ipi_idle_reserve(uint32_t cpu, నిర్మాణం k_thread *thread)
{
	uint32_t బిట్టు_2 = బిట్టు(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & బిట్టు_2) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_లక్ష్యం[cpu] == ఖాళీ);

	_kernel.sched_ipi_లక్ష్యం[cpu] = thread;
	_kernel.sched_ipi_reserved |= బిట్టు_2;
}

స్థిర శూన్య sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t బిట్టు_2 = బిట్టు(cpu);

	_kernel.sched_ipi_లక్ష్యం[cpu] = ఖాళీ;
	_kernel.sched_ipi_reserved &= ~బిట్టు_2;
}

శూన్య ipi_idle_thread_unreserve(నిర్మాణం k_thread *thread)
{
	పూర్ణాంకం cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	ఒకవేళ (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(నిర్మాణం k_thread *old_thread,
			    నిర్మాణం k_thread *new_thread)
{
	పూర్ణాంకం old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	ఒకవేళ (old_cpu < 0) {
		తిరిగి false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_చురుకైన(cpu);

#ఒకవేళ defined(CONFIG_SCHED_CPU_ముసుగు)
	eligible = eligible && ((new_thread->పునాది.cpu_ముసుగు & బిట్టు(cpu)) != 0U);
#అయితే_ముగింపు

	ఒకవేళ (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		తిరిగి false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_లక్ష్యం[cpu] = new_thread;
	తిరిగి true;
}

నిర్మాణం k_thread *ipi_idle_reserved_take(శూన్య)
{
	uint32_t cpu = _current_cpu->id;
	నిర్మాణం k_thread *thread = _kernel.sched_ipi_లక్ష్యం[cpu];

	__ASSERT_NO_MSG((thread != ఖాళీ) ==
			((_kernel.sched_ipi_reserved & బిట్టు(cpu)) != 0U));

	ఒకవేళ (thread != ఖాళీ) {
		sched_ipi_idle_unreserve(cpu);
	}

	తిరిగి thread;
}
#అయితే_ముగింపు

శూన్య flag_ipi(uint32_t ipi_ముసుగు)
{
#ఒకవేళ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ఒకవేళ (arch_అంకె_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_ముసుగు);
	}
#అయితే_ముగింపు /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_ముసుగు_create(నిర్మాణం k_thread *thread)
{
	ఒకవేళ (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		తిరిగి (CONFIG_MP_MAX_అంకె_CPUS > 1) ? IPI_ALL_CPUS_ముసుగు : 0;
	}

	uint32_t  ipi_ముసుగు = 0;
	uint32_t  అంకె_cpus = (uint32_t)arch_అంకె_cpus();
	uint32_t  id = _current_cpu->id;
	నిర్మాణం k_thread *cpu_thread;

#ఒకవేళ defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	ఒకవేళ (sched_ipi_thread_reservation(thread) >= 0) {
		తిరిగి 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t చివరి = thread->పునాది.cpu;
	నిర్మాణం k_thread *చివరి_thread = _kernel.cpus[చివరి].current;

	ఒకవేళ (చివరి != id &&
	    చివరి_thread != ఖాళీ &&
	    z_is_idle_thread_object(చివరి_thread) &&
	    sched_ipi_idle_cpu_చురుకైన(చివరి) &&
#ఒకవేళ defined(CONFIG_SCHED_CPU_ముసుగు)
	    (thread->పునాది.cpu_ముసుగు & బిట్టు(చివరి)) != 0 &&
#అయితే_ముగింపు
	    (_kernel.sched_ipi_reserved & బిట్టు(చివరి)) == 0U) {
		sched_ipi_idle_reserve(చివరి, thread);
		తిరిగి IPI_CPU_ముసుగు(చివరి);
	}

#అయితే_ముగింపు

	కొరకు (uint32_t i = 0; i < అంకె_cpus; i++) {
		ఒకవేళ (id == i) {
			కొనసాగు;
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

#ఒకవేళ defined(CONFIG_SCHED_CPU_ముసుగు)
		ఒకవేళ ((thread->పునాది.cpu_ముసుగు & బిట్టు(i)) == 0) {
			కొనసాగు;
		}
#అయితే_ముగింపు

		cpu_thread = _kernel.cpus[i].current;

		ఒకవేళ (cpu_thread == ఖాళీ) {
			కొనసాగు;
		}

#ఒకవేళ defined(CONFIG_IPI_OPTIMIZE_IDLE)
		ఒకవేళ ((_kernel.sched_ipi_reserved & బిట్టు(i)) != 0U) {
			కొనసాగు;
		}
		ఒకవేళ (i != (uint32_t)thread->పునాది.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_చురుకైన(i)) {
			sched_ipi_idle_reserve(i, thread);
			తిరిగి IPI_CPU_ముసుగు(i);
		}
#అయితే_ముగింపు

		ఒకవేళ ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_ముసుగు |= బిట్టు(i);
		}
	}

	తిరిగి (atomic_val_t)ipi_ముసుగు;
}

శూన్య signal_pending_ipi(శూన్య)
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

#ఒకవేళ defined(CONFIG_SCHED_IPI_SUPPORTED)
	ఒకవేళ (arch_అంకె_cpus() > 1) {
		uint32_t self_బిట్టు = బిట్టు(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_బిట్టు);
		cpu_bitmap &= ~self_బిట్టు;
		ఒకవేళ (cpu_bitmap != 0) {
#నిర్వచితమైతే CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#లేకపోతే
			arch_sched_ప్రసారం_ipi();
#అయితే_ముగింపు
		}
	}
#అయితే_ముగింపు /* CONFIG_SCHED_IPI_SUPPORTED */
}

#నిర్వచితమైతే CONFIG_SCHED_IPI_SUPPORTED
స్థిర నిర్మాణం k_ipi_work *మొదటి_ipi_work(sys_dlist_t *list, గుర్తులేని పూర్ణాంకం cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_తల(list);

	తిరిగి (work == ఖాళీ) ? ఖాళీ
			      : CONTAINER_యొక్క(work, నిర్మాణం k_ipi_work, కణుపు[cpu_id]);
}

పూర్ణాంకం k_ipi_work_కూడు(నిర్మాణం k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != ఖాళీ, "");
	__ASSERT(func != ఖాళీ, "");

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&ipi_తాళం);

	/* Verify the IPI work item is not currently in use */

	ఒకవేళ (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_తాళం, కీలకం);
		తిరిగి -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	గుర్తులేని పూర్ణాంకం cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_ముసుగు & ~బిట్టు(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_ముసుగు);
	work->func = func;
	work->bitmask = cpu_bitmask;

	కొరకు (గుర్తులేని పూర్ణాంకం id = 0; id < arch_అంకె_cpus(); id++) {
		ఒకవేళ ((cpu_bitmask & బిట్టు(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->కణుపు[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_తాళం, కీలకం);

	తిరిగి 0;
}

పూర్ణాంకం k_ipi_work_wait(నిర్మాణం k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	తిరిగి (rv == 0) ? -EAGAIN : 0;
}

శూన్య k_ipi_work_signal(శూన్య)
{
	signal_pending_ipi();
}

స్థిర శూన్య ipi_work_process(sys_dlist_t *list, గుర్తులేని పూర్ణాంకం cpu_id)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&ipi_తాళం);

	కొరకు (నిర్మాణం k_ipi_work *work = మొదటి_ipi_work(list, cpu_id);
	     work != ఖాళీ; work = మొదటి_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->కణుపు[cpu_id]);
		k_spin_unlock(&ipi_తాళం, కీలకం);

		work->func(work);

		కీలకం = k_spin_తాళం(&ipi_తాళం);
		k_event_post(&work->event, బిట్టు(cpu_id));
	}

	k_spin_unlock(&ipi_తాళం, కీలకం);
}
#అయితే_ముగింపు /* CONFIG_SCHED_IPI_SUPPORTED */

శూన్య z_sched_ipi(శూన్య)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#నిర్వచితమైతే CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#అయితే_ముగింపు /* CONFIG_TRACE_SCHED_IPI */

#నిర్వచితమైతే CONFIG_TIMESLICING
	z_సమయం_ముక్క();
#అయితే_ముగింపు /* CONFIG_TIMESLICING */

#నిర్వచితమైతే CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SCHED_IPI_SUPPORTED
	గుర్తులేని పూర్ణాంకం cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#అయితే_ముగింపు
}
