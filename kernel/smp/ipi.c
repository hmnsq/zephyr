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
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_IPI_SUPPORTED
ساكن بنية k_spinlock ipi_تالو;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_TRACE_SCHED_IPI
خارجي فراغ z_trace_sched_ipi(فراغ);
#جيڪڏهن_پورو

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
ساكن bool sched_ipi_idle_cpu_سرگرم(uint32_t cpu)
{
#إذا defined(CONFIG_PM)
	أرجع pm_state_ايندڙ_get((uint8_t)cpu)->state == PM_STATE_سرگرم;
#وإلا
	ARG_اڻ_استعمال_ٿيل(cpu);
	أرجع true;
#جيڪڏهن_پورو
}

ساكن مضمن صحيح sched_ipi_thread_reservation(ثابت بنية k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	طالما (reservations != 0U) {
		بنانشان صحيح cpu = (بنانشان صحيح)u32_ڳڻپ_trailing_zeros(reservations);

		reservations &= ~ٻٽو_انگ(cpu);
		إذا (_kernel.sched_ipi_هدف[cpu] == thread) {
			أرجع (صحيح)cpu;
		}
	}

	أرجع -1;
}

ساكن فراغ sched_ipi_idle_reserve(uint32_t cpu, بنية k_thread *thread)
{
	uint32_t ٻٽو_انگ_2 = ٻٽو_انگ(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & ٻٽو_انگ_2) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_هدف[cpu] == عدم);

	_kernel.sched_ipi_هدف[cpu] = thread;
	_kernel.sched_ipi_reserved |= ٻٽو_انگ_2;
}

ساكن فراغ sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t ٻٽو_انگ_2 = ٻٽو_انگ(cpu);

	_kernel.sched_ipi_هدف[cpu] = عدم;
	_kernel.sched_ipi_reserved &= ~ٻٽو_انگ_2;
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
	bool eligible = sched_ipi_idle_cpu_سرگرم(cpu);

#إذا defined(CONFIG_SCHED_CPU_نقاب)
	eligible = eligible && ((new_thread->بنياد.cpu_نقاب & ٻٽو_انگ(cpu)) != 0U);
#جيڪڏهن_پورو

	إذا (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		أرجع false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_هدف[cpu] = new_thread;
	أرجع true;
}

بنية k_thread *ipi_idle_reserved_take(فراغ)
{
	uint32_t cpu = _current_cpu->id;
	بنية k_thread *thread = _kernel.sched_ipi_هدف[cpu];

	__ASSERT_NO_MSG((thread != عدم) ==
			((_kernel.sched_ipi_reserved & ٻٽو_انگ(cpu)) != 0U));

	إذا (thread != عدم) {
		sched_ipi_idle_unreserve(cpu);
	}

	أرجع thread;
}
#جيڪڏهن_پورو

فراغ flag_ipi(uint32_t ipi_نقاب)
{
#إذا defined(CONFIG_SCHED_IPI_SUPPORTED)
	إذا (arch_انگ_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_نقاب);
	}
#جيڪڏهن_پورو /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_نقاب_create(بنية k_thread *thread)
{
	إذا (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		أرجع (CONFIG_MP_MAX_انگ_CPUS > 1) ? IPI_ALL_CPUS_نقاب : 0;
	}

	uint32_t  ipi_نقاب = 0;
	uint32_t  انگ_cpus = (uint32_t)arch_انگ_cpus();
	uint32_t  id = _current_cpu->id;
	بنية k_thread *cpu_thread;

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	إذا (sched_ipi_thread_reservation(thread) >= 0) {
		أرجع 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t آخري = thread->بنياد.cpu;
	بنية k_thread *آخري_thread = _kernel.cpus[آخري].current;

	إذا (آخري != id &&
	    آخري_thread != عدم &&
	    z_is_idle_thread_object(آخري_thread) &&
	    sched_ipi_idle_cpu_سرگرم(آخري) &&
#إذا defined(CONFIG_SCHED_CPU_نقاب)
	    (thread->بنياد.cpu_نقاب & ٻٽو_انگ(آخري)) != 0 &&
#جيڪڏهن_پورو
	    (_kernel.sched_ipi_reserved & ٻٽو_انگ(آخري)) == 0U) {
		sched_ipi_idle_reserve(آخري, thread);
		أرجع IPI_CPU_نقاب(آخري);
	}

#جيڪڏهن_پورو

	لكل (uint32_t i = 0; i < انگ_cpus; i++) {
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

#إذا defined(CONFIG_SCHED_CPU_نقاب)
		إذا ((thread->بنياد.cpu_نقاب & ٻٽو_انگ(i)) == 0) {
			استمر;
		}
#جيڪڏهن_پورو

		cpu_thread = _kernel.cpus[i].current;

		إذا (cpu_thread == عدم) {
			استمر;
		}

#إذا defined(CONFIG_IPI_OPTIMIZE_IDLE)
		إذا ((_kernel.sched_ipi_reserved & ٻٽو_انگ(i)) != 0U) {
			استمر;
		}
		إذا (i != (uint32_t)thread->بنياد.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_سرگرم(i)) {
			sched_ipi_idle_reserve(i, thread);
			أرجع IPI_CPU_نقاب(i);
		}
#جيڪڏهن_پورو

		إذا ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_نقاب |= ٻٽو_انگ(i);
		}
	}

	أرجع (atomic_val_t)ipi_نقاب;
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
	إذا (arch_انگ_cpus() > 1) {
		uint32_t self_ٻٽو_انگ = ٻٽو_انگ(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_ٻٽو_انگ);
		cpu_bitmap &= ~self_ٻٽو_انگ;
		إذا (cpu_bitmap != 0) {
#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#وإلا
			arch_sched_نشر_ipi();
#جيڪڏهن_پورو
		}
	}
#جيڪڏهن_پورو /* CONFIG_SCHED_IPI_SUPPORTED */
}

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_IPI_SUPPORTED
ساكن بنية k_ipi_work *پهريون_ipi_work(sys_dlist_t *list, بنانشان صحيح cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_سر(list);

	أرجع (work == عدم) ? عدم
			      : CONTAINER_جو(work, بنية k_ipi_work, ڳنڍ[cpu_id]);
}

صحيح k_ipi_work_جوڙيو(بنية k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != عدم, "");
	__ASSERT(func != عدم, "");

	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ipi_تالو);

	/* Verify the IPI work item is not currently in use */

	إذا (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_تالو, ڪنجي);
		أرجع -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	بنانشان صحيح cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_نقاب & ~ٻٽو_انگ(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_نقاب);
	work->func = func;
	work->bitmask = cpu_bitmask;

	لكل (بنانشان صحيح id = 0; id < arch_انگ_cpus(); id++) {
		إذا ((cpu_bitmask & ٻٽو_انگ(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->ڳنڍ[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_تالو, ڪنجي);

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

ساكن فراغ ipi_work_process(sys_dlist_t *list, بنانشان صحيح cpu_id)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&ipi_تالو);

	لكل (بنية k_ipi_work *work = پهريون_ipi_work(list, cpu_id);
	     work != عدم; work = پهريون_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->ڳنڍ[cpu_id]);
		k_spin_unlock(&ipi_تالو, ڪنجي);

		work->func(work);

		ڪنجي = k_spin_تالو(&ipi_تالو);
		k_event_post(&work->event, ٻٽو_انگ(cpu_id));
	}

	k_spin_unlock(&ipi_تالو, ڪنجي);
}
#جيڪڏهن_پورو /* CONFIG_SCHED_IPI_SUPPORTED */

فراغ z_sched_ipi(فراغ)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#جيڪڏهن_وصف_ٿيل CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#جيڪڏهن_پورو /* CONFIG_TRACE_SCHED_IPI */

#جيڪڏهن_وصف_ٿيل CONFIG_TIMESLICING
	z_وقت_ٽڪرو();
#جيڪڏهن_پورو /* CONFIG_TIMESLICING */

#جيڪڏهن_وصف_ٿيل CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_IPI_SUPPORTED
	بنانشان صحيح cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#جيڪڏهن_پورو
}
