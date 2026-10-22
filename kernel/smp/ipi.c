/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <kswap.h>
#شامل <ksched.h>
#شامل <ipi.h>

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#شامل <zephyr/pm/pm.h>
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SCHED_IPI_SUPPORTED
ساکن ساخت k_spinlock ipi_تالا;
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_TRACE_SCHED_IPI
خارجی خالی z_trace_sched_ipi(خالی);
#اگر_ختم

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE)
ساکن bool sched_ipi_idle_cpu_فعال(uint32_t cpu)
{
#اگر defined(CONFIG_PM)
	واپس pm_state_اگلا_get((uint8_t)cpu)->state == PM_STATE_فعال;
#ورنہ
	ARG_غیر_مستعمل(cpu);
	واپس true;
#اگر_ختم
}

ساکن اندرونی صحیح sched_ipi_thread_reservation(ثابت ساخت k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	جب (reservations != 0U) {
		غیرعلامتی صحیح cpu = (غیرعلامتی صحیح)u32_گنتی_trailing_zeros(reservations);

		reservations &= ~ثنائی_ہندسہ(cpu);
		اگر (_kernel.sched_ipi_ہدف[cpu] == thread) {
			واپس (صحیح)cpu;
		}
	}

	واپس -1;
}

ساکن خالی sched_ipi_idle_reserve(uint32_t cpu, ساخت k_thread *thread)
{
	uint32_t ثنائی_ہندسہ_2 = ثنائی_ہندسہ(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & ثنائی_ہندسہ_2) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_ہدف[cpu] == تہی);

	_kernel.sched_ipi_ہدف[cpu] = thread;
	_kernel.sched_ipi_reserved |= ثنائی_ہندسہ_2;
}

ساکن خالی sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t ثنائی_ہندسہ_2 = ثنائی_ہندسہ(cpu);

	_kernel.sched_ipi_ہدف[cpu] = تہی;
	_kernel.sched_ipi_reserved &= ~ثنائی_ہندسہ_2;
}

خالی ipi_idle_thread_unreserve(ساخت k_thread *thread)
{
	صحیح cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	اگر (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(ساخت k_thread *old_thread,
			    ساخت k_thread *new_thread)
{
	صحیح old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	اگر (old_cpu < 0) {
		واپس false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_فعال(cpu);

#اگر defined(CONFIG_SCHED_CPU_نقاب)
	eligible = eligible && ((new_thread->بنیاد.cpu_نقاب & ثنائی_ہندسہ(cpu)) != 0U);
#اگر_ختم

	اگر (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		واپس false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_ہدف[cpu] = new_thread;
	واپس true;
}

ساخت k_thread *ipi_idle_reserved_take(خالی)
{
	uint32_t cpu = _current_cpu->id;
	ساخت k_thread *thread = _kernel.sched_ipi_ہدف[cpu];

	__ASSERT_NO_MSG((thread != تہی) ==
			((_kernel.sched_ipi_reserved & ثنائی_ہندسہ(cpu)) != 0U));

	اگر (thread != تہی) {
		sched_ipi_idle_unreserve(cpu);
	}

	واپس thread;
}
#اگر_ختم

خالی flag_ipi(uint32_t ipi_نقاب)
{
#اگر defined(CONFIG_SCHED_IPI_SUPPORTED)
	اگر (arch_عدد_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_نقاب);
	}
#اگر_ختم /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_نقاب_create(ساخت k_thread *thread)
{
	اگر (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		واپس (CONFIG_MP_MAX_عدد_CPUS > 1) ? IPI_ALL_CPUS_نقاب : 0;
	}

	uint32_t  ipi_نقاب = 0;
	uint32_t  عدد_cpus = (uint32_t)arch_عدد_cpus();
	uint32_t  id = _current_cpu->id;
	ساخت k_thread *cpu_thread;

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	اگر (sched_ipi_thread_reservation(thread) >= 0) {
		واپس 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t آخری = thread->بنیاد.cpu;
	ساخت k_thread *آخری_thread = _kernel.cpus[آخری].current;

	اگر (آخری != id &&
	    آخری_thread != تہی &&
	    z_is_idle_thread_object(آخری_thread) &&
	    sched_ipi_idle_cpu_فعال(آخری) &&
#اگر defined(CONFIG_SCHED_CPU_نقاب)
	    (thread->بنیاد.cpu_نقاب & ثنائی_ہندسہ(آخری)) != 0 &&
#اگر_ختم
	    (_kernel.sched_ipi_reserved & ثنائی_ہندسہ(آخری)) == 0U) {
		sched_ipi_idle_reserve(آخری, thread);
		واپس IPI_CPU_نقاب(آخری);
	}

#اگر_ختم

	برائے (uint32_t i = 0; i < عدد_cpus; i++) {
		اگر (id == i) {
			جاری;
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

#اگر defined(CONFIG_SCHED_CPU_نقاب)
		اگر ((thread->بنیاد.cpu_نقاب & ثنائی_ہندسہ(i)) == 0) {
			جاری;
		}
#اگر_ختم

		cpu_thread = _kernel.cpus[i].current;

		اگر (cpu_thread == تہی) {
			جاری;
		}

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE)
		اگر ((_kernel.sched_ipi_reserved & ثنائی_ہندسہ(i)) != 0U) {
			جاری;
		}
		اگر (i != (uint32_t)thread->بنیاد.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_فعال(i)) {
			sched_ipi_idle_reserve(i, thread);
			واپس IPI_CPU_نقاب(i);
		}
#اگر_ختم

		اگر ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_نقاب |= ثنائی_ہندسہ(i);
		}
	}

	واپس (atomic_val_t)ipi_نقاب;
}

خالی signal_pending_ipi(خالی)
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

#اگر defined(CONFIG_SCHED_IPI_SUPPORTED)
	اگر (arch_عدد_cpus() > 1) {
		uint32_t self_ثنائی_ہندسہ = ثنائی_ہندسہ(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_ثنائی_ہندسہ);
		cpu_bitmap &= ~self_ثنائی_ہندسہ;
		اگر (cpu_bitmap != 0) {
#اگر_تعریف_شدہ CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#ورنہ
			arch_sched_نشر_ipi();
#اگر_ختم
		}
	}
#اگر_ختم /* CONFIG_SCHED_IPI_SUPPORTED */
}

#اگر_تعریف_شدہ CONFIG_SCHED_IPI_SUPPORTED
ساکن ساخت k_ipi_work *پہلا_ipi_work(sys_dlist_t *list, غیرعلامتی صحیح cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_سر(list);

	واپس (work == تہی) ? تہی
			      : CONTAINER_کا(work, ساخت k_ipi_work, گرہ[cpu_id]);
}

صحیح k_ipi_work_جوڑو(ساخت k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != تہی, "");
	__ASSERT(func != تہی, "");

	k_spinlock_کلید_t کلید = k_spin_تالا(&ipi_تالا);

	/* Verify the IPI work item is not currently in use */

	اگر (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_تالا, کلید);
		واپس -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	غیرعلامتی صحیح cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_نقاب & ~ثنائی_ہندسہ(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_نقاب);
	work->func = func;
	work->bitmask = cpu_bitmask;

	برائے (غیرعلامتی صحیح id = 0; id < arch_عدد_cpus(); id++) {
		اگر ((cpu_bitmask & ثنائی_ہندسہ(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->گرہ[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_تالا, کلید);

	واپس 0;
}

صحیح k_ipi_work_wait(ساخت k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	واپس (rv == 0) ? -EAGAIN : 0;
}

خالی k_ipi_work_signal(خالی)
{
	signal_pending_ipi();
}

ساکن خالی ipi_work_process(sys_dlist_t *list, غیرعلامتی صحیح cpu_id)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&ipi_تالا);

	برائے (ساخت k_ipi_work *work = پہلا_ipi_work(list, cpu_id);
	     work != تہی; work = پہلا_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->گرہ[cpu_id]);
		k_spin_unlock(&ipi_تالا, کلید);

		work->func(work);

		کلید = k_spin_تالا(&ipi_تالا);
		k_event_post(&work->event, ثنائی_ہندسہ(cpu_id));
	}

	k_spin_unlock(&ipi_تالا, کلید);
}
#اگر_ختم /* CONFIG_SCHED_IPI_SUPPORTED */

خالی z_sched_ipi(خالی)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#اگر_تعریف_شدہ CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#اگر_ختم /* CONFIG_TRACE_SCHED_IPI */

#اگر_تعریف_شدہ CONFIG_TIMESLICING
	z_وقت_قاش();
#اگر_ختم /* CONFIG_TIMESLICING */

#اگر_تعریف_شدہ CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SCHED_IPI_SUPPORTED
	غیرعلامتی صحیح cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#اگر_ختم
}
