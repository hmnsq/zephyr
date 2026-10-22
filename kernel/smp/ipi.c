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
#د_که_پای

#که_تعریف_شوی CONFIG_SCHED_IPI_SUPPORTED
ساكن بنية k_spinlock ipi_قلف;
#د_که_پای

#که_تعریف_شوی CONFIG_TRACE_SCHED_IPI
خارجي فراغ z_trace_sched_ipi(فراغ);
#د_که_پای

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
ساكن bool sched_ipi_idle_cpu_فعال(uint32_t cpu)
{
#إذا defined(CONFIG_PM)
	أرجع pm_state_بل_get((uint8_t)cpu)->state == PM_STATE_فعال;
#وإلا
	ARG_ناکارول_شوی(cpu);
	أرجع true;
#د_که_پای
}

ساكن مضمن صحيح sched_ipi_thread_reservation(ثابت بنية k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	طالما (reservations != 0U) {
		بینښه صحيح cpu = (بینښه صحيح)u32_شمېر_trailing_zeros(reservations);

		reservations &= ~بټ(cpu);
		إذا (_kernel.sched_ipi_موخه[cpu] == thread) {
			أرجع (صحيح)cpu;
		}
	}

	أرجع -1;
}

ساكن فراغ sched_ipi_idle_reserve(uint32_t cpu, بنية k_thread *thread)
{
	uint32_t بټ_2 = بټ(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & بټ_2) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_موخه[cpu] == عدم);

	_kernel.sched_ipi_موخه[cpu] = thread;
	_kernel.sched_ipi_reserved |= بټ_2;
}

ساكن فراغ sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t بټ_2 = بټ(cpu);

	_kernel.sched_ipi_موخه[cpu] = عدم;
	_kernel.sched_ipi_reserved &= ~بټ_2;
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
	bool eligible = sched_ipi_idle_cpu_فعال(cpu);

#إذا defined(CONFIG_SCHED_CPU_ماسک)
	eligible = eligible && ((new_thread->بنسټ.cpu_ماسک & بټ(cpu)) != 0U);
#د_که_پای

	إذا (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		أرجع false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_موخه[cpu] = new_thread;
	أرجع true;
}

بنية k_thread *ipi_idle_reserved_take(فراغ)
{
	uint32_t cpu = _current_cpu->id;
	بنية k_thread *thread = _kernel.sched_ipi_موخه[cpu];

	__ASSERT_NO_MSG((thread != عدم) ==
			((_kernel.sched_ipi_reserved & بټ(cpu)) != 0U));

	إذا (thread != عدم) {
		sched_ipi_idle_unreserve(cpu);
	}

	أرجع thread;
}
#د_که_پای

فراغ flag_ipi(uint32_t ipi_ماسک)
{
#إذا defined(CONFIG_SCHED_IPI_SUPPORTED)
	إذا (arch_عدد_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_ماسک);
	}
#د_که_پای /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_ماسک_create(بنية k_thread *thread)
{
	إذا (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		أرجع (CONFIG_MP_MAX_عدد_CPUS > 1) ? IPI_ALL_CPUS_ماسک : 0;
	}

	uint32_t  ipi_ماسک = 0;
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
	uint8_t وروستی = thread->بنسټ.cpu;
	بنية k_thread *وروستی_thread = _kernel.cpus[وروستی].current;

	إذا (وروستی != id &&
	    وروستی_thread != عدم &&
	    z_is_idle_thread_object(وروستی_thread) &&
	    sched_ipi_idle_cpu_فعال(وروستی) &&
#إذا defined(CONFIG_SCHED_CPU_ماسک)
	    (thread->بنسټ.cpu_ماسک & بټ(وروستی)) != 0 &&
#د_که_پای
	    (_kernel.sched_ipi_reserved & بټ(وروستی)) == 0U) {
		sched_ipi_idle_reserve(وروستی, thread);
		أرجع IPI_CPU_ماسک(وروستی);
	}

#د_که_پای

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

#إذا defined(CONFIG_SCHED_CPU_ماسک)
		إذا ((thread->بنسټ.cpu_ماسک & بټ(i)) == 0) {
			استمر;
		}
#د_که_پای

		cpu_thread = _kernel.cpus[i].current;

		إذا (cpu_thread == عدم) {
			استمر;
		}

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
		إذا ((_kernel.sched_ipi_reserved & بټ(i)) != 0U) {
			استمر;
		}
		إذا (i != (uint32_t)thread->بنسټ.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_فعال(i)) {
			sched_ipi_idle_reserve(i, thread);
			أرجع IPI_CPU_ماسک(i);
		}
#د_که_پای

		إذا ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_ماسک |= بټ(i);
		}
	}

	أرجع (atomic_val_t)ipi_ماسک;
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
		uint32_t self_بټ = بټ(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_بټ);
		cpu_bitmap &= ~self_بټ;
		إذا (cpu_bitmap != 0) {
#که_تعریف_شوی CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#وإلا
			arch_sched_خپرونه_ipi();
#د_که_پای
		}
	}
#د_که_پای /* CONFIG_SCHED_IPI_SUPPORTED */
}

#که_تعریف_شوی CONFIG_SCHED_IPI_SUPPORTED
ساكن بنية k_ipi_work *لومړی_ipi_work(sys_dlist_t *list, بینښه صحيح cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_سر(list);

	أرجع (work == عدم) ? عدم
			      : CONTAINER_د(work, بنية k_ipi_work, غوټه[cpu_id]);
}

صحيح k_ipi_work_جمع_کړه(بنية k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != عدم, "");
	__ASSERT(func != عدم, "");

	k_spinlock_کلی_t کلی = k_spin_قلف(&ipi_قلف);

	/* Verify the IPI work item is not currently in use */

	إذا (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_قلف, کلی);
		أرجع -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	بینښه صحيح cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_ماسک & ~بټ(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_ماسک);
	work->func = func;
	work->bitmask = cpu_bitmask;

	لكل (بینښه صحيح id = 0; id < arch_عدد_cpus(); id++) {
		إذا ((cpu_bitmask & بټ(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->غوټه[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_قلف, کلی);

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

ساكن فراغ ipi_work_process(sys_dlist_t *list, بینښه صحيح cpu_id)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&ipi_قلف);

	لكل (بنية k_ipi_work *work = لومړی_ipi_work(list, cpu_id);
	     work != عدم; work = لومړی_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->غوټه[cpu_id]);
		k_spin_unlock(&ipi_قلف, کلی);

		work->func(work);

		کلی = k_spin_قلف(&ipi_قلف);
		k_event_post(&work->event, بټ(cpu_id));
	}

	k_spin_unlock(&ipi_قلف, کلی);
}
#د_که_پای /* CONFIG_SCHED_IPI_SUPPORTED */

فراغ z_sched_ipi(فراغ)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#که_تعریف_شوی CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#د_که_پای /* CONFIG_TRACE_SCHED_IPI */

#که_تعریف_شوی CONFIG_TIMESLICING
	z_وخت_ټوټه();
#د_که_پای /* CONFIG_TIMESLICING */

#که_تعریف_شوی CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#د_که_پای

#که_تعریف_شوی CONFIG_SCHED_IPI_SUPPORTED
	بینښه صحيح cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#د_که_پای
}
